# Secrets Manager - Password Protection Guide

## ✅ Changes Implemented

Your Secrets Manager plugin now has **proper password-based encryption**:

### 1. **No Hardcoded Passwords**
- ❌ Removed all hardcoded passwords like "admin123"
- ✅ Always prompts user for password input
- ✅ Proper password creation dialog on first use

### 2. **Secure Password Dialog**
- Custom password input dialog with:
  - Password masking (ES_PASSWORD style)
  - Confirmation field for new passwords
  - Validation to prevent empty passwords
  - Mismatch detection

### 3. **Always Hidden Secrets**
- Secret values **always displayed as asterisks** (*)
- No toggle visibility button (removed)
- Values remain encrypted in memory
- ES_PASSWORD style on value field

### 4. **Password-Protected Operations**
- **Unlock**: Prompts for existing password OR creates new one
- **Lock**: Clears password from memory
- **Change Password**: 
  - Verifies current password
  - Prompts for new password with confirmation
  - Re-encrypts all secrets automatically

---

## 📋 Installation Instructions

### For 32-bit Notepad++ (Your Current Setup)

```powershell
# 1. Close Notepad++
Stop-Process -Name notepad++ -Force -ErrorAction SilentlyContinue

# 2. Install the plugin (Run PowerShell as Administrator)
Copy-Item "bin\NppPluginTemplate.dll" "C:\Program Files (x86)\Notepad++\plugins\SecretsManager\SecretsManager.dll" -Force

# 3. Start Notepad++
Start-Process "C:\Program Files (x86)\Notepad++\notepad++.exe"
```

### For 64-bit Notepad++

```powershell
# Build 64-bit version first
msbuild vs.proj\NppPluginTemplate.vcxproj /p:Configuration=Release /p:Platform=x64

# Install
Copy-Item "bin64\NppPluginTemplate.dll" "C:\Program Files\Notepad++\plugins\SecretsManager\SecretsManager.dll" -Force
```

---

## 🔐 First Time Usage

### Step 1: Open Secrets Manager
1. Open Notepad++
2. Go to **Plugins** → **Secrets Manager** → **Show Secrets Manager**
3. The panel opens on the right side

### Step 2: Create Master Password
1. Click the **"Unlock"** button
2. You'll see: "No master password set. Would you like to create one now?"
3. Click **Yes**
4. A dialog appears: **"Create Master Password"**
5. Enter your desired password in the first field
6. Re-enter the same password in the "Confirm password" field
7. Click **OK**

**Important:** Remember this password! It encrypts all your secrets.

### Step 3: Add Your First Secret
1. Click in the **Name** field and type (e.g., "GitHub API Key")
2. Click in the **Category** field (optional, e.g., "API Keys")
3. Click in the **Value** field and type your secret
4. Click **"Add"** button
5. Your secret is now encrypted and saved!

---

## 🔒 Daily Usage

### Unlock Secrets
1. Click **"Unlock"** button
2. Enter your master password
3. Click OK
4. Status shows: "Unlocked"

### Lock Secrets
1. Click **"Lock"** button
2. All secrets cleared from memory
3. Password cleared from memory
4. Status shows: "Locked"

### View a Secret
1. Make sure you're unlocked
2. Click a secret in the list
3. Name and category appear in fields
4. Value shows as asterisks: `***********`
5. Double-click to **copy** the actual value to clipboard

### Add/Update/Delete
- All buttons work only when **unlocked**
- Changes are automatically encrypted and saved

---

## 🔑 Change Master Password

**When to change:**
- Regular security rotation
- Suspected password compromise
- Sharing device with others

**How to change:**
1. Click **"Unlock"** first
2. Click **"Change Password"** button
3. Confirm you want to change
4. Enter your **current** password
5. Enter your **new** password twice (with confirmation)
6. Click OK

**What happens:**
- All secrets are automatically re-encrypted with the new password
- Old password becomes invalid
- You must use the new password from now on

---

## 🛡️ Security Features

### Encryption
- **Algorithm**: AES-256 via Windows DPAPI
- **Key Derivation**: PBKDF2 with SHA-256
- **Iterations**: 10,000 (industry standard)
- **Salt**: Random 16-byte salt per secret
- **User-Based**: Tied to your Windows user account

