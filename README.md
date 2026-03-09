# Secrets Manager Plugin for Notepad++

## Overview
Secrets Manager is a Notepad++ plugin that provides **password-protected**, encrypted storage and management of sensitive information such as API keys, passwords, database connection strings, and other credentials. Uses the same .ep file format as secrets_saver.py for cross-platform compatibility.

## Features

### 🔐 Security
- **Master Password Protection**: All secrets are protected by a master password
- **AES-GCM Encryption**: Industry-standard authenticated encryption
- **PBKDF2 Key Derivation**: SHA-256 with 600,000 iterations
- **Always Hidden**: Secret values are always displayed as asterisks (***) in the UI
- **Auto-Lock**: Automatically locks after 60 minutes of inactivity
- **Cross-Compatible**: Uses .ep file format compatible with secrets_saver.py
- **Local Storage**: All data is stored locally on your machine

### 🗂️ Management
- **Add Secrets**: Store key-value pairs with optional categories
- **Update Secrets**: Modify existing secrets easily
- **Delete Secrets**: Remove secrets you no longer need
- **Search/Filter**: Quickly find secrets by name
- **Lock/Unlock**: Manually lock and unlock your secrets vault
- **Change Password**: Change your master password and re-encrypt all secrets
- **Reset Password Store**: Delete all secrets and start fresh if you forget your password

### ⚡ Quick Access
- **Insert into Document**: Insert secret values directly into your current document
- **Copy to Clipboard**: Double-click or use the Copy button to copy secret values
- **Docking Panel**: Keep the Secrets Manager visible while you work

## Usage

### First Time Setup
1. Go to `Plugins` → `Secrets Manager` → `Show Secrets Manager`
2. Click the **Unlock** button
3. You'll be prompted to create a master password
4. Enter and confirm your password
5. ⚠️ **Remember this password!** There is no recovery method

### Opening the Secrets Manager
1. Go to `Plugins` → `Secrets Manager` → `Show Secrets Manager`
2. The Secrets Manager panel will dock to the right side of Notepad++
3. Click **Unlock** and enter your master password

### Adding a Secret
1. Click **Unlock** (if locked)
2. Enter a **Name** for your secret (e.g., "GitHub API Key")
3. Optionally enter a **Category** (e.g., "API Keys", "Databases")
4. Enter the **Value** (the actual secret)
5. Click **Add**

### Using Secrets
- **Insert into Document**: Select a secret from the list and click **Insert** (or use `Plugins` → `Secrets Manager` → `Insert Secret`)
- **Copy to Clipboard**: Double-click a secret in the list or click **Copy**
- **Edit**: Select a secret, modify the fields, and click **Update**
- **Delete**: Select a secret and click **Delete**

### Searching Secrets
- Use the **Search** box at the top to filter secrets by name
- The list updates as you type

### Locking and Security
- **Manual Lock**: Click the **Lock** button to immediately lock your vault
- **Auto-Lock**: Vault automatically locks after 60 minutes of inactivity
- **Change Password**: Click **Change Password** to update your master password
- **Reset**: Use `Plugins` → `Secrets Manager` → `Reset Password Store...` if you forget your password

## Storage Location
Secrets are stored in an encrypted .ep file at:
```
%APPDATA%\Notepad++\plugins\config\secrets.ep
```

## File Format (.ep)
The .ep file format is a JSON-based encrypted format compatible with secrets_saver.py:
```json
{
  "salt": "<base64-encoded-salt>",
  "nonce": "<base64-encoded-nonce>",
  "ciphertext": "<base64-encoded-encrypted-data>"
}
```

