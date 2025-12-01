    # Asirikuy Monitor - API Documentation

**Version**: 0.05  
**Last Updated**: December 2024

## Table of Contents

1. [Overview](#overview)
2. [Main Module (`checker.py`)](#main-module-checkerpy)
3. [Core Library (`include/asirikuy.py`)](#core-library-includeasirikuypy)
4. [Configuration Management (`include/config_manager.py`)](#configuration-management-includeconfig_managerpy)
5. [Email Security (`include/email_security.py`)](#email-security-includeemail_securitypy)
6. [Logging Configuration (`include/logging_config.py`)](#logging-configuration-includelogging_configpy)
7. [Platform Utilities (`include/platform_utils.py`)](#platform-utilities-includeplatform_utilspy)
8. [Process Management (`include/process_manager.py`)](#process-management-includeprocess_managerpy)
9. [Retry Mechanism (`include/retry_mechanism.py`)](#retry-mechanism-includeretry_mechanismpy)
10. [Miscellaneous Utilities (`include/misc.py`)](#miscellaneous-utilities-includemiscpy)
11. [Configuration Options](#configuration-options)

---

## Overview

The Asirikuy Monitor API consists of several modules that work together to provide monitoring, error detection, and alerting capabilities for trading platforms.

### Module Structure

```
asirikuy_monitor/
├── checker.py                    # Main entry point
└── include/
    ├── asirikuy.py               # Core library functions
    ├── config_manager.py         # Configuration management
    ├── email_security.py         # Email validation & security
    ├── logging_config.py         # Logging setup
    ├── platform_utils.py         # Platform detection
    ├── process_manager.py        # Process management
    ├── retry_mechanism.py        # Retry logic
    └── misc.py                   # Utility functions
```

---

## Main Module (`checker.py`)

### Functions

#### `main() -> None`

Main entry point for Asirikuy Monitor. Parses command-line arguments, loads configuration, and starts the monitoring loop.

**Command-Line Arguments:**
- `-c, --config`: Path to configuration file (default: `./config/checker.config`)
- `-v, --version`: Print version information and exit

**Example:**
```python
if __name__ == "__main__":
    main()
```

**Usage:**
```bash
python3 checker.py
python3 checker.py -c /path/to/config.config
python3 checker.py -v
```

#### `version() -> None`

Print version information and platform details, then exit.

**Example:**
```python
version()
# Output:
# Asirikuy Monitor v0.05
# Platform: Macos (Darwin 24.6.0)
```

---

## Core Library (`include/asirikuy.py`)

### Functions

#### `sendemail(from_addr, to_addr_list, cc_addr_list, subject, message, login, password, smtpserver)`

Send email with security enhancements (validation, sanitization, rate limiting).

**Parameters:**
- `from_addr` (str): Sender email address
- `to_addr_list` (str or list): Recipient email address(es)
- `cc_addr_list` (str or list): CC email address(es)
- `subject` (str): Email subject line
- `message` (str): Email message body
- `login` (str): SMTP login username
- `password` (str): SMTP password
- `smtpserver` (str): SMTP server address (format: `host:port`)

**Returns:**
- `dict`: Dictionary of failed recipients (empty if all succeeded)

**Raises:**
- `ValueError`: If email addresses are invalid or SMTP server format is invalid

**Security Features:**
- Email address validation (RFC 5322 compliant)
- Content sanitization (XSS prevention)
- Subject sanitization
- Per-recipient rate limiting (5 emails per 5 minutes)

**Example:**
```python
from include.asirikuy import sendemail

problems = sendemail(
    from_addr="sender@example.com",
    to_addr_list=["recipient@example.com"],
    cc_addr_list=[],
    subject="Alert: Trading Platform Error",
    message="An error was detected in the trading platform.",
    login="sender@example.com",
    password="app-password",
    smtpserver="smtp.gmail.com:587"
)

if problems:
    print(f"Failed to send to: {problems}")
```

#### `readConfigFile(configPath)`

Read and parse configuration file.

**Parameters:**
- `configPath` (str): Path to configuration file

**Returns:**
- `configparser.RawConfigParser`: Parsed configuration object

**Raises:**
- `FileNotFoundError`: If config file doesn't exist
- `ValueError`: If config file is invalid

**Example:**
```python
from include.asirikuy import readConfigFile

config = readConfigFile("./config/checker.config")
interval = config.get('general', 'monitoringInterval')
```

---

## Configuration Management (`include/config_manager.py`)

### Constants

#### `DEFAULT_CONFIG`

Dictionary containing default configuration values for all sections.

**Structure:**
```python
{
    'general': {
        'monitoringInterval': '60',
        'useEmail': '0',
        'fromEmail': '',
        'toEmail': '',
        'emailLogin': '',
        'emailPassword': '',
        'smtpServer': 'smtp.gmail.com:587',
        'weekOpenDay': '0',
        'weekOpenHour': '6',
        'weekCloseDay': '5',
        'weekCloseHour': '7',
        'logLevel': 'INFO',
        'logFile': './log/monitor.log',
        'logMaxBytes': '10485760',
        'logBackupCount': '5',
        'logConsole': 'false',
        'logConsoleLevel': 'WARNING',
    }
}
```

### Functions

#### `validate_config(config: configparser.RawConfigParser) -> List[str]`

Validate configuration file structure and values.

**Parameters:**
- `config`: ConfigParser instance to validate

**Returns:**
- `List[str]`: List of validation error messages (empty if valid)

**Example:**
```python
from include.config_manager import validate_config, load_config_with_defaults

config = load_config_with_defaults("./config/checker.config")
errors = validate_config(config)
if errors:
    for error in errors:
        print(f"Config error: {error}")
```

#### `load_config_with_defaults(config_path: str) -> configparser.RawConfigParser`

Load configuration file with default values applied.

**Parameters:**
- `config_path`: Path to configuration file (str or Path)

**Returns:**
- `configparser.RawConfigParser`: ConfigParser instance with defaults applied

**Raises:**
- `FileNotFoundError`: If config file doesn't exist
- `ValueError`: If config is invalid

**Example:**
```python
from include.config_manager import load_config_with_defaults

config = load_config_with_defaults("./config/checker.config")
# Default values are automatically applied for missing options
```

#### `get_config_value(config: configparser.RawConfigParser, section: str, option: str, default: Optional[str] = None) -> str`

Get configuration value with optional default.

**Parameters:**
- `config`: ConfigParser instance
- `section`: Configuration section name
- `option`: Configuration option name
- `default`: Default value if option not found (optional)

**Returns:**
- `str`: Configuration value or default

**Example:**
```python
from include.config_manager import get_config_value

interval = get_config_value(config, 'general', 'monitoringInterval', '60')
```

---

## Email Security (`include/email_security.py`)

### Functions

#### `validate_email_address(email: str) -> bool`

Validate email address format (RFC 5322 compliant).

**Parameters:**
- `email`: Email address string

**Returns:**
- `bool`: True if valid, False otherwise

**Example:**
```python
from include.email_security import validate_email_address

if validate_email_address("user@example.com"):
    print("Valid email")
```

#### `sanitize_email_content(content: str) -> str`

Sanitize email content to prevent XSS and injection attacks.

**Parameters:**
- `content`: Email content string

**Returns:**
- `str`: Sanitized content

**Features:**
- HTML entity escaping
- Null byte removal
- Control character removal
- Length limiting (10KB max)

**Example:**
```python
from include.email_security import sanitize_email_content

safe_content = sanitize_email_content(user_input)
```

#### `sanitize_email_subject(subject: str) -> str`

Sanitize email subject line.

**Parameters:**
- `subject`: Email subject string

**Returns:**
- `str`: Sanitized subject

**Example:**
```python
from include.email_security import sanitize_email_subject

safe_subject = sanitize_email_subject(user_input)
```

#### `check_rate_limit(recipient: str) -> Tuple[bool, str]`

Check if email can be sent to recipient (rate limiting).

**Parameters:**
- `recipient`: Recipient email address

**Returns:**
- `Tuple[bool, str]`: (allowed, reason) - True if allowed, False with reason if not

**Rate Limits:**
- 5 emails per recipient per 5 minutes

**Example:**
```python
from include.email_security import check_rate_limit

allowed, reason = check_rate_limit("recipient@example.com")
if not allowed:
    print(f"Rate limit exceeded: {reason}")
```

#### `reset_rate_limit(recipient: str) -> None`

Reset rate limit for a recipient.

**Parameters:**
- `recipient`: Recipient email address

**Example:**
```python
from include.email_security import reset_rate_limit

reset_rate_limit("recipient@example.com")
```

#### `get_rate_limit_status(recipient: str) -> dict`

Get rate limit status for a recipient.

**Parameters:**
- `recipient`: Recipient email address

**Returns:**
- `dict`: Status information including count, window, and remaining

**Example:**
```python
from include.email_security import get_rate_limit_status

status = get_rate_limit_status("recipient@example.com")
print(f"Emails sent: {status['count']}/{status['max']}")
```

---

## Logging Configuration (`include/logging_config.py`)

### Functions

#### `setup_logging(log_file_path='./log/monitor.log', log_level=logging.INFO, max_bytes=10*1024*1024, backup_count=5, enable_console=False, console_level=logging.WARNING) -> logging.Logger`

Set up enhanced logging with rotation and multiple handlers.

**Parameters:**
- `log_file_path`: Path to log file (default: `./log/monitor.log`)
- `log_level`: Logging level for file handler (default: `logging.INFO`)
- `max_bytes`: Maximum log file size before rotation (default: 10MB)
- `backup_count`: Number of backup log files to keep (default: 5)
- `enable_console`: Enable console logging (default: False)
- `console_level`: Logging level for console handler (default: `logging.WARNING`)

**Returns:**
- `logging.Logger`: Configured logger instance

**Example:**
```python
from include.logging_config import setup_logging
import logging

logger = setup_logging(
    log_file_path="./log/monitor.log",
    log_level=logging.INFO,
    max_bytes=10*1024*1024,
    backup_count=5,
    enable_console=True,
    console_level=logging.WARNING
)

logger.info("Monitor started")
```

#### `get_log_level_from_string(level_str: str) -> int`

Convert string to logging level constant.

**Parameters:**
- `level_str`: Log level string (DEBUG, INFO, WARNING, ERROR, CRITICAL)

**Returns:**
- `int`: Logging level constant

**Example:**
```python
from include.logging_config import get_log_level_from_string

level = get_log_level_from_string("INFO")  # Returns logging.INFO
```

#### `configure_logging_from_config(config: configparser.RawConfigParser) -> logging.Logger`

Configure logging from configuration file.

**Parameters:**
- `config`: ConfigParser instance with logging options

**Returns:**
- `logging.Logger`: Configured logger instance

**Configuration Options:**
- `logLevel`: Log level (DEBUG, INFO, WARNING, ERROR, CRITICAL)
- `logFile`: Log file path
- `logMaxBytes`: Maximum log file size
- `logBackupCount`: Number of backup files
- `logConsole`: Enable console logging (true/false)
- `logConsoleLevel`: Console log level

**Example:**
```python
from include.logging_config import configure_logging_from_config
from include.config_manager import load_config_with_defaults

config = load_config_with_defaults("./config/checker.config")
logger = configure_logging_from_config(config)
```

#### `log_structured(logger: logging.Logger, level: int, message: str, **kwargs) -> None`

Log structured data with additional context.

**Parameters:**
- `logger`: Logger instance
- `level`: Log level constant
- `message`: Log message
- `**kwargs`: Additional context fields

**Example:**
```python
from include.logging_config import log_structured
import logging

log_structured(
    logger,
    logging.INFO,
    "Heartbeat check completed",
    account="account1",
    status="ok",
    duration=0.5
)
```

---

## Platform Utilities (`include/platform_utils.py`)

### Classes

#### `PlatformType` (Enum)

Enumeration of supported platform types.

**Values:**
- `WINDOWS`: Windows platform
- `MACOS`: macOS platform
- `LINUX`: Linux platform
- `UNKNOWN`: Unknown platform

### Functions

#### `detect_platform() -> PlatformType`

Detect the current operating system platform.

**Returns:**
- `PlatformType`: The detected platform

**Example:**
```python
from include.platform_utils import detect_platform, PlatformType

platform = detect_platform()
if platform == PlatformType.MACOS:
    print("Running on macOS")
```

#### `get_platform_name() -> str`

Get human-readable platform name.

**Returns:**
- `str`: Platform name (Windows, macOS, Linux, or Unknown)

**Example:**
```python
from include.platform_utils import get_platform_name

print(f"Platform: {get_platform_name()}")
```

#### `is_windows() -> bool`

Check if running on Windows.

**Returns:**
- `bool`: True if Windows, False otherwise

#### `is_macos() -> bool`

Check if running on macOS.

**Returns:**
- `bool`: True if macOS, False otherwise

#### `is_linux() -> bool`

Check if running on Linux.

**Returns:**
- `bool`: True if Linux, False otherwise

#### `is_unix() -> bool`

Check if running on Unix-like system (macOS or Linux).

**Returns:**
- `bool`: True if Unix-like, False otherwise

#### `get_mt4_process_name() -> str`

Get the MT4 process name for the current platform.

**Returns:**
- `str`: Process name ('terminal.exe' for Windows, 'terminal' for Unix)

**Example:**
```python
from include.platform_utils import get_mt4_process_name

process_name = get_mt4_process_name()
# Returns 'terminal.exe' on Windows, 'terminal' on Unix
```

#### `get_platform_info() -> dict`

Get detailed platform information.

**Returns:**
- `dict`: Platform information including:
  - `platform`: Platform type (windows/macos/linux/unknown)
  - `system`: System name from platform.system()
  - `release`: OS release version
  - `version`: OS version string
  - `machine`: Machine architecture
  - `processor`: Processor type

**Example:**
```python
from include.platform_utils import get_platform_info

info = get_platform_info()
print(f"System: {info['system']} {info['release']}")
```

#### `get_platform_error_message(operation: str, error: Exception) -> str`

Generate platform-specific error messages with helpful suggestions.

**Parameters:**
- `operation`: Description of the operation that failed
- `error`: The exception that occurred

**Returns:**
- `str`: Platform-specific error message with suggestions

**Example:**
```python
from include.platform_utils import get_platform_error_message

try:
    # Some operation
    pass
except Exception as e:
    error_msg = get_platform_error_message("reading config file", e)
    logger.error(error_msg)
```

#### `get_path_separator() -> str`

Get the path separator for the current platform.

**Returns:**
- `str`: Path separator ('\\' for Windows, '/' for Unix)

#### `get_line_separator() -> str`

Get the line separator for the current platform.

**Returns:**
- `str`: Line separator ('\\r\\n' for Windows, '\\n' for Unix)

#### `format_platform_path(path: str) -> str`

Format a path for the current platform.

**Parameters:**
- `path`: Path string (may contain forward or backslashes)

**Returns:**
- `str`: Path formatted for current platform

---

## Process Management (`include/process_manager.py`)

### Functions

#### `kill_process_by_name(process_name: str) -> int`

Kill all processes matching the given name (cross-platform).

**Parameters:**
- `process_name`: Name of the process to kill (e.g., 'terminal.exe', 'terminal')

**Returns:**
- `int`: Number of processes killed

**Example:**
```python
from include.process_manager import kill_process_by_name

killed = kill_process_by_name("terminal.exe")
print(f"Killed {killed} process(es)")
```

#### `kill_mt4_processes() -> int`

Kill MT4 terminal processes (cross-platform). Uses platform detection to determine correct process name.

**Returns:**
- `int`: Number of processes killed

**Example:**
```python
from include.process_manager import kill_mt4_processes

killed = kill_mt4_processes()
if killed > 0:
    print(f"Killed {killed} MT4 process(es)")
```

---

## Retry Mechanism (`include/retry_mechanism.py`)

### Classes

#### `RetryConfig`

Configuration for retry behavior.

**Parameters:**
- `max_attempts` (int): Maximum number of retry attempts (default: 3)
- `initial_delay` (float): Initial delay in seconds before first retry (default: 1.0)
- `max_delay` (float): Maximum delay between retries in seconds (default: 60.0)
- `exponential_base` (float): Base for exponential backoff (default: 2.0)
- `retryable_exceptions` (tuple): Tuple of exception types to retry (None = all)

**Example:**
```python
from include.retry_mechanism import RetryConfig

config = RetryConfig(
    max_attempts=5,
    initial_delay=2.0,
    max_delay=30.0
)
```

### Functions

#### `retry_with_backoff(config: Optional[RetryConfig] = None)`

Decorator for retrying functions with exponential backoff.

**Parameters:**
- `config`: RetryConfig instance (uses default if None)

**Example:**
```python
from include.retry_mechanism import retry_with_backoff, RetryConfig

@retry_with_backoff(RetryConfig(max_attempts=3))
def my_function():
    # code that might fail
    pass
```

#### `retry_file_operation(operation: Callable, *args, max_attempts: int = 3, operation_name: str = "file operation", **kwargs) -> Any`

Retry a file operation with exponential backoff.

**Parameters:**
- `operation`: Function to retry
- `*args`: Arguments to pass to operation
- `max_attempts`: Maximum number of retry attempts (default: 3)
- `operation_name`: Name of operation for logging (default: "file operation")
- `**kwargs`: Keyword arguments to pass to operation

**Returns:**
- Result of the operation

**Raises:**
- Last exception if all attempts fail

**Example:**
```python
from include.retry_mechanism import retry_file_operation

def read_file():
    with open("file.txt", "r") as f:
        return f.read()

data = retry_file_operation(
    read_file,
    max_attempts=3,
    operation_name="Reading file.txt"
)
```

#### `retry_email_send(email_func: Callable, *args, max_attempts: int = 3, **kwargs) -> Any`

Retry email sending with exponential backoff.

**Parameters:**
- `email_func`: Email sending function
- `*args`: Arguments to pass to email function
- `max_attempts`: Maximum number of retry attempts (default: 3)
- `**kwargs`: Keyword arguments to pass to email function

**Returns:**
- Result of the email function

**Raises:**
- Last exception if all attempts fail

**Example:**
```python
from include.retry_mechanism import retry_email_send
from include.asirikuy import sendemail

result = retry_email_send(
    sendemail,
    from_addr="sender@example.com",
    to_addr_list=["recipient@example.com"],
    cc_addr_list=[],
    subject="Test",
    message="Test message",
    login="sender@example.com",
    password="password",
    smtpserver="smtp.gmail.com:587",
    max_attempts=3
)
```

#### `graceful_degradation(operation: Callable, fallback: Callable, *args, **kwargs) -> Any`

Execute operation with fallback on failure.

**Parameters:**
- `operation`: Primary operation to try
- `fallback`: Fallback operation if primary fails
- `*args`: Arguments to pass to operations
- `**kwargs`: Keyword arguments to pass to operations

**Returns:**
- Result of operation or fallback

**Example:**
```python
from include.retry_mechanism import graceful_degradation

def primary_operation():
    # Try primary method
    pass

def fallback_operation():
    # Use fallback method
    pass

result = graceful_degradation(primary_operation, fallback_operation)
```

---

## Miscellaneous Utilities (`include/misc.py`)

### Functions

#### `prettyXML(filePath: str) -> None`

Format XML file with proper indentation.

**Parameters:**
- `filePath`: Path to XML file

**Example:**
```python
from include.misc import prettyXML

prettyXML("./config.xml")
```

#### `insertLine(filePath: str, line: str, numberLine: int) -> None`

Insert a line into a file at specified position.

**Parameters:**
- `filePath`: Path to file
- `line`: Line to insert
- `numberLine`: Line number (0-based) to insert at

**Example:**
```python
from include.misc import insertLine

insertLine("./file.txt", "New line", 5)
```

#### `clearScreen() -> None`

Clear the terminal screen (cross-platform).

**Example:**
```python
from include.misc import clearScreen

clearScreen()
```

---

## Configuration Options

### General Section

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `monitoringInterval` | int | 60 | Check interval in seconds |
| `useEmail` | int | 0 | Enable email alerts (1) or disable (0) |
| `fromEmail` | str | "" | Sender email address |
| `toEmail` | str | "" | Recipient email address |
| `emailLogin` | str | "" | SMTP login username |
| `emailPassword` | str | "" | SMTP password |
| `smtpServer` | str | "smtp.gmail.com:587" | SMTP server address |
| `weekOpenDay` | int | 0 | Trading week start day (0=Monday) |
| `weekOpenHour` | int | 6 | Trading week start hour |
| `weekCloseDay` | int | 5 | Trading week end day (5=Friday) |
| `weekCloseHour` | int | 7 | Trading week end hour |
| `logLevel` | str | "INFO" | Log level (DEBUG, INFO, WARNING, ERROR, CRITICAL) |
| `logFile` | str | "./log/monitor.log" | Log file path |
| `logMaxBytes` | int | 10485760 | Maximum log file size (10 MB) |
| `logBackupCount` | int | 5 | Number of backup log files |
| `logConsole` | bool | false | Enable console logging |
| `logConsoleLevel` | str | "WARNING" | Console log level |

### Accounts Section

| Option | Type | Description |
|--------|------|-------------|
| `accounts` | str | Comma-separated list of account section names |

### Account Sections

Each account section (e.g., `[account1]`) requires:

| Option | Type | Description |
|--------|------|-------------|
| `accountNumber` | str | Account number |
| `path` | str | Path to trading platform data directory |
| `frontend` | str | Platform type ("MT4" or "AT") |

---

## Error Handling

All functions follow consistent error handling patterns:

- **File Operations**: Raise `IOError`, `OSError`, or `PermissionError`
- **Configuration**: Raise `FileNotFoundError` or `ValueError`
- **Email Operations**: Raise `ValueError` for invalid inputs, SMTP errors for network issues
- **Process Management**: Logs errors but doesn't raise exceptions

---

## Examples

### Complete Example: Setting Up and Running Monitor

```python
from include.config_manager import load_config_with_defaults, validate_config
from include.logging_config import configure_logging_from_config
from include.platform_utils import get_platform_name
import logging

# Load configuration
config = load_config_with_defaults("./config/checker.config")

# Validate configuration
errors = validate_config(config)
if errors:
    for error in errors:
        print(f"Error: {error}")
    exit(1)

# Set up logging
logger = configure_logging_from_config(config)
logger.info(f"Monitor starting on {get_platform_name()}")

# Get configuration values
interval = int(config.get('general', 'monitoringInterval'))
use_email = int(config.get('general', 'useEmail'))

# Start monitoring loop
# ... (see checker.py for full implementation)
```

---

**Last Updated**: December 2024

