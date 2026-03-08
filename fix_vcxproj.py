with open('vs.proj/NppPluginTemplate_original.vcxproj', 'r', encoding='utf-8') as f:
    content = f.read()

lines = content.splitlines()
new_lines = []

for i, line in enumerate(lines):
    if '    <ClInclude Include="..\src\DockingFeature\Window.h" />' in line and '`r`n' not in line:
        new_lines.append(line)
        new_lines.append('    <ClInclude Include="..\src\DockingFeature\SecretsManagerDlg.h" />')
    elif '    <ClCompile Include="..\src\DockingFeature\StaticDialog.cpp" />' in line and '`r`n' not in line:
        new_lines.append('    <ClCompile Include="..\src\DockingFeature\SecretsManagerDlg.cpp" />')
        new_lines.append(line)
    elif '    <ResourceCompile Include="..\src\DockingFeature\goLine.rc" />' in line and '`r`n' not in line:
        new_lines.append(line)
        new_lines.append('    <ResourceCompile Include="..\src\DockingFeature\SecretsManager.rc" />')
    elif '`r`n' not in line:
        new_lines.append(line)
    else:
        parts = line.split('`r`n')
        for part in parts:
            if '</' in part and part.strip().startswith('</'):
                new_lines.append(part)
            elif part.strip():
                elements = []
                temp = part
                while '<ClInclude' in temp or '<ClCompile' in temp or '<ResourceCompile' in temp:
                    if temp.count('/>') > 0:
                        idx = temp.find('/>')
                        if idx > 0:
                            elements.append(temp[:idx+2])
                            temp = temp[idx+2:]
                        else:
                            break
                    else:
                        break
                
                for elem in elements:
                    if elem.strip():
                        new_lines.append(elem)

with open('vs.proj/NppPluginTemplate.vcxproj', 'w', encoding='utf-8', newline='\r\n') as f:
    f.write('\r\n'.join(new_lines))

print("Fixed project file!")
