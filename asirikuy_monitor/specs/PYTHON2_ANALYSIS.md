# Python 2 to Python 3 Migration Analysis

**Date**: December 2024  
**Project**: Asirikuy Monitor  
**Status**: Analysis Complete

## Summary

This document identifies all Python 2 syntax and dependencies that need to be updated for Python 3 compatibility.

## Files Analyzed

1. `checker.py` (203 lines) - Main monitoring script
2. `include/asirikuy.py` (535 lines) - Core library functions
3. `include/misc.py` (27 lines) - Utility functions

## Python 2 Issues Found

### checker.py

#### Print Statements (3 instances)
- Line 125: `print output` → `print(output)`
- Line 139: `print output` → `print(output)`
- Line 198: `print "Asirikuy Monitor v0.05"` → `print("Asirikuy Monitor v0.05")`

#### ConfigParser Import
- Line 32: Uses `readConfigFile()` which internally uses `ConfigParser` (from `include/asirikuy.py`)
- Needs to be updated in `include/asirikuy.py`

#### map() Usage
- Line 46: `accountSections = map(str.strip, config.get('accounts', 'accounts').split(','))`
  - **Issue**: In Python 3, `map()` returns an iterator, not a list
  - **Fix**: `accountSections = [s.strip() for s in config.get('accounts', 'accounts').split(',')]`
  - **Or**: `accountSections = list(map(str.strip, config.get('accounts', 'accounts').split(',')))`

#### quit() Usage
- Line 21: `quit()` → `sys.exit(0)`

