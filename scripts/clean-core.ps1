#!/usr/bin/env pwsh
<#
.SYNOPSIS
    Clean AutoBBS core folder - removes build artifacts and temporary files
.DESCRIPTION
    Removes build artifacts, temporary files, and unnecessary items from the
    AutoBBS core C/C++ build system. Excludes Python projects (monitor, trader,
    ctester, PortfolioResult).
.PARAMETER DryRun
    Show what would be removed without actually deleting
.PARAMETER KeepReleases
    Keep the releases/ directory with packaged builds
.PARAMETER KeepVendorBackup
    Keep the vendor_backup/ directory
#>

param(
    [switch]$DryRun,
    [switch]$KeepReleases,
    [switch]$KeepVendorBackup
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

# Color helpers
function Write-Info($msg) { Write-Host $msg -ForegroundColor Cyan }
function Write-Success($msg) { Write-Host $msg -ForegroundColor Green }
function Write-Warning($msg) { Write-Host $msg -ForegroundColor Yellow }
function Write-Error2($msg) { Write-Host $msg -ForegroundColor Red }

# Change to script directory
$repoRoot = Split-Path -Parent (Split-Path -Parent $PSCommandPath)
Set-Location $repoRoot

Write-Info "================================================================"
Write-Info "  AutoBBS Core Cleanup Tool"
Write-Info "================================================================"
Write-Info ""

if ($DryRun) {
    Write-Warning "DRY RUN MODE - No files will be deleted"
    Write-Info ""
}

# Items to clean
$itemsToClean = @{
    "Build Artifacts" = @{
        Paths = @(
            "build/tmp",
            "build/vs2010"
        )
        Description = "Build intermediate files (.obj, .tlog, etc.)"
    }
    "Temporary Directories" = @{
        Paths = @(
            "tmp"
        )
        Description = "Temporary build files"
    }
    "Build Artifact Files in bin/" = @{
        Patterns = @(
            "bin/**/*.idb",
            "bin/**/*.ilk",
            "bin/**/*.exp"
        )
        Description = "Debug and export files in bin/"
    }
    "Vendor Build Artifacts" = @{
        Patterns = @(
            "vendor/**/*.idb",
            "vendor/**/*.ilk",
            "vendor/**/*.obj"
        )
        Description = "Build artifacts in vendor directories"
    }
    "macOS System Files" = @{
        Patterns = @(
            "**/.DS_Store"
        )
        Description = "macOS Finder metadata files"
    }
    "IDE Config Files" = @{
        Files = @(
            ".clangd",
            "corecrt_math_copy.h"
        )
        Description = "IDE configuration and copied header files"
    }
}

# Conditionally add vendor_backup
if (-not $KeepVendorBackup) {
    $itemsToClean["Vendor Backup"] = @{
        Paths = @("vendor_backup")
        Description = "Backup of removed vendor libraries (125 MB)"
    }
}

# Conditionally add releases
if (-not $KeepReleases) {
    $itemsToClean["Release Packages"] = @{
        Paths = @("releases")
        Description = "Packaged release builds"
    }
}

$totalSize = 0
$totalFiles = 0
$totalDirs = 0

# Process each category
foreach ($category in $itemsToClean.Keys | Sort-Object) {
    $config = $itemsToClean[$category]
    Write-Info "Checking: $category"
    Write-Host "  $($config.Description)" -ForegroundColor Gray
    
    $categorySize = 0
    $categoryFiles = 0
    $categoryDirs = 0
    
    # Process directories
    if ($config.ContainsKey('Paths')) {
        foreach ($path in $config.Paths) {
            $fullPath = Join-Path $repoRoot $path
            if (Test-Path $fullPath) {
                $size = (Get-ChildItem $fullPath -Recurse -File -ErrorAction SilentlyContinue | 
                         Measure-Object -Property Length -Sum).Sum
                $fileCount = (Get-ChildItem $fullPath -Recurse -File -ErrorAction SilentlyContinue).Count
                
                if ($size -gt 0) {
                    $sizeMB = [math]::Round($size / 1MB, 2)
                    Write-Host "  [REMOVE] $path ($fileCount files, $sizeMB MB)" -ForegroundColor Yellow
                    
                    $categorySize += $size
                    $categoryFiles += $fileCount
                    $categoryDirs++
                    
                    if (-not $DryRun) {
                        Remove-Item -LiteralPath $fullPath -Recurse -Force -ErrorAction SilentlyContinue
                        Write-Host "    -> Removed" -ForegroundColor Green
                    }
                }
            }
        }
    }
    
    # Process file patterns
    if ($config.ContainsKey('Patterns')) {
        foreach ($pattern in $config.Patterns) {
            $files = Get-ChildItem -Path $repoRoot -Filter ($pattern -replace '.*/', '') -Recurse -File -ErrorAction SilentlyContinue |
                     Where-Object { $_.FullName -match ($pattern -replace '\*\*/', '') } |
                     Where-Object { $_.FullName -notmatch '(asirikuy-trader|asirikuy_monitor|ctester|PortfolioResult)' }
            
            foreach ($file in $files) {
                $relPath = $file.FullName.Replace($repoRoot + '\', '')
                $sizeMB = [math]::Round($file.Length / 1MB, 2)
                Write-Host "  [REMOVE] $relPath ($sizeMB MB)" -ForegroundColor Yellow
                
                $categorySize += $file.Length
                $categoryFiles++
                
                if (-not $DryRun) {
                    Remove-Item -LiteralPath $file.FullName -Force -ErrorAction SilentlyContinue
                    Write-Host "    -> Removed" -ForegroundColor Green
                }
            }
        }
    }
    
    # Process individual files
    if ($config.ContainsKey('Files')) {
        foreach ($fileName in $config.Files) {
            $fullPath = Join-Path $repoRoot $fileName
            if (Test-Path $fullPath) {
                $file = Get-Item $fullPath
                $sizeMB = [math]::Round($file.Length / 1MB, 2)
                Write-Host "  [REMOVE] $fileName ($sizeMB MB)" -ForegroundColor Yellow
                
                $categorySize += $file.Length
                $categoryFiles++
                
                if (-not $DryRun) {
                    Remove-Item -LiteralPath $fullPath -Force -ErrorAction SilentlyContinue
                    Write-Host "    -> Removed" -ForegroundColor Green
                }
            }
        }
    }
    
    if ($categorySize -eq 0 -and $categoryFiles -eq 0) {
        Write-Host "  [SKIP] Nothing to clean" -ForegroundColor DarkGray
    }
    
    $totalSize += $categorySize
    $totalFiles += $categoryFiles
    $totalDirs += $categoryDirs
    
    Write-Host ""
}

# Summary
Write-Info "================================================================"
Write-Info "Summary"
Write-Info "================================================================"
Write-Host "Total files: $totalFiles" -ForegroundColor White
Write-Host "Total directories: $totalDirs" -ForegroundColor White
Write-Host "Total size: $([math]::Round($totalSize / 1MB, 2)) MB" -ForegroundColor White
Write-Host ""

if ($DryRun) {
    Write-Warning "DRY RUN complete. Run without -DryRun to execute cleanup."
} else {
    Write-Success "Cleanup complete!"
    Write-Info ""
    Write-Info "Excluded from cleanup:"
    Write-Host "  - asirikuy_monitor/ (Python monitoring)" -ForegroundColor DarkGray
    Write-Host "  - asirikuy-trader/ (Python trading)" -ForegroundColor DarkGray
    Write-Host "  - ctester/ (Python backtesting)" -ForegroundColor DarkGray
    Write-Host "  - PortfolioResult/ (Java portfolio)" -ForegroundColor DarkGray
    Write-Host "  - vendor/ (Essential libraries)" -ForegroundColor DarkGray
    if ($KeepVendorBackup) {
        Write-Host "  - vendor_backup/ (Kept by request)" -ForegroundColor DarkGray
    }
    if ($KeepReleases) {
        Write-Host "  - releases/ (Kept by request)" -ForegroundColor DarkGray
    }
}

Write-Info ""
