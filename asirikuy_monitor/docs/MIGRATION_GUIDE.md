# Python 2 to Python 3 Migration Guide

**Version**: 0.05  
**Last Updated**: December 2024

## Table of Contents

1. [Overview](#overview)
2. [Migration Checklist](#migration-checklist)
3. [Breaking Changes](#breaking-changes)
4. [Step-by-Step Migration](#step-by-step-migration)
5. [Upgrade Path](#upgrade-path)
6. [Common Issues and Solutions](#common-issues-and-solutions)
7. [Testing After Migration](#testing-after-migration)

---

## Overview

This guide documents the migration of Asirikuy Monitor from Python 2 to Python 3. The migration is **complete** and all code now runs on Python 3.8+.

### Why Migrate?

- **Python 2 End of Life**: Python 2 reached end of life on January 1, 2020
- **Security**: No security updates for Python 2
- **Modern Features**: Access to Python 3 features (f-strings, type hints, pathlib)
- **Dependencies**: Most modern libraries require Python 3
- **Cross-Platform**: Better cross-platform support

### Migration Status

✅ **Complete** - All code has been migrated to Python 3.8+

---

## Migration Checklist

### Pre-Migration

- [x] Identify all Python 2 syntax
- [x] Document dependencies
- [x] Create migration plan
- [x] Set up Python 3 environment

### Core Migration

- [x] Fix print statements
- [x] Update ConfigParser imports
- [x] Fix map() usage
- [x] Update exception handling
- [x] Fix file operations
- [x] Update string formatting

### Dependency Updates

- [x] Update colorama
- [x] Add python-dotenv
- [x] Add psutil
- [x] Create requirements.txt

### Code Modernization

- [x] Add error handling
- [x] Add logging
- [x] Improve security
- [x] Add cross-platform support
- [x] Add type hints
- [x] Improve code organization

### Testing

- [x] Create test suite
- [x] Run all tests
- [x] Verify functionality

---

## Breaking Changes

### Syntax Changes

#### 1. Print Statements

**Python 2:**
```python
print "Hello, World!"
print output
```

**Python 3:**
```python
print("Hello, World!")
print(output)
```

**Impact**: All print statements must use parentheses.

#### 2. ConfigParser Import

**Python 2:**
```python
import ConfigParser
config = ConfigParser.RawConfigParser()
```

**Python 3:**
```python
import configparser
config = configparser.RawConfigParser()
```

**Impact**: Module name changed from `ConfigParser` to `configparser`.

#### 3. map() Function

**Python 2:**
```python
result = map(str.strip, items)  # Returns list
```

**Python 3:**
```python
result = list(map(str.strip, items))  # Must convert to list
# Or use list comprehension:
result = [s.strip() for s in items]
```

**Impact**: `map()` returns an iterator in Python 3, not a list.

#### 4. Exception Handling

**Python 2:**
```python
except Exception, e:
    print e
```

**Python 3:**
```python
except Exception as e:
    print(e)
```

**Impact**: Exception variable must use `as` keyword.

#### 5. String Formatting

**Python 2:**
```python
message = "Hello %s" % name
message = "Hello {0}".format(name)
```

**Python 3 (Recommended):**
```python
message = f"Hello {name}"  # f-strings (Python 3.6+)
```

**Impact**: Old-style `%` formatting still works but f-strings are preferred.

#### 6. File Operations

**Python 2:**
```python
f = open("file.txt", "r")
content = f.read()
f.close()
```

**Python 3 (Recommended):**
```python
with open("file.txt", "r", encoding='utf-8') as f:
    content = f.read()
```

**Impact**: Should use context managers and specify encoding.

### Behavioral Changes

#### 1. Integer Division

**Python 2:**
```python
5 / 2  # Returns 2 (integer division)
```

**Python 3:**
```python
5 / 2   # Returns 2.5 (float division)
5 // 2  # Returns 2 (integer division)
```

**Impact**: Division behavior changed (not used in this project).

#### 2. Unicode Strings

**Python 2:**
```python
s = "Hello"  # bytes
u = u"Hello"  # unicode
```

**Python 3:**
```python
s = "Hello"  # unicode (str)
b = b"Hello"  # bytes
```

**Impact**: All strings are Unicode by default in Python 3.

---

## Step-by-Step Migration

### Step 1: Set Up Python 3 Environment

```bash
# Check Python 3 version
python3 --version  # Should be 3.8 or higher

# Create virtual environment
python3 -m venv venv

# Activate virtual environment
source venv/bin/activate  # macOS/Linux
# or
venv\Scripts\activate     # Windows

# Install dependencies
pip install -r requirements.txt
```

### Step 2: Update Imports

**Before:**
```python
import ConfigParser
from winnt import MAXBYTE  # Windows-specific, unused
```

**After:**
```python
import configparser
# Removed unused winnt import
```

### Step 3: Fix Print Statements

**Before:**
```python
print "Asirikuy Monitor v0.05"
print output
```

**After:**
```python
print("Asirikuy Monitor v0.05")
print(output)
```

### Step 4: Fix map() Usage

**Before:**
```python
accountSections = map(str.strip, config.get('accounts', 'accounts').split(','))
```

**After:**
```python
accountSections = [s.strip() for s in config.get('accounts', 'accounts').split(',')]
```

### Step 5: Update Exception Handling

**Before:**
```python
except:
    raise ValueError(sys.exc_info()[1])
```

**After:**
```python
except Exception as e:
    raise ValueError(str(e)) from e
```

### Step 6: Fix File Operations

**Before:**
```python
f = open(filePath, "r")
contents = f.read()
f.close()
```

**After:**
```python
with open(filePath, "r", encoding='utf-8') as f:
    contents = f.read()
```

### Step 7: Update String Formatting

**Before:**
```python
output = "Checking %s..." % files
message = "Local Hour = %s" % hour
```

**After:**
```python
output = f"Checking {files}..."
message = f"Local Hour = {hour}"
```

### Step 8: Replace Windows-Specific Code

**Before:**
```python
subprocess.call("taskkill /f /im terminal.exe")
```

**After:**
```python
from include.process_manager import kill_mt4_processes
kill_mt4_processes()
```

### Step 9: Update Dependencies

**Before:**
- colorama 0.2.5 (Python 2)
- No requirements.txt

**After:**
- colorama >= 0.4.6 (Python 3)
- python-dotenv (for environment variables)
- psutil (for cross-platform process management)
- requirements.txt created

### Step 10: Test Everything

```bash
# Run tests
python3 tests/test_runtime.py
python3 tests/test_checker_integration.py

# Test main script
python3 checker.py -v
```

---

## Upgrade Path

### For Existing Python 2 Installations

1. **Backup Your Configuration**
   ```bash
   cp config/checker.config config/checker.config.backup
   ```

2. **Install Python 3**
   - macOS: `brew install python3`
   - Linux: `sudo apt-get install python3 python3-pip`
   - Windows: Download from python.org

3. **Set Up Virtual Environment**
   ```bash
   python3 -m venv venv
   source venv/bin/activate
   ```

4. **Install Dependencies**
   ```bash
   pip install -r requirements.txt
   ```

5. **Update Configuration** (if needed)
   - Review `config/checker.config.example`
   - Update your config file with new options (optional)

6. **Test Migration**
   ```bash
   python3 checker.py -v
   python3 checker.py -c config/checker.config
   ```

7. **Update Startup Scripts**
   - Update `.bat` files to use `python3` instead of `python`
   - Or use virtual environment activation

### For New Installations

Simply follow the installation instructions in `README.md` - everything is already Python 3 compatible.

---

## Common Issues and Solutions

### Issue 1: Import Errors

**Error:**
```
ModuleNotFoundError: No module named 'ConfigParser'
```

**Solution:**
```python
# Change:
import ConfigParser
# To:
import configparser
```

### Issue 2: Print Statement Errors

**Error:**
```
SyntaxError: Missing parentheses in call to 'print'
```

**Solution:**
```python
# Change:
print "Hello"
# To:
print("Hello")
```

### Issue 3: map() Returns Iterator

**Error:**
```
TypeError: 'map' object is not subscriptable
```

**Solution:**
```python
# Change:
result = map(str.strip, items)
result[0]  # Error!
# To:
result = list(map(str.strip, items))
# Or:
result = [s.strip() for s in items]
```

### Issue 4: File Encoding Issues

**Error:**
```
UnicodeDecodeError: 'ascii' codec can't decode byte
```

**Solution:**
```python
# Always specify encoding:
with open("file.txt", "r", encoding='utf-8') as f:
    content = f.read()
```

### Issue 5: Process Management on Windows

**Error:**
```
'taskkill' is not recognized (on non-Windows systems)
```

**Solution:**
```python
# Use cross-platform process manager:
from include.process_manager import kill_mt4_processes
kill_mt4_processes()
```

### Issue 6: Environment Variables Not Loading

**Error:**
```
Email credentials not found
```

**Solution:**
```bash
# Install python-dotenv:
pip install python-dotenv

# Create .env file:
EMAIL_FROM=your-email@gmail.com
EMAIL_PASSWORD=your-password
```

---

## Testing After Migration

### 1. Syntax Validation

```bash
python3 -m py_compile checker.py
python3 -m py_compile include/*.py
```

### 2. Runtime Tests

```bash
# Run test suite
python3 tests/test_runtime.py
python3 tests/test_checker_integration.py
```

### 3. Functional Tests

```bash
# Test version command
python3 checker.py -v

# Test with config file
python3 checker.py -c config/checker.config.example
```

### 4. Integration Tests

- Test heartbeat monitoring
- Test log file reading
- Test email sending (with test credentials)
- Test process management

---

## Migration Summary

### Files Modified

1. `checker.py` - Main script migrated
2. `include/asirikuy.py` - Core library migrated
3. `include/misc.py` - Utility functions migrated

### New Files Created

1. `include/config_manager.py` - Configuration management
2. `include/email_security.py` - Email security
3. `include/logging_config.py` - Logging configuration
4. `include/platform_utils.py` - Platform detection
5. `include/process_manager.py` - Process management
6. `include/retry_mechanism.py` - Retry logic
7. `requirements.txt` - Dependencies
8. `requirements-dev.txt` - Development dependencies
9. `tests/` - Test suite

### Improvements Made

- ✅ Python 3 compatible
- ✅ Cross-platform support (Windows, macOS, Linux)
- ✅ Enhanced security (email validation, sanitization)
- ✅ Better error handling
- ✅ Comprehensive logging
- ✅ Retry mechanisms
- ✅ Type hints
- ✅ Modern code practices

---

## Additional Resources

- [Python 2 to 3 Migration Guide](https://docs.python.org/3/howto/pyporting.html)
- [What's New in Python 3](https://docs.python.org/3/whatsnew/)
- [Porting Python 2 Code to Python 3](https://docs.python.org/3/howto/pyporting.html)

---

## Support

If you encounter issues during migration:

1. Check `log/monitor.log` for error messages
2. Review `specs/PYTHON2_ANALYSIS.md` for detailed analysis
3. Review `specs/MIGRATION_PROGRESS.md` for migration details
4. Check test results in `specs/RUNTIME_TEST_RESULTS.md`

---

**Last Updated**: December 2024

