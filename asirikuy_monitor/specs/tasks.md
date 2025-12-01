# Tasks: Asirikuy Monitor Python 3 Migration & Modernization

**Project**: Asirikuy Monitor  
**Goal**: Migrate from Python 2 to Python 3 and modernize the codebase  
**Priority**: High (Python 2 is EOL since January 2020)

## Project Overview

Asirikuy Monitor is a monitoring tool that:
- Monitors trading platform heartbeats (MT4 and AT platforms)
- Checks log files for errors
- Sends email alerts on issues
- Kills MT4 processes when problems are detected

**Current State**:
- Python 2 codebase (EOL)
- Old vendor dependencies (colorama 0.2.5, oauth2 1.5.211, python-twitter 1.0)
- Windows-specific code (`winnt`, `taskkill`)
- Security issues (hardcoded passwords in config)
- No tests, no documentation
- Minimal error handling

## Execution Flow

```
1. Analysis & Planning
   → Identify all Python 2 dependencies
   → Document migration requirements
   → Create migration plan
2. Python 3 Migration
   → Fix syntax issues
   → Update imports
   → Fix string/bytes handling
   → Update dependencies
3. Code Modernization
   → Improve error handling
   → Add logging
   → Improve security
   → Add cross-platform support
4. Testing & Documentation
   → Add unit tests
   → Add integration tests
   → Create documentation
   → Update deployment guides
```

## Format: `[ID] [P?] Description`
- **[P]**: Can run in parallel (different files, no dependencies)
- Include exact file paths in descriptions

---

## Phase 1: Analysis & Planning

### Phase 1.1: Code Analysis
- [ ] T001 [P] Analyze Python 2 syntax usage in `checker.py`
  - Identify all `print` statements
  - Identify `ConfigParser` usage
  - Identify `map()` usage
  - Document all Python 2-specific code
- [ ] T002 [P] Analyze dependencies in `include/asirikuy.py`
  - Document `ConfigParser` usage
  - Document `ctypes` usage
  - Document string/bytes handling
  - Create dependency list
- [ ] T003 [P] Analyze vendor dependencies
  - Review `vendor/colorama-0.2.5/` compatibility
  - Review `vendor/oauth2-1.5.211/` compatibility
  - Review `vendor/python-twitter-1.0/` compatibility
  - Identify replacement libraries
- [ ] T004 [P] Document Windows-specific code
  - Identify `winnt` usage
  - Identify `taskkill` usage
  - Identify Windows path handling
  - Plan cross-platform alternatives
- [ ] T005 [P] Security audit
  - Identify hardcoded credentials in config files
  - Document security risks
  - Plan credential management
  - Create security recommendations

### Phase 1.2: Migration Planning
- [ ] T006 Create Python 3 migration checklist
  - List all syntax changes needed
  - List all import changes needed
  - List all dependency updates needed
  - Create migration timeline
- [ ] T007 Create dependency update plan
  - Plan colorama update (0.2.5 → latest)
  - Plan oauth2 replacement (deprecated, find alternative)
  - Plan python-twitter replacement (deprecated, find alternative)
  - Create requirements.txt structure
- [ ] T008 Create cross-platform support plan
  - Plan Windows alternative for `winnt`
  - Plan Windows alternative for `taskkill`
  - Plan path handling improvements
  - Document platform-specific code

---

## Phase 2: Python 3 Migration (Priority 1)

### Phase 2.1: Core Script Migration
- [x] T010 Migrate `checker.py` to Python 3
  - ✅ Convert `print` statements to `print()` function (4 instances fixed)
  - ✅ Fix `ConfigParser` → `configparser` import (via include/asirikuy.py)
  - ✅ Fix `map()` usage (changed to list comprehension)
  - ✅ Fix `winnt.MAXBYTE` import (removed unused import)
  - ✅ Fix `quit()` → `sys.exit(0)`
  - ✅ Fix string formatting (kept old-style for now, can improve later)
  - ✅ Fix `sleep()` import (changed to `time.sleep()`)
  - ✅ Fix file operations (added encoding='utf-8', used context managers)
  - ✅ Fixed email function call (tuple → string)
  - ✅ All syntax validated with Python 3 compiler
