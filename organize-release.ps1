#!/usr/bin/env pwsh
<#
.SYNOPSIS
Quick release package organizer for existing build artifacts

.DESCRIPTION
Organizes pre-built artifacts into a proper release package structure

.PARAMETER SourceDir
Source directory containing built artifacts (default: ./bin/vs2010)

.PARAMETER OutputDir
Output directory for release package (default: ./releases)

.PARAMETER Configurations
Configurations to include (default: all found)

.EXAMPLE
.\organize-release.ps1 -SourceDir ./bin/vs2010 -OutputDir ./releases
#>

param(
    [string]$SourceDir = "./bin/vs2010",
    [string]$OutputDir = "./releases",
    [switch]$Verbose,
    [switch]$Help
)

if ($Help) {
    Get-Help $PSCommandPath -Detailed
    exit 0
}

$ErrorActionPreference = "Stop"
$ProgressPreference = "SilentlyContinue"

$REPO_ROOT = Get-Location
$TIMESTAMP = Get-Date -Format "yyyyMMdd-HHmmss"

function Write-Info($msg) { Write-Host "$(Get-Date -Format 'HH:mm:ss') [INFO]  $msg" -ForegroundColor Cyan }
function Write-Success($msg) { Write-Host "$(Get-Date -Format 'HH:mm:ss') [OK]    $msg" -ForegroundColor Green }
function Write-Warn($msg) { Write-Host "$(Get-Date -Format 'HH:mm:ss') [WARN]  $msg" -ForegroundColor Yellow }
function Write-Error($msg) { Write-Host "$(Get-Date -Format 'HH:mm:ss') [ERROR] $msg" -ForegroundColor Red }

Write-Info "========================================================================="
Write-Info "AsirikuyFramework Release Package Organizer"
Write-Info "========================================================================="

# Verify source directory
$SourcePath = Join-Path $REPO_ROOT $SourceDir
if (-not (Test-Path $SourcePath)) {
    Write-Error "Source directory not found: $SourcePath"
    exit 1
}

Write-Info "Source directory: $SourcePath"

# Create release package structure
$ReleaseName = "AsirikuyFramework-$TIMESTAMP"
$OutputPath = Join-Path $REPO_ROOT $OutputDir
$ReleasePath = Join-Path $OutputPath $ReleaseName

$subdirs = @("bin", "lib", "include", "docs")
foreach ($subdir in $subdirs) {
    New-Item -ItemType Directory -Path (Join-Path $ReleasePath $subdir) -Force | Out-Null
}

Write-Info "Created release structure: $ReleasePath"

# Copy DLLs
Write-Info "Copying DLLs..."
$dllCount = 0
$binPath = Join-Path $ReleasePath "bin"
$dlls = Get-ChildItem -Recurse -Path $SourcePath -File -Include "*.dll" -ErrorAction SilentlyContinue
foreach ($dll in $dlls) {
    Copy-Item $dll.FullName $binPath -Force
    Write-Info "  [bin] $($dll.Name) ($([math]::Round($dll.Length/1KB, 2)) KB)"
    $dllCount++
}
Write-Success "Copied $dllCount DLL(s)"

# Copy Libraries
Write-Info "Copying libraries..."
$libCount = 0
$libPath = Join-Path $ReleasePath "lib"
$libs = Get-ChildItem -Recurse -Path $SourcePath -File -Include "*.lib" -ErrorAction SilentlyContinue
foreach ($lib in $libs) {
    Copy-Item $lib.FullName $libPath -Force
    Write-Info "  [lib] $($lib.Name) ($([math]::Round($lib.Length/1KB, 2)) KB)"
    $libCount++
}
Write-Success "Copied $libCount LIB(s)"

# Copy headers
Write-Info "Copying headers..."
$headerCount = 0
$includePath = Join-Path $ReleasePath "include"
if (Test-Path "core") {
    $includes = Get-ChildItem -Recurse -Path "core" -Directory -Filter "include" -ErrorAction SilentlyContinue
    foreach ($include in $includes) {
        $parentName = $include.Parent.Name
        $targetPath = Join-Path $includePath $parentName
        Copy-Item $include.FullName $targetPath -Recurse -Force -ErrorAction SilentlyContinue
        $headerCount += (Get-ChildItem -Recurse -Path $targetPath -File | Measure-Object).Count
        Write-Info "  [include] $parentName ($($headerCount) files)"
    }
}
Write-Success "Copied $headerCount header file(s)"

