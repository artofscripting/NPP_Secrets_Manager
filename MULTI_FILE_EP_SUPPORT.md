# Multi-File .ep Support - Feature Summary

## ✅ New Features Added

The Secrets Manager plugin now supports managing multiple .ep files!

### 1. **Load Different .ep Files**
- Click **"Load..."** button in the File section
- Browse and select any .ep file on your computer
- Each file can have its own password
- Switch between different secret vaults easily

### 2. **Create New .ep Files**
- Click **"New"** button in the File section
- Choose where to save the new .ep file
- Set a new password for the file
- Start fresh with an empty vault

### 3. **Current File Display**
- Shows currently loaded .ep file name
- Located at the top of the panel in the "File" section
- Updates automatically when you load/create files

---

## 🎯 Use Cases

### Personal & Work Secrets
```
1. Load: C:\Users\You\personal.ep (password: personal123)
2. Load: C:\Users\You\work.ep (password: work456)
```

### Project-Specific Secrets
```
1. Load: D:\Projects\ProjectA\secrets.ep
2. Load: D:\Projects\ProjectB\secrets.ep
```

### Team Shared Secrets
```
1. Load: \\NetworkShare\team-secrets.ep
2. Share password with team members securely
```

### Environment-Specific
```
1. Load: dev-secrets.ep (development credentials)
2. Load: staging-secrets.ep (staging credentials)
3. Load: prod-secrets.ep (production credentials)
```

---

## 🖥️ UI Layout

The dialog now has a **File section** at the top:

```
┌────────────────────────────────┐
│ File                       │
│ Current: [secrets.ep]          │
│          [Load...] [New]       │
├────────────────────────────────┤
│ Filter                         │
│ Search: [_____________]        │
├────────────────────────────────┤
│ Secrets                        │
│ [List of secrets...]           │
└────────────────────────────────┘
```

---

## 📋 How to Use

### Loading an Existing .ep File

1. Click **"Load..."** button
2. Navigate to your .ep file
3. Click **Open**
4. File name appears in "Current:" field
5. Click **Unlock** and enter the password for that file

### Creating a New .ep File

1. Click **"New"** button
2. Choose location and enter filename (e.g., `project-secrets.ep`)
3. Click **Save**
4. File name appears in "Current:" field
5. Click **Unlock** to set the initial password

### Switching Between Files

1. If currently unlocked, you'll be prompted to lock first
2. Click **"Load..."** or **"New"**
3. Select different file
4. Click **Unlock** with that file's password

---

## ⚙️ Technical Details

### File Tracking
- `_currentFilePath` member variable stores the active file
- Defaults to `%APPDATA%\Notepad++\plugins\config\secrets.ep`
- Can be any path on your system

### File Dialogs
- **Load**: `GetOpenFileName` with .ep filter
- **New**: `GetSaveFileName` with .ep filter and overwrite prompt
- Default extension: `.ep`

### Safety Features
- Prompts before switching files when unlocked
- Locks current vault when loading different file
- Shows confirmation for file overwrite
- Updates UI immediately after file selection

---

## 🔐 Security Considerations

### Each File Has Its Own Password
- Different .ep files can have different passwords
- Switching files requires unlocking with correct password
- No automatic password storage

### File Location Security
- Store sensitive .ep files in secure locations
- Use encrypted drives for highly sensitive data
- Don't share .ep files without encryption awareness

### Best Practices
✅ **DO**:
- Use descriptive filenames (e.g., `aws-secrets.ep`, `github-tokens.ep`)
- Keep different password for each file
- Store files in backed-up locations
- Lock vault when switching files

❌ **DON'T**:
- Store all secrets in one file
- Use same password for all files
- Leave files in public/shared folders
- Forget which file has which secrets

---

## 📂 File Organization Suggestions

### By Environment
```
C:\Secrets\
  ├── dev.ep
  ├── staging.ep
  └── production.ep
```

