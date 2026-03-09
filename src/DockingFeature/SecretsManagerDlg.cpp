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
#include <cctype>
#include <cstring>

#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "bcrypt.lib")

extern NppData nppData;

std::wstring SecretsManagerDlg::_dialogPassword;
bool SecretsManagerDlg::_dialogConfirmMode = false;

namespace {

std::string wideToUtf8(const std::wstring& text)
{
    if (text.empty())
        return std::string();

    int size = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), (int)text.size(), NULL, 0, NULL, NULL);
    if (size <= 0)
        return std::string();

    std::string out(size, '\0');
    WideCharToMultiByte(CP_UTF8, 0, text.c_str(), (int)text.size(), &out[0], size, NULL, NULL);
    return out;
}

std::wstring utf8ToWide(const std::string& text)
{
    if (text.empty())
        return std::wstring();

    int size = MultiByteToWideChar(CP_UTF8, 0, text.data(), (int)text.size(), NULL, 0);
    if (size <= 0)
        return std::wstring();

    std::wstring out(size, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.data(), (int)text.size(), &out[0], size);
    return out;
}

std::string base64Encode(const std::vector<BYTE>& data)
{
    if (data.empty())
        return std::string();

    DWORD outLen = 0;
    if (!CryptBinaryToStringA(data.data(), (DWORD)data.size(), CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, NULL, &outLen))
        return std::string();

    std::string out(outLen, '\0');
    if (!CryptBinaryToStringA(data.data(), (DWORD)data.size(), CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, &out[0], &outLen))
        return std::string();

    if (!out.empty() && out.back() == '\0')
        out.pop_back();
    return out;
}

bool base64Decode(const std::string& input, std::vector<BYTE>& out)
{
    out.clear();
    if (input.empty())
        return true;

    DWORD outLen = 0;
    if (!CryptStringToBinaryA(input.c_str(), 0, CRYPT_STRING_BASE64, NULL, &outLen, NULL, NULL))
        return false;

    out.resize(outLen);
    return CryptStringToBinaryA(input.c_str(), 0, CRYPT_STRING_BASE64, out.data(), &outLen, NULL, NULL) == TRUE;
}

std::string jsonEscape(const std::string& value)
{
    std::string out;
    out.reserve(value.size() + 8);
    for (size_t i = 0; i < value.size(); ++i)
    {
        unsigned char c = (unsigned char)value[i];
        switch (c)
        {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\b': out += "\\b"; break;
            case '\f': out += "\\f"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (c < 0x20)
                {
                    char buf[7] = {0};
                    sprintf_s(buf, "\\u%04x", (unsigned int)c);
                    out += buf;
                }
                else
                {
                    out += (char)c;
                }
                break;
        }
    }
    return out;
}

bool readJsonString(const std::string& text, size_t& pos, std::string& out)
{
    out.clear();
    if (pos >= text.size() || text[pos] != '"')
        return false;

    ++pos;
    while (pos < text.size())
    {
        char c = text[pos++];
        if (c == '"')
            return true;

        if (c != '\\')
        {
            out.push_back(c);
            continue;
        }

        if (pos >= text.size())
            return false;

        char esc = text[pos++];
        switch (esc)
        {
            case '"': out.push_back('"'); break;
            case '\\': out.push_back('\\'); break;
            case '/': out.push_back('/'); break;
            case 'b': out.push_back('\b'); break;
            case 'f': out.push_back('\f'); break;
            case 'n': out.push_back('\n'); break;
            case 'r': out.push_back('\r'); break;
            case 't': out.push_back('\t'); break;
            case 'u':
            {
                if (pos + 4 > text.size())
                    return false;

                unsigned int code = 0;
                for (int i = 0; i < 4; ++i)
                {
                    char h = text[pos++];
                    code <<= 4;
                    if (h >= '0' && h <= '9') code |= (h - '0');
                    else if (h >= 'a' && h <= 'f') code |= (10 + (h - 'a'));
                    else if (h >= 'A' && h <= 'F') code |= (10 + (h - 'A'));
                    else return false;
                }

                // Keep this parser simple: encode BMP code points back into UTF-8.
                if (code <= 0x7F)
                {
                    out.push_back((char)code);
                }
                else if (code <= 0x7FF)
                {
                    out.push_back((char)(0xC0 | (code >> 6)));
                    out.push_back((char)(0x80 | (code & 0x3F)));
                }
                else
                {
                    out.push_back((char)(0xE0 | (code >> 12)));
                    out.push_back((char)(0x80 | ((code >> 6) & 0x3F)));
                    out.push_back((char)(0x80 | (code & 0x3F)));
                }
                break;
            }
            default:
                return false;
        }
    }
    return false;
}

