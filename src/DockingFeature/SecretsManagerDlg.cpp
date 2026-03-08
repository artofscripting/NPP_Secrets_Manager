//this file is part of Secrets Manager plugin for Notepad++
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "SecretsManagerDlg.h"
#include "../PluginDefinition.h"
#include <windows.h>
#include <wincrypt.h>
#include <shlobj.h>
#include <fstream>
#include <algorithm>
#include <bcrypt.h>

#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "bcrypt.lib")

extern NppData nppData;

std::wstring SecretsManagerDlg::_dialogPassword;
bool SecretsManagerDlg::_dialogConfirmMode = false;

std::wstring SecretsManagerDlg::getSecretsFilePath()
{
    TCHAR configDir[MAX_PATH] = {0};
    ::SendMessage(nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, MAX_PATH, (LPARAM)configDir);

    std::wstring path = configDir;
    path += TEXT("\\secrets.dat");
    return path;
}

std::wstring SecretsManagerDlg::getPasswordHashFilePath()
{
    TCHAR configDir[MAX_PATH] = {0};
    ::SendMessage(nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, MAX_PATH, (LPARAM)configDir);

    std::wstring path = configDir;
    path += TEXT("\\secrets_hash.dat");
    return path;
}

void SecretsManagerDlg::hashPassword(const std::wstring& password, std::vector<BYTE>& hash)
{
    std::string utf8Password;
    int size = WideCharToMultiByte(CP_UTF8, 0, password.c_str(), -1, NULL, 0, NULL, NULL);
    if (size > 0)
    {
        utf8Password.resize(size);
        WideCharToMultiByte(CP_UTF8, 0, password.c_str(), -1, &utf8Password[0], size, NULL, NULL);
    }

    BCRYPT_ALG_HANDLE hAlg = NULL;
    BCRYPT_HASH_HANDLE hHash = NULL;
    DWORD hashLength = 0;
    DWORD resultLength = 0;

    if (BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, NULL, 0) == 0)
    {
        if (BCryptGetProperty(hAlg, BCRYPT_HASH_LENGTH, (PBYTE)&hashLength, sizeof(DWORD), &resultLength, 0) == 0)
        {
            hash.resize(hashLength);

            if (BCryptCreateHash(hAlg, &hHash, NULL, 0, NULL, 0, 0) == 0)
            {
                BCryptHashData(hHash, (PBYTE)utf8Password.c_str(), (ULONG)utf8Password.length(), 0);
                BCryptFinishHash(hHash, hash.data(), hashLength, 0);
                BCryptDestroyHash(hHash);
            }
        }
        BCryptCloseAlgorithmProvider(hAlg, 0);
    }
}

bool SecretsManagerDlg::verifyPassword(const std::wstring& password)
{
    std::vector<BYTE> hash;
    hashPassword(password, hash);

    std::wstring hashFile = getPasswordHashFilePath();
    std::ifstream file(hashFile, std::ios::binary);
    if (!file.is_open())
        return false;

    std::vector<BYTE> storedHash;
    DWORD hashSize = 0;
    file.read((char*)&hashSize, sizeof(hashSize));
    storedHash.resize(hashSize);
    file.read((char*)storedHash.data(), hashSize);
    file.close();

    return hash == storedHash;
}

bool SecretsManagerDlg::deriveKeyFromPassword(const std::wstring& password, const std::vector<BYTE>& salt, std::vector<BYTE>& key)
{
    std::string utf8Password;
    int size = WideCharToMultiByte(CP_UTF8, 0, password.c_str(), -1, NULL, 0, NULL, NULL);
    if (size > 0)
    {
        utf8Password.resize(size);
        WideCharToMultiByte(CP_UTF8, 0, password.c_str(), -1, &utf8Password[0], size, NULL, NULL);
    }

    BCRYPT_ALG_HANDLE hAlg = NULL;
    if (BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, NULL, BCRYPT_ALG_HANDLE_HMAC_FLAG) != 0)
        return false;

    key.resize(32);
    NTSTATUS status = BCryptDeriveKeyPBKDF2(
        hAlg,
        (PUCHAR)utf8Password.c_str(),
        (ULONG)utf8Password.length(),
        (PUCHAR)salt.data(),
        (ULONG)salt.size(),
        10000,
        key.data(),
        32,
        0
    );

    BCryptCloseAlgorithmProvider(hAlg, 0);
    return status == 0;
}