# Copy documentation
Write-Info "Copying documentation..."
$docCount = 0
$docsPath = Join-Path $ReleasePath "docs"
$docs = Get-ChildItem -Path "docs" -File -Include "*.md" -ErrorAction SilentlyContinue
foreach ($doc in $docs) {
    Copy-Item $doc.FullName $docsPath -Force
    Write-Info "  [docs] $($doc.Name)"
    $docCount++
}
Write-Success "Copied $docCount documentation file(s)"

# Create manifest
Write-Info "Creating release manifest..."

$manifest = @{
    Release = $ReleaseName
    Generated = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    Platform = "Windows"
    Architecture = "x64"
    BuildType = "Release"
    Artifacts = @{
        DLLs = $dllCount
        Libraries = $libCount
        Headers = $headerCount
        Documentation = $docCount
    }
} | ConvertTo-Json -Depth 3

$manifest | Set-Content (Join-Path $ReleasePath "MANIFEST.json")

# Create README
@"
# AsirikuyFramework Release Package

**Release:** $ReleaseName  
**Generated:** $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')  
**Platform:** Windows x64  

## Contents

### Executables & Libraries

- **bin/** - DLL files (shared libraries)
  - AsirikuyFrameworkAPI.dll - Main framework DLL (37 exported functions)
  - Other framework DLLs

- **lib/** - LIB files (import/static libraries)
  - AsirikuyFrameworkAPI.lib - Import library for linking
  - trading_strategies.lib - Strategy engine
  - Other support libraries

### Headers

- **include/** - C/C++ header files organized by module
  - AsirikuyFrameworkAPI/ - Main framework headers
  - TradingStrategies/ - Strategy engine headers
  - AsirikuyCommon/ - Common utilities
  - And others...

### Documentation

- **docs/** - Markdown documentation files
  - API guides
  - Usage examples
  - Architecture documentation

## Integration Instructions

### 1. For MT4 Integration

Copy `AsirikuyFrameworkAPI.dll` to MT4's Experts folder:
```
C:\Program Files\MetaTrader 4\experts\
```

Create an Expert Advisor that imports and calls:
- `initInstanceMQL4()` - Initialize with MT4 context
- `mql4_runStrategy()` - Execute strategy
- `deinitInstance()` - Cleanup

### 2. For C/C++ Development

Link against the import library:
```
lib/AsirikuyFrameworkAPI.lib
```

Include headers from:
```
include/AsirikuyFrameworkAPI/
```

### 3. Exported Functions (37 total)

Key functions:
- Core: initInstanceC, initInstanceMQL4, initInstanceMQL5, deinitInstance
- Execution: c_runStrategy, mql4_runStrategy, mql5_runStrategy, jf_runStrategy
- Utilities: Symbol parsing, currency conversion, info queries

Full function list available in API documentation.

## Build Information

- **Compiler:** MSVC (Visual Studio 2022)
- **Target:** x64 PE32+
- **Size:** AsirikuyFrameworkAPI.dll is approximately 681 KB
- **Dependencies:** Windows SDK, no external runtime dependencies
- **Note:** Windows stubs provided for NTP and EasyTrade functionality

## License

See individual source files for licensing information.

## Support

For issues or questions, contact the development team.
"@ | Set-Content (Join-Path $ReleasePath "README.txt")

Write-Success "Created README.txt and MANIFEST.json"

# Create archive
Write-Info "Creating release archive..."
$OutputPath = Join-Path $REPO_ROOT $OutputDir
$archivePath = Join-Path $OutputPath "${ReleaseName}.zip"

if (Get-Command Compress-Archive -ErrorAction SilentlyContinue) {
    Push-Location $OutputPath
    Compress-Archive -Path $ReleaseName -DestinationPath "${ReleaseName}.zip" -Force
    Pop-Location
    Write-Success "Created archive: $archivePath"
} else {
    Write-Warn "Compress-Archive not available, skipping archive creation"
}

# Summary
Write-Info ""
Write-Info "========================================================================="
Write-Info "Release Package Summary"
Write-Info "========================================================================="
Write-Success "Release Name: $ReleaseName"
Write-Success "Location: $ReleasePath"
Write-Success "DLLs: $dllCount"
Write-Success "Libraries: $libCount"
Write-Success "Headers: $headerCount header files"
Write-Success "Docs: $docCount markdown files"
Write-Info ""
Write-Success "Release package created successfully!"
Write-Info "Package is ready for distribution."

exit 0