void skipWs(const std::string& text, size_t& pos)
{
    while (pos < text.size() && std::isspace((unsigned char)text[pos]))
        ++pos;
}

bool findMatchingBrace(const std::string& text, size_t openPos, size_t& closePos)
{
    if (openPos >= text.size() || text[openPos] != '{')
        return false;

    int depth = 0;
    bool inString = false;
    bool escaped = false;
    for (size_t i = openPos; i < text.size(); ++i)
    {
        char c = text[i];
        if (inString)
        {
            if (escaped)
            {
                escaped = false;
            }
            else if (c == '\\')
            {
                escaped = true;
            }
            else if (c == '"')
            {
                inString = false;
            }
            continue;
        }

        if (c == '"')
        {
            inString = true;
        }
        else if (c == '{')
        {
            ++depth;
        }
        else if (c == '}')
        {
            --depth;
            if (depth == 0)
            {
                closePos = i;
                return true;
            }
        }
    }
    return false;
}

bool extractJsonStringField(const std::string& objectText, const char* fieldName, std::string& out)
{
    out.clear();
    std::string needle = std::string("\"") + fieldName + "\"";
    size_t keyPos = objectText.find(needle);
    if (keyPos == std::string::npos)
        return false;

    size_t colon = objectText.find(':', keyPos + needle.size());
    if (colon == std::string::npos)
        return false;

    size_t pos = colon + 1;
    skipWs(objectText, pos);
    return readJsonString(objectText, pos, out);
}

} // namespace

std::wstring SecretsManagerDlg::getSecretsFilePath()
{
    if (!_currentFilePath.empty())
    {
        return _currentFilePath;
    }

    TCHAR configDir[MAX_PATH] = {0};
    ::SendMessage(nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, MAX_PATH, (LPARAM)configDir);

    std::wstring path = configDir;
    path += TEXT("\\secrets.ep");
    return path;
}

bool SecretsManagerDlg::verifyPassword(const std::wstring& password)
{
    std::wstring filePath = getSecretsFilePath();
    if (GetFileAttributes(filePath.c_str()) == INVALID_FILE_ATTRIBUTES)
        return !password.empty();

    std::vector<Secret> probe;
    return loadSecretsForPassword(password, probe);
}

bool SecretsManagerDlg::deriveKeyFromPassword(const std::wstring& password, const std::vector<BYTE>& salt, std::vector<BYTE>& key)
{
    std::string utf8Password = wideToUtf8(password);
    if (utf8Password.empty() || salt.empty())
        return false;

    BCRYPT_ALG_HANDLE hAlg = NULL;
    if (BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, NULL, BCRYPT_ALG_HANDLE_HMAC_FLAG) != 0)
        return false;

    key.resize(32);
    NTSTATUS status = BCryptDeriveKeyPBKDF2(
        hAlg,
        (PUCHAR)utf8Password.data(),
        (ULONG)utf8Password.size(),
        const_cast<PUCHAR>(salt.data()),
        (ULONG)salt.size(),
        600000,
        key.data(),
        32,
        0
    );

    BCryptCloseAlgorithmProvider(hAlg, 0);
    return status == 0;
}

