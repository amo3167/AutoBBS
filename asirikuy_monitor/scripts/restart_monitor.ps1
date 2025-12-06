# Restart Monitor Script
# Kills any running monitor instances and starts a fresh one

Write-Host "============================================================" -ForegroundColor Cyan
Write-Host "Asirikuy Monitor - Restart Script" -ForegroundColor Cyan
Write-Host "============================================================" -ForegroundColor Cyan
Write-Host ""

# Find and kill any running checker.py processes
Write-Host "Stopping any running monitor processes..." -ForegroundColor Yellow

$monitorProcesses = Get-WmiObject Win32_Process | Where-Object {
    $_.CommandLine -like "*checker.py*" -and $_.Name -like "*python*"
}

if ($monitorProcesses) {
    foreach ($proc in $monitorProcesses) {
        Write-Host "  Killing process ID $($proc.ProcessId): $($proc.CommandLine)" -ForegroundColor Yellow
        Stop-Process -Id $proc.ProcessId -Force
    }
    Write-Host "  Stopped $($monitorProcesses.Count) monitor process(es)" -ForegroundColor Green
    Start-Sleep -Seconds 2
} else {
    Write-Host "  No running monitor processes found" -ForegroundColor Gray
}

Write-Host ""

# Clear Python cache
Write-Host "Clearing Python cache files..." -ForegroundColor Yellow
Get-ChildItem "$PSScriptRoot\.." -Filter "__pycache__" -Recurse -Directory | Remove-Item -Recurse -Force -ErrorAction SilentlyContinue
Write-Host "  Cache cleared" -ForegroundColor Green

Write-Host ""

# Get config file parameter
param(
    [string]$ConfigFile = "config\checker_MT4_Real.config"
)

# Change to monitor directory
$monitorDir = Split-Path -Parent $PSScriptRoot
Set-Location $monitorDir

Write-Host "Starting fresh monitor instance..." -ForegroundColor Yellow
Write-Host "  Config: $ConfigFile" -ForegroundColor Gray
Write-Host "  Directory: $monitorDir" -ForegroundColor Gray
Write-Host ""

# Start monitor in a new window
Start-Process powershell -ArgumentList @(
    "-ExecutionPolicy", "Bypass",
    "-NoExit",
    "-Command", "cd '$monitorDir' ; python3 checker.py -c $ConfigFile"
)

Start-Sleep -Seconds 2

Write-Host "Monitor restarted successfully!" -ForegroundColor Green
Write-Host ""
Write-Host "To view logs:" -ForegroundColor Cyan
Write-Host "  Get-Content log\monitor.log -Tail 20 -Wait" -ForegroundColor Gray