- [x] T011 Migrate `include/asirikuy.py` to Python 3
  - ✅ Fix `ConfigParser` → `configparser` import
  - ✅ Fix `ConfigParser.RawConfigParser()` → `configparser.RawConfigParser()`
  - ✅ Fix exception handling (`except:` → `except Exception as e:`)
  - ✅ Fixed typo in DukascopySymbolReverse ('USD/JPY':'AUDUSD' → 'USD/JPY':'USDJPY')
  - ✅ All syntax validated with Python 3 compiler
- [x] T012 Migrate `include/misc.py` to Python 3
  - ✅ Fix file operations (added encoding='utf-8', used context managers)
  - ✅ All syntax validated with Python 3 compiler

### Phase 2.2: Dependency Updates
- [x] T020 Update colorama dependency
  - ✅ Created requirements.txt with colorama>=0.4.6
  - ✅ Vendor directory can be removed (not needed with pip install)
  - ⏳ Installation: `pip install -r requirements.txt`
  - ⏳ Test color output (pending runtime testing)
- [x] T021 Replace oauth2 dependency
  - ✅ Verified: oauth2 is NOT used in main code
  - ✅ Vendor directory can be removed (not needed)
  - ✅ No replacement needed (not used)
- [x] T022 Replace python-twitter dependency
  - ✅ Verified: python-twitter is NOT used in main code
  - ✅ Vendor directory can be removed (not needed)
  - ✅ No replacement needed (not used)
- [x] T023 Create `requirements.txt`
  - ✅ Created requirements.txt with colorama>=0.4.6
  - ✅ Created requirements-dev.txt for development dependencies
  - ✅ Documented standard library modules (no installation needed)
  - ✅ Added optional dependencies for future enhancements (psutil, python-dotenv)
  - ✅ Installation process: `pip install -r requirements.txt`

### Phase 2.3: Syntax & Compatibility Fixes
- [x] T030 Fix all print statements
  - ✅ Converted all `print "text"` → `print("text")`
  - ✅ Converted all `print output` → `print(output)`
  - ✅ All print statements tested and working
- [x] T031 Fix ConfigParser imports
  - ✅ Changed `import ConfigParser` → `import configparser`
  - ✅ Changed `ConfigParser.RawConfigParser()` → `configparser.RawConfigParser()`
  - ✅ Config file reading tested and working
- [x] T032 Fix map() usage
  - ✅ Changed `map(str.strip, ...)` → `[s.strip() for s in ...]` (list comprehension)
  - ✅ List operations tested and working
- [x] T033 Fix string formatting
  - ✅ Converted all old-style `%` formatting to f-strings
  - ✅ Converted `.format()` calls to f-strings where appropriate
  - ✅ Updated checker.py (7 instances)
  - ✅ Updated include/asirikuy.py (5 instances)
  - ✅ All string operations tested and working
- [x] T034 Fix exception handling
  - ✅ Updated all `except Exception, e:` → `except Exception as e:`
  - ✅ Error handling tested and working
- [x] T035 Fix file operations
  - ✅ All file operations use proper encoding (`encoding='utf-8'`)
  - ✅ All file operations use context managers (`with` statements)
  - ✅ File I/O tested and working

---

## Phase 3: Code Modernization (Priority 2)

### Phase 3.1: Error Handling & Logging
- [x] T040 Improve error handling in `checker.py`
  - ✅ Added try/except blocks around file operations (heartbeat files, log files)
  - ✅ Added try/except blocks around directory operations (os.chdir, os.listdir)
  - ✅ Added try/except blocks around email sending
  - ✅ Added try/except blocks around process killing
  - ✅ Added try/except around config file reading
  - ✅ Added proper error messages with context
  - ✅ Added graceful error recovery (continue on account errors)
  - ✅ Added path validation (os.path.exists checks)
  - ✅ Added data validation (checking file length, data availability)
  - ✅ All existing tests still pass
- [x] T041 Improve logging configuration
  - ✅ Created `include/logging_config.py` module
  - ✅ Replaced basic logging with enhanced logger setup
  - ✅ Added log rotation (RotatingFileHandler with configurable size and backup count)
  - ✅ Added configurable log levels (DEBUG, INFO, WARNING, ERROR, CRITICAL)
  - ✅ Added structured logging support (key-value pairs)
  - ✅ Added config file support for logging settings
  - ✅ Added console logging option (configurable)
  - ✅ Improved log formatting (detailed timestamps, levels)
  - ✅ Graceful fallback to basic logging if module unavailable
  - ✅ Created comprehensive tests (5/5 passing)
  - ✅ All existing tests still pass
