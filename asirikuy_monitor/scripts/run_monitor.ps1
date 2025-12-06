# Asirikuy Monitor - Startup Script (Windows PowerShell)
# Starts the monitor with proper error handling and logging

param(
    [string]$ConfigFile = "config\checker.config"
)

$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectDir = Split-Path -Parent $ScriptDir

# Change to project directory
Set-Location $ProjectDir

Write-Host "============================================================" -ForegroundColor Cyan
Write-Host "Asirikuy Monitor - Starting" -ForegroundColor Cyan
Write-Host "============================================================" -ForegroundColor Cyan
Write-Host ""

# Check if checker.py exists
if (!(Test-Path "checker.py")) {
    Write-Host "Error: checker.py not found in project directory." -ForegroundColor Red
    exit 1
}

# Create log directory if it doesn't exist
if (!(Test-Path "log")) {
    New-Item -ItemType Directory -Path "log" | Out-Null
    Write-Host "Created log directory" -ForegroundColor Green
}

# Log startup
$StartupLog = "log\startup.log"
$Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
Add-Content -Path $StartupLog -Value "$Timestamp - Starting Asirikuy Monitor"
Add-Content -Path $StartupLog -Value "$Timestamp - Config file: $ConfigFile"

# Get Python version
try {
    $PythonVersion = python3 --version 2>&1
    Add-Content -Path $StartupLog -Value "$Timestamp - Python: $PythonVersion"
    Write-Host "Python: $PythonVersion" -ForegroundColor Green
} catch {
    Write-Host "Error: python3 not found. Please install Python 3.8 or higher." -ForegroundColor Red
    exit 1
}

# Get platform info
try {
    $Platform = python3 -c "from include.platform_utils import get_platform_name; print(get_platform_name())"
    Add-Content -Path $StartupLog -Value "$Timestamp - Platform: $Platform"
    Write-Host "Platform: $Platform" -ForegroundColor Green
} catch {
    Write-Host "Warning: Could not get platform info" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "Starting monitor with config: $ConfigFile" -ForegroundColor Cyan
Write-Host "Press Ctrl+C to stop" -ForegroundColor Yellow
Write-Host ""

# Function to handle errors
function Handle-Error {
    param($ErrorCode)
    $Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    Add-Content -Path $StartupLog -Value "$Timestamp - Error: Monitor exited with code $ErrorCode"
    Write-Host "Error: Monitor exited with code $ErrorCode" -ForegroundColor Red
}

# Function to handle Ctrl+C
$null = Register-EngineEvent -SourceIdentifier PowerShell.Exiting -Action {
    $Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    Add-Content -Path "$using:StartupLog" -Value "$Timestamp - Monitor stopped by user"
}

# Start the monitor
try {
    python3 checker.py -c $ConfigFile
    $ExitCode = $LASTEXITCODE
    
    if ($ExitCode -ne 0) {
        Handle-Error -ErrorCode $ExitCode
        exit $ExitCode
    }
    
    $Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    Add-Content -Path $StartupLog -Value "$Timestamp - Monitor stopped normally"
    Write-Host "Monitor stopped normally" -ForegroundColor Green
    
} catch {
    $Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    Add-Content -Path $StartupLog -Value "$Timestamp - Error: $_"
    Write-Host "Error running monitor: $_" -ForegroundColor Red
    exit 1
}
