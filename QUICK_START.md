# Secrets Manager for Notepad++ - Quick Start Guide

## Installation

1. Copy `NppPluginTemplate.dll` from the `bin64` folder to:
   ```
   C:\Program Files\Notepad++\plugins\SecretsManager\
   ```
   (Create the `SecretsManager` folder if it doesn't exist)

2. Optionally rename the DLL to `SecretsManager.dll`

3. Restart Notepad++

## First Time Usage

1. Open Notepad++
2. Go to **Plugins** → **Secrets Manager** → **Show Secrets Manager**
3. The Secrets Manager panel will dock on the right side

## Managing Secrets

### Add a New Secret
1. Enter a **Name** (e.g., "GitHub API Key")
2. Optionally enter a **Category** (e.g., "API Keys")
3. Enter the **Value** (your actual secret)
4. Click **Add**

### Edit a Secret
1. Select the secret from the list
2. Modify the Name, Category, or Value fields
3. Click **Update**

### Delete a Secret
1. Select the secret from the list
2. Click **Delete**
3. Confirm deletion

## Using Secrets in Your Work

### Copy to Clipboard
- **Double-click** any secret in the list
- OR select a secret and click **Copy**
- The secret value is copied and ready to paste

### Insert into Document
1. Place your cursor where you want the secret
2. Select a secret from the list
3. Click **Insert**
- OR use **Plugins** → **Secrets Manager** → **Insert Secret**

### Search/Filter
- Type in the **Search** box to filter secrets by name
- The list updates as you type

## Storage & Security

- Secrets are encrypted using **Windows DPAPI**
- Stored at: `%APPDATA%\Notepad++\plugins\config\secrets.dat`
- Encryption is tied to your Windows user account
- **Backup your secrets regularly!**

## Tips

- Use **Categories** to organize related secrets (e.g., "AWS", "Azure", "GitHub")
- The **Status** bar at the bottom shows feedback messages
- Secrets are automatically saved after each Add/Update/Delete operation
- The filter search is case-insensitive

## Keyboard Shortcuts (Optional)

You can assign shortcuts via **Settings** → **Shortcut Mapper** → **Plugin commands**:
- Show Secrets Manager
- Insert Secret

## Troubleshooting

**Panel not showing?**
- Check Plugins menu → Secrets Manager → Show Secrets Manager

**Can't insert secret?**
- Make sure you have a document open
- Select a secret from the list first

**Secrets not loading?**
- Ensure you're logged in with the same Windows account
- Check the secrets file exists at the storage location

## Security Warning

⚠️ This plugin provides **convenience security**, not enterprise-grade protection:
- Encryption depends on your Windows account security
- Always maintain separate backups of critical secrets
- Don't rely solely on this for production credentials
- Consider using a dedicated password manager for critical secrets

## Support

For issues, questions, or suggestions, please check the README.md file.

---

**Version:** 1.0  
**License:** GPL v2.0
