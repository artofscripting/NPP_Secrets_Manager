# Install and Test Secrets Manager with Password Protection

Write-Host ""
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "  Secrets Manager - Install & Test" -ForegroundColor Cyan  
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host ""

# 1. Stop Notepad++
Write-Host "[1/5] Stopping Notepad++..." -ForegroundColor Yellow
$npp = Get-Process -Name notepad++ -ErrorAction SilentlyContinue
if ($npp) {
    Stop-Process -Name notepad++ -Force
    Start-Sleep -Seconds 3
    Write-Host "  Stopped" -ForegroundColor Green
} else {
    Write-Host "  Not running" -ForegroundColor Gray
}

# 2. Check DLL exists
Write-Host "[2/5] Checking build..." -ForegroundColor Yellow
if (!(Test-Path "bin\NppPluginTemplate.dll")) {
    Write-Host "  ERROR: DLL not found!" -ForegroundColor Red
    Write-Host "  Run: msbuild vs.proj\NppPluginTemplate.vcxproj /p:Configuration=Release /p:Platform=Win32" -ForegroundColor Yellow
    exit 1
}

$dll = Get-Item "bin\NppPluginTemplate.dll"
Write-Host "  Found: $($dll.Length) bytes, modified $($dll.LastWriteTime)" -ForegroundColor Green

# 3. Install
Write-Host "[3/5] Installing plugin..." -ForegroundColor Yellow
$targetDir = "C:\Program Files (x86)\Notepad++\plugins\SecretsManager"

if (!(Test-Path $targetDir)) {
    New-Item -ItemType Directory -Path $targetDir -Force | Out-Null
}

try {
    Copy-Item "bin\NppPluginTemplate.dll" "$targetDir\SecretsManager.dll" -Force
    Write-Host "  Installed to: $targetDir\SecretsManager.dll" -ForegroundColor Green
} catch {
    Write-Host "  ERROR: $_" -ForegroundColor Red
    Write-Host "  Run PowerShell as Administrator!" -ForegroundColor Yellow
    exit 1
}

# 4. Verify
Write-Host "[4/5] Verifying installation..." -ForegroundColor Yellow
$installed = Get-Item "$targetDir\SecretsManager.dll"
Write-Host "  Size: $($installed.Length) bytes" -ForegroundColor Green
Write-Host "  Modified: $($installed.LastWriteTime)" -ForegroundColor Green

# 5. Check architecture
Write-Host "[5/5] Checking architecture..." -ForegroundColor Yellow
$arch = dumpbin /headers "bin\NppPluginTemplate.dll" 2>&1 | Select-String "machine"
Write-Host "  $arch" -ForegroundColor Green

Write-Host ""
Write-Host "=========================================" -ForegroundColor Green
Write-Host "  INSTALLATION COMPLETE!" -ForegroundColor Green
Write-Host "=========================================" -ForegroundColor Green
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Cyan
Write-Host "  1. Start Notepad++" -ForegroundColor White
Write-Host "  2. Plugins -> Secrets Manager -> Show Secrets Manager" -ForegroundColor White
Write-Host "  3. Click 'Unlock' button" -ForegroundColor White
Write-Host "  4. Create your master password" -ForegroundColor White
Write-Host ""
Write-Host "If Unlock button doesn't work:" -ForegroundColor Yellow
Write-Host "  - See DEBUG_UNLOCK_BUTTON.md for troubleshooting" -ForegroundColor White
Write-Host "  - Check Notepad++ version (should be 32-bit)" -ForegroundColor White
Write-Host "  - Try completely restarting Notepad++" -ForegroundColor White
Write-Host ""

# Start Notepad++
$response = Read-Host "Start Notepad++ now? (Y/N)"
if ($response -eq "Y" -or $response -eq "y") {
    Start-Process "C:\Program Files (x86)\Notepad++\notepad++.exe"
    Write-Host "Notepad++ started!" -ForegroundColor Green
}
