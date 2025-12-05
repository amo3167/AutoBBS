#!/usr/bin/env pwsh
<#
.SYNOPSIS
Comprehensive build and release script for AsirikuyFramework and related C/C++ projects

.DESCRIPTION
Builds libraries, DLLs, and applications for Windows platforms (x86/x64, Debug/Release).
Generates organized release packages with proper directory structure.

.PARAMETER Config
Build configuration: debug32, debug64, release32, release64 (default: release64)

.PARAMETER Project
Project to build: AsirikuyFrameworkAPI, TradingStrategies, CTesterFrameworkAPI, UnitTests, all (default: all)

.PARAMETER BuildType
Type of build: Debug, Release, All (default: All)

.PARAMETER Platform
Platform: x32, x64, all (default: all)

.PARAMETER Clean
Clean build directories before building

.PARAMETER Release
Create release package with proper folder structure

.PARAMETER OutputDir
Custom output directory for release artifacts (default: ./releases)

.PARAMETER NoVendor
Skip building vendor libraries

.PARAMETER Verbose
Verbose output

.EXAMPLE
# Full release build
.\build-release.ps1 -Config release64 -Release -OutputDir D:\artifacts

# Clean debug build with verbose output
.\build-release.ps1 -Config debug64 -Clean -Verbose

# Build all configurations
.\build-release.ps1 -BuildType All -Platform all -Release
#>

