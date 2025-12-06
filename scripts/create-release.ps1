#!/usr/bin/env pwsh
<#
.SYNOPSIS
Create a release package from built artifacts

.DESCRIPTION
Packages build artifacts (DLLs, LIBs, headers, docs) into an organized release folder structure.

.PARAMETER OutputDir
Output directory for release packages (default: ./releases)

.PARAMETER IncludeDocs
Include documentation files in the release

.PARAMETER IncludeHeaders
Include header files in the release

.PARAMETER IncludeLibs
Include library files in the release

.EXAMPLE
# Create release with all components
.\create-release.ps1

# Create release in custom location
.\create-release.ps1 -OutputDir D:\artifacts

# Create minimal release (binaries only)
.\create-release.ps1 -IncludeDocs:$false -IncludeHeaders:$false
#>

param(
    [string]$OutputDir = "./releases",
    [switch]$IncludeDocs = $true,
    [switch]$IncludeHeaders = $true,
    [switch]$IncludeLibs = $true,
    [switch]$Help
)

# ============================================================================
# UTILITY FUNCTIONS
# ============================================================================

function Write-Status($msg) { 
    Write-Host "$(Get-Date -Format 'HH:mm:ss') [INFO]  $msg" -ForegroundColor Cyan 
}

function Write-Success($msg) { 
    Write-Host "$(Get-Date -Format 'HH:mm:ss') [OK]    $msg" -ForegroundColor Green 
}

function Write-Warn($msg) { 
    Write-Host "$(Get-Date -Format 'HH:mm:ss') [WARN]  $msg" -ForegroundColor Yellow 
}

function Write-Error($msg) { 
    Write-Host "$(Get-Date -Format 'HH:mm:ss') [ERROR] $msg" -ForegroundColor Red 
}

function Show-Help {
    Get-Help $PSCommandPath -Detailed
}

# ============================================================================
# MAIN SCRIPT
# ============================================================================

if ($Help) {
    Show-Help
    exit 0
}

# Get repository root
$repoRoot = Split-Path $PSScriptRoot -Parent

# Make OutputDir absolute if it's relative
if (-not [System.IO.Path]::IsPathRooted($OutputDir)) {
    $OutputDir = Join-Path $repoRoot $OutputDir
}

$binDir = Join-Path (Join-Path (Join-Path (Join-Path $repoRoot "bin") "vs2010") "x64") "Release"
$includeDir = Join-Path $repoRoot "include"
$docsDir = Join-Path $repoRoot "docs"

# If no include dir at root, look for an existing release package with headers
if (-not (Test-Path $includeDir)) {
    $existingReleases = Get-ChildItem (Join-Path $repoRoot "releases") -Directory -ErrorAction SilentlyContinue | 
                        Sort-Object Name
    
    # Find the first release with an include directory that has content
    foreach ($rel in $existingReleases) {
        $srcIncludeDir = Join-Path $rel.FullName "include"
        if ((Test-Path $srcIncludeDir) -and ((Get-ChildItem $srcIncludeDir -Recurse -Filter "*.h" | Measure-Object).Count -gt 0)) {
            $includeDir = $srcIncludeDir
            break
        }
    }
}

Write-Status "Release Package Generator"
Write-Status "=================================="
Write-Status "Repository Root: $repoRoot"
Write-Status "Build Output Dir: $binDir"
Write-Status "Release Output Dir: $OutputDir"
Write-Host ""

# Verify build output exists
if (-not (Test-Path $binDir)) {
    Write-Error "Build directory not found: $binDir"
    Write-Warn "Run build-parallel-simple.bat or build-sequential.bat first"
    exit 1
}

# Create release directory with timestamp
$releaseName = "AsirikuyFramework-$(Get-Date -Format 'yyyyMMdd-HHmmss')"
$releaseDir = Join-Path $OutputDir $releaseName

Write-Status "Creating release: $releaseName"

# Create directory structure
$binReleaseDir = Join-Path $releaseDir "bin"
$libReleaseDir = Join-Path $releaseDir "lib"
$includeReleaseDir = Join-Path $releaseDir "include"
$docsReleaseDir = Join-Path $releaseDir "docs"

@($binReleaseDir, $libReleaseDir, $includeReleaseDir, $docsReleaseDir) | ForEach-Object {
    if (-not (Test-Path $_)) {
        New-Item -ItemType Directory -Path $_ -Force | Out-Null
    }
}

Write-Status "Directory structure created"

# Copy binaries (DLLs, EXEs)
$dllCount = 0
$exeCount = 0