- [x] T042 Add error recovery mechanisms
  - ✅ Created `include/retry_mechanism.py` module
  - ✅ Added retry logic for file operations
    - Exponential backoff (0.5s initial, max 5s)
    - Configurable max attempts (default: 3)
    - Specific exception handling (IOError, OSError, PermissionError)
  - ✅ Added retry logic for email sending
    - Exponential backoff (2s initial, max 30s)
    - Network error handling (SMTPException, ConnectionError, TimeoutError)
    - Integrated into heartbeat and error detection email sending
  - ✅ Added graceful degradation
    - Fallback functions
    - Fallback values
    - Error logging
  - ✅ Added health check system
    - HealthCheck class for status tracking
    - Register custom health checks
    - Run individual or all checks
    - Overall health status
  - ✅ Integrated retry logic into checker.py
    - Heartbeat file reading with retry
    - Log file reading with retry
    - Email sending with retry (heartbeat and error detection)
  - ✅ Created comprehensive tests (5/5 passing)
  - ✅ All existing tests still pass

### Phase 3.2: Security Improvements
- [x] T050 Remove hardcoded credentials
  - ✅ Move credentials to environment variables
  - ✅ Add `.env` file support (python-dotenv installed and integrated)
  - ✅ Created `.env.example` template
  - ✅ Environment variables take priority over config file
  - ✅ Config file used as fallback (backward compatible)
  - ✅ Added security warning when using config file credentials
  - ✅ Updated .gitignore to exclude .env files
  - ✅ Created config/checker.config.example (without credentials)
  - ✅ Documented credential management in README
  - ✅ Security tests created and passing
- [ ] T051 Secure config file handling
  - ⏳ Add config file validation (optional)
  - ⏳ Add sensitive data masking in logs (optional)
  - ⏳ Add config file permissions check (optional)
  - ✅ Document security best practices (in README)
- [x] T052 Improve email security
  - ✅ Use secure SMTP connections (TLS/SSL) - already using starttls()
  - ✅ Validate email addresses
    - RFC 5322 compliant validation
    - Format, length, and structure checks
    - Invalid email rejection with logging
  - ✅ Sanitize email content
    - HTML entity escaping (prevents XSS)
    - Null byte removal
    - Control character removal
    - Length limiting (10KB max)
    - Injection attack prevention
  - ✅ Add email rate limiting
    - Per-recipient rate limiting (5 emails per 5 minutes)
    - Automatic cleanup of old entries
    - Rate limit status checking
    - Reset functionality
  - ✅ Created email_security.py module
  - ✅ Enhanced sendemail() function with security features
  - ✅ Created comprehensive tests (5/5 passing)
  - ✅ All existing tests still pass

### Phase 3.3: Cross-Platform Support
- [x] T060 Replace Windows-specific code
  - ✅ Removed unused `winnt.MAXBYTE` import (already done in migration)
  - ✅ Replaced `taskkill` with cross-platform process management
  - ✅ Created `include/process_manager.py` with psutil-based functions
  - ✅ Replaced both `taskkill` calls in checker.py
  - ✅ Installed and integrated `psutil` package
  - ✅ Created process manager tests (3/3 passing)
  - ✅ Code compiles and works on macOS (verified)
  - ⏳ Test on Linux (pending)
- [x] T061 Improve path handling
  - ✅ Use `pathlib` for path operations (migrated from os.path.join)
  - ✅ Use `os.path.join()` consistently (replaced with pathlib)
  - ✅ Handle Windows/Unix path differences (pathlib handles automatically)
  - ✅ Tested on macOS (verified)
  - ⏳ Test on Linux (pending, but pathlib is cross-platform)
- [x] T062 Add platform detection
  - ✅ Detect OS type (Windows, macOS, Linux)
  - ✅ Use platform-specific code paths (MT4 process names, path separators)
  - ✅ Add platform-specific error messages (with platform-specific suggestions)
  - ✅ Created `include/platform_utils.py` module
  - ✅ Integrated into `checker.py`, `process_manager.py`, `misc.py`
  - ✅ Platform info in version command
  - ✅ Created comprehensive tests (10/10 passing)
  - ✅ Document platform requirements (in README)

