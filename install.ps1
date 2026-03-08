# Secrets Manager Plugin Installer for Notepad++

Write-Host "======================================" -ForegroundColor Cyan
Write-Host "Secrets Manager Plugin Installer" -ForegroundColor Cyan
Write-Host "======================================" -ForegroundColor Cyan
Write-Host ""

# Find Notepad++ installation
$nppPaths = @(
    "C:\Program Files\Notepad++",
    "C:\Program Files (x86)\Notepad++",
    "$env:ProgramFiles\Notepad++",
    "${env:ProgramFiles(x86)}\Notepad++"
)

$nppPath = $null
foreach ($path in $nppPaths) {
    if (Test-Path "$path\notepad++.exe") {
        $nppPath = $path
        break
    }
}

if (-not $nppPath) {
    Write-Host "❌ Notepad++ installation not found!" -ForegroundColor Red
    Write-Host ""
    Write-Host "Please install Notepad++ or specify the installation path manually." -ForegroundColor Yellow
    exit 1
}

Write-Host "✓ Found Notepad++ at: $nppPath" -ForegroundColor Green

# Check if DLL exists
$dllSource = "bin64\NppPluginTemplate.dll"
if (-not (Test-Path $dllSource)) {
    Write-Host "❌ Plugin DLL not found at: $dllSource" -ForegroundColor Red
    Write-Host "Please build the project first using:" -ForegroundColor Yellow
    Write-Host "  msbuild vs.proj\NppPluginTemplate.vcxproj /p:Configuration=Release /p:Platform=x64" -ForegroundColor Yellow
    exit 1
}

Write-Host "✓ Found plugin DLL: $dllSource" -ForegroundColor Green

# Create plugin directory
$pluginDir = Join-Path $nppPath "plugins\SecretsManager"
if (-not (Test-Path $pluginDir)) {
    Write-Host "Creating plugin directory: $pluginDir" -ForegroundColor Cyan
    New-Item -ItemType Directory -Path $pluginDir -Force | Out-Null
}

Write-Host "✓ Plugin directory ready: $pluginDir" -ForegroundColor Green

# Check if Notepad++ is running
$nppProcess = Get-Process notepad++ -ErrorAction SilentlyContinue
if ($nppProcess) {
    Write-Host ""
    Write-Host "⚠ WARNING: Notepad++ is currently running!" -ForegroundColor Yellow
    Write-Host "Please close Notepad++ before continuing." -ForegroundColor Yellow
    Write-Host ""
    $response = Read-Host "Press ENTER to continue after closing Notepad++, or Ctrl+C to cancel"
}

# Copy DLL
try {
    $dllDest = Join-Path $pluginDir "SecretsManager.dll"
    Copy-Item $dllSource $dllDest -Force
    Write-Host "✓ Plugin installed successfully!" -ForegroundColor Green
    Write-Host ""
    Write-Host "Installation complete!" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "Next steps:" -ForegroundColor White
    Write-Host "1. Start Notepad++" -ForegroundColor White
    Write-Host "2. Go to Plugins → Secrets Manager → Show Secrets Manager" -ForegroundColor White
    Write-Host "3. Start managing your secrets securely!" -ForegroundColor White
    Write-Host ""
    Write-Host "For usage guide, see QUICK_START.md" -ForegroundColor Gray
}
catch {
    Write-Host "❌ Installation failed: $_" -ForegroundColor Red
    Write-Host ""
    Write-Host "You may need to run this script as Administrator." -ForegroundColor Yellow
    exit 1
}
