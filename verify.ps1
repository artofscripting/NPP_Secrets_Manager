# Secrets Manager Plugin - Verification Script

Write-Host ""
Write-Host "═══════════════════════════════════════════════════" -ForegroundColor Cyan
Write-Host "    SECRETS MANAGER PLUGIN - VERIFICATION" -ForegroundColor Cyan
Write-Host "═══════════════════════════════════════════════════" -ForegroundColor Cyan
Write-Host ""

$allGood = $true

# Check DLL
Write-Host "Checking build output..." -ForegroundColor Yellow
if (Test-Path "bin64\NppPluginTemplate.dll") {
    $dll = Get-Item "bin64\NppPluginTemplate.dll"
    Write-Host "  ✓ Plugin DLL exists: $($dll.Length) bytes" -ForegroundColor Green
    Write-Host "    Last built: $($dll.LastWriteTime)" -ForegroundColor Gray
} else {
    Write-Host "  ✗ Plugin DLL not found!" -ForegroundColor Red
    $allGood = $false
}

Write-Host ""
Write-Host "Checking source files..." -ForegroundColor Yellow

$requiredFiles = @(
    "src\PluginDefinition.h",
    "src\PluginDefinition.cpp",
    "src\DockingFeature\SecretsManagerDlg.h",
    "src\DockingFeature\SecretsManagerDlg.cpp",
    "src\DockingFeature\SecretsManager.rc",
    "src\DockingFeature\resource.h"
)

foreach ($file in $requiredFiles) {
    if (Test-Path $file) {
        Write-Host "  ✓ $file" -ForegroundColor Green
    } else {
        Write-Host "  ✗ $file (MISSING)" -ForegroundColor Red
        $allGood = $false
    }
}

Write-Host ""
Write-Host "Checking documentation..." -ForegroundColor Yellow

$docs = @("README.md", "QUICK_START.md", "BUILD_SUMMARY.md", "OVERVIEW.md")
foreach ($doc in $docs) {
    if (Test-Path $doc) {
        Write-Host "  ✓ $doc" -ForegroundColor Green
    } else {
        Write-Host "  ✗ $doc (MISSING)" -ForegroundColor Red
        $allGood = $false
    }
}

Write-Host ""
Write-Host "Checking project file..." -ForegroundColor Yellow
if (Test-Path "vs.proj\NppPluginTemplate.vcxproj") {
    try {
        [xml]$proj = Get-Content "vs.proj\NppPluginTemplate.vcxproj"
        Write-Host "  ✓ Project file is valid XML" -ForegroundColor Green

        $hasSecretsManagerH = $proj.Project.ItemGroup.ClInclude | Where-Object { $_.Include -like "*SecretsManagerDlg.h" }
        $hasSecretsManagerCpp = $proj.Project.ItemGroup.ClCompile | Where-Object { $_.Include -like "*SecretsManagerDlg.cpp" }
        
        if ($hasSecretsManagerH) {
            Write-Host "  ✓ SecretsManagerDlg.h included in project" -ForegroundColor Green
        } else {
            Write-Host "  ✗ SecretsManagerDlg.h NOT in project" -ForegroundColor Red
            $allGood = $false
        }
        
        if ($hasSecretsManagerCpp) {
            Write-Host "  ✓ SecretsManagerDlg.cpp included in project" -ForegroundColor Green
        } else {
            Write-Host "  ✗ SecretsManagerDlg.cpp NOT in project" -ForegroundColor Red
            $allGood = $false
        }
    } catch {
        Write-Host "  ✗ Project file is invalid: $_" -ForegroundColor Red
        $allGood = $false
    }
} else {
    Write-Host "  ✗ Project file not found!" -ForegroundColor Red
    $allGood = $false
}

Write-Host ""
Write-Host "═══════════════════════════════════════════════════" -ForegroundColor Cyan
if ($allGood) {
    Write-Host "    ✅ ALL CHECKS PASSED!" -ForegroundColor Green
    Write-Host "═══════════════════════════════════════════════════" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "Your Secrets Manager plugin is ready!" -ForegroundColor White
    Write-Host ""
    Write-Host "To install:" -ForegroundColor Yellow
    Write-Host "  .\install.ps1" -ForegroundColor White
    Write-Host ""
    Write-Host "Or manually copy:" -ForegroundColor Yellow
    Write-Host "  bin64\NppPluginTemplate.dll" -ForegroundColor White
    Write-Host "  → C:\Program Files\Notepad++\plugins\SecretsManager\SecretsManager.dll" -ForegroundColor White
} else {
    Write-Host "    ⚠ SOME CHECKS FAILED" -ForegroundColor Red
    Write-Host "═══════════════════════════════════════════════════" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "Please review the errors above." -ForegroundColor Yellow
}
Write-Host ""