This allows you to:
- Use secrets across different tools (Notepad++ plugin and Python scripts)
- Backup and restore secrets easily
- Share encrypted secrets files (they're still password-protected)

## Security Details

### Encryption Specifications
- **Algorithm**: AES-256-GCM (Galois/Counter Mode)
- **Key Derivation**: PBKDF2-HMAC-SHA256
- **Iterations**: 600,000 rounds
- **Salt**: Random 16-byte salt per file
- **Nonce**: Random 12-byte nonce per encryption
- **Authentication**: Built-in authentication tag (GCM mode)

### Security Best Practices
✅ **DO**:
- Use a strong, unique master password
- Lock your secrets when not in use
- Change your password periodically
- Back up your .ep file to a secure location
- Use different passwords for different .ep files

❌ **DON'T**:
- Use simple passwords like "password123"
- Share your master password
- Leave secrets unlocked on shared computers
- Forget your password (no recovery available!)
- Share .ep files without encryption (they're portable!)

### Security Notes

⚠️ **Important Security Information**:
- **Password Protection**: All secrets require a master password to access
- **No Password Recovery**: If you forget your password, you must reset and lose all secrets
- **Portable Encryption**: .ep files can be moved between machines (password required)
- **Backup Compatible**: Easy to backup - just copy the .ep file
- **Cross-Platform**: Compatible with secrets_saver.py Python library
- **Auto-Lock**: Vault locks automatically after 60 minutes of inactivity

## Password Reset

If you forget your master password:

1. Go to `Plugins` → `Secrets Manager` → `Reset Password Store...`
2. Confirm deletion (⚠️ **ALL secrets will be permanently deleted**)
3. Click **Unlock** to create a new password
4. Re-add your secrets

Alternatively, delete the .ep file manually:
```powershell
Remove-Item "$env:APPDATA\Notepad++\plugins\config\secrets.ep" -Force
```

## Cross-Platform Usage

### Using with secrets_saver.py
The plugin uses the same .ep format as the Python secrets_saver library:

```python
from secrets_saver import SecretsSaver

# Open the same file used by the Notepad++ plugin
ss = SecretsSaver(filename=r"%APPDATA%\Notepad++\plugins\config\secrets.ep")

# Access secrets
secrets = ss.load()
print(secrets['GitHub API Key'])

# Add secrets
ss.save({'New Secret': 'value123'})
```

### Sharing .ep Files
You can:
- Use the same .ep file across multiple machines
- Share encrypted .ep files with team members (password required)
- Store .ep files in version control (encrypted)
- Backup .ep files to cloud storage (still encrypted)

## Building from Source

### Prerequisites
- Visual Studio 2022 or later (with v143 or v145 platform toolset)
- Windows SDK 10.0
- bcrypt.lib (included in Windows SDK)

### Build Steps
1. Open `vs.proj\NppPluginTemplate.vcxproj` in Visual Studio
2. Select your configuration:
   - **Release | Win32** for 32-bit Notepad++
   - **Release | x64** for 64-bit Notepad++
3. Build the solution
4. The DLL will be in `bin\` (32-bit) or `bin64\` (64-bit)

### Command Line Build
```powershell
# 32-bit build
msbuild vs.proj\NppPluginTemplate.vcxproj /p:Configuration=Release /p:Platform=Win32

# 64-bit build
msbuild vs.proj\NppPluginTemplate.vcxproj /p:Configuration=Release /p:Platform=x64
```

### Installation
1. Copy the built DLL to your Notepad++ plugins folder:
   - **32-bit**: `C:\Program Files (x86)\Notepad++\plugins\NppSecretsManager\NppSecretsManager.dll`
   - **64-bit**: `C:\Program Files\Notepad++\plugins\NppSecretsManager\NppSecretsManager.dll`
2. Restart Notepad++

## Menu Commands

### Plugins → Secrets Manager
- **Show Secrets Manager** - Opens/closes the docking panel (Hotkey: **Ctrl+Shift+Alt+S**)
- **Insert Secret** - Inserts selected secret into document
- **Reset Password Store...** - Deletes all secrets and password (⚠️ permanent!)
- **About** - Shows plugin information

## Keyboard Shortcuts

### Default Hotkey
- **Ctrl+Shift+Alt+S** - Toggle Secrets Manager panel (show/hide)

### Custom Shortcuts
You can assign additional keyboard shortcuts to plugin commands:
1. Go to `Settings` → `Shortcut Mapper`
2. Select `Plugin commands` tab
3. Find "Secrets Manager" commands and assign shortcuts

### Actions Panel
- **Unlock** - Enter master password to unlock vault
- **Lock** - Immediately lock the vault
- **Add** - Add a new secret
- **Update** - Update the selected secret
- **Delete** - Delete the selected secret
- **Copy** - Copy secret value to clipboard
- **Insert** - Insert secret value into document
- **Change Password** - Change master password and re-encrypt all secrets

### Status Indicators
- **Locked** - Vault is locked, secrets are not accessible
- **Unlocked** - Vault is unlocked, secrets can be accessed
- **Secret added/updated** - Confirmation of successful operation

## Keyboard Shortcuts
You can assign keyboard shortcuts to plugin commands:
1. Go to `Settings` → `Shortcut Mapper`
2. Select `Plugin commands` tab
3. Find "Secrets Manager" commands and assign shortcuts

## Troubleshooting

### Cannot unlock / "Incorrect password"
- Verify you're entering the correct password (case-sensitive)
- Ensure the .ep file hasn't been corrupted
- Try `Reset Password Store...` if you've forgotten the password

### Secrets not loading after password change
- Check that secrets.ep file exists in the config folder
- Verify file permissions allow read/write access

### Cannot insert secret
- Make sure vault is unlocked
- Ensure you have a document open in Notepad++
- Verify a secret is selected in the list

### Auto-lock not working
- Auto-lock triggers after 60 minutes of no interaction with the plugin
- Timer resets on any button click or action
- Closing Notepad++ stops the timer

### "Failed to decrypt" error
- Password is incorrect
- .ep file may be corrupted - restore from backup
- File format may be incompatible - check it's a valid .ep file

## Technical Details

### Dependencies
- BCrypt API (`bcrypt.lib`) for AES-GCM and PBKDF2
- Standard C++ libraries
- RapidJSON (for .ep file parsing)

### Compatibility
- **Windows**: 7, 8, 10, 11
- **Notepad++**: All modern versions
- **Architecture**: 32-bit and 64-bit
- **File Format**: Compatible with secrets_saver.py

### Plugin Architecture
- **Language**: C++
- **Framework**: Notepad++ Plugin Interface
- **UI**: Win32 dialogs
- **Storage**: JSON-based .ep format with AES-GCM encryption
- **Encryption**: BCrypt CNG APIs

## FAQ

**Q: What happens if I lose my master password?**  
A: You must use "Reset Password Store" which deletes all secrets. There is no recovery method.

**Q: Can I sync secrets between computers?**  
A: Yes! Just copy the .ep file. The same password works on any machine.

**Q: How secure is this?**  
A: It uses AES-256-GCM with PBKDF2 (600,000 iterations), which is industry-standard encryption.

**Q: Can I use the same secrets in Python?**  
A: Yes! Use the secrets_saver.py library with the same .ep file.

**Q: Can I change the auto-lock timeout?**  
A: Currently fixed at 60 minutes. Modify `AUTOLOCK_TIMEOUT_MS` in source code to change.

**Q: Why are secrets always shown as asterisks?**  
A: For security - prevents shoulder surfing and accidental exposure. Use Copy or Insert to access values.

**Q: Can I backup my secrets?**  
A: Yes! Just copy the secrets.ep file. It's encrypted, so it's safe to backup anywhere.

**Q: Is the .ep format portable?**  
A: Yes! Copy it between Windows/Mac/Linux. Use secrets_saver.py on non-Windows platforms.

## Version History

### v2.0 (Current)
- ✨ Master password protection
- ✨ AES-256-GCM encryption
- ✨ PBKDF2 with 600,000 iterations
- ✨ .ep file format (compatible with secrets_saver.py)
- ✨ Auto-lock after 60 minutes
- ✨ Password change functionality
- ✨ Reset password store option
- ✨ Always-hidden secret values
- ✨ Improved UI with better dialog sizing
- ✨ Cross-platform file format

### v1.0 (Initial)
- Basic secret storage
- Windows DPAPI encryption
- .dat binary format
- Docking panel UI
- Copy and insert functionality

## License
This plugin is distributed under the GNU General Public License v2.0.

## Contributing
Contributions are welcome! Feel free to:
- Report bugs
- Suggest features
- Submit pull requests

## Support
For issues, questions, or feedback, please open an issue on the GitHub repository.

## Related Projects
- **secrets_saver.py** - Python library using the same .ep format
- Compatible with the Notepad++ Secrets Manager plugin

## Credits
- Based on Notepad++ Plugin Template
- Uses BCrypt CNG APIs for encryption
- Compatible with secrets_saver.py Python library
- Developed for secure credential management in development workflows

---

**⚠️ Remember**: Always use a strong master password and keep backups of your .ep file!
- Suggest features
- Submit pull requests

## Disclaimer
This software is provided "as is" without warranty of any kind. Use at your own risk.