#### winnt Import
- Line 10: `from winnt import MAXBYTE`
  - **Issue**: Windows-specific, not available on macOS/Linux
  - **Status**: Not used anywhere in the code
  - **Fix**: Remove this import (it's unused)

#### String Formatting
- Line 89: `print( "Local Hour = %s" % hour )` - Already using parentheses, but old-style formatting
- Line 90: `print( "Loca Day = %s" % day )` - Already using parentheses, but old-style formatting
- Line 118: `output = "Checking %s..." % files` - Old-style formatting
- Line 124: Uses `.format()` which is fine, but could use f-strings
- Line 128: `output = "Heart-Beat problem for instance %s" % files` - Old-style formatting
- Line 141: `("Heart-Beat problem for instance %s, system not updating for more than %s secs" % files, str(monitoringInterval*2.5))` - Old-style formatting (also has tuple issue)
- Line 173: `output = "Error detected on account %s" % accountSection` - Old-style formatting
- Line 192: `output = 'Last execution %s, sleeping for %s secs.' % (strftime("%a, %d %b %Y %X"), monitoringInterval)` - Old-style formatting

#### File Operations
- Line 114: `fp = open(files,'r')` - Should specify encoding: `open(files, 'r', encoding='utf-8')`
- Line 151: `file = open(fname,'r')` - Should specify encoding: `open(fname, 'r', encoding='utf-8')`

#### sleep() Import
- Line 195: `sleep(monitoringInterval)` - Uses `from time import *` which imports `sleep`
- **Issue**: Wildcard imports are discouraged
- **Fix**: Use `time.sleep(monitoringInterval)` and import `time` module

#### Other Issues
- Line 81: `mktime(time.localtime(time.time()))` - Redundant, can use `time.time()` directly
- Line 82: `currentGmTime = gmtime()` - Uses `from time import *`, variable is defined but never used
- Line 141: Email function call has tuple as message parameter - needs to be fixed

### include/asirikuy.py

#### ConfigParser Import
- Line 3: `import ConfigParser` → `import configparser`
- Line 68: `config = ConfigParser.RawConfigParser()` → `config = configparser.RawConfigParser()`

#### Exception Handling
- Line 71-72: `except:` → Should catch specific exceptions
- Line 72: `raise ValueError(sys.exc_info()[1])` - Python 2 style, should use `raise ValueError(str(e)) from e`

#### String Formatting
- Line 11-14: Uses old-style `%` formatting - Can be updated to f-strings
- Line 81: Uses old-style `%` formatting - Can be updated to f-strings

#### Other Issues
- Line 2: `from time import *` - Wildcard import, should be specific
- Line 1: `from ctypes import *` - Wildcard import, should be specific
- Line 36: DukascopySymbolReverse has typo: `'USD/JPY':'AUDUSD'` should be `'USD/JPY':'USDJPY'`

### include/misc.py

#### File Operations
- Line 7: `f = open(filePath, "w")` - Should specify encoding: `open(filePath, "w", encoding='utf-8')`
- Line 12: `f = open(filePath, "r")` - Should specify encoding: `open(filePath, "r", encoding='utf-8')`
- Line 18: `f = open(filePath, "w")` - Should specify encoding: `open(filePath, "w", encoding='utf-8')`

#### String Operations
- Line 19: `contents = "\n".join(contents)` - Should use `os.linesep` for cross-platform compatibility

## Dependencies Analysis

### Vendor Dependencies

#### colorama-0.2.5
- **Status**: Old version (current is 0.4.6+)
- **Python 3 Compatible**: Yes (colorama supports Python 3)
- **Action**: Update to latest version via pip
- **Usage**: Used in `checker.py` for colored terminal output

#### oauth2-1.5.211
- **Status**: Deprecated library
- **Python 3 Compatible**: No (Python 2 only)
- **Action**: Replace with `requests-oauthlib` or `authlib`
- **Usage**: Not directly used in main code (may be used by python-twitter)

#### python-twitter-1.0
- **Status**: Deprecated library (last updated 2013)
- **Python 3 Compatible**: No (Python 2 only)
- **Action**: Replace with `tweepy` or remove if not needed
- **Usage**: Not used in main monitoring code

## Windows-Specific Code

### winnt Module
- **File**: `checker.py` line 10
- **Usage**: Imported but never used
- **Action**: Remove import

### taskkill Command
- **File**: `checker.py` lines 135, 181
- **Usage**: `subprocess.call("taskkill /f /im terminal.exe")`
- **Action**: Replace with cross-platform solution using `psutil`

## Security Issues

### Hardcoded Credentials
- **File**: `config/checker.config`
- **Issue**: Email password hardcoded in config file
- **Action**: Move to environment variables or `.env` file

## Summary of Required Changes

### Syntax Changes
1. ✅ 3 print statements to fix
2. ✅ 1 ConfigParser import to fix
3. ✅ 1 map() usage to fix
4. ✅ 1 quit() to fix
5. ✅ 1 unused winnt import to remove
6. ✅ Multiple string formatting updates (optional but recommended)
7. ✅ File operations need encoding specification
8. ✅ Wildcard imports should be fixed

### Dependency Updates
1. ✅ Update colorama to latest version
2. ✅ Remove/replace oauth2 (if needed)
3. ✅ Remove/replace python-twitter (if needed)

### Cross-Platform
1. ✅ Remove winnt import
2. ✅ Replace taskkill with psutil

### Security
1. ✅ Move credentials to environment variables

## Migration Priority

**High Priority** (Required for Python 3):
1. Fix print statements
2. Fix ConfigParser import
3. Fix map() usage
4. Fix quit() usage
5. Remove unused winnt import
6. Add encoding to file operations

**Medium Priority** (Best practices):
1. Fix wildcard imports
2. Update string formatting
3. Fix exception handling
4. Update dependencies

**Low Priority** (Nice to have):
1. Cross-platform improvements
2. Security improvements
3. Code quality improvements

## Estimated Effort

- **Syntax fixes**: 1-2 hours
- **Dependency updates**: 1 hour
- **Testing**: 2-3 hours
- **Total**: 4-6 hours for basic migration

---

**Next Steps**: Begin Phase 2 migration tasks (T010-T035)

