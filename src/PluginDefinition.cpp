//this file is part of notepad++
//Copyright (C)2022 Don HO <don.h@free.fr>
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
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "PluginDefinition.h"
#include "menuCmdID.h"
#include "DockingFeature/SecretsManagerDlg.h"

SecretsManagerDlg _secretsDlg;

//
// The plugin data that Notepad++ needs
//
FuncItem funcItem[nbFunc];

//
// The data of Notepad++ that you can use in your plugin commands
//
NppData nppData;

//
// Initialize your plugin data here
// It will be called while plugin loading   
void pluginInit(HANDLE hModule)
{
    _secretsDlg.init((HINSTANCE)hModule, NULL);
}

//
// Here you can do the clean up, save the parameters (if any) for the next session
//
void pluginCleanUp()
{
    _secretsDlg.destroy();
}

//
// Initialization of your plugin commands
// You should fill your plugins commands here
void commandMenuInit()
{

    //--------------------------------------------//
    //-- STEP 3. CUSTOMIZE YOUR PLUGIN COMMANDS --//
    //--------------------------------------------//
    // with function :
    // setCommand(int index,                      // zero based number to indicate the order of command
    //            TCHAR *commandName,             // the command name that you want to see in plugin menu
    //            PFUNCPLUGINCMD functionPointer, // the symbol of function (function pointer) associated with this command. The body should be defined below. See Step 4.
    //            ShortcutKey *shortcut,          // optional. Define a shortcut to trigger this command
    //            bool check0nInit                // optional. Make this menu item be checked visually
    //            );
    setCommand(0, TEXT("&Show Secrets Manager"), showSecretsManager, NULL, false);
    setCommand(1, TEXT("&Insert Secret"), insertSecret, NULL, false);
    setCommand(2, TEXT("&Reset Password Store..."), resetPasswordStore, NULL, false);
    setCommand(3, TEXT("&About"), aboutSecretsManager, NULL, false);
}

//
// Here you can do the clean up (especially for the shortcut)
//
void commandMenuCleanUp()
{
	// Don't forget to deallocate your shortcut here
}


//
// This function help you to initialize your plugin commands
//
bool setCommand(size_t index, TCHAR *cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey *sk, bool check0nInit) 
{
    if (index >= nbFunc)
        return false;

    if (!pFunc)
        return false;

    lstrcpy(funcItem[index]._itemName, cmdName);
    funcItem[index]._pFunc = pFunc;
    funcItem[index]._init2Check = check0nInit;
    funcItem[index]._pShKey = sk;

    return true;
}

//----------------------------------------------//
//-- STEP 4. DEFINE YOUR ASSOCIATED FUNCTIONS --//
//----------------------------------------------//
void showSecretsManager()
{
    _secretsDlg.setParent(nppData._nppHandle);
    tTbData	data = {0};

    if (!_secretsDlg.isCreated())
    {
        _secretsDlg.create(&data);

        data.uMask = DWS_DF_CONT_RIGHT | DWS_ADDINFO;
        data.pszModuleName = _secretsDlg.getPluginFileName();
        data.dlgID = -1;

        ::SendMessage(nppData._nppHandle, NPPM_DMMREGASDCKDLG, 0, (LPARAM)&data);
    }
    _secretsDlg.display();
}

void insertSecret()
{
    if (!_secretsDlg.isCreated())
    {
        ::MessageBox(nppData._nppHandle, TEXT("Please open the Secrets Manager first."), TEXT("Secrets Manager"), MB_OK | MB_ICONINFORMATION);
        return;
    }
    _secretsDlg.insertSelectedSecret();
}

void resetPasswordStore()
{
    if (!_secretsDlg.isCreated())
    {
        _secretsDlg.setParent(nppData._nppHandle);
        tTbData data = {0};
        _secretsDlg.create(&data);
        data.uMask = DWS_DF_CONT_RIGHT | DWS_ADDINFO;
        data.pszModuleName = _secretsDlg.getPluginFileName();
        data.dlgID = -1;
        ::SendMessage(nppData._nppHandle, NPPM_DMMREGASDCKDLG, 0, (LPARAM)&data);
    }

    _secretsDlg.resetPasswordAndSecrets();
}

void aboutSecretsManager()
{
    ::MessageBox(NULL, 
        TEXT("Secrets Manager for Notepad++\n\n")
        TEXT("A secure plugin to store and manage sensitive information like API keys, passwords, and tokens.\n\n")
        TEXT("Features:\n")
        TEXT("- Password-protected encrypted storage\n")
        TEXT("- PBKDF2 + AES-256 encryption\n")
        TEXT("- Quick secret insertion into documents\n")
        TEXT("- Copy to clipboard\n")
        TEXT("- Search and filter secrets\n\n")
        TEXT("Security:\n")
        TEXT("- Master password required\n")
        TEXT("- Secrets always hidden (shown as ***)\n")
        TEXT("- Use 'Reset Password Store' if you forget your password"),
        TEXT("About Secrets Manager"), 
        MB_OK | MB_ICONINFORMATION);
}