param(
    [ValidateSet("debug32", "debug64", "release32", "release64")]
    [string]$Config = "release64",
    
    [ValidateSet("AsirikuyFrameworkAPI", "TradingStrategies", "CTesterFrameworkAPI", "UnitTests", "all")]
    [string]$Project = "all",
    
    [ValidateSet("Debug", "Release", "All")]
    [string]$BuildType = "All",
    
    [ValidateSet("x32", "x64", "all")]
    [string]$Platform = "all",
    
    [switch]$Clean,
    [switch]$CleanAfter,
    [switch]$Release,
    [string]$OutputDir = "./releases",
    [switch]$NoVendor,
    [switch]$Verbose,
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

function Find-MSBuild {
    $paths = @(
        "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe",
        "C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe",
        "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe",
        "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe",
        "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin\MSBuild.exe",
        "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\MSBuild\Current\Bin\MSBuild.exe"
    )
    
    foreach ($path in $paths) {
        if (Test-Path $path) {
            return $path
        }
    }
    
    # Try to find via registry
    $msbuild = Get-Command msbuild.exe -ErrorAction SilentlyContinue
    if ($null -ne $msbuild) {
        return $msbuild.Path
    }
    
    return $null
}

function Invoke-Build {
    param(
        [string]$SlnPath,
        [string]$Configuration,
        [string]$Platform,
        [string]$ProjectName
    )
    
    $msbuildPath = Find-MSBuild
    if (-not $msbuildPath) {
        Write-Error "MSBuild not found. Please install Visual Studio 2019 or 2022."
        return $false
    }
    
    $verbosity = if ($Verbose) { "normal" } else { "minimal" }
    $target = if ($ProjectName -and $ProjectName -ne "all") { "/t:$ProjectName" } else { "" }
    
    $cmdArgs = @(
        $SlnPath,
        "/m",
        "/p:Configuration=$Configuration",
        "/p:Platform=$Platform",
        "/p:TrackFileAccess=false",
        "/v:$verbosity",
        "/p:WarningLevel=4"
    )
    
    if ($target) { $cmdArgs += $target }
    
    Write-Status "Building: $ProjectName [$Configuration|$Platform]"
    Write-Status "Command: `"$msbuildPath`" $($cmdArgs -join ' ')"
    
    & $msbuildPath @cmdArgs
    
    if ($LASTEXITCODE -eq 0) {
        Write-Success "Build completed successfully"
        return $true
    } else {
        Write-Error "Build failed with exit code $LASTEXITCODE"
        return $false
    }
}

function Initialize-Premake {
    $repoRoot = Get-Location
    $premakeExe = Join-Path $repoRoot "premake4.exe"
    
    if (-not (Test-Path $premakeExe)) {
        Write-Warn "premake4.exe not found in repo root"
        return $false
    }
    
    Write-Status "Running premake4 to generate solution files..."
    & $premakeExe --file=premake4.lua vs2010
    
    if ($LASTEXITCODE -ne 0) {
        Write-Error "premake4 failed with exit code $LASTEXITCODE"
        return $false
    }
    
    Write-Success "premake4 completed"
    return $true
}

function Get-BuildConfigurations {
    $configs = @()
    
    $buildTypes = @()
    $platforms = @()
    
    if ($BuildType -eq "All") {
        $buildTypes = @("Debug", "Release")
    } else {
        $buildTypes = @($BuildType)
    }
    
    if ($Platform -eq "all") {
        $platforms = @("Win32", "x64")
    } else {
        $platforms = @($Platform)
    }
    
    foreach ($bt in $buildTypes) {
        foreach ($plat in $platforms) {
            $configs += @{
                Configuration = $bt
                Platform = $plat
                ShortName = "$($bt.ToLower())$($plat -eq 'x64' ? '64' : '32')"
            }
        }
    }
    
    return $configs
}

function Verify-Outputs {
    param(
        [string]$BinDir
    )
    
    if (-not (Test-Path $BinDir)) {
        return @()
    }
    
    $artifacts = @()
    $artifacts += @(Get-ChildItem -Recurse -ErrorAction SilentlyContinue -Path $BinDir -File -Include *.dll)
    $artifacts += @(Get-ChildItem -Recurse -ErrorAction SilentlyContinue -Path $BinDir -File -Include *.lib)
    $artifacts += @(Get-ChildItem -Recurse -ErrorAction SilentlyContinue -Path $BinDir -File -Include *.exe)
    
    return $artifacts
}

function Remove-BuildArtifacts {
    param(
        [string]$RepoRoot,
        [string]$ItemType = "tmp"  # "tmp" for object files, "all" for everything
    )
    
    Write-Status "Cleaning build artifacts..."
    
    $dirToClean = @()
    if ($ItemType -eq "tmp") {
        $dirToClean = @("build\vs2010\tmp", "tmp\vs2010")
    } elseif ($ItemType -eq "all") {
        $dirToClean = @("build\vs2010\tmp", "tmp\vs2010", "build\vs2010\bin", "build\bin")
    }
    
    foreach ($dir in $dirToClean) {
        $path = Join-Path $RepoRoot $dir
        if (Test-Path $path) {
            $size = [math]::Round((Get-ChildItem -Recurse -Path $path -File | Measure-Object -Property Length -Sum).Sum / 1MB, 2)
            Write-Status "Removing: $dir ($size MB)"
            Remove-Item -LiteralPath $path -Recurse -Force -ErrorAction SilentlyContinue
        }
    }
    
    Write-Success "Cleanup completed"
}

function Verify-Outputs {
    param(
        [string]$BinDir
    )
    
    if (-not (Test-Path $BinDir)) {
        return @()
    }
    
    $artifacts = @()
    $artifacts += @(Get-ChildItem -Recurse -ErrorAction SilentlyContinue -Path $BinDir -File -Include *.dll)
    $artifacts += @(Get-ChildItem -Recurse -ErrorAction SilentlyContinue -Path $BinDir -File -Include *.lib)
    $artifacts += @(Get-ChildItem -Recurse -ErrorAction SilentlyContinue -Path $BinDir -File -Include *.exe)
    
    return $artifacts
}

function New-ReleasePackage {
    param(
        [string]$SourceBinDir,
        [string]$ReleaseName,
        [string]$OutputDir
    )
    
    # Create release directory structure
    $releaseDir = Join-Path $OutputDir $ReleaseName
    $binDir = Join-Path $releaseDir "bin"
    $libDir = Join-Path $releaseDir "lib"
    $includeDir = Join-Path $releaseDir "include"
    $docsDir = Join-Path $releaseDir "docs"
    
    @($releaseDir, $binDir, $libDir, $includeDir, $docsDir) | ForEach-Object {
        if (-not (Test-Path $_)) {
            New-Item -ItemType Directory -Path $_ -Force | Out-Null
        }
    }
    
    # Copy DLLs to bin
    Get-ChildItem -Recurse -Path $SourceBinDir -File -Include *.dll -ErrorAction SilentlyContinue | 
        ForEach-Object {
            Copy-Item $_.FullName (Join-Path $binDir $_.Name) -Force
            Write-Status "Copied DLL: $($_.Name)"
        }
    
    # Copy LIBs to lib
    Get-ChildItem -Recurse -Path $SourceBinDir -File -Include *.lib -ErrorAction SilentlyContinue | 
        ForEach-Object {
            Copy-Item $_.FullName (Join-Path $libDir $_.Name) -Force
            Write-Status "Copied LIB: $($_.Name)"
        }
    
    # Copy EXEs to bin
    Get-ChildItem -Recurse -Path $SourceBinDir -File -Include *.exe -ErrorAction SilentlyContinue | 
        ForEach-Object {
            Copy-Item $_.FullName (Join-Path $binDir $_.Name) -Force
            Write-Status "Copied EXE: $($_.Name)"
        }
    
    # Copy headers (if available)
    if (Test-Path "core") {
        Get-ChildItem -Recurse -Path "core" -Directory -Filter "include" -ErrorAction SilentlyContinue |
            ForEach-Object {
                Copy-Item $_.FullName (Join-Path $includeDir $_.Parent.Name) -Recurse -Force -ErrorAction SilentlyContinue
                Write-Status "Copied headers from: $($_.Parent.Name)"
            }
    }
    
    # Copy documentation
    if (Test-Path "docs") {
        Get-ChildItem -Path "docs" -File -Include *.md -ErrorAction SilentlyContinue |
            ForEach-Object {
                Copy-Item $_.FullName (Join-Path $docsDir $_.Name) -Force
                Write-Status "Copied doc: $($_.Name)"
            }
    }
    
    # Create release manifest
    $manifest = @{
        ReleaseName = $ReleaseName
        BuildDate = Get-Date -Format 'yyyy-MM-dd HH:mm:ss'
        Configuration = @()
        Artifacts = @()
    } | ConvertTo-Json
    
    $manifest | Set-Content (Join-Path $releaseDir "RELEASE.json")
    Write-Status "Created release manifest"
    
    return $releaseDir
}

function Create-BuildReport {
    param(
        [hashtable]$BuildResults,
        [string]$OutputPath
    )
    
    $report = @"
# Build Report
Generated: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')

## Summary
- Total Configurations: $($BuildResults.Count)
- Successful: $($BuildResults.Values | Where-Object { $_ } | Measure-Object | Select-Object -ExpandProperty Count)
- Failed: $($BuildResults.Values | Where-Object { -not $_ } | Measure-Object | Select-Object -ExpandProperty Count)

## Details
$($BuildResults.GetEnumerator() | ForEach-Object {
    $status = if ($_.Value) { "✅ PASSED" } else { "❌ FAILED" }
    "- $($_.Key): $status"
} | Join-String -Separator "`n")

## Artifacts
$(Verify-Outputs (Join-Path (Get-Location) "bin") | ForEach-Object {
    "- $($_.FullName) ($($_.Length) bytes)"
} | Join-String -Separator "`n")
"@
    
    $report | Set-Content $OutputPath
    Write-Success "Build report saved to: $OutputPath"
}

# ============================================================================
# MAIN EXECUTION
# ============================================================================

if ($Help) {
    Show-Help
    exit 0
}

Write-Status "========================================================================="
Write-Status "AsirikuyFramework Build & Release Script"
Write-Status "========================================================================="

$repoRoot = Get-Location
Write-Status "Repository root: $repoRoot"

# Set BOOST_ROOT if needed
if (-not $env:BOOST_ROOT) {
    $defaultBoost = "e:\workspace\boost_1_49_0"
    if (Test-Path $defaultBoost) {
        $env:BOOST_ROOT = $defaultBoost
        Write-Status "BOOST_ROOT=$env:BOOST_ROOT"
    } else {
        Write-Warn "BOOST_ROOT not set. Some builds may fail."
    }
}

# Step 1: Clean if requested
if ($Clean) {
    Write-Status "Cleaning build directories..."
    @("build\vs2010", "bin\vs2010", "tmp\vs2010") | ForEach-Object {
        $path = Join-Path $repoRoot $_
        if (Test-Path $path) {
            Write-Status "Removing: $_"
            Remove-Item -LiteralPath $path -Recurse -Force -ErrorAction SilentlyContinue
        }
    }
    Write-Success "Clean completed"
}

# Step 2: Initialize premake
if (-not (Initialize-Premake)) {
    Write-Error "Failed to initialize premake. Exiting."
    exit 1
}

# Step 3: Determine build configurations
$configs = Get-BuildConfigurations
Write-Status "Build configurations:"
$configs | ForEach-Object { Write-Host "  - $($_.ShortName): $($_.Configuration)|$($_.Platform)" }

# Step 4: Build
$slnPath = Join-Path $repoRoot "build\vs2010\AsirikuyFramework.sln"
if (-not (Test-Path $slnPath)) {
    Write-Error "Solution file not found: $slnPath"
    exit 1
}

$buildResults = @{}
$successCount = 0
$failCount = 0

foreach ($config in $configs) {
    $key = $config.ShortName
    $result = Invoke-Build -SlnPath $slnPath -Configuration $config.Configuration -Platform $config.Platform -ProjectName $Project
    $buildResults[$key] = $result
    
    if ($result) {
        $successCount++
    } else {
        $failCount++
    }
}

# Step 5: Verify outputs
Write-Status ""
Write-Status "========================================================================="
Write-Status "Verifying Build Outputs"
Write-Status "========================================================================="

$artifacts = Verify-Outputs (Join-Path $repoRoot "bin")
Write-Status "Found $($artifacts.Count) artifacts:"
$artifacts | ForEach-Object {
    $size = "{0:N0}" -f $_.Length
    Write-Host "  📦 $($_.Name) ($size bytes)" -ForegroundColor Cyan
}

# Step 6: Cleanup if requested
if ($CleanAfter) {
    Write-Status ""
    Write-Status "========================================================================="
    Write-Status "Post-Build Cleanup"
    Write-Status "========================================================================="
    Remove-BuildArtifacts -RepoRoot $repoRoot -ItemType "tmp"
}

# Step 7: Create release package
if ($Release) {
    Write-Status ""
    Write-Status "========================================================================="
    Write-Status "Creating Release Package"
    Write-Status "========================================================================="
    
    $releaseName = "AsirikuyFramework-$(Get-Date -Format 'yyyyMMdd-HHmmss')"
    $releaseDir = New-ReleasePackage -SourceBinDir (Join-Path $repoRoot "bin") -ReleaseName $releaseName -OutputDir $OutputDir
    Write-Success "Release package created: $releaseDir"
}

# Step 8: Create build report
Write-Status ""
Write-Status "Creating build report..."
$reportPath = Join-Path $repoRoot "build-report.md"
Create-BuildReport -BuildResults $buildResults -OutputPath $reportPath

# Step 8: Summary
Write-Status ""
Write-Status "========================================================================="
Write-Status "Build Summary"
Write-Status "========================================================================="
Write-Success "Successful builds: $successCount"
Write-Warn "Failed builds: $failCount"
Write-Status "Report: $reportPath"

if ($CleanAfter) {
    Write-Status "Cleanup: Enabled (object files removed)"
} else {
    Write-Status "Cleanup: Disabled (use -CleanAfter to remove object files)"
}

if ($failCount -gt 0) {
    exit 1
} else {
    Write-Success "All builds completed successfully!"
    exit 0
}
