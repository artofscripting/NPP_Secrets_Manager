# Secrets Manager - Smart Installer (Detects Architecture)

Write-Host ""
Write-Host "═══════════════════════════════════════" -ForegroundColor Cyan
Write-Host "  SECRETS MANAGER - SMART INSTALLER" -ForegroundColor Cyan
Write-Host "═══════════════════════════════════════" -ForegroundColor Cyan
Write-Host ""

# Find Notepad++
$nppPaths = @(
    "C:\Program Files\Notepad++\notepad++.exe",
    "C:\Program Files (x86)\Notepad++\notepad++.exe"
)

$nppExe = $null
foreach ($path in $nppPaths) {
    if (Test-Path $path) {
        $nppExe = $path
        break
    }
}

if (-not $nppExe) {
    Write-Host "❌ Notepad++ not found!" -ForegroundColor Red
    Write-Host ""
    Write-Host "Please install Notepad++ first from:" -ForegroundColor Yellow
    Write-Host "  https://notepad-plus-plus.org/downloads/" -ForegroundColor White
    exit 1
}

Write-Host "✓ Found Notepad++: $nppExe" -ForegroundColor Green

# Detect Notepad++ architecture
$nppDir = Split-Path $nppExe
$is64bit = $nppDir -like "*Program Files\*" -and $nppDir -notlike "*Program Files (x86)*"

if ($is64bit) {
    Write-Host "  Architecture: 64-bit" -ForegroundColor Cyan
    $dllSource = "bin64\NppPluginTemplate.dll"
    $pluginSubDir = "SecretsManager"
} else {
    Write-Host "  Architecture: 32-bit" -ForegroundColor Cyan
    $dllSource = "bin\NppPluginTemplate.dll"
    $pluginSubDir = "SecretsManager"
}

# Check if source DLL exists
if (-not (Test-Path $dllSource)) {
    Write-Host ""
    Write-Host "❌ Plugin DLL not found: $dllSource" -ForegroundColor Red
    Write-Host ""
    if ($is64bit) {
        Write-Host "You need to build the 64-bit version:" -ForegroundColor Yellow
        Write-Host "  msbuild vs.proj\NppPluginTemplate.vcxproj /p:Configuration=Release /p:Platform=x64" -ForegroundColor White
    } else {
        Write-Host "You need to build the 32-bit version:" -ForegroundColor Yellow
        Write-Host "  msbuild vs.proj\NppPluginTemplate.vcxproj /p:Configuration=Release /p:Platform=Win32" -ForegroundColor White
    }
    exit 1
}

Write-Host "✓ Found plugin DLL: $dllSource" -ForegroundColor Green

# Verify DLL architecture
try {
    $dllInfo = dumpbin /headers $dllSource 2>&1 | Select-String "machine"
    if ($dllInfo -match "8664" -and $is64bit) {
        Write-Host "  ✓ DLL is 64-bit (matches Notepad++)" -ForegroundColor Green
    } elseif ($dllInfo -match "14C" -and -not $is64bit) {
        Write-Host "  ✓ DLL is 32-bit (matches Notepad++)" -ForegroundColor Green
    } else {
        Write-Host ""
        Write-Host "  ⚠ WARNING: Architecture mismatch!" -ForegroundColor Yellow
        Write-Host "    Notepad++: $($is64bit ? '64-bit' : '32-bit')" -ForegroundColor Yellow
        Write-Host "    Plugin DLL: Check the build configuration" -ForegroundColor Yellow
    }
} catch {
    Write-Host "  (Unable to verify DLL architecture)" -ForegroundColor Gray
}

# Check if Notepad++ is running
$nppRunning = Get-Process -Name notepad++ -ErrorAction SilentlyContinue
if ($nppRunning) {
    Write-Host ""
    Write-Host "⚠ Notepad++ is currently running!" -ForegroundColor Yellow
    Write-Host ""
    $response = Read-Host "Close Notepad++ and press ENTER to continue (or Ctrl+C to cancel)"
    
    # Wait for it to close
    Start-Sleep -Seconds 1
    $nppRunning = Get-Process -Name notepad++ -ErrorAction SilentlyContinue
    if ($nppRunning) {
        Write-Host "❌ Please close Notepad++ completely before installing." -ForegroundColor Red
        exit 1
    }
}

# Create plugin directory
$pluginDir = Join-Path (Split-Path $nppExe) "plugins\$pluginSubDir"
Write-Host ""
Write-Host "Installing to: $pluginDir" -ForegroundColor Cyan

if (-not (Test-Path $pluginDir)) {
    try {
        New-Item -ItemType Directory -Path $pluginDir -Force | Out-Null
        Write-Host "  ✓ Created plugin directory" -ForegroundColor Green
    } catch {
        Write-Host "  ❌ Failed to create directory: $_" -ForegroundColor Red
        Write-Host ""
        Write-Host "Try running as Administrator:" -ForegroundColor Yellow
        Write-Host "  Right-click PowerShell → Run as Administrator" -ForegroundColor White
        Write-Host "  Then run: .\install_smart.ps1" -ForegroundColor White
        exit 1
    }
}

# Copy DLL
try {
    $dllDest = Join-Path $pluginDir "SecretsManager.dll"
    Copy-Item $dllSource $dllDest -Force
    Write-Host "  ✓ Plugin DLL copied" -ForegroundColor Green
    Write-Host ""
    Write-Host "═══════════════════════════════════════" -ForegroundColor Green
    Write-Host "  ✅ INSTALLATION SUCCESSFUL!" -ForegroundColor Green
    Write-Host "═══════════════════════════════════════" -ForegroundColor Green
    Write-Host ""
    Write-Host "Next steps:" -ForegroundColor White
    Write-Host "  1. Start Notepad++" -ForegroundColor White
    Write-Host "  2. Plugins → Secrets Manager → Show Secrets Manager" -ForegroundColor White
    Write-Host "  3. Add your first secret!" -ForegroundColor White
    Write-Host ""
    Write-Host "Documentation: See QUICK_START.md" -ForegroundColor Gray
    Write-Host ""
} catch {
    Write-Host "  ❌ Installation failed: $_" -ForegroundColor Red
    Write-Host ""
    Write-Host "Try running as Administrator or manually copy:" -ForegroundColor Yellow
    Write-Host "  $dllSource" -ForegroundColor White
    Write-Host "  → $pluginDir\SecretsManager.dll" -ForegroundColor White
    exit 1
}
