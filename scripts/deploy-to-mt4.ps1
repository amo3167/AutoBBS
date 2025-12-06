#!/usr/bin/env pwsh
# Asirikuy MT4 Deployment Script
# Deploys AsirikuyFrameworkAPI.dll and CTesterFrameworkAPI.dll from latest release to MT4
#
# Features:
# - Auto-detects latest release from releases/ folder
# - Reads MT4 path from monitor config file (checker.config)
# - Auto-discovers MT4 installation in common paths
# - Backs up existing DLLs before deployment
# - Supports dry-run mode for testing
#
# Usage:
#   .\deploy-to-mt4.ps1                                                  # Auto-detect everything
#   .\deploy-to-mt4.ps1 -ConfigFile asirikuy_monitor\config\checker.config  # Use monitor config
#   .\deploy-to-mt4.ps1 -MT4Path "C:\...\MQL4\Libraries"                # Specify MT4 path
#   .\deploy-to-mt4.ps1 -DryRun                                         # Preview without deploying

param(
    [string]$MT4Path = "",
    [string]$ReleasePath = "",
    [string]$ConfigFile = "",
    [switch]$DryRun = $false
)

$ErrorActionPreference = "Stop"
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Split-Path -Parent $ScriptDir
$ReleasesFolder = Join-Path $ProjectRoot "releases"

function Write-Info { param([string]$msg) Write-Host "[INFO] $msg" -ForegroundColor Cyan }
function Write-Success { param([string]$msg) Write-Host "[OK] $msg" -ForegroundColor Green }
function Write-Warning { param([string]$msg) Write-Host "[WARN] $msg" -ForegroundColor Yellow }
function Write-Error { param([string]$msg) Write-Host "[ERROR] $msg" -ForegroundColor Red }

function Get-LatestRelease {
    Write-Info "Searching for latest release in $ReleasesFolder"
    $releases = Get-ChildItem -Path $ReleasesFolder -Directory -Filter "AsirikuyFramework-*" |
        Sort-Object Name -Descending |
        Select-Object -First 1
    
    if (-not $releases) {
        Write-Error "No releases found in $ReleasesFolder"
        exit 1
    }
    
    return $releases.FullName
}

function Get-DefaultMT4Path {
    Write-Info "Searching for MT4 installation"
    
    $possiblePaths = @(
        "$env:USERPROFILE\AppData\Roaming\MetaQuotes\Terminal\*\MQL4\Libraries",
        "C:\Program Files\MetaTrader 4\MQL4\Libraries",
        "C:\Program Files (x86)\MetaTrader 4\MQL4\Libraries",
        "$env:USERPROFILE\Desktop\*MetaTrader*\MQL4\Libraries"
    )
    
    foreach ($pattern in $possiblePaths) {
        $found = Get-Item -Path $pattern -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($found) {
            Write-Success "Found MT4 at: $($found.FullName)"
            return $found.FullName
        }
    }
    
    return $null
}

function Get-MT4PathFromConfig {
    param([string]$ConfigPath)
    
    if (-not (Test-Path $ConfigPath)) {
        Write-Warning "Config file not found: $ConfigPath"
        return $null
    }
    
    Write-Info "Reading MT4 path from config: $ConfigPath"
    
    try {
        $content = Get-Content $ConfigPath -Raw
        
        # Parse INI-style config to find [accounts] section and get account list
        if ($content -match '\[accounts\]\s*accounts\s*=\s*([^\r\n]+)') {
            $accountNames = $matches[1].Trim() -split ',' | ForEach-Object { $_.Trim() }
            Write-Info "Found accounts: $($accountNames -join ', ')"
            
            # Try each account name
            foreach ($accountName in $accountNames) {
                if ($content -match "\[$accountName\][^\[]*path\s*=\s*([^\r\n]+)") {
                    $mt4Path = $matches[1].Trim()
                    $librariesPath = Join-Path $mt4Path "MQL4\Libraries"
                    
                    if (Test-Path $librariesPath) {
                        Write-Success "MT4 path from config [$accountName]: $librariesPath"
                        return $librariesPath
                    }
                    else {
                        Write-Warning "MQL4\Libraries not found at: $librariesPath"
                    }
                }
            }
        }
        
        # If no accounts match, try to find any section with a path that looks like MT4
        if ($content -match '\[([^\]]+)\][^\[]*path\s*=\s*([^\r\n]+)[^\[]*frontend\s*=\s*MT4') {
            $accountName = $matches[1].Trim()
            $mt4Path = $matches[2].Trim()
            $librariesPath = Join-Path $mt4Path "MQL4\Libraries"
            
            if (Test-Path $librariesPath) {
                Write-Success "MT4 path from config [$accountName]: $librariesPath"
                return $librariesPath
            }
        }
    }
    catch {
        Write-Warning "Error parsing config file: $_"
    }
    
    return $null
}

