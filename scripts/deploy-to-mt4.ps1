#!/usr/bin/env pwsh
# Asirikuy MT4 Deployment Script
# Deploys AsirikuyFrameworkAPI.dll and CTesterFrameworkAPI.dll from latest release to MT4

param(
    [string]$MT4Path = "",
    [string]$ReleasePath = "",
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
    
    Write-Host "`nDLLs to deploy:"
    foreach ($dll in $dlls) {
        $destFile = Join-Path $DestPath $dll.Name
        $size = "{0:N2} KB" -f ($dll.Length / 1KB)
        Write-Info "  $($dll.Name) - $size"
        
        if (Test-Path $destFile) {
            Write-Warning "    Existing file will be overwritten"
        }
    }
    
    if ($IsDryRun) {
        Write-Warning "`n[DRY RUN] No files were copied"
        return
    }
    
    Write-Host ""
    foreach ($dll in $dlls) {
        $destFile = Join-Path $DestPath $dll.Name
        
        try {
            Copy-Item -Path $dll.FullName -Destination $destFile -Force
            Write-Success "Copied: $($dll.Name)"
        }
        catch {
            Write-Error "Failed to copy $($dll.Name): $_"
            exit 1
        }
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
    $MT4Path = Get-DefaultMT4Path
    
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
