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

#ifndef SECRETS_MANAGER_DLG_H
#define SECRETS_MANAGER_DLG_H

#include "DockingDlgInterface.h"
#include "resource.h"
#include <string>
#include <vector>
#include <map>

struct Secret {
    std::wstring name;
    std::wstring value;
    std::wstring category;
};

class SecretsManagerDlg : public DockingDlgInterface
{
public :
    SecretsManagerDlg() : DockingDlgInterface(IDD_SECRETS_MANAGER_DLG) {};

    virtual void display(bool toShow = true) const {
        DockingDlgInterface::display(toShow);
        if (toShow)
            ::SetFocus(::GetDlgItem(_hSelf, IDC_SECRET_NAME_EDIT));
    };

    void setParent(HWND parent2set){
        _hParent = parent2set;
    };

    void insertSelectedSecret();
    void resetPasswordAndSecrets();

protected :
    virtual INT_PTR CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);
    void initDialog();

private :
    std::vector<Secret> _secrets;
    int _selectedIndex = -1;
    bool _isUnlocked = false;
    std::wstring _masterPassword;
    std::vector<BYTE> _passwordHash;
    std::wstring _currentFilePath;

    static const UINT_PTR AUTOLOCK_TIMER_ID = 1;
    static const UINT AUTOLOCK_TIMEOUT_MS = 60 * 60 * 1000; // 60 minutes

    static std::wstring _dialogPassword;
    static bool _dialogConfirmMode;

    void loadSecrets();
    void saveSecrets();
    bool encryptDataWithPassword(const std::wstring& plainText, const std::wstring& password, std::vector<BYTE>& encrypted);
    bool decryptDataWithPassword(const std::vector<BYTE>& encrypted, const std::wstring& password, std::wstring& plainText);
    bool deriveKeyFromPassword(const std::wstring& password, const std::vector<BYTE>& salt, std::vector<BYTE>& key);
    bool loadSecretsForPassword(const std::wstring& password, std::vector<Secret>& outSecrets);
    bool parseSecretsPayload(const std::string& payload, std::vector<Secret>& outSecrets) const;
    std::string buildSecretsPayload() const;
    bool verifyPassword(const std::wstring& password);

    void unlockSecrets();
    void lockSecrets();
    void changePassword();
    void loadEpFile();
    void newEpFile();
    void addSecret();
    void updateSecret();
    void deleteSecret();
    void copyToClipboard();
    void showSecret();
    void refreshSecretsList();
    void filterSecrets();
    void updateUIState();
    void updateFileDisplay();
    bool promptForPassword(const wchar_t* title, const wchar_t* prompt, std::wstring& password, bool requireConfirm = false);
    static INT_PTR CALLBACK passwordDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    void startAutoLockTimer();
    void stopAutoLockTimer();
    void resetAutoLockTimer();
    void onAutoLockTimer();

    std::wstring getSecretsFilePath();
};

#endif //SECRETS_MANAGER_DLG_H
