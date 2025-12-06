# Test Heartbeat Error Script
# Creates an old heartbeat file to trigger heartbeat failure alert

param(
    [string]$ConfigFile = "config\checker_MT4_Real.config"
)

$ErrorActionPreference = "Stop"

Write-Host "============================================================" -ForegroundColor Cyan
Write-Host "Asirikuy Monitor - Heartbeat Error Test" -ForegroundColor Cyan
Write-Host "============================================================" -ForegroundColor Cyan
Write-Host ""

# Get script directory and change to project root
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectDir = Split-Path -Parent $ScriptDir
Set-Location $ProjectDir

# Check if config file exists
if (!(Test-Path $ConfigFile)) {
    Write-Host "Error: Config file not found: $ConfigFile" -ForegroundColor Red
    exit 1
}

# Parse config file to get MT4 path
Write-Host "Reading config file: $ConfigFile" -ForegroundColor Yellow
$ConfigContent = Get-Content $ConfigFile
$PathLine = $ConfigContent | Where-Object { $_ -match "^path\s*=" }
if (!$PathLine) {
    Write-Host "Error: Could not find 'path' in config file" -ForegroundColor Red
    exit 1
}

$MT4Path = ($PathLine -split "=", 2)[1].Trim()
Write-Host "Found MT4 path: $MT4Path" -ForegroundColor Green
Write-Host ""

# Construct heartbeat directory path
$HeartbeatDir = Join-Path $MT4Path "MQL4\Files"

if (!(Test-Path $HeartbeatDir)) {
    Write-Host "Error: Heartbeat directory not found: $HeartbeatDir" -ForegroundColor Red
    exit 1
}

Write-Host "Heartbeat directory: $HeartbeatDir" -ForegroundColor Green
Write-Host ""

# Create test heartbeat file with old timestamp
$TestInstanceID = "999999"
$HeartbeatFile = Join-Path $HeartbeatDir "${TestInstanceID}_heartBeat.hb"

Write-Host "Creating test heartbeat file..." -ForegroundColor Yellow
Write-Host "File: $HeartbeatFile" -ForegroundColor Gray

# Create heartbeat file with old date (5 minutes ago to trigger alert)
$OldDate = (Get-Date).AddMinutes(-5)
$HeartbeatContent = @"
$TestInstanceID
$($OldDate.ToString("dd/MM/yy HH:mm"))
"@

Set-Content -Path $HeartbeatFile -Value $HeartbeatContent -Encoding ASCII

Write-Host ""
Write-Host "✓ Test heartbeat file created successfully!" -ForegroundColor Green
Write-Host ""
Write-Host "Heartbeat details:" -ForegroundColor Cyan
Write-Host "  Instance ID: $TestInstanceID" -ForegroundColor Gray
Write-Host "  Timestamp: $($OldDate.ToString("dd/MM/yy HH:mm"))" -ForegroundColor Gray
Write-Host "  Age: 5 minutes (will trigger alert if threshold exceeded)" -ForegroundColor Gray
Write-Host ""
Write-Host "File contents:" -ForegroundColor Cyan
Get-Content $HeartbeatFile | ForEach-Object { Write-Host "  $_" -ForegroundColor Gray }
Write-Host ""

Write-Host "============================================================" -ForegroundColor Cyan
Write-Host "What will happen:" -ForegroundColor Cyan
Write-Host "============================================================" -ForegroundColor Cyan
Write-Host "What will happen:" -ForegroundColor Cyan
Write-Host "============================================================" -ForegroundColor Cyan
Write-Host "1. Monitor will detect old heartbeat (> 150 seconds)" -ForegroundColor White
Write-Host "2. Send email alert about heartbeat problem" -ForegroundColor White
Write-Host "3. Send Telegram alert about heartbeat problem" -ForegroundColor White
Write-Host "4. Delete the heartbeat file" -ForegroundColor White
Write-Host "5. Kill MT4 processes (if running)" -ForegroundColor White
Write-Host ""
Write-Host "The monitor should detect this within the next check cycle." -ForegroundColor Yellow