function Copy-DLLs {
    param(
        [string]$SourcePath,
        [string]$DestPath,
        [bool]$IsDryRun
    )
    
    $binFolder = Join-Path $SourcePath "bin"
    
    if (-not (Test-Path $binFolder)) {
        Write-Error "Release bin folder not found: $binFolder"
        exit 1
    }
    
    $dlls = Get-ChildItem -Path $binFolder -Filter "*.dll"
    
    if ($dlls.Count -eq 0) {
        Write-Error "No DLL files found in $binFolder"
        exit 1
    }
    
    # Create backup folder with timestamp
    $timestamp = Get-Date -Format "yyyyMMdd-HHmmss"
    $backupFolder = Join-Path $DestPath "backup_$timestamp"
    
    Write-Host "`nDLLs to deploy:"
    $hasExistingFiles = $false
    foreach ($dll in $dlls) {
        $destFile = Join-Path $DestPath $dll.Name
        $size = "{0:N2} KB" -f ($dll.Length / 1KB)
        Write-Info "  $($dll.Name) - $size"
        
        if (Test-Path $destFile) {
            $hasExistingFiles = $true
            Write-Warning "    Existing file will be backed up"
        }
    }
    
    if ($IsDryRun) {
        if ($hasExistingFiles) {
            Write-Info "`n[DRY RUN] Would create backup folder: $backupFolder"
        }
        Write-Warning "`n[DRY RUN] No files were copied"
        return
    }
    
    # Create backup folder and backup existing DLLs
    if ($hasExistingFiles) {
        try {
            New-Item -Path $backupFolder -ItemType Directory -Force | Out-Null
            Write-Success "`nCreated backup folder: $backupFolder"
            
            foreach ($dll in $dlls) {
                $destFile = Join-Path $DestPath $dll.Name
                if (Test-Path $destFile) {
                    $backupFile = Join-Path $backupFolder $dll.Name
                    Copy-Item -Path $destFile -Destination $backupFile -Force
                    Write-Success "Backed up: $($dll.Name)"
                }
            }
        }
        catch {
            Write-Error "Failed to create backup: $_"
            exit 1
        }
    }
    
    # Deploy new DLLs
    Write-Host ""
    foreach ($dll in $dlls) {
        $destFile = Join-Path $DestPath $dll.Name
        
        try {
            Copy-Item -Path $dll.FullName -Destination $destFile -Force
            Write-Success "Deployed: $($dll.Name)"
        }
        catch {
            Write-Error "Failed to deploy $($dll.Name): $_"
            Write-Warning "You can restore from backup: $backupFolder"
            exit 1
        }
    }
    
    if ($hasExistingFiles) {
        Write-Host ""
        Write-Info "Backup location: $backupFolder"
    }
}

# Main
Write-Host "`n=== Asirikuy MT4 Deployment ===" -ForegroundColor Blue

# Get release path
if ($ReleasePath -eq "") {
    $ReleasePath = Get-LatestRelease
}
else {
    if (-not (Test-Path $ReleasePath)) {
        Write-Error "Release path not found: $ReleasePath"
        exit 1
    }
}

Write-Success "Using release: $(Split-Path -Leaf $ReleasePath)"

# Get MT4 path
if ($MT4Path -eq "") {
    # Try config file first if specified
    if ($ConfigFile -ne "") {
        $MT4Path = Get-MT4PathFromConfig $ConfigFile
    }
    
    # Fall back to auto-detection if config didn't work
    if ($null -eq $MT4Path) {
        $MT4Path = Get-DefaultMT4Path
    }
    
    if ($null -eq $MT4Path) {
        Write-Warning "MT4 not found automatically"
        Write-Host "Enter MT4 MQL4\Libraries path:"
        $MT4Path = Read-Host "Path"
    }
}

if (-not (Test-Path $MT4Path)) {
    Write-Error "MT4 path does not exist: $MT4Path"
    exit 1
}

Write-Success "Target: $MT4Path"

# Deploy
Copy-DLLs -SourcePath $ReleasePath -DestPath $MT4Path -IsDryRun $DryRun

# Summary
Write-Host "`n=== Deployment Complete ===" -ForegroundColor Green
Write-Host "Release: $(Split-Path -Leaf $ReleasePath)" -ForegroundColor Cyan
Write-Host "Target: $MT4Path" -ForegroundColor Cyan

if (-not $DryRun) {
    Write-Host "`nNext steps:" -ForegroundColor Blue
    Write-Host "1. Restart MT4 if running" -ForegroundColor Cyan
    Write-Host "2. Enable Tools - Options - Expert Advisors - Allow DLL imports" -ForegroundColor Cyan
    Write-Host "3. Add Libraries path to trusted paths if needed" -ForegroundColor Cyan
}

Write-Host ""
