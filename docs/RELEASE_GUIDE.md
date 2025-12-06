# AutoBBS Release Guide

Complete guide for building, releasing, and deploying the Asirikuy Framework on Windows.

**Version:** 1.0  
**Last Updated:** December 6, 2025  
**Platform:** Windows 10/11 with Visual Studio 2022

---

## Table of Contents

1. [Creating a Release](#creating-a-release)
2. [MT4 Installation & Setup](#mt4-installation--setup)
3. [Deploying DLLs to MT4](#deploying-dlls-to-mt4)
4. [Configuring the Monitor](#configuring-the-monitor)
5. [Running the Monitor](#running-the-monitor)
6. [Troubleshooting](#troubleshooting)

---

## Creating a Release

### Prerequisites

- Visual Studio 2022 Community (or higher)
- Windows SDK 10.0.26100.0 (or compatible version)
- Git for Windows
- PowerShell 5.1 or higher

### Build Steps

1. **Clean Build Environment**
   ```powershell
   cd E:\AutoBBS
   .\scripts\build-parallel-simple.bat clean
   ```

2. **Build Release DLLs**
   ```powershell
   # Build both AsirikuyFrameworkAPI.dll and CTesterFrameworkAPI.dll
   .\scripts\build-parallel-simple.bat
   ```

   Expected output:
   - `bin\vs2010\x64\Release\AsirikuyFrameworkAPI.dll` (777 KB)
   - `bin\vs2010\x64\Release\CTesterFrameworkAPI.dll` (272 KB)

3. **Create Release Package**
   ```powershell
   # Creates organized release folder with timestamp
   powershell -ExecutionPolicy Bypass -File .\scripts\create-release.ps1
   ```

   Output location: `releases\AsirikuyFramework-YYYYMMDD-HHMMSS\`

### Release Package Contents

```
releases\AsirikuyFramework-YYYYMMDD-HHMMSS\
├── bin\
│   ├── AsirikuyFrameworkAPI.dll    # Main trading framework (777 KB)
│   └── CTesterFrameworkAPI.dll     # Backtesting framework (272 KB)
├── lib\
│   ├── TradingStrategies.lib       # 15 strategy libraries
│   ├── TALib_func.lib
│   └── ... (15 total libraries)
├── include\
│   └── *.h                         # 106 header files
├── docs\
│   └── *.md                        # Documentation files
├── MANIFEST.json                   # Package inventory
└── README.txt                      # Quick reference
```

### Build Verification

```powershell
# Verify DLL exports
$vsPath = "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC"
$dumpbin = Get-ChildItem -Path $vsPath -Recurse -Filter "dumpbin.exe" | Select-Object -First 1
& $dumpbin.FullName /exports .\bin\vs2010\x64\Release\AsirikuyFrameworkAPI.dll

# Expected exports:
# - initInstanceMQL4
# - initInstanceMQL5
# - mql4_runStrategy
# - mql5_runStrategy
# - jf_runStrategy
# (37 total exports)
```

---

## MT4 Installation & Setup

### Installing MT4

1. **Download MT4 from Your Broker**
   - Recommended brokers: IC Markets, Alpari, FXCM, Pepperstone
   - Download the installer (usually MetaTrader4Setup.exe)

2. **Install MT4**
   ```
   Default location: C:\Program Files\MetaTrader 4\
   
   Data folder: C:\Users\<username>\AppData\Roaming\MetaQuotes\Terminal\<HASH>\
   ```

3. **Identify Your Terminal Hash**
   ```powershell
   # Find all MT4 installations
   Get-ChildItem "$env:USERPROFILE\AppData\Roaming\MetaQuotes\Terminal\*\MQL4" -Directory
   
   # Example output:
   # C:\Users\amo31\AppData\Roaming\MetaQuotes\Terminal\E5D34929BCBAC87916D356DECB463DE3\MQL4
   ```

### Copy Foundation MQL4 Files (First-Time Setup Only)

**Only needed for brand new MT4 installations.** If you already have AutoBBS Expert Advisors in your MT4, skip this step.

The AutoBBS project includes a complete MQL4 foundation with Expert Advisors, scripts, and configuration files.

**For first-time setup, copy the entire MQL4 structure to your MT4 terminal:**

```powershell
# Set your MT4 terminal path
$mt4Terminal = "C:\Users\$env:USERNAME\AppData\Roaming\MetaQuotes\Terminal\<HASH>"

# Copy foundation MQL4 files from AutoBBS (first-time only)
Copy-Item "E:\AutoBBS\core\FrontEnds\MQL4\*" "$mt4Terminal\MQL4\" -Recurse -Force

# Verify Expert Advisors were copied
Get-ChildItem "$mt4Terminal\MQL4\Experts\*.ex4" | Measure-Object | Select-Object Count
```

**What gets copied:**
- `Experts\` - 80+ compiled Expert Advisors (.ex4) and source files (.mq4)
  - AutoBBS strategies (Ichikomo, MACD, BBS, Limit trading, etc.)
  - Strategy implementations (Atipaq, Ayotl, Coatl, Kantu, Qallaryi, etc.)
  - Utility EAs (mt4_heart_beat.ex4, account_risk_calculator.ex4)
- `Files\` - Configuration templates
- `Include\` - Header files and libraries
- `Indicators\` - Custom indicators
- `Scripts\` - Utility scripts
- `Libraries\` - MQL4 libraries (note: C++ DLLs go here separately)

**Important Notes:**
- This is a **one-time setup** for new MT4 installations
- The foundation includes both compiled (.ex4) and source (.mq4) files
- You can customize strategies by editing .mq4 files and recompiling in MetaEditor
- DLL files are deployed separately using the deployment script (see next section)

### Configure MT4 for DLL Usage

1. **Enable DLL Imports**
   - Open MT4
   - Go to **Tools → Options → Expert Advisors**
   - Check **"Allow DLL imports"**
   - Check **"Allow WebRequest for listed URL"** (if using web features)

2. **Add Trusted Paths**
   - In the same Expert Advisors tab
   - Click **"Add"** under "Allow DLL imports from the following paths"
   - Add: `C:\Users\<username>\AppData\Roaming\MetaQuotes\Terminal\<HASH>\MQL4\Libraries`

3. **Verify Settings**
   - Restart MT4 for changes to take effect
   - Check Experts log tab for any DLL-related errors

### MT4 Directory Structure

```
Terminal\<HASH>\
├── MQL4\
│   ├── Experts\          # Expert Advisors (.ex4 files) - 80+ from foundation
│   │   ├── AutoBBS*.mq4/ex4           # AutoBBS strategy variants
│   │   ├── mt4_heart_beat.ex4         # Heartbeat monitor EA
│   │   └── [strategy_name].mq4/ex4    # Individual strategies
│   ├── Libraries\        # DLL files (AsirikuyFrameworkAPI.dll, CTesterFrameworkAPI.dll)
│   ├── Include\          # Include files (.mqh) - from foundation
│   ├── Indicators\       # Custom indicators - from foundation
│   ├── Scripts\          # Utility scripts - from foundation
│   ├── Files\            # Config files (AsirikuyConfig.xml, *.hb heartbeats)
│   └── Logs\             # Framework logs (AsirikuyFramework.log)
├── Tester\               # Strategy tester data
└── config\               # MT4 configuration
```

---

## Deploying DLLs to MT4

### Prerequisites

- **First-time setup:** Copy foundation MQL4 files (see [Copy Foundation MQL4 Files](#copy-foundation-mql4-files-first-time-setup-only) above)
- **Regular updates:** Only need to deploy DLLs after building new releases

### Automatic Deployment (Recommended)

Use the PowerShell deployment script for automated, safe deployment with backups.

#### Method 1: Using Monitor Config File

```powershell
cd E:\AutoBBS

# Deploy using monitor configuration
.\scripts\deploy-to-mt4.ps1 -ConfigFile asirikuy_monitor\config\checker.config

# Or use specific account config
.\scripts\deploy-to-mt4.ps1 -ConfigFile asirikuy_monitor\config\checker_MT4_Real.config
```

#### Method 2: Specify MT4 Path

```powershell
# Deploy to specific MT4 installation
.\scripts\deploy-to-mt4.ps1 -MT4Path "C:\Users\amo31\AppData\Roaming\MetaQuotes\Terminal\E5D34929BCBAC87916D356DECB463DE3\MQL4\Libraries"
```

#### Method 3: Auto-Detection

```powershell
# Let script auto-detect MT4 installation
.\scripts\deploy-to-mt4.ps1
```

#### Dry Run (Preview Changes)

```powershell
# See what would be deployed without actually copying
.\scripts\deploy-to-mt4.ps1 -ConfigFile asirikuy_monitor\config\checker.config -DryRun
```

### Deployment Features

- ✅ **Automatic Backup:** Creates timestamped backup folder before deployment
- ✅ **Config Integration:** Reads MT4 path from monitor config
- ✅ **Auto-Detection:** Searches common MT4 installation paths
- ✅ **Safety Checks:** Validates paths and file integrity
- ✅ **Rollback Support:** Keeps backups for easy restoration

### Backup Management

Backups are stored in: `MQL4\Libraries\backup_YYYYMMDD-HHMMSS\`

```powershell
# List recent backups
Get-ChildItem "C:\Users\amo31\AppData\Roaming\MetaQuotes\Terminal\*\MQL4\Libraries\backup_*" -Directory | 
  Sort-Object CreationTime -Descending | 
  Select-Object Name, CreationTime -First 5

# Restore from backup (if needed)
Copy-Item ".\MQL4\Libraries\backup_20251206-212858\*.dll" ".\MQL4\Libraries\" -Force
```

### Manual Deployment

If you prefer manual deployment:

```powershell
# Copy DLLs from latest release
$release = Get-ChildItem "E:\AutoBBS\releases" -Directory | Sort-Object Name -Descending | Select-Object -First 1
$mt4Libs = "C:\Users\amo31\AppData\Roaming\MetaQuotes\Terminal\E5D34929BCBAC87916D356DECB463DE3\MQL4\Libraries"

# Copy DLLs
Copy-Item "$($release.FullName)\bin\*.dll" $mt4Libs -Force
```

---

## Configuring the Monitor

### Monitor Configuration Files

The monitor uses INI-style configuration files located in `asirikuy_monitor\config\`:

- `checker.config` - Default configuration (template)
- `checker_MT4_Real.config` - Real MT4 account configuration

### Configuration Structure

```ini
[general]
monitoringInterval = 60              # Check interval in seconds
useEmail = 1                         # Enable email alerts (0=disabled, 1=enabled)
useTelegram = 0                      # Enable Telegram alerts (0=disabled, 1=enabled)
fromEmail = your-email@gmail.com     # Sender email
toEmail = alert-email@gmail.com      # Alert recipient
emailLogin = your-email@gmail.com    # SMTP login
emailPassword = your-app-password    # App password (see below)
smtpServer = smtp.gmail.com:587      # SMTP server and port
weekOpenDay = 0                      # Sunday = 0, Monday = 1, etc.
weekOpenHour = 6                     # Trading week start hour (UTC)
weekCloseDay = 5                     # Friday = 5
weekCloseHour = 7                    # Trading week end hour (UTC)

[accounts]
accounts = ICMarkets,Pepperstone     # Comma-separated account list

[ICMarkets]
accountNumber = 1234567              # MT4 account number
path = C:\Users\...\Terminal\HASH    # MT4 terminal path
frontend = MT4                       # MT4 or MT5

[Pepperstone]
accountNumber = 9876543
path = C:\Users\...\Terminal\HASH2
frontend = MT4
```

### Setting Up Email Alerts

1. **Gmail App Password (Recommended)**
   - Go to Google Account → Security → 2-Step Verification
   - Scroll to "App passwords"
   - Generate password for "Mail" app
   - Use this password in config (NOT your Gmail password)

2. **Environment Variables (More Secure)**
   ```powershell
   # Set environment variables for sensitive data
   $env:EMAIL_PASSWORD = "your-app-password"
   $env:EMAIL_FROM = "your-email@gmail.com"
   $env:EMAIL_TO = "alert-email@gmail.com"
   $env:SMTP_SERVER = "smtp.gmail.com:587"
   ```

   The monitor will use environment variables if available, falling back to config file.

### Setting Up Telegram Alerts (Optional)

1. **Create Telegram Bot**
   - Message [@BotFather](https://t.me/botfather) on Telegram
   - Send `/newbot` and follow instructions
   - Save the bot token: `123456789:ABCdefGHIjklMNOpqrsTUVwxyz`

2. **Get Chat ID**
   - Message your bot
   - Visit: `https://api.telegram.org/bot<YOUR_BOT_TOKEN>/getUpdates`
   - Find `"chat":{"id":123456789}` in the response

3. **Configure Telegram**
   ```ini
   [general]
   useTelegram = 1
   telegramBotToken = 123456789:ABCdefGHIjklMNOpqrsTUVwxyz
   telegramChatId = 123456789
   ```

   Or use environment variables:
   ```powershell
   $env:TELEGRAM_BOT_TOKEN = "123456789:ABCdefGHIjklMNOpqrsTUVwxyz"
   $env:TELEGRAM_CHAT_ID = "123456789"
   ```

### Finding Your MT4 Terminal Path

```powershell
# Find all MT4 terminal folders
Get-ChildItem "$env:USERPROFILE\AppData\Roaming\MetaQuotes\Terminal" -Directory

# Example output:
# E5D34929BCBAC87916D356DECB463DE3
# 3F8A2B1C4D5E6F7890ABCDEF12345678

# Each folder represents one MT4 installation
# Use the full path in your config:
# path = C:\Users\amo31\AppData\Roaming\MetaQuotes\Terminal\E5D34929BCBAC87916D356DECB463DE3
```

---

## Running the Monitor

### Prerequisites

1. **Install Python 3.13** (Windows Store version recommended)
   ```powershell
   # Verify installation
   python3 --version
   # Should show: Python 3.13.x
   ```

2. **Install Dependencies**
   ```powershell
   cd E:\AutoBBS\asirikuy_monitor
   python3 -m pip install -r requirements.txt
   ```

   Required packages:
   - colorama >= 0.4.6
   - python-dotenv >= 1.0.0
   - psutil >= 5.9.0
   - requests >= 2.31.0 (for Telegram)

### Running the Monitor

#### Basic Usage

```powershell
cd E:\AutoBBS\asirikuy_monitor

# Run with default config
python3 checker.py

# Run with specific config
python3 checker.py -c config/checker_MT4_Real.config

# Show version
python3 checker.py --version
```

#### Running as Background Service

**Option 1: PowerShell Background Job**
```powershell
# Start monitor in background
Start-Job -ScriptBlock {
    Set-Location "E:\AutoBBS\asirikuy_monitor"
    python3 checker.py -c config/checker_MT4_Real.config
} -Name "AsirikuyMonitor"

# Check status
Get-Job -Name "AsirikuyMonitor"

# View output
Receive-Job -Name "AsirikuyMonitor" -Keep

# Stop monitor
Stop-Job -Name "AsirikuyMonitor"
Remove-Job -Name "AsirikuyMonitor"
```

**Option 2: Windows Task Scheduler**
```powershell
# Create scheduled task (runs at startup, always)
$action = New-ScheduledTaskAction -Execute "python3.exe" `
    -Argument "checker.py -c config/checker_MT4_Real.config" `
    -WorkingDirectory "E:\AutoBBS\asirikuy_monitor"

$trigger = New-ScheduledTaskTrigger -AtStartup

$settings = New-ScheduledTaskSettingsSet -AllowStartIfOnBatteries -DontStopIfGoingOnBatteries

Register-ScheduledTask -TaskName "AsirikuyMonitor" `
    -Action $action -Trigger $trigger -Settings $settings `
    -Description "Asirikuy MT4 Monitoring Service"

# Start task
Start-ScheduledTask -TaskName "AsirikuyMonitor"

# Check status
Get-ScheduledTask -TaskName "AsirikuyMonitor"

# Remove task
Unregister-ScheduledTask -TaskName "AsirikuyMonitor" -Confirm:$false
```

### What the Monitor Does

1. **Heartbeat Monitoring**
   - Checks `MQL4/Files/*.hb` heartbeat files every 60 seconds
   - Verifies EA is running and updating timestamps
   - Alerts if heartbeat stops for more than 2.5 × monitoring interval

2. **Log Monitoring**
   - Scans `MQL4/Logs/AsirikuyFramework.log` for errors
   - Detects keywords: "Error", "Emergency", "Critical"
   - Sends alerts on detection

3. **Trading Hours**
   - Only monitors during configured trading hours
   - Default: Sunday 6:00 - Friday 7:00 (UTC)
   - Sleeps outside trading hours

4. **Alert Actions**
   - Sends email notifications (if enabled)
   - Sends Telegram alerts (if enabled)
   - Kills MT4 process on critical errors (Windows only)
   - Logs all events to `log/monitor.log`

### Monitor Output

```
Local Hour = 21
Local Day = 4
Checking EA_Instance_001.hb...
Current Time: 2025-12-06 21:30:00, Current UTC Time 2025-12-06 13:30:00, Last heart-beat time: 2025-12-06 13:29:45, Difference in 150.0 secs: 15.0
Last execution Fri, 06 Dec 2025 21:30:00, sleeping for 60 secs.
```

### Stopping the Monitor

- **Foreground:** Press `Ctrl+C`
- **Background Job:** `Stop-Job -Name "AsirikuyMonitor"`
- **Scheduled Task:** `Stop-ScheduledTask -TaskName "AsirikuyMonitor"`

---

## Troubleshooting

### Build Issues

**Error: "MSBuild not found"**
```powershell
# Add MSBuild to PATH
$env:PATH += ";C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin"
```

**Error: "LINK : fatal error LNK1181: cannot open input file 'talib_cdr.lib'"**
- Ensure TA-Lib is installed in `vendor/` folder
- Check `premake4.lua` for correct library paths

### Deployment Issues

**Error: "MT4 path does not exist"**
```powershell
# Verify MT4 installation
Test-Path "C:\Users\$env:USERNAME\AppData\Roaming\MetaQuotes\Terminal"

# List all MT4 terminals
Get-ChildItem "$env:USERPROFILE\AppData\Roaming\MetaQuotes\Terminal" -Directory
```

**DLLs not loading in MT4**
1. Check "Allow DLL imports" is enabled in MT4
2. Verify DLL is in correct folder: `MQL4\Libraries\`
3. Check MT4 Experts log for error messages
4. Ensure DLL is 64-bit if using 64-bit MT4

### Monitor Issues

**Error: "Config file not found"**
```powershell
# Check config file exists
Test-Path "E:\AutoBBS\asirikuy_monitor\config\checker.config"

# Use absolute path
python3 checker.py -c "E:\AutoBBS\asirikuy_monitor\config\checker.config"
```

**Email alerts not working**
1. Verify Gmail app password (not regular password)
2. Check environment variables: `$env:EMAIL_PASSWORD`
3. Test SMTP connection:
   ```powershell
   python3 -c "import smtplib; s = smtplib.SMTP('smtp.gmail.com', 587); s.starttls(); print('OK')"
   ```

**Telegram alerts not working**
1. Verify bot token is correct
2. Check you've messaged the bot first
3. Test Telegram API:
   ```powershell
   $token = "YOUR_BOT_TOKEN"
   $chatId = "YOUR_CHAT_ID"
   Invoke-RestMethod "https://api.telegram.org/bot$token/sendMessage?chat_id=$chatId&text=Test"
   ```

**Monitor not detecting heartbeat files**
1. Verify MT4 is running and EA is active
2. Check heartbeat files exist:
   ```powershell
   Get-ChildItem "C:\Users\...\Terminal\HASH\MQL4\Files\*.hb"
   ```
3. Ensure path in config matches actual MT4 installation

### Performance Issues

**High CPU usage**
- Increase `monitoringInterval` in config (e.g., from 60 to 120 seconds)
- Reduce number of monitored accounts

**Memory leaks**
- Restart monitor periodically using scheduled task
- Update to latest version

---

## Best Practices

### Release Management

1. **Tag releases in Git**
   ```bash
   git tag -a v1.0.0 -m "Release 1.0.0 - Windows build"
   git push origin v1.0.0
   ```

2. **Keep release archives**
   - Store ZIP files of releases
   - Document changes in CHANGELOG.md

3. **Version numbering**
   - Major.Minor.Patch (e.g., 1.0.0)
   - Increment patch for bug fixes
   - Increment minor for new features
   - Increment major for breaking changes

### Monitor Configuration

1. **Use environment variables for secrets**
   - Never commit passwords to Git
   - Use .env files or system environment variables

2. **Test in demo account first**
   - Verify alerts work correctly
   - Ensure no false alarms

3. **Regular backups**
   - Backup config files
   - Keep deployment backups for 30 days

### MT4 Maintenance

1. **Update DLLs after code changes**
   ```powershell
   # Rebuild and deploy in one command
   .\scripts\build-parallel-simple.bat
   .\scripts\deploy-to-mt4.ps1 -ConfigFile asirikuy_monitor\config\checker.config
   ```

2. **Monitor log files**
   - Check `MQL4\Logs\AsirikuyFramework.log` regularly
   - Review monitor logs: `asirikuy_monitor\log\monitor.log`

3. **Restart MT4 weekly**
   - Prevents memory leaks
   - Updates market data

---

## Quick Reference

### Common Commands

```powershell
# Build release
.\scripts\build-parallel-simple.bat
powershell -ExecutionPolicy Bypass -File .\scripts\create-release.ps1

# Copy foundation MQL4 files to MT4 (first-time setup only)
$mt4Terminal = "C:\Users\$env:USERNAME\AppData\Roaming\MetaQuotes\Terminal\<HASH>"
Copy-Item "E:\AutoBBS\core\FrontEnds\MQL4\*" "$mt4Terminal\MQL4\" -Recurse -Force

# Deploy DLLs to MT4 (after each build)
.\scripts\deploy-to-mt4.ps1 -ConfigFile asirikuy_monitor\config\checker.config

# Run monitor
cd asirikuy_monitor
python3 checker.py -c config/checker_MT4_Real.config

# Check monitor status
Get-Process python | Where-Object {$_.CommandLine -like "*checker.py*"}

# View recent logs
Get-Content "asirikuy_monitor\log\monitor.log" -Tail 50
```

### File Locations

```
E:\AutoBBS\
├── bin\vs2010\x64\Release\       # Built DLLs
├── releases\                     # Release packages
├── scripts\
│   ├── build-parallel-simple.bat # Build script
│   ├── create-release.ps1       # Release packaging
│   └── deploy-to-mt4.ps1        # Deployment script
└── asirikuy_monitor\
    ├── checker.py               # Monitor main script
    ├── config\                  # Configuration files
    └── log\                     # Monitor logs

C:\Users\<username>\AppData\Roaming\MetaQuotes\Terminal\<HASH>\
├── MQL4\
│   ├── Libraries\               # DLLs here
│   ├── Files\                   # Heartbeat files, configs
│   └── Logs\                    # Framework logs
```

### Support & Resources

- **Documentation:** `E:\AutoBBS\docs\`
- **Build Status:** `docs\WINDOWS_DLL_BUILD_TASKS.md`
- **API Reference:** `docs\IStrategy-API.md`
- **Git Branch:** `window-build`

---

**Document Version:** 1.0  
**Last Updated:** December 6, 2025  
**Author:** AutoBBS Team