bool SecretsManagerDlg::encryptDataWithPassword(const std::wstring& plainText, const std::wstring& password, std::vector<BYTE>& encrypted)
{
    encrypted.clear();

    std::string utf8Plain = wideToUtf8(plainText);
    std::vector<BYTE> plainBytes(utf8Plain.begin(), utf8Plain.end());

    std::vector<BYTE> salt(16);
    std::vector<BYTE> nonce(12);
    if (BCryptGenRandom(NULL, salt.data(), (ULONG)salt.size(), BCRYPT_USE_SYSTEM_PREFERRED_RNG) != 0)
        return false;
    if (BCryptGenRandom(NULL, nonce.data(), (ULONG)nonce.size(), BCRYPT_USE_SYSTEM_PREFERRED_RNG) != 0)
        return false;

    std::vector<BYTE> key;
    if (!deriveKeyFromPassword(password, salt, key))
        return false;

    BCRYPT_ALG_HANDLE hAlg = NULL;
    BCRYPT_KEY_HANDLE hKey = NULL;
    PBYTE keyObj = NULL;
    DWORD keyObjLen = 0;
    DWORD resultLen = 0;

    NTSTATUS status = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_AES_ALGORITHM, NULL, 0);
    if (status != 0)
        return false;

    status = BCryptSetProperty(hAlg, BCRYPT_CHAINING_MODE, (PUCHAR)BCRYPT_CHAIN_MODE_GCM, (ULONG)(wcslen(BCRYPT_CHAIN_MODE_GCM) + 1) * sizeof(wchar_t), 0);
    if (status != 0)
    {
        BCryptCloseAlgorithmProvider(hAlg, 0);
        return false;
    }

    status = BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH, (PUCHAR)&keyObjLen, sizeof(keyObjLen), &resultLen, 0);
    if (status != 0)
    {
        BCryptCloseAlgorithmProvider(hAlg, 0);
        return false;
    }

    keyObj = (PBYTE)HeapAlloc(GetProcessHeap(), 0, keyObjLen);
    if (!keyObj)
    {
        BCryptCloseAlgorithmProvider(hAlg, 0);
        return false;
    }

    status = BCryptGenerateSymmetricKey(hAlg, &hKey, keyObj, keyObjLen, key.data(), (ULONG)key.size(), 0);
    if (status != 0)
    {
        HeapFree(GetProcessHeap(), 0, keyObj);
        BCryptCloseAlgorithmProvider(hAlg, 0);
        return false;
    }

    std::vector<BYTE> cipher(plainBytes.size());
    std::vector<BYTE> tag(16);
    BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO authInfo;
    BCRYPT_INIT_AUTH_MODE_INFO(authInfo);
    authInfo.pbNonce = nonce.data();
    authInfo.cbNonce = (ULONG)nonce.size();
    authInfo.pbTag = tag.data();
    authInfo.cbTag = (ULONG)tag.size();

    status = BCryptEncrypt(
        hKey,
        plainBytes.empty() ? NULL : plainBytes.data(),
        (ULONG)plainBytes.size(),
        &authInfo,
        NULL,
        0,
        cipher.empty() ? NULL : cipher.data(),
        (ULONG)cipher.size(),
        &resultLen,
        0
    );

    BCryptDestroyKey(hKey);
    HeapFree(GetProcessHeap(), 0, keyObj);
    BCryptCloseAlgorithmProvider(hAlg, 0);

    if (status != 0)
        return false;

    encrypted.reserve(salt.size() + nonce.size() + cipher.size() + tag.size());
    encrypted.insert(encrypted.end(), salt.begin(), salt.end());
    encrypted.insert(encrypted.end(), nonce.begin(), nonce.end());
    encrypted.insert(encrypted.end(), cipher.begin(), cipher.end());
    encrypted.insert(encrypted.end(), tag.begin(), tag.end());
    return true;
}