bool SecretsManagerDlg::encryptDataWithPassword(const std::wstring& plainText, const std::wstring& password, std::vector<BYTE>& encrypted)
{
    std::vector<BYTE> salt(16);
    BCryptGenRandom(NULL, salt.data(), 16, BCRYPT_USE_SYSTEM_PREFERRED_RNG);

    std::vector<BYTE> key;
    if (!deriveKeyFromPassword(password, salt, key))
        return false;

    DATA_BLOB dataIn, dataOut, entropy;
    dataIn.pbData = (BYTE*)plainText.c_str();
    dataIn.cbData = (DWORD)((plainText.length() + 1) * sizeof(wchar_t));

    entropy.pbData = key.data();
    entropy.cbData = (DWORD)key.size();

    if (CryptProtectData(&dataIn, L"Secrets Manager", &entropy, NULL, NULL, 0, &dataOut))
    {
        encrypted.clear();
        encrypted.insert(encrypted.end(), salt.begin(), salt.end());
        DWORD dataSize = dataOut.cbData;
        encrypted.insert(encrypted.end(), (BYTE*)&dataSize, (BYTE*)&dataSize + sizeof(dataSize));
        encrypted.insert(encrypted.end(), dataOut.pbData, dataOut.pbData + dataOut.cbData);
        LocalFree(dataOut.pbData);
        return true;
    }
    return false;
}

bool SecretsManagerDlg::decryptDataWithPassword(const std::vector<BYTE>& encrypted, const std::wstring& password, std::wstring& plainText)
{
    if (encrypted.size() < 16 + sizeof(DWORD))
        return false;

    std::vector<BYTE> salt(encrypted.begin(), encrypted.begin() + 16);

    std::vector<BYTE> key;
    if (!deriveKeyFromPassword(password, salt, key))
        return false;

    DWORD dataSize;
    memcpy(&dataSize, &encrypted[16], sizeof(DWORD));

    if (encrypted.size() < 16 + sizeof(DWORD) + dataSize)
        return false;

    DATA_BLOB dataIn, dataOut, entropy;
    dataIn.pbData = (BYTE*)&encrypted[16 + sizeof(DWORD)];
    dataIn.cbData = dataSize;

    entropy.pbData = key.data();
    entropy.cbData = (DWORD)key.size();

    if (CryptUnprotectData(&dataIn, NULL, &entropy, NULL, NULL, 0, &dataOut))
    {
        plainText = (wchar_t*)dataOut.pbData;
        LocalFree(dataOut.pbData);
        return true;
    }
    return false;
}

bool SecretsManagerDlg::promptForPassword(const wchar_t* title, const wchar_t* prompt, std::wstring& password, bool requireConfirm)
{
    _dialogPassword.clear();
    _dialogConfirmMode = requireConfirm;

    HINSTANCE hInst = (HINSTANCE)::GetWindowLongPtr(_hSelf, GWLP_HINSTANCE);
    if (!hInst)
    {
        hInst = (HINSTANCE)::GetModuleHandle(NULL);
    }

    INT_PTR result = DialogBoxParam(
        hInst,
        MAKEINTRESOURCE(IDD_PASSWORD_DLG),
        _hSelf,
        passwordDlgProc,
        (LPARAM)prompt
    );

    if (result == -1)
    {
        DWORD err = GetLastError();
        TCHAR errMsg[256];
        wsprintf(errMsg, L"Failed to create password dialog. Error: %d", err);
        ::MessageBox(_hSelf, errMsg, L"Error", MB_OK | MB_ICONERROR);
        return false;
    }

    if (result == IDOK)
    {
        password = _dialogPassword;
        return !password.empty();
    }
    return false;
}