### Phase 3.4: Code Quality
- [x] T070 Add type hints
  - ✅ Added type hints to function signatures (`main() -> None`, `version() -> None`)
  - ✅ Added type hints to variables (`account_sections: List[str]`, `last_error: List[str]`)
  - ✅ Used `typing` module (`List`, `Optional`)
  - ⏳ Validate with mypy (optional, can be done later)
- [x] T071 Improve code organization
  - ✅ Extracted constants (`DEFAULT_CONFIG_PATH`, `DEFAULT_LOG_FILE`)
  - ✅ Improved variable naming (PEP 8 compliant: `account_section`, `last_error`, `local_time`, `frontend`, `account_number`)
  - ✅ Added docstrings to main functions
  - ✅ Improved code readability (extracted `is_trading_hours` variable)
  - ✅ Better argument parsing (added descriptions, proper types)
  - ⏳ Split large functions (main() is still large but functional - can be refactored later if needed)
- [x] T072 Add configuration management
  - ✅ Created `include/config_manager.py` module
  - ✅ Added config validation (`validate_config()` function)
  - ✅ Added default values (`DEFAULT_CONFIG` dictionary)
  - ✅ Added config schema (validation checks for all required fields)
  - ✅ Created `load_config_with_defaults()` function
  - ✅ Created `get_config_value()` helper function
  - ✅ Integrated into checker.py with proper error handling
  - ✅ Created comprehensive tests (3/3 passing)
  - ✅ All existing tests still pass

---

## Phase 4: Testing & Documentation (Priority 3)

### Phase 4.1: Unit Testing
- [x] T080 Create test structure
  - ✅ Created `tests/` directory
  - ✅ Created `tests/fixtures/` directory
  - ✅ Created test fixtures (config, heartbeat, log files)
  - ⏳ pytest configuration (optional, basic tests work without it)
- [x] T081 Write unit tests for `checker.py`
  - ✅ Test config file reading
  - ✅ Test heartbeat checking
  - ✅ Test log file parsing
  - ⏳ Test email sending (mocked) - requires SMTP mock
- [x] T082 Write unit tests for `include/asirikuy.py`
  - ✅ Test config file reading
  - ⏳ Test email sending (mocked) - requires SMTP mock
  - ⏳ Test ctypes integration (mocked) - not critical for runtime
- [x] T083 Write unit tests for `include/misc.py`
  - ✅ Test XML pretty printing
  - ✅ Test file operations
  - ✅ Test screen clearing
- [x] T084 Add test fixtures
  - ✅ Created mock config files
  - ✅ Created mock log files
  - ✅ Created mock heartbeat files
  - ✅ Created test data generators

### Phase 4.2: Integration Testing
- [x] T090 Create integration tests
  - ✅ Test module imports
  - ✅ Test version command
  - ✅ Test config file loading
  - ⏳ Test full monitoring cycle (requires trading platform)
  - ⏳ Test email sending (with test SMTP server)
  - ⏳ Test process killing (with mock processes)
- [ ] T091 Add CI/CD configuration
  - ⏳ Add GitHub Actions workflow
  - ⏳ Add test automation
  - ⏳ Add code quality checks
  - ⏳ Add Python 3.x matrix testing

### Phase 4.3: Documentation
- [ ] T100 Create README.md
  - Add project description
  - Add installation instructions
  - Add usage examples
  - Add configuration guide
- [ ] T101 Create API documentation
  - Document all functions
  - Document all classes
  - Document configuration options
  - Add code examples
- [ ] T102 Create deployment guide
  - Document installation steps
  - Document configuration setup
  - Document credential management
  - Document troubleshooting
- [ ] T103 Create migration guide
  - Document Python 2 → 3 migration steps
  - Document breaking changes
  - Document upgrade path
  - Add migration checklist

---

## Phase 5: Deployment & Maintenance

### Phase 5.1: Deployment Improvements
- [ ] T110 Create deployment scripts
  - Create `setup.py` or `pyproject.toml`
  - Create installation script
  - Create startup script (systemd/service)
  - Create uninstall script
- [ ] T111 Add monitoring improvements
  - Add health check endpoint
  - Add metrics collection
  - Add alerting improvements
  - Add dashboard (optional)
- [ ] T112 Improve startup scripts
  - Update `.bat` files for Windows
  - Create `.sh` scripts for Unix
  - Add proper error handling
  - Add logging to startup