bool SecretsManagerDlg::decryptDataWithPassword(const std::vector<BYTE>& encrypted, const std::wstring& password, std::wstring& plainText)
{
    plainText.clear();
    if (encrypted.size() < 16 + 12 + 16)
        return false;

    const size_t saltLen = 16;
    const size_t nonceLen = 12;
    const size_t tagLen = 16;

    std::vector<BYTE> salt(encrypted.begin(), encrypted.begin() + saltLen);
    std::vector<BYTE> nonce(encrypted.begin() + saltLen, encrypted.begin() + saltLen + nonceLen);

    size_t cipherLen = encrypted.size() - saltLen - nonceLen - tagLen;
    std::vector<BYTE> cipher(encrypted.begin() + saltLen + nonceLen, encrypted.begin() + saltLen + nonceLen + cipherLen);
    std::vector<BYTE> tag(encrypted.end() - tagLen, encrypted.end());

    std::vector<BYTE> key;
    if (!deriveKeyFromPassword(password, salt, key))
        return false;

    BCRYPT_ALG_HANDLE hAlg = NULL;
    BCRYPT_KEY_HANDLE hKey = NULL;
    PBYTE keyObj = NULL;
    DWORD keyObjLen = 0;
    DWORD resultLen = 0;

    NTSTATUS status = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_AES_ALGORITHM, NULL, 0);
    if (status != 0)
        return false;

    status = BCryptSetProperty(hAlg, BCRYPT_CHAINING_MODE, (PUCHAR)BCRYPT_CHAIN_MODE_GCM, (ULONG)(wcslen(BCRYPT_CHAIN_MODE_GCM) + 1) * sizeof(wchar_t), 0);
    if (status != 0)
    {
        BCryptCloseAlgorithmProvider(hAlg, 0);
        return false;
    }

    status = BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH, (PUCHAR)&keyObjLen, sizeof(keyObjLen), &resultLen, 0);
    if (status != 0)
    {
        BCryptCloseAlgorithmProvider(hAlg, 0);
        return false;
    }

    keyObj = (PBYTE)HeapAlloc(GetProcessHeap(), 0, keyObjLen);
    if (!keyObj)
    {
        BCryptCloseAlgorithmProvider(hAlg, 0);
        return false;
    }

    status = BCryptGenerateSymmetricKey(hAlg, &hKey, keyObj, keyObjLen, key.data(), (ULONG)key.size(), 0);
    if (status != 0)
    {
        HeapFree(GetProcessHeap(), 0, keyObj);
        BCryptCloseAlgorithmProvider(hAlg, 0);
        return false;
    }

    std::vector<BYTE> plain(cipher.size());
    BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO authInfo;
    BCRYPT_INIT_AUTH_MODE_INFO(authInfo);
    authInfo.pbNonce = nonce.data();
    authInfo.cbNonce = (ULONG)nonce.size();
    authInfo.pbTag = tag.data();
    authInfo.cbTag = (ULONG)tag.size();

    status = BCryptDecrypt(
        hKey,
        cipher.empty() ? NULL : cipher.data(),
        (ULONG)cipher.size(),
        &authInfo,
        NULL,
        0,
        plain.empty() ? NULL : plain.data(),
        (ULONG)plain.size(),
        &resultLen,
        0
    );

    BCryptDestroyKey(hKey);
    HeapFree(GetProcessHeap(), 0, keyObj);
    BCryptCloseAlgorithmProvider(hAlg, 0);

    if (status != 0)
        return false;

    std::string utf8Plain(plain.begin(), plain.end());
    plainText = utf8ToWide(utf8Plain);
    return true;
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

    std::wstring secretsFile = getSecretsFilePath();
    bool hasSecretsFile = (GetFileAttributes(secretsFile.c_str()) != INVALID_FILE_ATTRIBUTES);

    if (hasSecretsFile)
    {
        if (!promptForPassword(L"Unlock Secrets", L"Enter your master password:", password, false))
        {
            return;
        }

        if (!verifyPassword(password))
        {
            ::MessageBox(_hSelf, L"Incorrect password or invalid .ep file!", L"Error", MB_OK | MB_ICONERROR);
            return;
        }
    }
    else
    {
        if (IDNO == ::MessageBox(_hSelf, 
            L"No local secrets.ep file found.\n\nWould you like to create one now?", 
            L"First Time Setup", 
            MB_YESNO | MB_ICONINFORMATION))
        {
            return;
        }

        if (!promptForPassword(L"Create Master Password", L"Create a new master password:", password, true))
        {
            return;
        }

        _masterPassword = password;
        _isUnlocked = true;
        _secrets.clear();
        saveSecrets();
        if (!_isUnlocked)
        {
            return;
        }

        ::MessageBox(_hSelf, L"Master password created and secrets.ep initialized.", L"Success", MB_OK | MB_ICONINFORMATION);

        loadSecrets();
        updateUIState();
        startAutoLockTimer();
        ::SetDlgItemText(_hSelf, IDC_STATUS_TEXT, L"Unlocked");
        return;
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

    _masterPassword = newPassword;
    saveSecrets();

    if (!_isUnlocked)
        return;

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

    std::wstring secretsFile = getSecretsFilePath();

    bool secretsDeleted = false;

    if (GetFileAttributes(secretsFile.c_str()) != INVALID_FILE_ATTRIBUTES)
    {
        secretsDeleted = (DeleteFile(secretsFile.c_str()) != 0);
    }
    else
    {
        secretsDeleted = true;
    }

    if (secretsDeleted)
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

    std::vector<Secret> loaded;
    if (!loadSecretsForPassword(_masterPassword, loaded))
    {
        ::MessageBox(_hSelf, L"Failed to decrypt or parse secrets.ep.", L"Error", MB_OK | MB_ICONERROR);
        lockSecrets();
        return;
    }

    _secrets.swap(loaded);
    refreshSecretsList();
}

