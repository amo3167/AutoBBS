# Test Email Alert - Asirikuy Monitor
# This script adds a test error to the MT4 log to trigger an email alert

param(
    [string]$ConfigFile = "config\checker_MT4_Real.config"
)

Write-Host "============================================================" -ForegroundColor Cyan
Write-Host "Asirikuy Monitor - Email Alert Test" -ForegroundColor Cyan
Write-Host "============================================================" -ForegroundColor Cyan
Write-Host ""

# Parse config to find MT4 path
$Config = Get-Content $ConfigFile
$AccountSection = ""
foreach ($line in $Config) {
    if ($line -match '^\[([^\]]+)\]$') {
        $section = $Matches[1]
        if ($section -ne "general" -and $section -ne "accounts") {
            $AccountSection = $section
            break
        }
    }
}

if ($AccountSection -eq "") {
    Write-Host "Error: No account section found in config" -ForegroundColor Red
    exit 1
}

# Get path from config
$MT4Path = ""
foreach ($line in $Config) {
    if ($line -match "^path\s*=\s*(.+)$") {
        $MT4Path = $Matches[1].Trim()
        break
    }
}

if ($MT4Path -eq "") {
    Write-Host "Error: MT4 path not found in config" -ForegroundColor Red
    exit 1
}

Write-Host "Found MT4 path: $MT4Path" -ForegroundColor Green

# Construct log file path
$LogPath = Join-Path $MT4Path "MQL4\Logs\AsirikuyFramework.log"

if (!(Test-Path $LogPath)) {
    Write-Host "Creating log file: $LogPath" -ForegroundColor Yellow
    $LogDir = Split-Path $LogPath
    if (!(Test-Path $LogDir)) {
        New-Item -ItemType Directory -Path $LogDir -Force | Out-Null
    }
    New-Item -ItemType File -Path $LogPath -Force | Out-Null
}

# Add test error message in MT4 format
$Timestamp = Get-Date -Format "yyyy/MM/dd HH:mm:ss.fff"
$TestError = "[AsirikuyFramework.TEST, $Timestamp; Emergency]: This is a test error message to trigger email alert"

Write-Host ""
Write-Host "Adding test error to log file..." -ForegroundColor Yellow
Write-Host "Message: $TestError" -ForegroundColor Cyan

Add-Content -Path $LogPath -Value $TestError

Write-Host ""
Write-Host "Test error added successfully!" -ForegroundColor Green
Write-Host "This error is now the LAST LINE in the log (which is what the monitor checks)." -ForegroundColor Yellow
Write-Host ""
Write-Host "The monitor should detect this error and send an email alert." -ForegroundColor Cyan
Write-Host "Check your email inbox for the alert." -ForegroundColor Cyan
Write-Host ""
Write-Host "To view the log file:" -ForegroundColor Yellow
Write-Host "  Get-Content `"$LogPath`" -Tail 10" -ForegroundColor Gray
Write-Host ""
