# Password Reset Feature - Quick Guide

## ✅ **New Feature Added: Reset Password Store**

If you forget your master password, you can now reset everything and start fresh!

---

## 📋 How to Reset Password & Secrets

### Step 1: Access the Menu
1. Open Notepad++
2. Go to **Plugins** → **Secrets Manager** → **Reset Password Store...**

### Step 2: Confirm Deletion
You'll see **two confirmation dialogs**:

**First Warning:**
```
⚠️ WARNING ⚠️

This will PERMANENTLY DELETE:
• Your master password
• ALL stored secrets

This action CANNOT be undone!

Are you absolutely sure?
```
Click **Yes** to continue, or **No** to cancel.

**Final Confirmation:**
```
Last chance!

Click YES to permanently delete everything.
Click NO to cancel.
```
Click **Yes** to proceed.

### Step 3: Reset Complete
You'll see:
```
✓ Password and secrets deleted successfully!

Click 'Unlock' to create a new password.
```

### Step 4: Create New Password
1. Click the **"Unlock"** button
2. You'll see: "No master password set. Would you like to create one now?"
3. Click **Yes**
4. Enter your new password twice
5. Click **OK**
6. Done! You can now add secrets again

---

## 🔒 What Gets Deleted

When you reset, these files are permanently deleted:

| File | Location | Contents |
|------|----------|----------|
| `secrets_hash.dat` | `%APPDATA%\Notepad++\plugins\config\` | Your password hash |
| `secrets.dat` | `%APPDATA%\Notepad++\plugins\config\` | All encrypted secrets |

---

## ⚠️ Important Notes

### Before Resetting
- **Backup any important secrets** - write them down somewhere safe
- This action is **irreversible**
- You will lose **all stored secrets**

### After Resetting
- Choose a **strong new password**
- Write it down in a secure place
- Re-add all your secrets

### Alternative: Export First
If you want to keep your secrets:

1. **Before resetting**, manually copy all secrets:
   - Unlock the secrets manager
   - Select each secret
   - Click "Copy" button
   - Paste into a temporary text file
   - Save the file in a secure location

2. **After resetting**:
   - Create new password
   - Re-add secrets from your backup file

---

## 🎯 Use Cases

### "I forgot my password"
✅ Use **Reset Password Store**

### "I want to change my password"
❌ Don't use reset!
✅ Use the **"Change Password"** button in the dialog instead
   (This keeps your secrets and just changes the password)

### "My password is compromised"
1. If you remember the old password:
   - Use **"Change Password"** button
2. If you don't remember it:
   - Use **"Reset Password Store"**
   - Re-add secrets manually

---

## 🚀 Quick Reset Commands

### Reset via Menu
```
Plugins → Secrets Manager → Reset Password Store...
```

### Manual File Deletion (Alternative)
If the menu doesn't work, delete the files manually:

```powershell
# In PowerShell:
Remove-Item "$env:APPDATA\Notepad++\plugins\config\secrets_hash.dat" -Force
Remove-Item "$env:APPDATA\Notepad++\plugins\config\secrets.dat" -Force
```

Then restart Notepad++ and click Unlock to create a new password.

---

## ✨ Menu Structure

After installation, your menu will look like:

```
Plugins
└── Secrets Manager
    ├── Show Secrets Manager
    ├── Insert Secret
    ├── Reset Password Store...    ← NEW!
    └── About
```

---

## 🔐 Security Features Summary

| Feature | Purpose |
|---------|---------|
| **Unlock** | Enter password to access secrets |
| **Lock** | Clear password from memory |
| **Change Password** | Change password, keep secrets |
| **Reset Password Store** | Delete everything, start fresh |

---

## 📝 Installation

Make sure you have the latest build:

```powershell
# Build the 32-bit version
msbuild vs.proj\NppPluginTemplate.vcxproj /p:Configuration=Release /p:Platform=Win32

# Install (as Administrator)
Copy-Item "bin\NppPluginTemplate.dll" `
    "C:\Program Files (x86)\Notepad++\plugins\SecretsManager\SecretsManager.dll" -Force

# Restart Notepad++
```

---

## 🎉 Complete Feature List

### Password Management
- ✅ Master password protection
- ✅ PBKDF2 key derivation
- ✅ SHA-256 password hashing
- ✅ Change password (keeps secrets)
- ✅ **Reset password store (NEW!)**

### Secret Management
- ✅ Add secrets with name/value/category
- ✅ Update existing secrets
- ✅ Delete secrets
- ✅ Search/filter secrets
- ✅ Copy to clipboard
- ✅ Insert into document

### Security
- ✅ Secrets always hidden (shown as ***)
- ✅ Password-protected encryption
- ✅ Lock/Unlock functionality
- ✅ Read-only fields when locked

---

## 🆘 Troubleshooting

### "Access Denied" when resetting
**Solution:** Close Notepad++ completely and try again

### Files not deleted
**Solution:** Delete manually using PowerShell (see above)

### Want to keep some secrets
**Solution:** Export them first, then reset, then re-add

---

## 🎯 Best Practice

### Recommended Workflow

1. **Regular use:**
   - Keep secrets manager locked when not in use
   - Use Change Password periodically
   
2. **If you forget password:**
   - Use Reset Password Store
   - Create a strong new password
   - Write it down safely

3. **Backup strategy:**
   - Keep a separate encrypted backup of critical secrets
   - Don't rely solely on the plugin

---

**You're all set!** Install the updated DLL and you'll have the password reset feature in your menu.