### Phase 5.2: Maintenance
- [ ] T120 Add changelog
  - Document all changes
  - Document version history
  - Document migration notes
- [ ] T121 Add version management
  - Add version number to code
  - Add version command-line option
  - Update version in all places
- [ ] T122 Add code quality tools
  - Add `black` for code formatting
  - Add `flake8` for linting
  - Add `mypy` for type checking
  - Add pre-commit hooks

---

## Task Dependencies

```
# Analysis Phase
T001, T002, T003, T004, T005 (parallel analysis)
T001-T005 → T006, T007, T008 (planning)

# Migration Phase
T006, T007, T008 → T010, T011, T012 (core migration)
T010, T011, T012 → T020, T021, T022 (dependency updates)
T010-T012, T020-T022 → T030-T035 (syntax fixes)

# Modernization Phase
T030-T035 → T040, T041, T042 (error handling)
T030-T035 → T050, T051, T052 (security)
T030-T035 → T060, T061, T062 (cross-platform)
T030-T035 → T070, T071, T072 (code quality)

# Testing Phase
T040-T072 → T080, T081, T082, T083 (unit tests)
T080-T083 → T090, T091 (integration tests)
T080-T091 → T100, T101, T102, T103 (documentation)

# Deployment Phase
T100-T103 → T110, T111, T112 (deployment)
T110-T112 → T120, T121, T122 (maintenance)
```

## Critical Path

The critical path is:
```
T001 → T006 → T010 → T020 → T030 → T040 → T080 → T090 → T100 → T110
```

This represents the minimum sequence to complete migration and basic functionality.

## Parallel Execution Examples

### Example 1: Analysis Phase
```
T001, T002, T003, T004, T005 can run in parallel
(All analysis tasks are independent)
```

### Example 2: Syntax Fixes
```
T030, T031, T032, T033, T034, T035 can run in parallel
(All syntax fixes are independent)
```

### Example 3: Modernization
```
T040, T050, T060, T070 can run in parallel
(Error handling, security, cross-platform, code quality)
```

## Notes

- **Priority**: Phase 2 (Python 3 Migration) is critical - Python 2 is EOL
- **Dependencies**: Some vendor libraries are deprecated and need replacement
- **Security**: Hardcoded credentials must be removed before production use
- **Testing**: Add tests before major refactoring to ensure functionality
- **Platform**: Currently Windows-specific, should support macOS/Linux
- **Estimated Effort**: 40-60 story points
- **Estimated Timeline**: 2-3 weeks for migration, 1-2 weeks for modernization

## Migration Checklist

### Python 3 Syntax Changes
- [ ] All `print` statements → `print()` function
- [ ] `ConfigParser` → `configparser`
- [ ] `map()` wrapped in `list()` or use list comprehensions
- [ ] `except Exception, e:` → `except Exception as e:`
- [ ] String formatting updated (f-strings or `.format()`)
- [ ] File operations use proper encoding
- [ ] `quit()` → `sys.exit()`

### Dependency Updates
- [ ] colorama updated to latest version
- [ ] oauth2 replaced with modern alternative
- [ ] python-twitter replaced or removed
- [ ] requirements.txt created
- [ ] All dependencies tested

### Security
- [ ] Hardcoded credentials removed
- [ ] Environment variables used
- [ ] Config files secured
- [ ] Email security improved

### Testing
- [ ] Unit tests added
- [ ] Integration tests added
- [ ] All tests passing
- [ ] Test coverage > 70%

### Documentation
- [ ] README.md created
- [ ] API documentation created
- [ ] Deployment guide created
- [ ] Migration guide created

---

## Progress Summary

### Current Status
- **Python Version**: Python 2 (EOL)
- **Dependencies**: Old/Deprecated
- **Platform**: Windows-only
- **Security**: ⚠️ Hardcoded credentials
- **Testing**: ❌ No tests
- **Documentation**: ❌ Minimal

### Next Steps
1. ✅ T001-T005: Complete code analysis
2. ⏳ T006-T008: Create migration plan
3. ⏳ T010-T012: Migrate core scripts
4. ⏳ T020-T022: Update dependencies
5. ⏳ T030-T035: Fix syntax issues

---

**Last Updated**: December 2024  
**Status**: Planning Phase

