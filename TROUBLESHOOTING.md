# Troubleshooting: "Cannot load 32-bit plugin" Error

## Problem
You're seeing an error like:
- "Cannot load 32-bit plugin"
- "Not compatible with current version of Notepad++"
- Plugin doesn't appear in the Plugins menu

## Root Cause
**Architecture mismatch** between Notepad++ and the plugin DLL.

---

## Solution

### Step 1: Determine Your Notepad++ Architecture

Run this PowerShell command:
```powershell
if (Test-Path "C:\Program Files\Notepad++\notepad++.exe") { 
    Write-Host "You have 64-bit Notepad++" 
} elseif (Test-Path "C:\Program Files (x86)\Notepad++\notepad++.exe") { 
    Write-Host "You have 32-bit Notepad++" 
} else { 
    Write-Host "Notepad++ not found in standard location" 
}
```

### Step 2: Use the Correct DLL

#### For 64-bit Notepad++ (Most Common)
✅ **Use:** `bin64\NppPluginTemplate.dll`

```powershell
# Build if not already done
msbuild vs.proj\NppPluginTemplate.vcxproj /p:Configuration=Release /p:Platform=x64

# Install
Copy-Item bin64\NppPluginTemplate.dll "C:\Program Files\Notepad++\plugins\SecretsManager\SecretsManager.dll" -Force
```

#### For 32-bit Notepad++
✅ **Use:** `bin\NppPluginTemplate.dll`

```powershell
# Build 32-bit version
msbuild vs.proj\NppPluginTemplate.vcxproj /p:Configuration=Release /p:Platform=Win32

# Install
Copy-Item bin\NppPluginTemplate.dll "C:\Program Files (x86)\Notepad++\plugins\SecretsManager\SecretsManager.dll" -Force
```

### Step 3: Automated Installation

Use the smart installer that auto-detects architecture:
```powershell
.\install_smart.ps1
```

---

## Quick Fix Commands

### Option 1: Auto-detect and Install
```powershell
PowerShell -ExecutionPolicy Bypass -File install_smart.ps1
```

### Option 2: Manual 64-bit Install
```powershell
# Stop Notepad++ if running
Stop-Process -Name notepad++ -ErrorAction SilentlyContinue

# Create directory
New-Item -ItemType Directory -Path "C:\Program Files\Notepad++\plugins\SecretsManager" -Force

# Copy 64-bit DLL
Copy-Item bin64\NppPluginTemplate.dll "C:\Program Files\Notepad++\plugins\SecretsManager\SecretsManager.dll" -Force

Write-Host "✓ Installed! Start Notepad++ now."
```

### Option 3: Manual 32-bit Install
```powershell
# Stop Notepad++ if running
Stop-Process -Name notepad++ -ErrorAction SilentlyContinue

# Create directory
New-Item -ItemType Directory -Path "C:\Program Files (x86)\Notepad++\plugins\SecretsManager" -Force

# Copy 32-bit DLL
Copy-Item bin\NppPluginTemplate.dll "C:\Program Files (x86)\Notepad++\plugins\SecretsManager\SecretsManager.dll" -Force

Write-Host "✓ Installed! Start Notepad++ now."
```

---

## Verification

After installation, verify the plugin loaded correctly:

1. **Start Notepad++**
2. **Check Plugins menu** - You should see "Secrets Manager"
3. **Test**: Plugins → Secrets Manager → About

If you still see the error:

### Additional Checks

1. **Verify DLL architecture:**
```powershell
dumpbin /headers bin64\NppPluginTemplate.dll | Select-String "machine"
# Should show: "8664 machine (x64)" for 64-bit
# Should show: "14C machine (x86)" for 32-bit
```

2. **Check Notepad++ version:**
   - Help → About Notepad++
   - Look for "(32-bit)" or "(64-bit)" in the title

3. **Ensure correct folder structure:**
```
C:\Program Files\Notepad++\
└── plugins\
    └── SecretsManager\
        └── SecretsManager.dll   ← Must be here
```

4. **Remove old versions:**
```powershell
# Remove any incorrectly placed DLLs
Remove-Item "C:\Program Files\Notepad++\plugins\*.dll" -ErrorAction SilentlyContinue
Remove-Item "C:\Program Files (x86)\Notepad++\plugins\*.dll" -ErrorAction SilentlyContinue
```

---

## Building for Different Architectures

### Build 64-bit (Default)
```powershell
msbuild vs.proj\NppPluginTemplate.vcxproj /p:Configuration=Release /p:Platform=x64
# Output: bin64\NppPluginTemplate.dll
```

### Build 32-bit
```powershell
msbuild vs.proj\NppPluginTemplate.vcxproj /p:Configuration=Release /p:Platform=Win32
# Output: bin\NppPluginTemplate.dll
```

### Build Both
```powershell
msbuild vs.proj\NppPluginTemplate.vcxproj /p:Configuration=Release /p:Platform=x64
msbuild vs.proj\NppPluginTemplate.vcxproj /p:Configuration=Release /p:Platform=Win32
```

---

## Still Not Working?

### Check Windows Event Viewer
1. Windows Key + X → Event Viewer
2. Windows Logs → Application
3. Look for errors from "notepad++.exe"

### Enable Notepad++ Debug Logging
1. In Notepad++: Settings → Preferences → Misc
2. Enable "Show debug info"
3. Restart and check console output

### Manual Test
Try loading the DLL directly:
```powershell
$dll = [System.Reflection.Assembly]::LoadFile("$PWD\bin64\NppPluginTemplate.dll")
Write-Host "DLL loaded successfully: $dll"
```

---

## Common Issues

| Error | Cause | Solution |
|-------|-------|----------|
| "Cannot load 32-bit plugin" | 32-bit DLL + 64-bit Notepad++ | Use bin64\NppPluginTemplate.dll |
| "Cannot load 64-bit plugin" | 64-bit DLL + 32-bit Notepad++ | Use bin\NppPluginTemplate.dll |
| Plugin not in menu | Wrong folder structure | Must be in plugins\SecretsManager\ |
| Access denied | Need admin rights | Run PowerShell as Administrator |
| DLL not found | Wrong path | Use install_smart.ps1 |

---

## Quick Fix - Use Smart Installer

**Recommended:** Just run the smart installer:
```powershell
PowerShell -ExecutionPolicy Bypass -File install_smart.ps1
```

It will:
- ✅ Auto-detect Notepad++ architecture
- ✅ Select correct DLL
- ✅ Verify compatibility
- ✅ Install to correct location
- ✅ Provide clear error messages

---

## Contact

If issues persist after trying these solutions, please provide:
1. Your Notepad++ version and architecture
2. The exact error message
3. Output of: `dumpbin /headers bin64\NppPluginTemplate.dll | Select-String "machine"`