INT_PTR CALLBACK SecretsManagerDlg::passwordDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
        {
            const wchar_t* prompt = (const wchar_t*)lParam;
            if (prompt)
            {
                ::SetDlgItemText(hwnd, IDC_PASSWORD_PROMPT, prompt);
            }

            if (!_dialogConfirmMode)
            {
                ::ShowWindow(::GetDlgItem(hwnd, IDC_PASSWORD_CONFIRM_EDIT), SW_HIDE);
                ::ShowWindow(::GetDlgItem(hwnd, IDC_PASSWORD_CONFIRM_LABEL), SW_HIDE);

                // Move the buttons to be visible in the resized dialog
                HWND hOkButton = ::GetDlgItem(hwnd, IDOK);
                HWND hCancelButton = ::GetDlgItem(hwnd, IDCANCEL);

                // Position buttons at Y=85 (in dialog units)
                RECT dialogRect;
                GetClientRect(hwnd, &dialogRect);

                // Convert dialog units to pixels for button positioning
                RECT tempRect = {0, 0, 95, 85};
                MapDialogRect(hwnd, &tempRect);

                int buttonY = tempRect.bottom; // Y=85 in pixels
                int okButtonX = tempRect.right; // X=95 in pixels

                tempRect.right = 180;
                MapDialogRect(hwnd, &tempRect);
                int cancelButtonX = tempRect.right; // X=180 in pixels

                SetWindowPos(hOkButton, NULL, okButtonX, buttonY, 75, 28, SWP_NOZORDER);
                SetWindowPos(hCancelButton, NULL, cancelButtonX, buttonY, 75, 28, SWP_NOZORDER);
            }

            ::SetFocus(::GetDlgItem(hwnd, IDC_PASSWORD_EDIT));
            return TRUE;
        }

        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case IDOK:
                {
                    TCHAR password[256] = {0};
                    ::GetDlgItemText(hwnd, IDC_PASSWORD_EDIT, password, 256);

                    if (lstrlen(password) == 0)
                    {
                        ::MessageBox(hwnd, L"Password cannot be empty!", L"Error", MB_OK | MB_ICONWARNING);
                        return TRUE;
                    }

                    if (_dialogConfirmMode)
                    {
                        TCHAR confirm[256] = {0};
                        ::GetDlgItemText(hwnd, IDC_PASSWORD_CONFIRM_EDIT, confirm, 256);

                        if (lstrcmp(password, confirm) != 0)
                        {
                            ::MessageBox(hwnd, L"Passwords do not match!", L"Error", MB_OK | MB_ICONWARNING);
                            ::SetDlgItemText(hwnd, IDC_PASSWORD_EDIT, L"");
                            ::SetDlgItemText(hwnd, IDC_PASSWORD_CONFIRM_EDIT, L"");
                            ::SetFocus(::GetDlgItem(hwnd, IDC_PASSWORD_EDIT));
                            return TRUE;
                        }
                    }

                    _dialogPassword = password;
                    EndDialog(hwnd, IDOK);
                    return TRUE;
                }

                case IDCANCEL:
                    _dialogPassword.clear();
                    EndDialog(hwnd, IDCANCEL);
                    return TRUE;
            }
            break;
        }
    }
    return FALSE;
}

void SecretsManagerDlg::unlockSecrets()
{
    std::wstring password;

    std::wstring hashFile = getPasswordHashFilePath();
    bool hasPassword = (GetFileAttributes(hashFile.c_str()) != INVALID_FILE_ATTRIBUTES);

    if (hasPassword)
    {
        if (!promptForPassword(L"Unlock Secrets", L"Enter your master password:", password, false))
        {
            return;
        }

        if (!verifyPassword(password))
        {
            ::MessageBox(_hSelf, L"Incorrect password!", L"Error", MB_OK | MB_ICONERROR);
            return;
        }
    }
    else
    {
        if (IDNO == ::MessageBox(_hSelf, 
            L"No master password set.\n\nWould you like to create one now?", 
            L"First Time Setup", 
            MB_YESNO | MB_ICONINFORMATION))
        {
            return;
        }

        if (!promptForPassword(L"Create Master Password", L"Create a new master password:", password, true))
        {
            return;
        }

        std::vector<BYTE> hash;
        hashPassword(password, hash);

        std::ofstream hashFileStream(hashFile, std::ios::binary | std::ios::trunc);
        if (hashFileStream.is_open())
        {
            DWORD hashSize = (DWORD)hash.size();
            hashFileStream.write((char*)&hashSize, sizeof(hashSize));
            hashFileStream.write((char*)hash.data(), hashSize);
            hashFileStream.close();

            ::MessageBox(_hSelf, L"Master password created successfully!", L"Success", MB_OK | MB_ICONINFORMATION);
        }
        else
        {
            ::MessageBox(_hSelf, L"Failed to save password hash!", L"Error", MB_OK | MB_ICONERROR);
            return;
        }
    }

    _masterPassword = password;
    _isUnlocked = true;

    loadSecrets();
    updateUIState();
    startAutoLockTimer();

    ::SetDlgItemText(_hSelf, IDC_STATUS_TEXT, L"Unlocked");
}