if (Test-Path $binDir) {
    Get-ChildItem $binDir -Filter "*.dll" | ForEach-Object {
        Copy-Item -Path $_.FullName -Destination $binReleaseDir -Force
        $dllCount++
        Write-Warn "  Copied DLL: $($_.Name)"
    }

    Get-ChildItem $binDir -Filter "*.exe" | ForEach-Object {
        Copy-Item -Path $_.FullName -Destination $binReleaseDir -Force
        $exeCount++
        Write-Warn "  Copied EXE: $($_.Name)"
    }
}

Write-Success "Copied $dllCount DLLs and $exeCount EXEs"

# Copy libraries (LIBs)
$libCount = 0
if ($IncludeLibs -and (Test-Path $binDir)) {
    Get-ChildItem $binDir -Filter "*.lib" | ForEach-Object {
        Copy-Item -Path $_.FullName -Destination $libReleaseDir -Force
        $libCount++
    }
    Write-Success "Copied $libCount libraries"
}

# Copy headers
$headerCount = 0
if ($IncludeHeaders -and (Test-Path $includeDir)) {
    # Copy the entire include directory structure
    Get-ChildItem $includeDir -Directory | ForEach-Object {
        Copy-Item -Path $_.FullName -Destination $includeReleaseDir -Recurse -Force -ErrorAction SilentlyContinue
    }
    $headerCount = (Get-ChildItem $includeReleaseDir -Recurse -Filter "*.h*" | Measure-Object).Count
    if ($headerCount -gt 0) {
        Write-Success "Copied $headerCount header files"
    } else {
        Write-Warn "No headers found in $includeDir"
    }
}

# Copy documentation
$docCount = 0
if ($IncludeDocs -and (Test-Path $docsDir)) {
    Get-ChildItem $docsDir -Filter "*.md" | ForEach-Object {
        Copy-Item -Path $_.FullName -Destination $docsReleaseDir -Force
        $docCount++
    }
    Write-Success "Copied $docCount documentation files"
}

# Create manifest
$manifest = @{
    "Release" = $releaseName
    "Generated" = (Get-Date -Format "yyyy-MM-dd HH:mm:ss")
    "Platform" = "Windows"
    "Architecture" = "x64"
    "BuildType" = "Release"
    "Artifacts" = @{
        "DLLs" = $dllCount
        "EXEs" = $exeCount
        "Libraries" = $libCount
        "Headers" = $headerCount
        "Documentation" = $docCount
    }
} | ConvertTo-Json

$manifest | Set-Content (Join-Path $releaseDir "MANIFEST.json")
Write-Success "Created MANIFEST.json"

# Create README
$readme = @"
# $releaseName

Release package generated on $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")

## Contents

- **bin/** - DLLs and EXEs
- **lib/** - Static libraries (.lib files)
- **include/** - Header files
- **docs/** - Documentation
- **MANIFEST.json** - Release metadata

## Build Information

- Architecture: x64
- Platform: Windows
- Configuration: Release
- Toolset: VS2022 (v143)

## Usage

1. Copy DLLs from `bin/` to your MT4/MT5 installation folder
2. Link against libraries in `lib/` when building
3. Include headers from `include/` in your projects

See individual headers and documentation for API details.
"@

$readme | Set-Content (Join-Path $releaseDir "README.txt")
Write-Success "Created README.txt"

# Create ZIP archive
Write-Status "Creating ZIP archive..."

$zipPath = Join-Path $OutputDir "$releaseName.zip"

# Compress the release directory
try {
    # PowerShell 5+ uses Compress-Archive
    if (Get-Command Compress-Archive -ErrorAction SilentlyContinue) {
        Compress-Archive -Path $releaseDir -DestinationPath $zipPath -Force
        Write-Success "ZIP archive created: $(Split-Path $zipPath -Leaf)"
    } else {
        Write-Warn "Compress-Archive not available, skipping ZIP creation"
    }
} catch {
    Write-Warn "Failed to create ZIP archive: $_"
}

# Summary
Write-Host ""
Write-Success "Release package created successfully!"
Write-Host ""
Write-Status "Summary:"
Write-Host "  Release Name: $releaseName"
Write-Host "  Output Dir: $(Resolve-Path $releaseDir)"
Write-Host "  Archive: $(if (Test-Path $zipPath) { "$(Split-Path $zipPath -Leaf)" } else { "Not created" })"
Write-Host "  DLLs: $dllCount"
Write-Host "  EXEs: $exeCount"
Write-Host "  Libraries: $libCount"
Write-Host "  Headers: $headerCount"
Write-Host "  Docs: $docCount"
Write-Host ""
Write-Status "To use this release:"
Write-Host "  1. Extract the ZIP file: $releaseName.zip"
Write-Host "  2. Copy DLLs from bin/ to your MT4/MT5 folder"
Write-Host "  3. Reference headers from include/ in your projects"
Write-Host ""
