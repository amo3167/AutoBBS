# Changelog

All notable changes to Asirikuy Monitor will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.05] - 2024-12-02

### Added
- **Python 3 Migration**: Complete migration from Python 2 to Python 3.8+
- **Cross-Platform Support**: Works on Windows, macOS, and Linux
- **Environment Variables**: Support for secure credential management via `.env` files
- **Email Security**: 
  - Email address validation (RFC 5322 compliant)
  - Content sanitization (XSS prevention)
  - Subject sanitization
  - Per-recipient rate limiting (5 emails per 5 minutes)
- **Enhanced Logging**:
  - Log rotation with configurable size and backup count
  - Configurable log levels
  - Console logging option
  - Structured logging support
- **Error Recovery**:
  - Retry mechanism with exponential backoff
  - Graceful degradation for file operations
  - Health check system
- **Process Management**: Cross-platform process management using `psutil` (replaces Windows-specific `taskkill`)
- **Platform Detection**: Platform-aware utilities for OS detection and platform-specific operations
- **Path Handling**: Modern pathlib-based path operations (replaces os.path)
- **Configuration Management**: Centralized configuration with validation and default values
- **Type Hints**: Added type hints to function signatures
- **Code Quality**: Improved code organization, PEP 8 compliance, docstrings
- **Testing**: Comprehensive test suite with unit and integration tests
- **Documentation**:
  - README.md with installation and usage guide
  - API.md with complete API reference
  - DEPLOYMENT.md with deployment guide
  - MIGRATION_GUIDE.md with Python 2 to 3 migration steps

### Changed
- **Python Version**: Migrated from Python 2 (EOL) to Python 3.8+
- **Dependencies**: 
  - Updated `colorama` from 0.2.5 to >=0.4.6
  - Added `python-dotenv` for environment variable support
  - Added `psutil` for cross-platform process management
- **ConfigParser**: Updated from `ConfigParser` to `configparser` module
- **String Formatting**: Migrated from old-style `%` formatting to f-strings
- **File Operations**: Updated to use context managers with explicit encoding
- **Exception Handling**: Updated to Python 3 syntax (`except Exception as e`)
- **Process Management**: Replaced Windows-specific `taskkill` with cross-platform `psutil`
- **Path Operations**: Migrated from `os.path` to `pathlib.Path`

### Fixed
- **Syntax Errors**: Fixed all Python 2 syntax issues
- **Import Errors**: Fixed ConfigParser import
- **map() Usage**: Fixed map() to return list instead of iterator
- **File Encoding**: Added explicit UTF-8 encoding to all file operations
- **Exception Handling**: Fixed exception handling syntax
- **String Formatting**: Fixed all string formatting issues
- **Cross-Platform**: Fixed Windows-specific code for cross-platform compatibility

### Security
- **Credentials**: Moved from hardcoded config file to environment variables
- **Email Validation**: Added RFC 5322 compliant email validation
- **Content Sanitization**: Added XSS prevention in email content
- **Rate Limiting**: Added per-recipient email rate limiting
- **Secure SMTP**: Uses TLS/SSL encryption for email transmission

### Removed
- **Python 2 Support**: Removed all Python 2 specific code
- **Windows-Specific Code**: Removed `winnt` import and `taskkill` commands
- **Old Dependencies**: Removed unused vendor dependencies (oauth2, python-twitter)
- **Vendor Folder**: Removed entire `vendor/` directory (1.3MB) containing unused Python 2 dependencies

### Migration Notes
- **Breaking Changes**: 
  - Requires Python 3.8 or higher
  - ConfigParser import changed to configparser
  - map() now returns iterator (use list() or list comprehension)
- **Upgrade Path**: See MIGRATION_GUIDE.md for detailed migration steps
- **Configuration**: New optional logging configuration options available

---

## [0.04] - Pre-Migration (Python 2)

### Features
- Basic heartbeat monitoring
- Log file error detection
- Email alerts
- Windows-specific process management

### Limitations
- Python 2 only (EOL)
- Windows-specific code
- Hardcoded credentials
- Minimal error handling
- No tests

---

## Version History

- **0.05** (2024-12-02): Complete Python 3 migration and modernization
- **0.04** (Pre-2024): Original Python 2 version

---

## Future Plans

### Planned for 0.06
- CI/CD configuration
- Additional integration tests
- Performance improvements
- Enhanced monitoring dashboard

### Under Consideration
- Health check HTTP endpoint
- Metrics collection
- Web-based configuration UI
- Docker containerization

---

**Note**: This changelog documents changes from version 0.04 (Python 2) to 0.05 (Python 3). Earlier versions are not documented.