void SecretsManagerDlg::lockSecrets()
{
    _isUnlocked = false;
    _masterPassword.clear();
    _secrets.clear();
    _selectedIndex = -1;

    refreshSecretsList();

    ::SetDlgItemText(_hSelf, IDC_SECRET_NAME_EDIT, L"");
    ::SetDlgItemText(_hSelf, IDC_CATEGORY_EDIT, L"");
    ::SetDlgItemText(_hSelf, IDC_SECRET_VALUE_EDIT, L"");

    stopAutoLockTimer();
    updateUIState();

    ::SetDlgItemText(_hSelf, IDC_STATUS_TEXT, L"Locked");
}

void SecretsManagerDlg::updateUIState()
{
    BOOL enable = _isUnlocked ? TRUE : FALSE;

    ::EnableWindow(::GetDlgItem(_hSelf, IDC_ADD_BUTTON), enable);
    ::EnableWindow(::GetDlgItem(_hSelf, IDC_UPDATE_BUTTON), enable);
    ::EnableWindow(::GetDlgItem(_hSelf, IDC_DELETE_BUTTON), enable);
    ::EnableWindow(::GetDlgItem(_hSelf, IDC_COPY_BUTTON), enable);
    ::EnableWindow(::GetDlgItem(_hSelf, IDC_INSERT_BUTTON), enable);
    ::EnableWindow(::GetDlgItem(_hSelf, IDC_FILTER_EDIT), enable);
    ::EnableWindow(::GetDlgItem(_hSelf, IDC_SECRETS_LIST), enable);
    ::EnableWindow(::GetDlgItem(_hSelf, IDC_CHANGE_PASSWORD_BUTTON), enable);

    ::EnableWindow(::GetDlgItem(_hSelf, IDC_UNLOCK_BUTTON), !enable);
    ::EnableWindow(::GetDlgItem(_hSelf, IDC_LOCK_BUTTON), enable);

    HWND hNameEdit = ::GetDlgItem(_hSelf, IDC_SECRET_NAME_EDIT);
    HWND hCategoryEdit = ::GetDlgItem(_hSelf, IDC_CATEGORY_EDIT);
    HWND hValueEdit = ::GetDlgItem(_hSelf, IDC_SECRET_VALUE_EDIT);

    if (_isUnlocked)
    {
        SendMessage(hNameEdit, EM_SETREADONLY, FALSE, 0);
        SendMessage(hCategoryEdit, EM_SETREADONLY, FALSE, 0);
        SendMessage(hValueEdit, EM_SETREADONLY, FALSE, 0);
    }
    else
    {
        SendMessage(hNameEdit, EM_SETREADONLY, TRUE, 0);
        SendMessage(hCategoryEdit, EM_SETREADONLY, TRUE, 0);
        SendMessage(hValueEdit, EM_SETREADONLY, TRUE, 0);
    }
}

