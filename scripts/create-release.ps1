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

$binDirX64 = Join-Path (Join-Path (Join-Path (Join-Path $repoRoot "bin") "vs2010") "x64") "Release"
$binDirX32 = Join-Path (Join-Path (Join-Path (Join-Path $repoRoot "bin") "vs2010") "x32") "Release"
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
Write-Status "Build Output x64: $binDirX64"
Write-Status "Build Output x32: $binDirX32"
Write-Status "Release Output Dir: $OutputDir"
Write-Host ""

# Verify build output exists
$hasX64 = Test-Path $binDirX64
$hasX32 = Test-Path $binDirX32

if (-not $hasX64 -and -not $hasX32) {
    Write-Error "Build directories not found: $binDirX64 and $binDirX32"
    Write-Warn "Run build-parallel-simple.bat or build-sequential.bat first"
    exit 1
}

if (-not $hasX64) {
    Write-Warn "x64 build directory not found: $binDirX64"
}
if (-not $hasX32) {
    Write-Warn "x32 build directory not found: $binDirX32"
}

# Create release directory with timestamp
$releaseName = "AsirikuyFramework-$(Get-Date -Format 'yyyyMMdd-HHmmss')"
$releaseDir = Join-Path $OutputDir $releaseName

Write-Status "Creating release: $releaseName"

# Create directory structure
$binReleaseX64Dir = Join-Path (Join-Path $releaseDir "bin") "x64"
$binReleaseX32Dir = Join-Path (Join-Path $releaseDir "bin") "x32"
$libReleaseX64Dir = Join-Path (Join-Path $releaseDir "lib") "x64"
$libReleaseX32Dir = Join-Path (Join-Path $releaseDir "lib") "x32"
$includeReleaseDir = Join-Path $releaseDir "include"
$docsReleaseDir = Join-Path $releaseDir "docs"

@($binReleaseX64Dir, $binReleaseX32Dir, $libReleaseX64Dir, $libReleaseX32Dir, $includeReleaseDir, $docsReleaseDir) | ForEach-Object {
    if (-not (Test-Path $_)) {
        New-Item -ItemType Directory -Path $_ -Force | Out-Null
    }
}

Write-Status "Directory structure created"

# Copy binaries (DLLs, EXEs) - x64
$dllCountX64 = 0
$exeCountX64 = 0

if (Test-Path $binDirX64) {
    Write-Status "Copying x64 binaries..."
    Get-ChildItem $binDirX64 -Filter "*.dll" | ForEach-Object {
        Copy-Item -Path $_.FullName -Destination $binReleaseX64Dir -Force
        $dllCountX64++
        Write-Warn "  [x64] Copied DLL: $($_.Name)"
    }

    Get-ChildItem $binDirX64 -Filter "*.exe" | ForEach-Object {
        Copy-Item -Path $_.FullName -Destination $binReleaseX64Dir -Force
        $exeCountX64++
        Write-Warn "  [x64] Copied EXE: $($_.Name)"
    }
}

# Copy binaries (DLLs, EXEs) - x32
$dllCountX32 = 0
$exeCountX32 = 0

if (Test-Path $binDirX32) {
    Write-Status "Copying x32 binaries..."
    Get-ChildItem $binDirX32 -Filter "*.dll" | ForEach-Object {
        Copy-Item -Path $_.FullName -Destination $binReleaseX32Dir -Force
        $dllCountX32++
        Write-Warn "  [x32] Copied DLL: $($_.Name)"
    }

    Get-ChildItem $binDirX32 -Filter "*.exe" | ForEach-Object {
        Copy-Item -Path $_.FullName -Destination $binReleaseX32Dir -Force
        $exeCountX32++
        Write-Warn "  [x32] Copied EXE: $($_.Name)"
    }
}

Write-Success "Copied $dllCountX64 x64 DLLs and $exeCountX64 x64 EXEs"
Write-Success "Copied $dllCountX32 x32 DLLs and $exeCountX32 x32 EXEs"

