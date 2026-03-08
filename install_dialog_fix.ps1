# Install Password Dialog Fix

Write-Host ""
Write-Host "Installing updated password dialog..." -ForegroundColor Cyan
Write-Host ""

# Stop Notepad++
Stop-Process -Name notepad++ -Force -ErrorAction SilentlyContinue
Start-Sleep -Seconds 2

# Install
Copy-Item "bin\NppPluginTemplate.dll" `
    "C:\Program Files (x86)\Notepad++\plugins\SecretsManager\SecretsManager.dll" -Force

Write-Host "✓ Installed!" -ForegroundColor Green
Write-Host ""
Write-Host "Password Dialog Improvements:" -ForegroundColor Yellow
Write-Host "  • Height: 140 → 180 dialog units (much taller!)" -ForegroundColor White
Write-Host "  • Width: 300 → 320 dialog units" -ForegroundColor White
Write-Host "  • Larger controls (20px edit boxes, 24px buttons)" -ForegroundColor White
Write-Host "  • Better spacing and margins" -ForegroundColor White
Write-Host "  • Modern Segoe UI font (9pt)" -ForegroundColor White
Write-Host "  • Centered on screen" -ForegroundColor White
Write-Host ""
Write-Host "You should now see:" -ForegroundColor Cyan
Write-Host "  ✓ Full password input box" -ForegroundColor Green
Write-Host "  ✓ OK and Cancel buttons" -ForegroundColor Green
Write-Host "  ✓ All text labels clearly visible" -ForegroundColor Green
Write-Host ""

# Start Notepad++
Start-Process "C:\Program Files (x86)\Notepad++\notepad++.exe"
Write-Host "Notepad++ started. Test the Unlock button!" -ForegroundColor Green
