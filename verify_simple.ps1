# Secrets Manager Plugin - Verification

Write-Host ""
Write-Host "SECRETS MANAGER PLUGIN - VERIFICATION" -ForegroundColor Cyan
Write-Host "======================================" -ForegroundColor Cyan
Write-Host ""

Write-Host "Build Output:" -ForegroundColor Yellow
if (Test-Path "bin64\NppPluginTemplate.dll") {
    $dll = Get-Item "bin64\NppPluginTemplate.dll"
    Write-Host "  ✓ DLL: $($dll.Name) - $([math]::Round($dll.Length/1KB,2)) KB" -ForegroundColor Green
    Write-Host "  Built: $($dll.LastWriteTime)" -ForegroundColor Gray
} else {
    Write-Host "  ✗ DLL NOT FOUND" -ForegroundColor Red
}

Write-Host ""
Write-Host "Key Source Files:" -ForegroundColor Yellow
$files = @(
    "src\DockingFeature\SecretsManagerDlg.h",
    "src\DockingFeature\SecretsManagerDlg.cpp",
    "src\DockingFeature\SecretsManager.rc"
)

foreach ($f in $files) {
    if (Test-Path $f) {
        Write-Host "  ✓ $f" -ForegroundColor Green
    } else {
        Write-Host "  ✗ $f" -ForegroundColor Red
    }
}

Write-Host ""
Write-Host "Documentation:" -ForegroundColor Yellow
Get-ChildItem -Filter "*.md" | ForEach-Object {
    Write-Host "  ✓ $($_.Name)" -ForegroundColor Green
}

Write-Host ""
Write-Host "Installation:" -ForegroundColor Yellow
Write-Host "  Run: .\install.ps1" -ForegroundColor White
Write-Host "  Or manually copy bin64\NppPluginTemplate.dll" -ForegroundColor Gray
Write-Host ""
