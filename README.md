# Secrets Manager Plugin for Notepad++

## Overview
Secrets Manager is a Notepad++ plugin that provides secure storage and management of sensitive information such as API keys, passwords, database connection strings, and other credentials.

## Features

### ?? Security
- **Encrypted Storage**: All secrets are encrypted using Windows DPAPI (Data Protection API)
- **User-Specific**: Secrets are tied to your Windows user account
- **Local Storage**: All data is stored locally on your machine

### ?? Management
- **Add Secrets**: Store key-value pairs with optional categories
- **Update Secrets**: Modify existing secrets easily
- **Delete Secrets**: Remove secrets you no longer need
- **Search/Filter**: Quickly find secrets by name

### ?? Quick Access
- **Insert into Document**: Insert secret values directly into your current document
- **Copy to Clipboard**: Double-click or use the Copy button to copy secret values
- **Docking Panel**: Keep the Secrets Manager visible while you work

## Usage

### Opening the Secrets Manager
1. Go to `Plugins` ? `Secrets Manager` ? `Show Secrets Manager`
2. The Secrets Manager panel will dock to the right side of Notepad++

### Adding a Secret
1. Enter a **Name** for your secret (e.g., "GitHub API Key")
2. Optionally enter a **Category** (e.g., "API Keys", "Databases")
3. Enter the **Value** (the actual secret)
4. Click **Add**

### Using Secrets
- **Insert into Document**: Select a secret from the list and click **Insert** (or use `Plugins` ? `Secrets Manager` ? `Insert Secret`)
- **Copy to Clipboard**: Double-click a secret in the list or click **Copy**
- **Edit**: Select a secret, modify the fields, and click **Update**
- **Delete**: Select a secret and click **Delete**

### Searching Secrets
- Use the **Search** box at the top to filter secrets by name
- The list updates as you type

## Storage Location
Secrets are stored in an encrypted file at:
```
%APPDATA%\Notepad++\plugins\config\secrets.dat
```

## Security Notes

?? **Important Security Information**:
- Secrets are encrypted using Windows DPAPI, which ties encryption to your Windows user account
- If you reinstall Windows or change your user account, you may lose access to your secrets
- The encryption is only as secure as your Windows login
- Always maintain backups of critical secrets in a separate secure location
- This plugin is designed for convenience, not enterprise-grade security

## Building from Source

### Prerequisites
- Visual Studio 2022 or later
- Windows SDK

### Build Steps
1. Open `vs.proj\NppPluginTemplate.vcxproj` in Visual Studio
2. Select your configuration (Release/x64 recommended)
3. Build the solution
4. The DLL will be in `bin64\` (or `bin\` for 32-bit)

### Installation
1. Copy the built DLL to your Notepad++ plugins folder:
   - `%ProgramFiles%\Notepad++\plugins\SecretsManager\` (create the SecretsManager folder)
2. Restart Notepad++

## Keyboard Shortcuts
You can assign keyboard shortcuts to plugin commands:
1. Go to `Settings` ? `Shortcut Mapper`
2. Select `Plugin commands` tab
3. Find "Secrets Manager" commands and assign shortcuts

## Troubleshooting

### Secrets not loading
- Check that the secrets file exists at the storage location
- Ensure you're logged in with the same Windows account that created the secrets
- Try restarting Notepad++

### Cannot insert secret
- Make sure you have a document open in Notepad++
- Ensure a secret is selected in the list

## License
This plugin is distributed under the GNU General Public License v2.0.

## Contributing
Contributions are welcome! Feel free to:
- Report bugs
- Suggest features
- Submit pull requests

## Disclaimer
This software is provided "as is" without warranty of any kind. Use at your own risk.
