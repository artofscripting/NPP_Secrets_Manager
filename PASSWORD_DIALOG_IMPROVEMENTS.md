# Password Dialog Improvements

## ✅ Changes Made

### 1. **Centered on Screen**
- Added `DS_CENTER` style to the dialog
- Dialog now appears in the center of your screen
- No longer positioned relative to the parent window

### 2. **Increased Height**
- Height increased from 120 → 140 dialog units
- More comfortable spacing between controls
- Larger text labels (8 → 12 height)
- Taller edit boxes (14 → 16 height)
- Bigger buttons (14 → 18 height)

### 3. **Better Layout**
- Increased padding around controls
- More readable text
- Better button sizing
- Professional appearance

---

## 📐 Dialog Specifications

### Full Dialog (with password confirmation)
- **Size**: 300 × 140 dialog units
- **Position**: Center of screen
- **Controls**:
  - Prompt label: 270 width × 12 height
  - Password field: 270 width × 16 height
  - Confirm label: 270 width × 12 height
  - Confirm field: 270 width × 16 height
  - OK button: 80 width × 18 height
  - Cancel button: 80 width × 18 height

### Simple Dialog (password only, no confirmation)
- **Size**: 300 × 100 dialog units
- **Position**: Center of screen
- **Controls**:
  - Prompt label: 270 width × 12 height
  - Password field: 270 width × 16 height
  - OK button: 80 width × 18 height
  - Cancel button: 80 width × 18 height

---

## 🎨 Visual Improvements

### Before
```
┌─────────────────────────────┐
│ Password Required       [×] │ (Too short)
├─────────────────────────────┤
│ Enter password: [cramped]   │
│ Password: [_______________] │
│ Confirm: [____] (too small) │
│              [OK] [Cancel]  │
└─────────────────────────────┘
```

### After
```
┌──────────────────────────────────┐
│ Password Required            [×] │
├──────────────────────────────────┤
│                                  │
│ Enter password:                  │
│ [____________________________]   │
│                                  │
│ Confirm password:                │
│ [____________________________]   │
│                                  │
│        [  OK  ]  [ Cancel ]      │
│                                  │
└──────────────────────────────────┘
```

---

## 🔧 Technical Details

### Resource File Changes
**File**: `src\DockingFeature\SecretsManager.rc`

**Changes**:
- Dialog size: `250, 120` → `300, 140`
- Added `DS_CENTER` style flag
- Increased control sizes and spacing
- Better alignment

### Code Changes
**File**: `src\DockingFeature\SecretsManagerDlg.cpp`

**Changes**:
- Updated height when hiding confirm fields: `90` → `100`
- Maintains proper proportions when showing single-password dialog

---

## 📋 Installation

```powershell
# Build
msbuild vs.proj\NppPluginTemplate.vcxproj /p:Configuration=Release /p:Platform=Win32

# Install (as Administrator)
Stop-Process -Name notepad++ -Force -ErrorAction SilentlyContinue
Copy-Item "bin\NppPluginTemplate.dll" `
    "C:\Program Files (x86)\Notepad++\plugins\SecretsManager\SecretsManager.dll" -Force

# Start Notepad++
Start-Process "C:\Program Files (x86)\Notepad++\notepad++.exe"
```

---

## ✨ User Experience Improvements

### When Unlocking (First Time)
1. Click "Unlock" button
2. Dialog appears **centered on screen** (not in corner)
3. See: "No master password set. Would you like to create one now?"
4. Click Yes
5. **Larger password dialog** appears with clear labels
6. Enter password in **bigger text box**
7. Confirm password in second **bigger text box**
8. Click **bigger OK button**

### When Unlocking (Existing Password)
1. Click "Unlock" button
2. Dialog appears **centered on screen**
3. **Single password field** with proper height
4. Confirmation field is hidden
5. Dialog automatically resizes to 100 units tall

### When Changing Password
1. Click "Change Password" button
2. First dialog: Enter current password (centered, proper size)
3. Second dialog: Enter new password twice (centered, proper size)
4. All fields clearly visible and easy to use

---

## 🎯 Benefits

### Usability
- ✅ **Easier to find** - centered on screen
- ✅ **Easier to read** - larger text
- ✅ **Easier to type** - bigger input boxes
- ✅ **Easier to click** - larger buttons

### Accessibility
- ✅ Better for users with visual impairments
- ✅ Less eye strain
- ✅ Clearer visual hierarchy
- ✅ More professional appearance

### Consistency
- ✅ Matches modern dialog standards
- ✅ Consistent sizing across all password prompts
- ✅ Professional Windows application feel

---

## 🔍 Dialog Style Flags Explained

### DS_CENTER
- Centers dialog on screen
- Ignores parent window position
- Uses primary monitor

### DS_MODALFRAME
- Creates modal dialog frame
- Cannot interact with parent while open
- Proper window border

### DS_SETFONT
- Allows custom font specification
- Uses "MS Shell Dlg" (Windows standard)

### WS_POPUP
- Creates popup window
- No child window relationship for positioning

### WS_CAPTION
- Shows title bar
- Includes window title and close button

### WS_SYSMENU
- Adds system menu
- Includes close button (X)

---

## 📏 Dialog Unit Conversion

Dialog units are relative to the font:
- **Horizontal**: 1/4 of average character width
- **Vertical**: 1/8 of character height

For "MS Shell Dlg" 8pt:
- 300 DLU ≈ 450 pixels wide
- 140 DLU ≈ 210 pixels tall

This provides a comfortable size for the password dialog.

---

## 🎨 Control Layout

### Spacing Guidelines
```
Top margin:    15 DLU
Left margin:   15 DLU
Right margin:  15 DLU
Bottom margin: 10 DLU

Between label and field:  8 DLU
Between fields:           5 DLU
Before buttons:          10 DLU
Between buttons:          5 DLU
```

### Control Heights
```
Labels:       12 DLU
Edit boxes:   16 DLU
Buttons:      18 DLU
```

---

## 🆕 Updated Build

**Build time**: Latest
**File**: `bin\NppPluginTemplate.dll`
**Size**: ~244 KB
**Architecture**: 32-bit (x86)

---

## ✅ Testing Checklist

After installation, test these scenarios:

- [ ] Click "Unlock" - dialog centered?
- [ ] Dialog big enough to read easily?
- [ ] Password field easy to click?
- [ ] Buttons easy to click?
- [ ] Create new password - both fields visible?
- [ ] Change password - all dialogs centered?
- [ ] Text readable at normal viewing distance?

---

## 🎯 Summary

Your password dialog is now:
- ✅ **Centered on screen** (DS_CENTER flag)
- ✅ **Taller** (140 units vs 120)
- ✅ **Wider** (300 units vs 250)
- ✅ **Larger controls** (better spacing)
- ✅ **More professional** (consistent sizing)
- ✅ **Easier to use** (improved UX)

**Install the updated DLL and enjoy the improved password dialog!**
