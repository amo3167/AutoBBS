#requires -Version 5
<#
.SYNOPSIS
    Migrate build artifacts (lib, idb, pdb) from source directories to build folder structure.
    
.DESCRIPTION
    Moves all .lib, .idb, and .pdb files from core module source directories into the proper
    build folder structure (bin/vs2010/x64/Release/lib, etc) to keep the source tree clean.
    
.PARAMETER SourceDir
    Root directory containing the source code (default: e:\AutoBBS)
    
.PARAMETER DestinationDir
    Destination parent directory (default: same as SourceDir)
    
.PARAMETER DryRun
    If set, shows what would be moved without actually moving files
    
.PARAMETER Verbose
    Enables verbose output showing each file operation
    
.EXAMPLE
    .\migrate-artifacts.ps1 -DryRun
    .\migrate-artifacts.ps1 -Verbose
    .\migrate-artifacts.ps1
#>

param(
    [string]$SourceDir = "e:\AutoBBS",
    [string]$DestinationDir = $SourceDir,
    [switch]$DryRun,
    [switch]$Verbose
)

$ErrorActionPreference = "Stop"

# Map of module names to their locations
$modules = @{
    "AsirikuyCommon" = "core/AsirikuyCommon"
    "AsirikuyEasyTrade" = "core/AsirikuyEasyTrade"
    "AsirikuyTechnicalAnalysis" = "core/AsirikuyTechnicalAnalysis"
    "Log" = "core/Log"
    "NTPClient" = "core/NTPClient"
    "OrderManager" = "core/OrderManager"
    "SymbolAnalyzer" = "core/SymbolAnalyzer"
    "TradingStrategies" = "core/TradingStrategies"
    "AsirikuyFrameworkAPI" = "core/AsirikuyFrameworkAPI"
    "CTesterFrameworkAPI" = "core/CTesterFrameworkAPI"
    "UnitTests" = "core/UnitTests"
    "AsirikuyRWrapper" = "core/AsirikuyRWrapper"
}

Write-Host "==================================================================" -ForegroundColor Cyan
Write-Host "Build Artifact Migration Tool" -ForegroundColor Cyan
Write-Host "==================================================================" -ForegroundColor Cyan
Write-Host ""

if ($DryRun) {
    Write-Host "DRY RUN MODE - No files will be moved" -ForegroundColor Yellow
    Write-Host ""
}

$totalSize = 0
$fileCount = 0
$movedCount = 0

foreach ($moduleName in $modules.Keys) {
    $modulePath = Join-Path $SourceDir $modules[$moduleName]
    
    if (-not (Test-Path $modulePath)) {
        Write-Host "Skipping $moduleName (not found at $modulePath)" -ForegroundColor Gray
        continue
    }
    
    Write-Host "Processing: $moduleName" -ForegroundColor Green
    
    # Find all artifact files in this module
    $artifacts = @()
    
    # Find lib files
    $libFiles = Get-ChildItem -Path $modulePath -Filter "$moduleName.lib" -ErrorAction SilentlyContinue
    $artifacts += $libFiles
    
    # Find idb files
    $idbFiles = Get-ChildItem -Path $modulePath -Filter "$moduleName.idb" -ErrorAction SilentlyContinue
    $artifacts += $idbFiles
    
    # Find pdb files
    $pdbFiles = Get-ChildItem -Path $modulePath -Filter "$moduleName.pdb" -ErrorAction SilentlyContinue
    $artifacts += $pdbFiles
    
    if ($artifacts.Count -eq 0) {
        Write-Host "  No artifacts found" -ForegroundColor Gray
        continue
    }
    
    foreach ($artifact in $artifacts) {
        $fileCount++
        $totalSize += $artifact.Length
        
        # Determine destination - use vs2010/x64/Release as default
        # In future, this could be smarter based on PDB content or file timestamps
        $destPath = Join-Path $DestinationDir "bin/vs2010/x64/Release/lib"
        $destFile = Join-Path $destPath $artifact.Name
        
        # Create destination directory if it doesn't exist
        if (-not (Test-Path $destPath)) {
            if ($Verbose) {
                Write-Host "  Creating directory: $destPath" -ForegroundColor Cyan
            }
            if (-not $DryRun) {
                New-Item -ItemType Directory -Path $destPath -Force | Out-Null
            }
        }
        
        Write-Host "  Moving: $($artifact.Name) to bin/vs2010/x64/Release/lib/" -ForegroundColor White
        
        if ($Verbose) {
            Write-Host "    From: $($artifact.FullName)" -ForegroundColor Gray
            Write-Host "    To:   $destFile" -ForegroundColor Gray
            Write-Host "    Size: $([math]::Round($artifact.Length / 1KB, 2)) KB" -ForegroundColor Gray
        }
        
        if (-not $DryRun) {
            # Move the file
            Move-Item -Path $artifact.FullName -Destination $destFile -Force
            $movedCount++
            
            if ($Verbose) {
                Write-Host "    Status: MOVED" -ForegroundColor Green
            }
        } else {
            Write-Host "    [DRY RUN] Would move" -ForegroundColor Yellow
        }
    }
}

Write-Host ""
Write-Host "==================================================================" -ForegroundColor Cyan
Write-Host "Migration Summary" -ForegroundColor Cyan
Write-Host "==================================================================" -ForegroundColor Cyan
Write-Host "Total files found:  $fileCount" -ForegroundColor White
Write-Host "Total size:         $([math]::Round($totalSize / 1MB, 2)) MB" -ForegroundColor White

if (-not $DryRun) {
    Write-Host "Files moved:        $movedCount" -ForegroundColor Green
    Write-Host ""
    Write-Host "Artifacts successfully migrated to build folder structure!" -ForegroundColor Green
} else {
    Write-Host "Files would move:   $fileCount" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "DRY RUN complete. Run without -DryRun to execute migration." -ForegroundColor Yellow
}

Write-Host ""
