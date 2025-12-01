# Asirikuy Monitor - Deployment Guide

**Version**: 0.05  
**Last Updated**: December 2024

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Installation](#installation)
3. [Configuration](#configuration)
4. [Running the Monitor](#running-the-monitor)
5. [Production Deployment](#production-deployment)
6. [Troubleshooting](#troubleshooting)
7. [Maintenance](#maintenance)

---

## Prerequisites

### System Requirements

- **Operating System**: Windows, macOS, or Linux
- **Python**: 3.8 or higher
- **Disk Space**: ~50 MB (excluding log files)
- **Memory**: Minimal (monitor runs lightweight)

### Required Software

1. **Python 3.8+**
   ```bash
   # Check Python version
   python3 --version
   # Should show Python 3.8.0 or higher
   ```

2. **pip** (Python package manager)
   ```bash
   # Usually comes with Python
   pip3 --version
   ```

3. **Git** (optional, for cloning repository)
   ```bash
   git --version
   ```

### Access Requirements

- Read access to MT4/AT data directories
- Write access to log directory
- Network access for email (SMTP)

---

## Installation

### Step 1: Get the Code

**Option A: Clone from Repository**
```bash
git clone <repository-url>
cd asirikuy_monitor
```

**Option B: Download and Extract**
```bash
# Download ZIP and extract
cd asirikuy_monitor
```

### Step 2: Create Virtual Environment (Recommended)

```bash
# Create virtual environment
python3 -m venv venv

# Activate virtual environment
# On macOS/Linux:
source venv/bin/activate

# On Windows:
venv\Scripts\activate
```

**Note**: You should see `(venv)` in your terminal prompt when activated.

### Step 3: Install Dependencies

```bash
# Make sure virtual environment is activated
pip install -r requirements.txt
```

**Expected output:**
```
Collecting colorama>=0.4.6
Collecting python-dotenv>=1.0.0
Collecting psutil>=5.9.0
Successfully installed colorama-0.4.6 python-dotenv-1.2.1 psutil-7.1.3
```

### Step 4: Verify Installation

```bash
# Test version command
python3 checker.py -v

# Expected output:
# Asirikuy Monitor v0.05
```

---

## Configuration

### Step 1: Set Up Environment Variables (Recommended)

1. **Copy the example file:**
   ```bash
   cp .env.example .env
   ```

2. **Edit `.env` file:**
   ```bash
   # On macOS/Linux
   nano .env
   # or
   vi .env
   
   # On Windows
   notepad .env
   ```

3. **Add your credentials:**
   ```env
   EMAIL_FROM=your-email@gmail.com
   EMAIL_TO=recipient@gmail.com
   EMAIL_LOGIN=your-email@gmail.com
   EMAIL_PASSWORD=your-app-password
   SMTP_SERVER=smtp.gmail.com:587
   ```

   **For Gmail:**
   - Use an [App Password](https://myaccount.google.com/apppasswords) instead of your regular password
   - Enable "Less secure app access" if not using App Password (not recommended)

### Step 2: Configure Accounts

1. **Copy the example config:**
   ```bash
   cp config/checker.config.example config/checker.config
   ```

2. **Edit `config/checker.config`:**
   ```bash
   nano config/checker.config
   ```

3. **Update the configuration:**

   ```ini
   [general]
   monitoringInterval = 60          # Check every 60 seconds
   useEmail = 1                     # 1 = enable email, 0 = disable
   # Email settings (use .env file instead - more secure)
   fromEmail = your-email@gmail.com
   toEmail = recipient@gmail.com
   emailLogin = your-email@gmail.com
   emailPassword = your-password
   smtpServer = smtp.gmail.com:587
   weekOpenDay = 0                  # Monday (0-6, Monday=0)
   weekOpenHour = 6                 # 6 AM
   weekCloseDay = 5                 # Friday
   weekCloseHour = 7                # 7 PM

   [accounts]
   accounts = account1, account2     # Comma-separated list

   [account1]
   accountNumber = 123456
   path = /path/to/mt4/data          # Full path to MT4/AT data
   frontend = MT4                    # or AT

   [account2]
   accountNumber = 789012
   path = /path/to/at/data
   frontend = AT
   ```

### Step 3: Verify Paths

**For MT4:**
- Heartbeat files: `{path}/MQL4/Files/*.hb`
- Log files: `{path}/MQL4/Logs/AsirikuyFramework.log`

**For AT:**
- Heartbeat files: `{path}/tmp/*.hb`
- Log files: `{path}/log/{accountNumber}AsirikuyFramework.log`

**Test paths:**
```bash
# Check if directories exist
ls /path/to/mt4/data/MQL4/Files
ls /path/to/mt4/data/MQL4/Logs
```

### Step 4: Create Log Directory

```bash
mkdir -p log
chmod 755 log
```

---

## Running the Monitor

### Basic Usage

```bash
# Activate virtual environment first
source venv/bin/activate  # macOS/Linux
# or
venv\Scripts\activate     # Windows

# Run with default config
python3 checker.py

# Run with custom config
python3 checker.py -c /path/to/config/file.config

# Check version
python3 checker.py -v
```

### Running in Background

**On macOS/Linux:**

```bash
# Using nohup
nohup python3 checker.py > monitor_output.log 2>&1 &

# Using screen
screen -S monitor
python3 checker.py
# Press Ctrl+A then D to detach

# Using tmux
tmux new -s monitor
python3 checker.py
# Press Ctrl+B then D to detach
```

**On Windows:**

```powershell
# Using PowerShell background job
Start-Process python -ArgumentList "checker.py" -WindowStyle Hidden

# Or use Task Scheduler for automatic startup
```

### Running as a Service

**On Linux (systemd):**

1. **Create service file** `/etc/systemd/system/asirikuy-monitor.service`:
   ```ini
   [Unit]
   Description=Asirikuy Monitor
   After=network.target

   [Service]
   Type=simple
   User=your-username
   WorkingDirectory=/path/to/asirikuy_monitor
   Environment="PATH=/path/to/asirikuy_monitor/venv/bin"
   ExecStart=/path/to/asirikuy_monitor/venv/bin/python3 checker.py
   Restart=always
   RestartSec=10

   [Install]
   WantedBy=multi-user.target
   ```

2. **Enable and start:**
   ```bash
   sudo systemctl daemon-reload
   sudo systemctl enable asirikuy-monitor
   sudo systemctl start asirikuy-monitor
   sudo systemctl status asirikuy-monitor
   ```

**On macOS (launchd):**

1. **Create plist file** `~/Library/LaunchAgents/com.asirikuy.monitor.plist`:
   ```xml
   <?xml version="1.0" encoding="UTF-8"?>
   <!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
   <plist version="1.0">
   <dict>
       <key>Label</key>
       <string>com.asirikuy.monitor</string>
       <key>ProgramArguments</key>
       <array>
           <string>/path/to/asirikuy_monitor/venv/bin/python3</string>
           <string>/path/to/asirikuy_monitor/checker.py</string>
       </array>
       <key>WorkingDirectory</key>
       <string>/path/to/asirikuy_monitor</string>
       <key>RunAtLoad</key>
       <true/>
       <key>KeepAlive</key>
       <true/>
   </dict>
   </plist>
   ```

2. **Load and start:**
   ```bash
   launchctl load ~/Library/LaunchAgents/com.asirikuy.monitor.plist
   launchctl start com.asirikuy.monitor
   ```

---

## Production Deployment

### Security Checklist

- [ ] Use environment variables for credentials (`.env` file)
- [ ] Restrict `.env` file permissions: `chmod 600 .env`
- [ ] Restrict config file permissions: `chmod 600 config/checker.config`
- [ ] Use application-specific passwords (Gmail App Passwords)
- [ ] Use dedicated email account for monitoring
- [ ] Enable firewall rules if needed
- [ ] Review log files regularly
- [ ] Keep Python and dependencies updated

### Performance Considerations

1. **Monitoring Interval:**
   - Default: 60 seconds
   - Adjust based on needs: `monitoringInterval = 30` (more frequent) or `120` (less frequent)

2. **Log Rotation:**
   - Monitor log file size: `log/monitor.log`
   - Set up log rotation if needed:
     ```bash
     # Add to crontab
     0 0 * * * find /path/to/asirikuy_monitor/log -name "*.log" -size +100M -exec gzip {} \;
     ```

3. **Resource Usage:**
   - Monitor is lightweight
   - Typical memory usage: < 50 MB
   - CPU usage: < 1% (mostly idle)

### Backup Recommendations

1. **Configuration Files:**
   ```bash
   # Backup config and .env (without passwords in version control)
   tar -czf config-backup-$(date +%Y%m%d).tar.gz config/checker.config.example .env.example
   ```

2. **Log Files:**
   - Archive old logs regularly
   - Keep last 30 days of logs

---

## Troubleshooting

### Common Issues

#### 1. "ModuleNotFoundError: No module named 'colorama'"

**Solution:**
```bash
# Activate virtual environment
source venv/bin/activate

# Install dependencies
pip install -r requirements.txt
```

#### 2. "Error reading config file"

**Solution:**
- Check config file path is correct
- Verify config file format (INI format)
- Check file permissions: `chmod 644 config/checker.config`

#### 3. "Heartbeat directory does not exist"

**Solution:**
- Verify path in config file is correct
- Check path exists: `ls /path/to/mt4/data/MQL4/Files`
- Ensure MT4/AT is running and creating heartbeat files

#### 4. "Error sending email"

**Solution:**
- Check `.env` file has correct credentials
- For Gmail: Use App Password instead of regular password
- Verify SMTP server and port: `smtp.gmail.com:587`
- Check firewall/network allows SMTP connections
- Test email manually:
  ```python
  python3 -c "from include.asirikuy import sendemail; sendemail('from@email.com', 'to@email.com', '', 'Test', 'Test message', 'login@email.com', 'password', 'smtp.gmail.com:587')"
  ```

#### 5. "No MT4 processes found to kill"

**Solution:**
- This is a warning, not an error
- MT4 may have already closed
- Check if MT4 process name matches (terminal.exe on Windows, terminal on Linux/macOS)

#### 6. "Permission denied" errors

**Solution:**
- Check file/directory permissions
- Ensure user has read access to MT4/AT data
- Ensure user has write access to log directory

### Debug Mode

Enable more verbose logging by checking `log/monitor.log`:

```bash
# View live log
tail -f log/monitor.log

# Search for errors
grep -i error log/monitor.log

# View last 100 lines
tail -n 100 log/monitor.log
```

### Testing Configuration

```bash
# Test config file reading
python3 -c "from include.asirikuy import readConfigFile; config = readConfigFile('config/checker.config'); print('Config OK')"

# Test environment variables
python3 -c "import os; from dotenv import load_dotenv; load_dotenv(); print('EMAIL_FROM:', os.getenv('EMAIL_FROM'))"
```

---

## Maintenance

### Regular Tasks

1. **Weekly:**
   - Review log files for errors
   - Check email notifications are working
   - Verify monitor is running

2. **Monthly:**
   - Update Python dependencies: `pip install --upgrade -r requirements.txt`
   - Review and archive old log files
   - Check disk space

3. **Quarterly:**
   - Review configuration
   - Update email credentials if needed
   - Test disaster recovery procedures

### Updating the Monitor

```bash
# Pull latest changes (if using git)
git pull

# Update dependencies
source venv/bin/activate
pip install --upgrade -r requirements.txt

# Test
python3 checker.py -v
```

### Monitoring the Monitor

**Check if monitor is running:**
```bash
# On Linux/macOS
ps aux | grep checker.py

# Check systemd service
sudo systemctl status asirikuy-monitor

# Check log file
tail -f log/monitor.log
```

**Set up alerts:**
- Monitor log file for "ERROR" messages
- Set up external monitoring for the monitor process
- Use email notifications (already built-in)

---

## Quick Reference

### File Locations

```
asirikuy_monitor/
├── checker.py              # Main script
├── .env                    # Environment variables (credentials)
├── config/
│   └── checker.config      # Configuration file
├── log/
│   └── monitor.log         # Monitor log file
├── venv/                   # Virtual environment
└── requirements.txt        # Dependencies
```

### Important Commands

```bash
# Activate virtual environment
source venv/bin/activate

# Run monitor
python3 checker.py

# Run with custom config
python3 checker.py -c config/checker.config

# Check version
python3 checker.py -v

# View logs
tail -f log/monitor.log
```

### Environment Variables

```env
EMAIL_FROM=your-email@gmail.com
EMAIL_TO=recipient@gmail.com
EMAIL_LOGIN=your-email@gmail.com
EMAIL_PASSWORD=your-app-password
SMTP_SERVER=smtp.gmail.com:587
```

---

## Support

### Getting Help

1. Check `log/monitor.log` for error messages
2. Review this deployment guide
3. Check `README.md` for usage information
4. Review `specs/` directory for technical documentation

### Reporting Issues

When reporting issues, include:
- Python version: `python3 --version`
- Operating system and version
- Error messages from `log/monitor.log`
- Configuration (without credentials)
- Steps to reproduce

---

**Last Updated**: December 2024  
**Version**: 0.05

