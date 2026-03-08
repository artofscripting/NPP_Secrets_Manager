import re

# Read the original corrupted file  
with open('vs.proj/NppPluginTemplate_original.vcxproj', 'r', encoding='utf-8-sig') as f:
    content = f.read()

# Fix all the inline elements by splitting them properly
content = content.replace('`r`n', '\n')

# Split elements that are combined on one line
def split_xml_elements(match):
    line = match.group(0)
    parts = []
    current = ""
    depth = 0
    
    for char in line:
        current += char
        if char == '<':
            if current.strip() and not current.strip().startswith('<'):
                parts.append(current[:-1])
                current = '<'
            depth += 1
        elif char == '>' and '/>' in current:
            parts.append(current)
            current = ""
            depth = 0
    
    if current.strip():
        parts.append(current)
    
    # Add proper indentation
    result = []
    for part in parts:
        part = part.strip()
        if part:
            if part.startswith('</'):
                result.append('  ' + part)
            elif 'Include=' in part:
                result.append('    ' + part)
            else:
                result.append(part)
    
    return '\n'.join(result)

# Find and fix lines with multiple XML elements
lines = content.split('\n')
fixed_lines = []

for line in lines:
    # Count XML tags in line
    open_tags = line.count('<Cl')
    if open_tags > 1:
        # Multiple elements on one line - need to split
        indent = len(line) - len(line.lstrip())
        elements = []
        temp = line.strip()
        
        while temp:
            # Find complete XML element
            if temp.startswith('<'):
                end_idx = temp.find('/>')
                if end_idx > 0:
                    elements.append(' ' * indent + temp[:end_idx+2])
                    temp = temp[end_idx+2:].strip()
                else:
                    elements.append(' ' * indent + temp)
                    break
            else:
                temp = temp[1:]
        
        fixed_lines.extend(elements)
    else:
        fixed_lines.append(line)

# Add the new files in proper locations
output_lines = []
for i, line in enumerate(fixed_lines):
    output_lines.append(line)
    
    # Add SecretsManagerDlg.h after GoToLineDlg.h
    if 'GoToLineDlg.h' in line and 'ClInclude' in line:
        output_lines.append('    <ClInclude Include="..\src\DockingFeature\SecretsManagerDlg.h" />')
    
    # Add SecretsManagerDlg.cpp after GoToLineDlg.cpp
    if 'GoToLineDlg.cpp' in line and 'ClCompile' in line:
        output_lines.append('    <ClCompile Include="..\src\DockingFeature\SecretsManagerDlg.cpp" />')
    
    # Add SecretsManager.rc after goLine.rc
    if 'goLine.rc' in line and 'ResourceCompile' in line:
        output_lines.append('    <ResourceCompile Include="..\src\DockingFeature\SecretsManager.rc" />')

# Write the fixed file
with open('vs.proj/NppPluginTemplate.vcxproj', 'w', encoding='utf-8-sig', newline='\r\n') as f:
    f.write('\r\n'.join(output_lines))

print("Successfully fixed and updated project file!")