### By Service
```
C:\Secrets\
  ├── aws.ep
  ├── azure.ep
  ├── github.ep
  └── database.ep
```

### By Project
```
D:\Projects\
  ├── ProjectA\secrets.ep
  ├── ProjectB\secrets.ep
  └── ProjectC\secrets.ep
```

### By Team
```
\\NetworkShare\Secrets\
  ├── devteam.ep
  ├── ops.ep
  └── admin.ep
```

---

## 🆕 New UI Components

### Resource IDs Added
- `IDC_LOAD_FILE_BUTTON` (2615) - Load .ep file button
- `IDC_NEW_FILE_BUTTON` (2616) - Create new .ep file button  
- `IDC_CURRENT_FILE_TEXT` (2617) - Current file display label

### Dialog Changes
- Dialog height: 420 → 440 pixels (added 20px for File section)
- New "File" group box at top
- Adjusted all section positions down by 40 pixels

---

## 🔄 Workflow Example

### Scenario: Developer with Multiple Projects

**Morning - Project A**:
```
1. Open Notepad++
2. Plugins → Secrets Manager
3. Load... → D:\ProjectA\secrets.ep
4. Unlock with password_A
5. Copy AWS key for Project A
```

**Afternoon - Project B**:
```
1. Load... → D:\ProjectB\secrets.ep
2. Unlock with password_B  
3. Insert GitHub token for Project B
```

**Evening - Personal**:
```
1. Load... → C:\Users\Me\personal.ep
2. Unlock with password_personal
3. Access personal API keys
```

---

## 🚀 Advantages Over Single File

### Before (Single File)
- All secrets in one file
- One password for everything
- No organization by project/service
- Risk of one file getting corrupted

### After (Multiple Files)
- ✅ Separate files for different contexts
- ✅ Different passwords for different purposes
- ✅ Better organization
- ✅ Reduced risk (one file corruption doesn't affect others)
- ✅ Easier sharing (share specific .ep file with team)
- ✅ Better security (separate secrets by sensitivity level)

---

## 📦 Installation

```powershell
# Build and install
Stop-Process -Name notepad++ -Force -ErrorAction SilentlyContinue
Copy-Item "bin\NppPluginTemplate.dll" `
    "C:\Program Files (x86)\Notepad++\plugins\SecretsManager\SecretsManager.dll" -Force
Start-Process "C:\Program Files (x86)\Notepad++\notepad++.exe"
```

---

## 🎨 UI Preview

```
╔══════════════════════════════════════╗
║ File                              [×] ║
║ Current: aws-secrets.ep               ║
║          [Load...] [New]              ║
╠══════════════════════════════════════╣
║ Filter                                ║
║ Search: [_______________]             ║
╠══════════════════════════════════════╣
║ Secrets                               ║
║ ┌──────────────────────────────────┐ ║
║ │ AWS - AccessKeyId                │ ║
║ │ AWS - SecretAccessKey            │ ║
║ │ Database - ConnectionString      │ ║
║ └──────────────────────────────────┘ ║
╠══════════════════════════════════════╣
║ Secret Details                        ║
║ Name:     [________________]          ║
║ Category: [________________]          ║
║ Value:    [________________]          ║
║           [________________]          ║
║           [________________]          ║
╠══════════════════════════════════════╣
║ Actions                               ║
║ [Unlock] [Lock]                       ║
║ [Add] [Update] [Delete]               ║
║ [Copy] [Insert]                       ║
║ [Change Password]                     ║
║                                       ║
║ Status: Unlocked                      ║
╚══════════════════════════════════════╝
```

---

## 🎯 Summary

Your Secrets Manager plugin can now:
- ✅ **Load** any .ep file from any location
- ✅ **Create** new .ep files anywhere
- ✅ **Display** current file name
- ✅ **Switch** between different vaults
- ✅ **Organize** secrets by project/service/environment
- ✅ **Share** specific .ep files with teams
- ✅ **Secure** each file with its own password

**Happy secret managing!** 🎉
