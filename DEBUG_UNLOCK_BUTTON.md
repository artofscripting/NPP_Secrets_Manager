# Debugging the Unlock Button Issue

## Possible Causes

### 1. **Dialog Resource Not Found**
The password dialog (IDD_PASSWORD_DLG) might not be loading properly.

**Test:** After clicking Unlock, do you see:
- An error message saying "Failed to create password dialog. Error: [number]"?
- Nothing happens at all?
- A crash?

### 2. **Static Variables Not Initialized**
The static variables `_dialogPassword` and `_dialogConfirmMode` might have issues.

**Fixed in latest build:**
- Added proper static variable initialization
- Added error checking to DialogBoxParam

### 3. **Wrong HINSTANCE**
The dialog needs the correct module instance.

**Fixed in latest build:**
- Now gets HINSTANCE from window property
- Falls back to GetModuleHandle(NULL) if needed

### 4. **Resource Not Compiled**
The password dialog might not be in the compiled resources.

**Check:** Look at the file:
```
bin\NppPluginTemplate.dll
```

### 5. **Button Handler Not Called**
The WM_COMMAND for IDC_UNLOCK_BUTTON might not be wired up.

**Check the run_dlgProc:**
```cpp
case IDC_UNLOCK_BUTTON:
    unlockSecrets();
    return TRUE;
```

---

## Quick Test Steps

### Test 1: Check if button handler fires
1. Open Notepad++
2. Open Secrets Manager
3. Click "Unlock" button
4. What happens?

**Expected:**
- Password dialog should appear
- If first time: "No master password set. Would you like to create one now?"
- If exists: Password dialog immediately

**If nothing happens:**
- Button handler not firing
- Check if button ID matches: IDC_UNLOCK_BUTTON = 2612

### Test 2: Check resource IDs
Open `resource.h` and verify:
```cpp
#define IDD_PASSWORD_DLG            2550
#define IDC_PASSWORD_EDIT           2551
#define IDC_PASSWORD_CONFIRM_EDIT   2552
#define IDC_PASSWORD_PROMPT         2553
#define IDC_PASSWORD_CONFIRM_LABEL  2554

#define IDD_SECRETS_MANAGER_DLG     2600
#define IDC_UNLOCK_BUTTON           2612
```

### Test 3: Manually test password dialog
Add this test code to see if the dialog can be created:

```cpp
// In run_dlgProc, temporarily add:
case IDC_UNLOCK_BUTTON:
{
    HINSTANCE hInst = (HINSTANCE)::GetWindowLongPtr(_hSelf, GWLP_HINSTANCE);
    INT_PTR result = ::MessageBox(_hSelf, 
        L"Unlock button clicked! Instance handle is valid?", 
        L"Debug", 
        MB_OK);
    
    // Then call actual unlock
    unlockSecrets();
    return TRUE;
}
```

---

## Installation Command

Make sure you're installing the latest build:

```powershell
# Stop Notepad++
Stop-Process -Name notepad++ -Force -ErrorAction SilentlyContinue
Start-Sleep -Seconds 2

# Install (as Administrator)
Copy-Item "bin\NppPluginTemplate.dll" `
    "C:\Program Files (x86)\Notepad++\plugins\SecretsManager\SecretsManager.dll" -Force

# Verify it was copied
Get-Item "C:\Program Files (x86)\Notepad++\plugins\SecretsManager\SecretsManager.dll" | `
    Select-Object Name, Length, LastWriteTime
```

---

## Common Issues & Fixes

### Issue: "Nothing happens when I click Unlock"

**Solution 1:** Button ID mismatch
- Verify in SecretsManager.rc that button has: `IDC_UNLOCK_BUTTON`
- Verify in resource.h: `#define IDC_UNLOCK_BUTTON  (IDD_SECRETS_MANAGER_DLG + 12)`

**Solution 2:** Handler not added
- Check run_dlgProc has case for IDC_UNLOCK_BUTTON

**Solution 3:** Old DLL still loaded
- Completely close Notepad++
- Check Task Manager for lingering notepad++.exe
- Kill it if found
- Reinstall DLL
- Start Notepad++ fresh

### Issue: "Dialog appears but doesn't accept input"

**Solution:** Static variables not accessible
- Make sure these are at file scope in .cpp:
```cpp
std::wstring SecretsManagerDlg::_dialogPassword;
bool SecretsManagerDlg::_dialogConfirmMode = false;
```

### Issue: Error message with dialog creation

**If you see:** "Failed to create password dialog. Error: [number]"

**Common error codes:**
- **Error 1813**: Resource not found - rebuild project
- **Error 6**: Invalid handle - HINSTANCE is wrong
- **Error 1400**: Invalid window handle - parent window issue

---

## Debug Build Option

If issues persist, build a debug version:

```powershell
msbuild vs.proj\NppPluginTemplate.vcxproj `
    /p:Configuration=Debug `
    /p:Platform=Win32
```

This will:
- Include debug symbols
- Allow you to attach a debugger
- Show more detailed error messages

---

## Verify Build Output

Check the build included the password dialog:

```powershell
# Check if resource is in DLL
dumpbin /resources bin\NppPluginTemplate.dll | Select-String "2550"
```

Should show:
```
    2550        0x9f6      0x3e8     ENGLISH  Dialog
```

---

## Last Resort: Simplified Test

If nothing works, try this simplified unlock function:

```cpp
void SecretsManagerDlg::unlockSecrets()
{
    ::MessageBox(_hSelf, 
        L"Unlock button was clicked!", 
        L"Test", 
        MB_OK);
    
    // For now, just unlock without password
    _masterPassword = L"test123";
    _isUnlocked = true;
    
    // Save a simple hash file
    std::vector<BYTE> hash;
    hashPassword(_masterPassword, hash);
    
    std::wstring hashFile = getPasswordHashFilePath();
    std::ofstream file(hashFile, std::ios::binary);
    if (file.is_open())
    {
        DWORD size = (DWORD)hash.size();
        file.write((char*)&size, sizeof(size));
        file.write((char*)hash.data(), size);
        file.close();
    }
    
    loadSecrets();
    updateUIState();
    
    ::MessageBox(_hSelf, 
        L"Unlocked successfully!", 
        L"Success", 
        MB_OK);
}
```

This will at least prove the button is working and you can add secrets.

---

## Report Back

Please tell me:
1. **What happens when you click Unlock?**
   - Nothing?
   - Error message? (what does it say?)
   - Dialog appears?
   - Crash?

2. **Did you install the latest DLL?**
   - Check file timestamp
   - Should be very recent

3. **Any error in Notepad++ console?**
   - Check if Notepad++ shows any debug output

4. **Windows version?**
   - Windows 10/11?
   - 32-bit or 64-bit Windows?

This information will help me pinpoint the exact issue!