bool SecretsManagerDlg::loadSecretsForPassword(const std::wstring& password, std::vector<Secret>& outSecrets)
{
    outSecrets.clear();

    std::wstring filePath = getSecretsFilePath();
    std::ifstream file(filePath, std::ios::binary);

    if (!file.is_open())
        return true;

    std::string envelope((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    std::string saltB64;
    std::string nonceB64;
    std::string ciphertextB64;

    if (!extractJsonStringField(envelope, "salt", saltB64) ||
        !extractJsonStringField(envelope, "nonce", nonceB64) ||
        !extractJsonStringField(envelope, "ciphertext", ciphertextB64))
    {
        return false;
    }

    std::vector<BYTE> salt;
    std::vector<BYTE> nonce;
    std::vector<BYTE> cipherAndTag;
    if (!base64Decode(saltB64, salt) || !base64Decode(nonceB64, nonce) || !base64Decode(ciphertextB64, cipherAndTag))
        return false;

    if (salt.size() != 16 || nonce.size() != 12 || cipherAndTag.size() < 16)
        return false;

    std::vector<BYTE> encrypted;
    encrypted.reserve(salt.size() + nonce.size() + cipherAndTag.size());
    encrypted.insert(encrypted.end(), salt.begin(), salt.end());
    encrypted.insert(encrypted.end(), nonce.begin(), nonce.end());
    encrypted.insert(encrypted.end(), cipherAndTag.begin(), cipherAndTag.end());

    std::wstring payloadWide;
    if (!decryptDataWithPassword(encrypted, password, payloadWide))
        return false;

    std::string payloadUtf8 = wideToUtf8(payloadWide);
    return parseSecretsPayload(payloadUtf8, outSecrets);
}

bool SecretsManagerDlg::parseSecretsPayload(const std::string& payload, std::vector<Secret>& outSecrets) const
{
    outSecrets.clear();

    size_t secretsKey = payload.find("\"secrets\"");
    if (secretsKey == std::string::npos)
        return false;

    size_t secretsObjStart = payload.find('{', secretsKey);
    if (secretsObjStart == std::string::npos)
        return false;

    size_t secretsObjEnd = 0;
    if (!findMatchingBrace(payload, secretsObjStart, secretsObjEnd) || secretsObjEnd <= secretsObjStart)
        return false;

    std::string secretsObj = payload.substr(secretsObjStart + 1, secretsObjEnd - secretsObjStart - 1);
    size_t pos = 0;
    while (pos < secretsObj.size())
    {
        skipWs(secretsObj, pos);
        if (pos >= secretsObj.size())
            break;

        if (secretsObj[pos] == ',')
        {
            ++pos;
            continue;
        }

        std::string key;
        if (!readJsonString(secretsObj, pos, key))
            return false;

        skipWs(secretsObj, pos);
        if (pos >= secretsObj.size() || secretsObj[pos] != ':')
            return false;
        ++pos;
        skipWs(secretsObj, pos);

        if (pos >= secretsObj.size() || secretsObj[pos] != '{')
            return false;

        size_t valObjEnd = 0;
        if (!findMatchingBrace(secretsObj, pos, valObjEnd) || valObjEnd <= pos)
            return false;

        std::string itemObj = secretsObj.substr(pos, valObjEnd - pos + 1);

        std::string valueUtf8;
        std::string groupUtf8;
        if (!extractJsonStringField(itemObj, "value", valueUtf8) || !extractJsonStringField(itemObj, "group", groupUtf8))
            return false;

        std::wstring fullKey = utf8ToWide(key);
        std::wstring defaultName = fullKey;
        std::wstring category = utf8ToWide(groupUtf8);
        size_t sep = fullKey.find(L"::");
        if (sep != std::wstring::npos)
        {
            category = fullKey.substr(0, sep);
            defaultName = fullKey.substr(sep + 2);
        }

        Secret secret;
        secret.name = defaultName;
        secret.category = category;
        secret.value = utf8ToWide(valueUtf8);
        outSecrets.push_back(secret);

        pos = valObjEnd + 1;
    }

    return true;
}

std::string SecretsManagerDlg::buildSecretsPayload() const
{
    std::string payload = "{\"secrets\":{";
    for (size_t i = 0; i < _secrets.size(); ++i)
    {
        const Secret& secret = _secrets[i];
        if (i > 0)
            payload += ",";

        std::string name = wideToUtf8(secret.name);
        std::string value = wideToUtf8(secret.value);
        std::string group = wideToUtf8(secret.category.empty() ? L"Default" : secret.category);
        std::string key = group + "::" + name;

        payload += "\"" + jsonEscape(key) + "\":{";
        payload += "\"value\":\"" + jsonEscape(value) + "\",";
        payload += "\"group\":\"" + jsonEscape(group) + "\",";
        payload += "\"url\":\"\"";
        payload += "}";
    }

    payload += "},\"access_logs\":[],\"config\":{\"change_password\":false},\"password_logs\":[]}";
    return payload;
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

    std::wstring payloadWide = utf8ToWide(buildSecretsPayload());
    std::vector<BYTE> encrypted;
    if (!encryptDataWithPassword(payloadWide, _masterPassword, encrypted) || encrypted.size() < 16 + 12 + 16)
    {
        file.close();
        ::MessageBox(_hSelf, TEXT("Failed to encrypt secrets payload."), TEXT("Error"), MB_OK | MB_ICONERROR);
        return;
    }

    const size_t saltLen = 16;
    const size_t nonceLen = 12;
    const std::vector<BYTE> salt(encrypted.begin(), encrypted.begin() + saltLen);
    const std::vector<BYTE> nonce(encrypted.begin() + saltLen, encrypted.begin() + saltLen + nonceLen);
    const std::vector<BYTE> cipherAndTag(encrypted.begin() + saltLen + nonceLen, encrypted.end());

    std::string envelope = "{";
    envelope += "\"salt\":\"" + base64Encode(salt) + "\",";
    envelope += "\"nonce\":\"" + base64Encode(nonce) + "\",";
    envelope += "\"ciphertext\":\"" + base64Encode(cipherAndTag) + "\"";
    envelope += "}";

    file.write(envelope.data(), (std::streamsize)envelope.size());

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
    updateFileDisplay();
    ::SetDlgItemText(_hSelf, IDC_STATUS_TEXT, L"Locked - Click Unlock");
}

void SecretsManagerDlg::updateFileDisplay()
{
    std::wstring filePath = getSecretsFilePath();

    // Extract just the filename
    size_t lastSlash = filePath.find_last_of(L"\\");
    std::wstring filename = (lastSlash != std::wstring::npos) ? filePath.substr(lastSlash + 1) : filePath;

    ::SetDlgItemText(_hSelf, IDC_CURRENT_FILE_TEXT, filename.c_str());
}

void SecretsManagerDlg::loadEpFile()
{
    if (_isUnlocked)
    {
        if (IDNO == ::MessageBox(_hSelf,
            L"Loading a different file will lock the current vault.\n\nContinue?",
            L"Load .ep File",
            MB_YESNO | MB_ICONQUESTION))
        {
            return;
        }
        lockSecrets();
    }

    OPENFILENAME ofn = {0};
    TCHAR szFile[MAX_PATH] = {0};

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = _hSelf;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile) / sizeof(TCHAR);
    ofn.lpstrFilter = TEXT("Encrypted Secrets Files (*.ep)\0*.ep\0All Files (*.*)\0*.*\0");
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn) == TRUE)
    {
        _currentFilePath = szFile;
        updateFileDisplay();
        ::SetDlgItemText(_hSelf, IDC_STATUS_TEXT, L"File loaded - Click Unlock");
    }
}