### Password Storage
- **Never stored in plain text**
- Only SHA-256 hash is saved
- Hash stored in: `%APPDATA%\Notepad++\plugins\config\secrets_hash.dat`
- Secrets stored in: `%APPDATA%\Notepad++\plugins\config\secrets.dat`

### Memory Protection
- Password cleared when locked
- Secrets cleared from memory when locked
- No password caching

---

## ⚠️ Important Security Notes

### DO:
- ✅ Use a **strong, unique** password
- ✅ Remember your password (write it down safely if needed)
- ✅ Lock secrets when stepping away
- ✅ Back up your secrets file regularly
- ✅ Change password periodically

### DON'T:
- ❌ Use simple passwords like "password123"
- ❌ Share your master password
- ❌ Leave secrets unlocked on shared computers
- ❌ Forget your password (no recovery method!)

---

## 🚨 Password Recovery

**There is NO password recovery!**

If you forget your master password:
- All secrets are **permanently inaccessible**
- You must delete the secrets and start over
- This is by design for maximum security

**To start fresh:**
```powershell
# Delete password hash and secrets (PERMANENT!)
Remove-Item "$env:APPDATA\Notepad++\plugins\config\secrets_hash.dat" -Force
Remove-Item "$env:APPDATA\Notepad++\plugins\config\secrets.dat" -Force
```

---

## 📊 UI State Reference

| Button | When Enabled | Action |
|--------|--------------|--------|
| **Unlock** | Always (when locked) | Prompts for password, loads secrets |
| **Lock** | When unlocked | Clears secrets and password from memory |
| **Add** | When unlocked | Adds new secret |
| **Update** | When unlocked | Updates selected secret |
| **Delete** | When unlocked | Deletes selected secret |
| **Copy** | When unlocked | Copies secret value to clipboard |
| **Insert** | When unlocked | Inserts secret into document |
| **Change Password** | When unlocked | Changes master password |
| **Filter** | When unlocked | Filters secret list |

---

## 🐛 Troubleshooting

### "Password cannot be empty!"
- Enter at least one character
- Password field must not be blank

### "Passwords do not match!"
- Retype both fields carefully
- Make sure Caps Lock is off

### "Incorrect password!"
- Double-check your password
- Remember passwords are case-sensitive
- If forgotten, see **Password Recovery** section

### Secrets not appearing after unlock
- Make sure the correct password was entered
- Check if secrets.dat file exists
- Verify file permissions

### Can't click Add/Update/Delete
- Click **Unlock** button first
- Enter your password
- Buttons enable when unlocked

---

## 📁 File Locations

| File | Purpose | Location |
|------|---------|----------|
| **secrets.dat** | Encrypted secrets | `%APPDATA%\Notepad++\plugins\config\` |
| **secrets_hash.dat** | Password hash | `%APPDATA%\Notepad++\plugins\config\` |
| **Plugin DLL** | Plugin binary | `C:\Program Files (x86)\Notepad++\plugins\SecretsManager\` |

---

## 🎯 Best Practices

### Strong Password Examples
- ✅ `My$ecr3t#Pass2024!`
- ✅ `TreasureBox_42_Unlock`
- ✅ `N0t3p@d++S3cr3t5!`

### Weak Password Examples
- ❌ `password`
- ❌ `12345678`
- ❌ `admin`

### Backup Strategy
1. Export your secrets (manually copy to text file)
2. Encrypt the text file separately
3. Store in secure location (encrypted USB, password manager)
4. Test recovery periodically

---

## 📞 Support

**Built with:**
- Windows DPAPI (Data Protection API)
- BCrypt (CNG Cryptography)
- PBKDF2 key derivation
- SHA-256 hashing

**Compatible with:**
- Notepad++ (all versions)
- Windows 7 and later
- 32-bit and 64-bit architectures

---

## ✨ Summary

Your Secrets Manager now has:
- ✅ **No hardcoded passwords** - always prompts user
- ✅ **Proper password dialogs** - with confirmation
- ✅ **Always hidden secrets** - shown as asterisks
- ✅ **Secure password change** - verifies old, prompts for new
- ✅ **Strong encryption** - PBKDF2 + AES-256
- ✅ **User-controlled locking** - explicit unlock/lock buttons

**Ready to use!** Install the updated DLL and start securely managing your secrets in Notepad++.
