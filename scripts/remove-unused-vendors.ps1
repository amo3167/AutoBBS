#requires -Version 5
<#
.SYNOPSIS
    Remove unused vendor libraries from the project to save disk space.
    
.DESCRIPTION
    Removes vendor libraries that are not referenced in the codebase.
    This script will remove 143 MB of unused dependencies while keeping
    the essential libraries (TALib, MiniXML, Boost, curl).
    
.PARAMETER DryRun
    If set, shows what would be removed without actually deleting
    
.PARAMETER Backup
    If set, moves vendors to a backup folder instead of deleting
    
.PARAMETER BackupPath
    Path to backup folder (default: vendor_backup)
    
.EXAMPLE
    .\remove-unused-vendors.ps1 -DryRun
    .\remove-unused-vendors.ps1 -Backup
    .\remove-unused-vendors.ps1
#>

param(
    [switch]$DryRun,
    [switch]$Backup,
    [string]$BackupPath = "vendor_backup"
)

$ErrorActionPreference = "Stop"

$vendorDir = "e:\AutoBBS\vendor"

# Libraries to KEEP (actively used in codebase)
$keepLibraries = @(
    "TALib",        # Technical analysis library - heavily used
    "MiniXML",      # XML parsing - used in AsirikuyFrameworkAPI
    "Boost",        # Core dependency throughout project (external via BOOST_ROOT)
    "curl"          # Used on macOS/Linux for AsirikuyEasyTrade
)

# Libraries to REMOVE (no references found or explicitly commented out)
$removeLibraries = @(
    @{Name="ChartDirector"; Size=45.79; Reason="Commented out in code - not available"},
    @{Name="Shark"; Size=31.96; Reason="No references found"},
    @{Name="FANN"; Size=11.21; Reason="Only FANN_NO_DLL define, no actual usage"},
    @{Name="STLSoft"; Size=9.13; Reason="No references found"},
    @{Name="LittleCMS"; Size=4.38; Reason="No references found"},
    @{Name="LibMNG"; Size=4.52; Reason="No references found"},
    @{Name="LibJPEG"; Size=3.64; Reason="No references found"},
    @{Name="Waffles"; Size=3.37; Reason="No references found"},
    @{Name="Jasper"; Size=3.16; Reason="No references found"},
    @{Name="LibPNG"; Size=2.81; Reason="No references found"},
    @{Name="Zlib"; Size=2.29; Reason="No references found"},
    @{Name="DevIL"; Size=1.93; Reason="No references found"},
    @{Name="dSFMT"; Size=0.8; Reason="No references found"}
)

Write-Host "================================================================" -ForegroundColor Cyan
Write-Host "  Unused Vendor Library Removal Tool" -ForegroundColor Cyan
Write-Host "================================================================" -ForegroundColor Cyan
Write-Host ""

if ($DryRun) {
    Write-Host "DRY RUN MODE - No files will be removed" -ForegroundColor Yellow
    Write-Host ""
}

if ($Backup) {
    Write-Host "BACKUP MODE - Will move to backup folder" -ForegroundColor Yellow
    Write-Host ""
}

Write-Host "Libraries to KEEP:" -ForegroundColor Green
foreach ($lib in $keepLibraries) {
    $libPath = Join-Path $vendorDir $lib
    if (Test-Path $libPath) {
        Write-Host "  [KEEP] $lib" -ForegroundColor Green
    } else {
        Write-Host "  [WARN] $lib (not found - may be external)" -ForegroundColor Yellow
    }
}

Write-Host ""
Write-Host "Libraries to REMOVE:" -ForegroundColor Red

$totalSize = 0
$removedCount = 0
$skippedCount = 0

foreach ($lib in $removeLibraries) {
    $libName = $lib.Name
    $libPath = Join-Path $vendorDir $libName
    
    if (Test-Path $libPath) {
        $totalSize += $lib.Size
        Write-Host "  [REMOVE] $libName ($($lib.Size) MB) - $($lib.Reason)" -ForegroundColor Red
        
        if (-not $DryRun) {
            if ($Backup) {
                # Move to backup
                $backupDir = Join-Path (Get-Location) $BackupPath
                if (-not (Test-Path $backupDir)) {
                    New-Item -ItemType Directory -Path $backupDir -Force | Out-Null
                }
                
                $destPath = Join-Path $backupDir $libName
                Write-Host "    -> Moving to backup: $destPath" -ForegroundColor Yellow
                Move-Item -Path $libPath -Destination $destPath -Force
                $removedCount++
            } else {
                # Delete permanently
                Write-Host "    -> Removing..." -ForegroundColor Red
                Remove-Item -Path $libPath -Recurse -Force
                $removedCount++
            }
        }
    } else {
        Write-Host "  [SKIP] $libName (already removed)" -ForegroundColor Gray
        $skippedCount++
    }
}

Write-Host ""
Write-Host "================================================================" -ForegroundColor Cyan
Write-Host "Summary" -ForegroundColor Cyan
Write-Host "================================================================" -ForegroundColor Cyan
Write-Host "Total size to remove: $([math]::Round($totalSize, 2)) MB" -ForegroundColor White

if (-not $DryRun) {
    if ($Backup) {
        Write-Host "Libraries backed up: $removedCount" -ForegroundColor Green
        Write-Host "Backup location: $(Join-Path (Get-Location) $BackupPath)" -ForegroundColor Yellow
    } else {
        Write-Host "Libraries removed: $removedCount" -ForegroundColor Green
    }
    Write-Host "Already removed: $skippedCount" -ForegroundColor Gray
    Write-Host ""
    Write-Host "Cleanup complete! Saved $([math]::Round($totalSize, 2)) MB of disk space." -ForegroundColor Green
} else {
    Write-Host "Would remove: $($removeLibraries.Count - $skippedCount) libraries" -ForegroundColor Yellow
    Write-Host "Already removed: $skippedCount" -ForegroundColor Gray
    Write-Host ""
    Write-Host "DRY RUN complete. Run without -DryRun to execute removal." -ForegroundColor Yellow
}

Write-Host ""

# Show what's kept
Write-Host "Keeping essential libraries:" -ForegroundColor Cyan
foreach ($lib in $keepLibraries) {
    $libPath = Join-Path $vendorDir $lib
    if (Test-Path $libPath) {
        $size = (Get-ChildItem -Path $libPath -Recurse -File | Measure-Object -Property Length -Sum).Sum
        $sizeMB = [math]::Round($size / 1MB, 2)
        Write-Host "  [KEEP] $lib ($sizeMB MB)" -ForegroundColor Green
    }
}

Write-Host ""
Write-Host "Note: Also keeping FANN_NO_DLL define in TradingStrategies/premake4.lua" -ForegroundColor Yellow
Write-Host "      (can be removed manually if FANN is confirmed unused)" -ForegroundColor Yellow
Write-Host ""