void SecretsManagerDlg::changePassword()
{
    if (!_isUnlocked)
    {
        ::MessageBox(_hSelf, L"Please unlock first!", L"Error", MB_OK | MB_ICONWARNING);
        return;
    }

    if (IDNO == ::MessageBox(_hSelf, 
        L"Change master password?\n\nThis will re-encrypt all secrets with the new password.", 
        L"Change Password", 
        MB_YESNO | MB_ICONQUESTION))
    {
        return;
    }

    std::wstring oldPassword;
    if (!promptForPassword(L"Verify Current Password", L"Enter your current password:", oldPassword, false))
    {
        return;
    }

    if (!verifyPassword(oldPassword))
    {
        ::MessageBox(_hSelf, L"Current password is incorrect!", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    std::wstring newPassword;
    if (!promptForPassword(L"New Password", L"Enter your new master password:", newPassword, true))
    {
        return;
    }

    std::vector<BYTE> hash;
    hashPassword(newPassword, hash);

    std::wstring hashFile = getPasswordHashFilePath();
    std::ofstream hashFileStream(hashFile, std::ios::binary | std::ios::trunc);
    if (hashFileStream.is_open())
    {
        DWORD hashSize = (DWORD)hash.size();
        hashFileStream.write((char*)&hashSize, sizeof(hashSize));
        hashFileStream.write((char*)hash.data(), hashSize);
        hashFileStream.close();
    }
    else
    {
        ::MessageBox(_hSelf, L"Failed to save new password!", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    _masterPassword = newPassword;
    saveSecrets();

    ::MessageBox(_hSelf, L"Password changed successfully!\n\nAll secrets have been re-encrypted.", L"Success", MB_OK | MB_ICONINFORMATION);
    ::SetDlgItemText(_hSelf, IDC_STATUS_TEXT, L"Password changed");
}

void SecretsManagerDlg::resetPasswordAndSecrets()
{
    int result = ::MessageBox(_hSelf,
        L"⚠️ WARNING ⚠️\n\n"
        L"This will PERMANENTLY DELETE:\n"
        L"• Your master password\n"
        L"• ALL stored secrets\n\n"
        L"This action CANNOT be undone!\n\n"
        L"Are you absolutely sure?",
        L"Reset Password & Delete All Secrets",
        MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2);

    if (result != IDYES)
    {
        return;
    }

    result = ::MessageBox(_hSelf,
        L"Last chance!\n\n"
        L"Click YES to permanently delete everything.\n"
        L"Click NO to cancel.",
        L"Final Confirmation",
        MB_YESNO | MB_ICONEXCLAMATION | MB_DEFBUTTON2);

    if (result != IDYES)
    {
        ::MessageBox(_hSelf, L"Operation cancelled.", L"Cancelled", MB_OK | MB_ICONINFORMATION);
        return;
    }

    lockSecrets();

    std::wstring hashFile = getPasswordHashFilePath();
    std::wstring secretsFile = getSecretsFilePath();

    bool hashDeleted = false;
    bool secretsDeleted = false;

    if (GetFileAttributes(hashFile.c_str()) != INVALID_FILE_ATTRIBUTES)
    {
        hashDeleted = (DeleteFile(hashFile.c_str()) != 0);
    }
    else
    {
        hashDeleted = true;
    }

    if (GetFileAttributes(secretsFile.c_str()) != INVALID_FILE_ATTRIBUTES)
    {
        secretsDeleted = (DeleteFile(secretsFile.c_str()) != 0);
    }
    else
    {
        secretsDeleted = true;
    }

    if (hashDeleted && secretsDeleted)
    {
        ::MessageBox(_hSelf,
            L"✓ Password and secrets deleted successfully!\n\n"
            L"Click 'Unlock' to create a new password.",
            L"Reset Complete",
            MB_OK | MB_ICONINFORMATION);

        ::SetDlgItemText(_hSelf, IDC_STATUS_TEXT, L"Reset complete - Create new password");
    }
    else
    {
        std::wstring message = L"Errors occurred:\n\n";
        if (!hashDeleted) message += L"• Failed to delete password file\n";
        if (!secretsDeleted) message += L"• Failed to delete secrets file\n";
        message += L"\nYou may need administrator privileges.";

        ::MessageBox(_hSelf, message.c_str(), L"Error", MB_OK | MB_ICONERROR);
    }
}

void SecretsManagerDlg::loadSecrets()
{
    _secrets.clear();

    if (!_isUnlocked || _masterPassword.empty())
        return;

    std::wstring filePath = getSecretsFilePath();
    std::ifstream file(filePath, std::ios::binary);

    if (!file.is_open())
        return;

    DWORD count = 0;
    file.read((char*)&count, sizeof(count));

    for (DWORD i = 0; i < count; i++)
    {
        Secret secret;

        DWORD nameSize = 0;
        file.read((char*)&nameSize, sizeof(nameSize));
        std::vector<BYTE> nameData(nameSize);
        file.read((char*)nameData.data(), nameSize);
        decryptDataWithPassword(nameData, _masterPassword, secret.name);

        DWORD valueSize = 0;
        file.read((char*)&valueSize, sizeof(valueSize));
        std::vector<BYTE> valueData(valueSize);
        file.read((char*)valueData.data(), valueSize);
        decryptDataWithPassword(valueData, _masterPassword, secret.value);

        DWORD categorySize = 0;
        file.read((char*)&categorySize, sizeof(categorySize));
        std::vector<BYTE> categoryData(categorySize);
        file.read((char*)categoryData.data(), categorySize);
        decryptDataWithPassword(categoryData, _masterPassword, secret.category);

        _secrets.push_back(secret);
    }

    file.close();
    refreshSecretsList();
}

void SecretsManagerDlg::saveSecrets()
{
    if (!_isUnlocked || _masterPassword.empty())
    {
        ::MessageBox(_hSelf, TEXT("Cannot save: Not unlocked"), TEXT("Error"), MB_OK | MB_ICONERROR);
        return;
    }

    std::wstring filePath = getSecretsFilePath();
    std::ofstream file(filePath, std::ios::binary | std::ios::trunc);

    if (!file.is_open())
    {
        ::MessageBox(_hSelf, TEXT("Failed to save secrets file."), TEXT("Error"), MB_OK | MB_ICONERROR);
        return;
    }

    DWORD count = (DWORD)_secrets.size();
    file.write((char*)&count, sizeof(count));

    for (const auto& secret : _secrets)
    {
        std::vector<BYTE> nameData, valueData, categoryData;

        encryptDataWithPassword(secret.name, _masterPassword, nameData);
        DWORD nameSize = (DWORD)nameData.size();
        file.write((char*)&nameSize, sizeof(nameSize));
        file.write((char*)nameData.data(), nameSize);

        encryptDataWithPassword(secret.value, _masterPassword, valueData);
        DWORD valueSize = (DWORD)valueData.size();
        file.write((char*)&valueSize, sizeof(valueSize));
        file.write((char*)valueData.data(), valueSize);

        encryptDataWithPassword(secret.category, _masterPassword, categoryData);
        DWORD categorySize = (DWORD)categoryData.size();
        file.write((char*)&categorySize, sizeof(categorySize));
        file.write((char*)categoryData.data(), categorySize);
    }

    file.close();
}

void SecretsManagerDlg::refreshSecretsList()
{
    HWND hList = ::GetDlgItem(_hSelf, IDC_SECRETS_LIST);
    SendMessage(hList, LB_RESETCONTENT, 0, 0);
    
    TCHAR filterText[256] = {0};
    ::GetDlgItemText(_hSelf, IDC_FILTER_EDIT, filterText, 256);
    std::wstring filter = filterText;
    std::transform(filter.begin(), filter.end(), filter.begin(), ::towlower);
    
    for (size_t i = 0; i < _secrets.size(); i++)
    {
        std::wstring searchText = _secrets[i].name;
        std::transform(searchText.begin(), searchText.end(), searchText.begin(), ::towlower);
        
        if (filter.empty() || searchText.find(filter) != std::wstring::npos)
        {
            std::wstring displayText = _secrets[i].name;
            if (!_secrets[i].category.empty())
                displayText = _secrets[i].category + TEXT(" - ") + displayText;
            
            int index = (int)SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)displayText.c_str());
            SendMessage(hList, LB_SETITEMDATA, index, (LPARAM)i);
        }
    }
}

void SecretsManagerDlg::addSecret()
{
    if (!_isUnlocked)
    {
        ::MessageBox(_hSelf, TEXT("Please unlock first!"), TEXT("Error"), MB_OK | MB_ICONWARNING);
        return;
    }

    TCHAR name[256] = {0};
    TCHAR value[1024] = {0};
    TCHAR category[256] = {0};

    ::GetDlgItemText(_hSelf, IDC_SECRET_NAME_EDIT, name, 256);
    ::GetDlgItemText(_hSelf, IDC_SECRET_VALUE_EDIT, value, 1024);
    ::GetDlgItemText(_hSelf, IDC_CATEGORY_EDIT, category, 256);

    if (lstrlen(name) == 0)
    {
        ::MessageBox(_hSelf, TEXT("Please enter a secret name."), TEXT("Validation Error"), MB_OK | MB_ICONWARNING);
        return;
    }

    if (lstrlen(value) == 0)
    {
        ::MessageBox(_hSelf, TEXT("Please enter a secret value."), TEXT("Validation Error"), MB_OK | MB_ICONWARNING);
        return;
    }

    Secret secret;
    secret.name = name;
    secret.value = value;
    secret.category = category;

    _secrets.push_back(secret);
    saveSecrets();
    refreshSecretsList();

    ::SetDlgItemText(_hSelf, IDC_SECRET_NAME_EDIT, TEXT(""));
    ::SetDlgItemText(_hSelf, IDC_SECRET_VALUE_EDIT, TEXT(""));
    ::SetDlgItemText(_hSelf, IDC_CATEGORY_EDIT, TEXT(""));

    ::SetDlgItemText(_hSelf, IDC_STATUS_TEXT, TEXT("Secret added"));
}

void SecretsManagerDlg::updateSecret()
{
    if (!_isUnlocked)
    {
        ::MessageBox(_hSelf, TEXT("Please unlock first!"), TEXT("Error"), MB_OK | MB_ICONWARNING);
        return;
    }

    if (_selectedIndex < 0 || _selectedIndex >= (int)_secrets.size())
    {
        ::MessageBox(_hSelf, TEXT("Please select a secret to update."), TEXT("Error"), MB_OK | MB_ICONWARNING);
        return;
    }

    TCHAR name[256] = {0};
    TCHAR value[1024] = {0};
    TCHAR category[256] = {0};

    ::GetDlgItemText(_hSelf, IDC_SECRET_NAME_EDIT, name, 256);
    ::GetDlgItemText(_hSelf, IDC_SECRET_VALUE_EDIT, value, 1024);
    ::GetDlgItemText(_hSelf, IDC_CATEGORY_EDIT, category, 256);

    if (lstrlen(name) == 0 || lstrlen(value) == 0)
    {
        ::MessageBox(_hSelf, TEXT("Name and value cannot be empty."), TEXT("Validation Error"), MB_OK | MB_ICONWARNING);
        return;
    }

    _secrets[_selectedIndex].name = name;
    _secrets[_selectedIndex].value = value;
    _secrets[_selectedIndex].category = category;

    saveSecrets();
    refreshSecretsList();

    ::SetDlgItemText(_hSelf, IDC_STATUS_TEXT, TEXT("Secret updated"));
}

void SecretsManagerDlg::deleteSecret()
{
    if (_selectedIndex < 0 || _selectedIndex >= (int)_secrets.size())
    {
        ::MessageBox(_hSelf, TEXT("Please select a secret to delete."), TEXT("Error"), MB_OK | MB_ICONWARNING);
        return;
    }
    
    int result = ::MessageBox(_hSelf, 
        TEXT("Are you sure you want to delete this secret?"), 
        TEXT("Confirm Delete"), 
        MB_YESNO | MB_ICONQUESTION);
    
    if (result == IDYES)
    {
        _secrets.erase(_secrets.begin() + _selectedIndex);
        _selectedIndex = -1;
        
        saveSecrets();
        refreshSecretsList();
        
        ::SetDlgItemText(_hSelf, IDC_SECRET_NAME_EDIT, TEXT(""));
        ::SetDlgItemText(_hSelf, IDC_SECRET_VALUE_EDIT, TEXT(""));
        ::SetDlgItemText(_hSelf, IDC_CATEGORY_EDIT, TEXT(""));
    }
}

void SecretsManagerDlg::copyToClipboard()
{
    if (_selectedIndex < 0 || _selectedIndex >= (int)_secrets.size())
        return;
    
    const std::wstring& value = _secrets[_selectedIndex].value;
    
    if (OpenClipboard(_hSelf))
    {
        EmptyClipboard();
        
        HGLOBAL hClipboardData = GlobalAlloc(GMEM_DDESHARE, (value.length() + 1) * sizeof(TCHAR));
        if (hClipboardData)
        {
            TCHAR* pchData = (TCHAR*)GlobalLock(hClipboardData);
            if (pchData)
            {
                lstrcpy(pchData, value.c_str());
                GlobalUnlock(hClipboardData);
                SetClipboardData(CF_UNICODETEXT, hClipboardData);
            }
        }
        CloseClipboard();
        
        ::SetDlgItemText(_hSelf, IDC_STATUS_TEXT, TEXT("Secret copied to clipboard!"));
    }
}

void SecretsManagerDlg::showSecret()
{
    if (_selectedIndex < 0 || _selectedIndex >= (int)_secrets.size())
        return;

    const Secret& secret = _secrets[_selectedIndex];
    ::SetDlgItemText(_hSelf, IDC_SECRET_NAME_EDIT, secret.name.c_str());
    ::SetDlgItemText(_hSelf, IDC_CATEGORY_EDIT, secret.category.c_str());

    std::wstring masked(secret.value.length(), L'*');
    ::SetDlgItemText(_hSelf, IDC_SECRET_VALUE_EDIT, masked.c_str());
}

void SecretsManagerDlg::insertSelectedSecret()
{
    if (_selectedIndex < 0 || _selectedIndex >= (int)_secrets.size())
    {
        ::MessageBox(_hSelf, TEXT("Please select a secret first."), TEXT("Error"), MB_OK | MB_ICONWARNING);
        return;
    }
    
    int which = -1;
    ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
    if (which == -1)
        return;
    
    HWND curScintilla = (which == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;
    
    const std::wstring& value = _secrets[_selectedIndex].value;
    std::string utf8Value;
    int utf8Size = WideCharToMultiByte(CP_UTF8, 0, value.c_str(), -1, NULL, 0, NULL, NULL);
    if (utf8Size > 0)
    {
        utf8Value.resize(utf8Size);
        WideCharToMultiByte(CP_UTF8, 0, value.c_str(), -1, &utf8Value[0], utf8Size, NULL, NULL);
        ::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM)utf8Value.c_str());
    }
}

void SecretsManagerDlg::filterSecrets()
{
    refreshSecretsList();
}

void SecretsManagerDlg::startAutoLockTimer()
{
    if (_hSelf && _isUnlocked)
    {
        ::SetTimer(_hSelf, AUTOLOCK_TIMER_ID, AUTOLOCK_TIMEOUT_MS, NULL);
    }
}

void SecretsManagerDlg::stopAutoLockTimer()
{
    if (_hSelf)
    {
        ::KillTimer(_hSelf, AUTOLOCK_TIMER_ID);
    }
}

void SecretsManagerDlg::resetAutoLockTimer()
{
    if (_isUnlocked)
    {
        stopAutoLockTimer();
        startAutoLockTimer();
    }
}

void SecretsManagerDlg::onAutoLockTimer()
{
    if (_isUnlocked)
    {
        lockSecrets();
        ::MessageBox(_hSelf, 
            L"Secrets have been automatically locked due to inactivity (60 minutes).", 
            L"Auto-Locked", 
            MB_OK | MB_ICONINFORMATION);
    }
}

void SecretsManagerDlg::initDialog()
{
    _isUnlocked = false;
    _masterPassword.clear();
    updateUIState();
    ::SetDlgItemText(_hSelf, IDC_STATUS_TEXT, L"Locked - Click Unlock");
}

INT_PTR CALLBACK SecretsManagerDlg::run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) 
    {
        case WM_INITDIALOG:
        {
            initDialog();
            return TRUE;
        }

        case WM_TIMER:
        {
            if (wParam == AUTOLOCK_TIMER_ID)
            {
                onAutoLockTimer();
                return TRUE;
            }
            break;
        }

        case WM_COMMAND:
        {
            resetAutoLockTimer();

            switch (LOWORD(wParam))
            {
                case IDC_UNLOCK_BUTTON:
                    unlockSecrets();
                    return TRUE;

                case IDC_LOCK_BUTTON:
                    lockSecrets();
                    return TRUE;

                case IDC_CHANGE_PASSWORD_BUTTON:
                    changePassword();
                    return TRUE;

                case IDC_ADD_BUTTON:
                    addSecret();
                    return TRUE;

                case IDC_UPDATE_BUTTON:
                    updateSecret();
                    return TRUE;

                case IDC_DELETE_BUTTON:
                    deleteSecret();
                    return TRUE;

                case IDC_COPY_BUTTON:
                    copyToClipboard();
                    return TRUE;

                case IDC_INSERT_BUTTON:
                    insertSelectedSecret();
                    return TRUE;

                case IDC_FILTER_EDIT:
                    if (HIWORD(wParam) == EN_CHANGE)
                    {
                        filterSecrets();
                    }
                    return TRUE;

                case IDC_SECRETS_LIST:
                    if (HIWORD(wParam) == LBN_SELCHANGE)
                    {
                        HWND hList = ::GetDlgItem(_hSelf, IDC_SECRETS_LIST);
                        int sel = (int)SendMessage(hList, LB_GETCURSEL, 0, 0);
                        if (sel != LB_ERR)
                        {
                            _selectedIndex = (int)SendMessage(hList, LB_GETITEMDATA, sel, 0);
                            showSecret();
                        }
                        return TRUE;
                    }
                    else if (HIWORD(wParam) == LBN_DBLCLK)
                    {
                        copyToClipboard();
                        return TRUE;
                    }
                    break;
            }
            break;
        }

        default:
            return DockingDlgInterface::run_dlgProc(message, wParam, lParam);
    }

    return FALSE;
}