void SecretsManagerDlg::newEpFile()
{
    if (_isUnlocked)
    {
        if (IDNO == ::MessageBox(_hSelf,
            L"Creating a new file will lock the current vault.\n\nContinue?",
            L"New .ep File",
            MB_YESNO | MB_ICONQUESTION))
        {
            return;
        }
        lockSecrets();
    }

    OPENFILENAME ofn = {0};
    TCHAR szFile[MAX_PATH] = {0};

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = _hSelf;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile) / sizeof(TCHAR);
    ofn.lpstrFilter = TEXT("Encrypted Secrets Files (*.ep)\0*.ep\0All Files (*.*)\0*.*\0");
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrDefExt = TEXT("ep");
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

    if (GetSaveFileName(&ofn) == TRUE)
    {
        _currentFilePath = szFile;

        // Check if file already exists
        if (GetFileAttributes(_currentFilePath.c_str()) != INVALID_FILE_ATTRIBUTES)
        {
            if (IDNO == ::MessageBox(_hSelf,
                L"File already exists. Overwrite?",
                L"Confirm Overwrite",
                MB_YESNO | MB_ICONWARNING))
            {
                _currentFilePath.clear();
                return;
            }
        }

        updateFileDisplay();
        ::SetDlgItemText(_hSelf, IDC_STATUS_TEXT, L"New file - Click Unlock to set password");
    }
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
                case IDC_LOAD_FILE_BUTTON:
                    loadEpFile();
                    return TRUE;

                case IDC_NEW_FILE_BUTTON:
                    newEpFile();
                    return TRUE;

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
