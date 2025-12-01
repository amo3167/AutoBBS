# Asirikuy Monitor

A monitoring tool for trading platforms (MT4 and AT) that checks heartbeats and log files for errors, sending email alerts when issues are detected.

## Features

- **Heartbeat Monitoring**: Monitors trading platform heartbeats to detect if systems are running
- **Error Detection**: Scans log files for errors, emergencies, and critical issues
- **Email Alerts**: Sends email notifications when problems are detected
- **Process Management**: Automatically kills MT4 processes when errors are detected
- **Cross-Platform**: Works on Windows, macOS, and Linux (with Python 3)
- **Error Recovery**: Automatic retry logic with exponential backoff for file operations and email sending
- **Robust Logging**: Log rotation, configurable levels, and structured logging

## Requirements

- **Python**: 3.8 or higher
- **Dependencies**: See `requirements.txt`

## Installation

1. Clone or download this repository
2. Create a virtual environment (recommended):
   ```bash
   python3 -m venv venv
   source venv/bin/activate  # On Windows: venv\Scripts\activate
   ```
3. Install dependencies:
   ```bash
   pip install -r requirements.txt
   ```

**Note**: If you prefer not to use a virtual environment, you can install with `--user` flag:
```bash
pip install --user -r requirements.txt
```

## Configuration

1. Copy `config/checker.config.example` to `config/checker.config`
2. Edit the configuration file:
   ```ini
   [general]
   monitoringInterval = 60  # Check interval in seconds
   useEmail = 1             # Enable email alerts (1) or disable (0)
   fromEmail = your-email@gmail.com
   toEmail = recipient@gmail.com
   emailLogin = your-email@gmail.com
   emailPassword = your-password
   smtpServer = smtp.gmail.com:587
   weekOpenDay = 0          # Monday (0-6, Monday=0)
   weekOpenHour = 6         # 6 AM
   weekCloseDay = 5         # Friday
   weekCloseHour = 7        # 7 PM

   # Optional: Logging configuration
   # logLevel = INFO          # DEBUG, INFO, WARNING, ERROR, CRITICAL
   # logMaxBytes = 10485760   # 10 MB (default)
   # logBackupCount = 5       # Number of backup log files
   # logConsole = false        # Enable console logging
   # logConsoleLevel = WARNING # Console log level

   [accounts]
   accounts = account1, account2

   [account1]
   accountNumber = 123456
   path = /path/to/mt4/data
   frontend = MT4  # or AT
   ```

## Usage

### Basic Usage

```bash
python3 checker.py
```

### With Custom Config File

```bash
python3 checker.py -c /path/to/config/file.config
```

### Check Version

```bash
python3 checker.py -v
```

## How It Works

1. **Heartbeat Check**: 
   - Looks for `.hb` (heartbeat) files in the platform's data directory
   - Checks if the heartbeat timestamp is within the monitoring interval
   - If heartbeat is stale, triggers alert and kills MT4 if needed

2. **Error Detection**:
   - Reads the last line of the platform's log file
   - Checks for "Error", "Emergency", or "Critical" keywords
   - Sends email alert if error detected

3. **Monitoring Loop**:
   - Runs continuously, checking at the configured interval
   - Only monitors during configured trading hours
   - Logs all activities to `./log/monitor.log`

## File Structure

```
asirikuy_monitor/
├── checker.py              # Main monitoring script
├── include/
│   ├── asirikuy.py        # Core library functions
│   ├── email_security.py  # Email security utilities
│   ├── misc.py            # Utility functions
│   └── process_manager.py # Cross-platform process management
├── config/
│   └── checker.config     # Configuration file template
├── log/                   # Log files directory (created automatically)
├── tests/                 # Test suite
├── requirements.txt       # Python dependencies
└── README.md             # This file
```

## Platform Support

### Cross-Platform
The monitor now works on **Windows, macOS, and Linux**. Process management uses `psutil` instead of Windows-specific commands.

### MT4 (MetaTrader 4)
- Heartbeat files: `{path}/MQL4/Files/*.hb`
- Log files: `{path}/MQL4/Logs/AsirikuyFramework.log`
- Process: `terminal.exe` (Windows) or `terminal` (Linux/macOS)

### AT (Asirikuy Trader)
- Heartbeat files: `{path}/tmp/*.hb`
- Log files: `{path}/log/{accountNumber}AsirikuyFramework.log`

## Security

### Email Security Features

The monitor includes comprehensive email security:

- **Email Validation**: All email addresses are validated before sending (RFC 5322 compliant)
- **Content Sanitization**: Email content is sanitized to prevent XSS and injection attacks
- **Rate Limiting**: Per-recipient rate limiting (5 emails per 5 minutes) prevents spam
- **Secure SMTP**: Uses TLS/SSL encryption for email transmission

### Environment Variables (Recommended)

For better security, use environment variables for email credentials instead of storing them in the config file:

1. Copy `.env.example` to `.env`:
   ```bash
   cp .env.example .env
   ```

2. Edit `.env` and add your credentials:
   ```bash
   EMAIL_FROM=your-email@gmail.com
   EMAIL_TO=recipient@gmail.com
   EMAIL_LOGIN=your-email@gmail.com
   EMAIL_PASSWORD=your-app-password
   SMTP_SERVER=smtp.gmail.com:587
   ```

3. The monitor will automatically use environment variables if set, falling back to config file if not.

**Note**: The `.env` file is automatically ignored by git (see `.gitignore`).

### Config File (Legacy)

If you prefer to use the config file:
- ⚠️ **Warning**: Credentials are stored in plain text
- Restrict file permissions: `chmod 600 config/checker.config`
- Consider using application-specific passwords for Gmail
- Use a dedicated email account for monitoring
- **Do not commit config files with credentials to version control**

## Troubleshooting

### Email Not Sending
- Check SMTP server settings
- Verify email credentials
- Check firewall/network settings
- For Gmail: Enable "Less secure app access" or use App Password

### Heartbeat Not Detected
- Verify the path to heartbeat files is correct
- Check file permissions
- Ensure the trading platform is running
- Verify heartbeat files are being created

### Log File Not Found
- Verify the log file path in configuration
- Check file permissions
- Ensure the trading platform is generating logs

## Development

### Setup Development Environment

```bash
pip install -r requirements-dev.txt
```

### Code Quality

```bash
# Format code
black checker.py include/

# Lint code
flake8 checker.py include/

# Type checking
mypy checker.py include/
```

## Migration from Python 2

This project has been migrated from Python 2 to Python 3. See `specs/PYTHON2_ANALYSIS.md` and `specs/MIGRATION_PROGRESS.md` for details.

## License

[Add your license here]

## Deployment

For detailed deployment instructions, see [DEPLOYMENT.md](DEPLOYMENT.md).

Quick start:
1. Install dependencies: `pip install -r requirements.txt`
2. Set up `.env` file with credentials
3. Configure `config/checker.config`
4. Run: `python3 checker.py`

## Support

### Getting Help

1. Check `log/monitor.log` for error messages
2. Review [DEPLOYMENT.md](DEPLOYMENT.md) for deployment guide
3. Review `specs/` directory for technical documentation

### Reporting Issues

When reporting issues, include:
- Python version
- Operating system
- Error messages from logs
- Configuration (without credentials)

---

**Version**: 0.05  
**Last Updated**: December 2024  
**Changelog**: See [CHANGELOG.md](CHANGELOG.md)

