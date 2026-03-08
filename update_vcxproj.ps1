$projectFile = "vs.proj\NppPluginTemplate.vcxproj"

$lines = Get-Content $projectFile

$newLines = @()

foreach ($line in $lines) {
    $newLines += $line
    
    if ($line -match '<ClInclude Include="\.\.\\src\\DockingFeature\\Window\.h" />') {
        $newLines += '    <ClInclude Include="..\src\DockingFeature\SecretsManagerDlg.h" />'
    }
    
    if ($line -match '<ClCompile Include="\.\.\\src\\DockingFeature\\StaticDialog\.cpp" />') {
        $newLines += '    <ClCompile Include="..\src\DockingFeature\SecretsManagerDlg.cpp" />'
    }
    
    if ($line -match '<ResourceCompile Include="\.\.\\src\\DockingFeature\\goLine\.rc" />') {
        $newLines += '    <ResourceCompile Include="..\src\DockingFeature\SecretsManager.rc" />'
    }
}

$newLines | Set-Content $projectFile -Encoding UTF8
Write-Host "Project file updated successfully!"
