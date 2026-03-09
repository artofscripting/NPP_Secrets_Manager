# Hotkey Feature - Ctrl+Shift+Alt+S

## ✅ Feature Added

**Toggle Secrets Manager panel with a single hotkey!**

### Hotkey
**Ctrl+Shift+Alt+S** - Opens or closes the Secrets Manager docking panel

---

## 🎯 How It Works

### First Press
- Press **Ctrl+Shift+Alt+S**
- Secrets Manager panel opens on the right side
- Panel is docked and ready to use

### Second Press
- Press **Ctrl+Shift+Alt+S** again
- Secrets Manager panel closes
- Returns focus to your document

---

## 💡 Benefits

### Quick Access
- No need to navigate menus
- Instant toggle with one keystroke
- Faster workflow

### Convenience
- Keep both hands on keyboard
- Switch between editing and secrets quickly
- Perfect for copy/paste workflows

### Productivity
```
Old way:
1. Click Plugins menu
2. Click Secrets Manager
3. Click Show Secrets Manager
4. Panel appears

New way:
1. Press Ctrl+Shift+Alt+S
2. Panel appears
```

---

## 🎹 Usage Examples

### Quick Copy Workflow
```
1. Working in your code
2. Press Ctrl+Shift+Alt+S → Panel opens
3. Double-click secret → Copied!
4. Press Ctrl+Shift+Alt+S → Panel closes
5. Press Ctrl+V → Secret pasted
```

### Quick Insert Workflow
```
1. Position cursor in document
2. Press Ctrl+Shift+Alt+S → Panel opens
3. Select secret
4. Click Insert button
5. Press Ctrl+Shift+Alt+S → Panel closes
```

### Keep Panel Open
```
1. Press Ctrl+Shift+Alt+S → Panel opens
2. Work with multiple secrets
3. Panel stays visible
4. Press Ctrl+Shift+Alt+S → Panel closes when done
```

---

## ⚙️ Technical Details

### Implementation
- **Shortcut Keys**: Ctrl + Shift + Alt + S
- **Behavior**: Toggle (show/hide)
- **Function**: `showSecretsManager()`
- **Location**: `PluginDefinition.cpp`

### Code Changes
```cpp
// Hotkey definition
ShortcutKey *shKey = new ShortcutKey;
shKey->_isAlt = true;
shKey->_isCtrl = true;
shKey->_isShift = true;
shKey->_key = 'S';

// Register command with hotkey
setCommand(0, TEXT("&Show Secrets Manager"), showSecretsManager, shKey, false);

// Toggle visibility
_secretsDlg.display(!_secretsDlg.isVisible());
```

---

## 🔧 Customization

### Change the Hotkey

If you want a different hotkey, you can change it in Notepad++:

1. Open Notepad++
2. Go to `Settings` → `Shortcut Mapper`
3. Click `Plugin commands` tab
4. Find **"Show Secrets Manager"**
5. Double-click to modify
6. Press your desired key combination
7. Click OK

### Popular Alternatives
- **Ctrl+Alt+S** (simpler, but may conflict)
- **Ctrl+Shift+S** (conflicts with Save As in some apps)
- **Alt+S** (simplest, but may conflict with menus)
- **F12** (single key, easy to reach)

---

## 📋 Installation

```powershell
# Build and install
Stop-Process -Name notepad++ -Force -ErrorAction SilentlyContinue
Copy-Item "bin\NppSecretsManager.dll" `
    "C:\Program Files (x86)\Notepad++\plugins\NppSecretsManager\NppSecretsManager.dll" -Force
Start-Process "C:\Program Files (x86)\Notepad++\notepad++.exe"
```

---

## 🎨 Visual Workflow

### Before (Menu Navigation)
```
[Notepad++]
   ↓ Mouse click
[Plugins Menu]
   ↓ Mouse click
[Secrets Manager]
   ↓ Mouse click
[Show Secrets Manager]
   ↓ Panel appears
[Secrets Manager Panel]
```

### After (Hotkey)
```
[Notepad++]
   ↓ Ctrl+Shift+Alt+S
[Secrets Manager Panel]
   ↓ Ctrl+Shift+Alt+S
[Notepad++]
```

---

## ✨ Features Summary

### Hotkey Benefits
- ✅ **Fast**: Opens in milliseconds
- ✅ **Toggle**: One key to open and close
- ✅ **Keyboard-only**: No mouse required
- ✅ **Customizable**: Can be changed in settings
- ✅ **Memorable**: Ctrl+Shift+Alt+**S** for **S**ecrets
- ✅ **Productive**: Faster workflow

### Panel Behavior
- Opens on first press (if closed)
- Closes on second press (if open)
- Retains state (locked/unlocked)
- Auto-lock still works (60 minutes)
- All features accessible

---

## 🎯 Best Practices

### Workflow Integration

**Daily Use:**
```
1. Open Notepad++ in morning
2. Press Ctrl+Shift+Alt+S
3. Unlock secrets once
4. Close panel (Ctrl+Shift+Alt+S)
5. Throughout day:
   - Press Ctrl+Shift+Alt+S when need secret
   - Copy/insert secret
   - Press Ctrl+Shift+Alt+S to close
6. Auto-locks after 60 minutes
```

**Quick Tasks:**
```
1. Need to paste API key
2. Ctrl+Shift+Alt+S → Open
3. Double-click secret → Copy
4. Ctrl+Shift+Alt+S → Close
5. Ctrl+V → Paste
```

**Extended Use:**
```
1. Working on config files
2. Ctrl+Shift+Alt+S → Open
3. Keep panel open
4. Insert multiple secrets
5. Ctrl+Shift+Alt+S → Close when done
```

---

## 🆚 Comparison

### Menu Method
- **Steps**: 3 clicks
- **Time**: ~3 seconds
- **Requires**: Mouse
- **Efficiency**: Low

### Hotkey Method
- **Steps**: 1 keystroke
- **Time**: Instant
- **Requires**: Keyboard only
- **Efficiency**: High

---

## 💻 Development Details

### Files Modified
1. **PluginDefinition.cpp**
   - Added `ShortcutKey` allocation
   - Updated `commandMenuInit()` to register hotkey
   - Updated `commandMenuCleanUp()` to free memory
   - Modified `showSecretsManager()` to toggle visibility

2. **README.md**
   - Documented hotkey in Menu Commands section
   - Added Keyboard Shortcuts section

### Memory Management
```cpp
// Allocated in commandMenuInit()
ShortcutKey *shKey = new ShortcutKey;

// Freed in commandMenuCleanUp()
delete shKey;
```

---

## 🚀 Quick Start

### First Time
1. Install plugin (see Installation above)
2. Open Notepad++
3. Press **Ctrl+Shift+Alt+S**
4. Panel opens
5. Click Unlock
6. Create master password
7. Start adding secrets!

### Daily Use
```
Morning:
  Ctrl+Shift+Alt+S → Unlock → Ctrl+Shift+Alt+S

When Needed:
  Ctrl+Shift+Alt+S → Select Secret → Copy/Insert → Ctrl+Shift+Alt+S
```

---

## 📝 Summary

Your Secrets Manager now has:
- ✅ **Hotkey**: Ctrl+Shift+Alt+S to toggle panel
- ✅ **Toggle**: Opens if closed, closes if open
- ✅ **Fast**: Instant response
- ✅ **Convenient**: No mouse required
- ✅ **Customizable**: Can change in Shortcut Mapper
- ✅ **Productive**: Streamlined workflow

**Press Ctrl+Shift+Alt+S and manage your secrets with ease!** 🎉
