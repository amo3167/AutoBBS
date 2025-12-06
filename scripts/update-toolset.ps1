#!/usr/bin/env pwsh
# Update all .vcxproj files to use VS2022 (v143) toolset

$projDir = "build\vs2010\projects"
$count = 0

Get-ChildItem -Path $projDir -Filter "*.vcxproj" | ForEach-Object {
    $path = $_.FullName
    [xml]$xml = Get-Content $path
    $ns = @{ms='http://schemas.microsoft.com/developer/msbuild/2003'}
    $configs = Select-Xml -Xml $xml -XPath '//ms:PropertyGroup[@Label="Configuration"]' -Namespace $ns
    
    foreach ($config in $configs) {
        if (-not $config.Node.PlatformToolset) {
            $toolset = $xml.CreateElement('PlatformToolset', $xml.DocumentElement.NamespaceURI)
            $toolset.InnerText = 'v143'
            $config.Node.AppendChild($toolset) | Out-Null
            $count++
        }
    }
    
    $xml.Save($path)
}

Write-Host "Updated $count projects with v143 toolset"
