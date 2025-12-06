# Set Email Password Environment Variable
# This script helps you securely set the email password as an environment variable

param(
    [Parameter(Mandatory=$false)]
    [string]$Password,
    
    [Parameter(Mandatory=$false)]
    [switch]$Persistent
)

Write-Host "============================================================" -ForegroundColor Cyan
Write-Host "Asirikuy Monitor - Email Password Setup" -ForegroundColor Cyan
Write-Host "============================================================" -ForegroundColor Cyan
Write-Host ""

# Get password if not provided
if (-not $Password) {
    Write-Host "This script will set the EMAIL_PASSWORD environment variable" -ForegroundColor Yellow
    Write-Host "so you don't need to store it in the config file." -ForegroundColor Yellow
    Write-Host ""
    
    $SecurePassword = Read-Host "Enter email password" -AsSecureString
    $BSTR = [System.Runtime.InteropServices.Marshal]::SecureStringToBSTR($SecurePassword)
    $Password = [System.Runtime.InteropServices.Marshal]::PtrToStringAuto($BSTR)
}

if ($Persistent) {
    # Set as user environment variable (persists across sessions)
    Write-Host "Setting EMAIL_PASSWORD as persistent user environment variable..." -ForegroundColor Yellow
    [Environment]::SetEnvironmentVariable("EMAIL_PASSWORD", $Password, "User")
    Write-Host "✓ EMAIL_PASSWORD set as user environment variable" -ForegroundColor Green
    Write-Host ""
    Write-Host "Note: You'll need to restart PowerShell/terminals for this to take effect." -ForegroundColor Yellow
    Write-Host "      Or you can also set it for current session immediately." -ForegroundColor Yellow
    Write-Host ""
    
    $SetNow = Read-Host "Set for current session now? (Y/N)"
    if ($SetNow -eq "Y" -or $SetNow -eq "y") {
        $env:EMAIL_PASSWORD = $Password
        Write-Host "✓ EMAIL_PASSWORD also set for current session" -ForegroundColor Green
    }
} else {
    # Set only for current session
    Write-Host "Setting EMAIL_PASSWORD for current session only..." -ForegroundColor Yellow
    $env:EMAIL_PASSWORD = $Password
    Write-Host "✓ EMAIL_PASSWORD set for current session" -ForegroundColor Green
    Write-Host ""
    Write-Host "Note: This will only last until you close this PowerShell window." -ForegroundColor Yellow
    Write-Host "      Use -Persistent flag to make it permanent." -ForegroundColor Yellow
}

Write-Host ""
Write-Host "============================================================" -ForegroundColor Cyan
Write-Host "Next Steps:" -ForegroundColor Cyan
Write-Host "============================================================" -ForegroundColor Cyan
Write-Host "1. Remove 'emailPassword' line from your config files" -ForegroundColor White
Write-Host "2. Commit config files to git (password no longer in files)" -ForegroundColor White
Write-Host "3. Run the monitor - it will use the environment variable" -ForegroundColor White
Write-Host ""
Write-Host "To verify: " -ForegroundColor Cyan
Write-Host '  Write-Host "EMAIL_PASSWORD is set: $($null -ne $env:EMAIL_PASSWORD)"' -ForegroundColor Gray
Write-Host ""