# Copy libraries (LIBs) - x64
$libCountX64 = 0
if ($IncludeLibs -and (Test-Path $binDirX64)) {
    Get-ChildItem $binDirX64 -Filter "*.lib" | ForEach-Object {
        Copy-Item -Path $_.FullName -Destination $libReleaseX64Dir -Force
        $libCountX64++
    }
    
    # Also copy from lib subdirectory if it exists
    $libSubDirX64 = Join-Path $binDirX64 "lib"
    if (Test-Path $libSubDirX64) {
        Get-ChildItem $libSubDirX64 -Filter "*.lib" | ForEach-Object {
            Copy-Item -Path $_.FullName -Destination $libReleaseX64Dir -Force
            $libCountX64++
        }
    }
    Write-Success "Copied $libCountX64 x64 libraries"
}

# Copy libraries (LIBs) - x32
$libCountX32 = 0
if ($IncludeLibs -and (Test-Path $binDirX32)) {
    Get-ChildItem $binDirX32 -Filter "*.lib" | ForEach-Object {
        Copy-Item -Path $_.FullName -Destination $libReleaseX32Dir -Force
        $libCountX32++
    }
    
    # Also copy from lib subdirectory if it exists
    $libSubDirX32 = Join-Path $binDirX32 "lib"
    if (Test-Path $libSubDirX32) {
        Get-ChildItem $libSubDirX32 -Filter "*.lib" | ForEach-Object {
            Copy-Item -Path $_.FullName -Destination $libReleaseX32Dir -Force
            $libCountX32++
        }
    }
    Write-Success "Copied $libCountX32 x32 libraries"
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
    "Architectures" = @("x64", "x32")
    "BuildType" = "Release"
    "Artifacts" = @{
        "x64" = @{
            "DLLs" = $dllCountX64
            "EXEs" = $exeCountX64
            "Libraries" = $libCountX64
        }
        "x32" = @{
            "DLLs" = $dllCountX32
            "EXEs" = $exeCountX32
            "Libraries" = $libCountX32
        }
        "Headers" = $headerCount
        "Documentation" = $docCount
    }
} | ConvertTo-Json -Depth 5

$manifest | Set-Content (Join-Path $releaseDir "MANIFEST.json")
Write-Success "Created MANIFEST.json"

# Create README
$readme = @"
# $releaseName

Release package generated on $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")

## Contents

- **bin/x64/** - x64 DLLs and EXEs
- **bin/x32/** - x32 DLLs and EXEs
- **lib/x64/** - x64 Static libraries (.lib files)
- **lib/x32/** - x32 Static libraries (.lib files)
- **include/** - Header files
- **docs/** - Documentation
- **MANIFEST.json** - Release metadata

## Build Information

- Architectures: x64 (64-bit), x32 (32-bit)
- Platform: Windows
- Configuration: Release
- Toolset: VS2022 (v143)

## Usage

1. Copy DLLs from `bin/x64/` or `bin/x32/` to your MT4/MT5 installation folder (match the architecture)
2. Link against libraries in `lib/x64/` or `lib/x32/` when building (match your target architecture)
3. Include headers from `include/` in your projects

### For 32-bit MT4
Use files from `bin/x32/` and `lib/x32/`

### For 64-bit MT4/MT5
Use files from `bin/x64/` and `lib/x64/`

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
Write-Host ""
Write-Host "  x64 Artifacts:"
Write-Host "    DLLs: $dllCountX64"
Write-Host "    EXEs: $exeCountX64"
Write-Host "    Libraries: $libCountX64"
Write-Host ""
Write-Host "  x32 Artifacts:"
Write-Host "    DLLs: $dllCountX32"
Write-Host "    EXEs: $exeCountX32"
Write-Host "    Libraries: $libCountX32"
Write-Host ""
Write-Host "  Shared:"
Write-Host "    Headers: $headerCount"
Write-Host "    Docs: $docCount"
Write-Host ""
Write-Status "To use this release:"
Write-Host "  1. Extract the ZIP file: $releaseName.zip"
Write-Host "  2. For 32-bit MT4: Copy DLLs from bin/x32/ to your MT4 folder"
Write-Host "  3. For 64-bit MT4/MT5: Copy DLLs from bin/x64/ to your MT4/MT5 folder"
Write-Host "  4. Reference headers from include/ in your projects"
Write-Host ""
