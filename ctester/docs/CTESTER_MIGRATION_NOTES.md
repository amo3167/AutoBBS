# CTester Migration Notes

**Date**: December 2024  
**Status**: Complete  
**Task**: T065 - Create comprehensive integration documentation  
**Version**: Python 3.8+

## Table of Contents

1. [Recent Changes (December 2024)](#recent-changes-december-2024)
2. [Breaking Changes](#breaking-changes)
3. [Bug Fixes](#bug-fixes)
4. [New Features](#new-features)
5. [Configuration Changes](#configuration-changes)
6. [Migration Guide](#migration-guide)

## Recent Changes (December 2024)

### Log Level Handling Fix

**Issue**: When multiple loggers initialize (CTester and Framework), the logger was using the highest severity level instead of the most restrictive one, causing INFO messages to appear despite WARNING level settings.

**Fix**: Updated `AsirikuyLogger.c` to use the most restrictive (lowest) severity level when multiple loggers initialize.

**Impact**: Log level settings now work correctly. If CTester sets severity 4 (WARNING) and Framework sets severity 7 (DEBUG), the logger will use severity 4 (WARNING), filtering out INFO and DEBUG messages.

**Files Changed**:
- `core/AsirikuyCommon/src/AsirikuyLogger.c`

**Migration**: No code changes required. Log level behavior is now correct.

### Currency Conversion Error Fixes

**Issue**: False ERROR messages were logged when no currency conversion was needed (e.g., BTCUSD with USD account currency).

**Fix**: Updated `tester.c` to properly detect when no conversion is needed and log INFO messages instead of ERROR messages.

**Impact**: 
- No more false error messages for symbols where account currency matches quote/base currency
- Better logging clarity
- Optimized conversion symbol checking logic

**Files Changed**:
- `core/CTesterFrameworkAPI/src/tester.c`

**Migration**: No code changes required. Error messages are now more accurate.

### Instance State File Naming Fix

**Issue**: `.state` files had literal newline characters (`\n\n\n\n\n`) embedded in their filenames, causing display issues.

**Fix**: Removed literal newline characters from file path construction in `InstanceStates.c`.

**Impact**: 
- `.state` files now have normal filenames (e.g., `1.state`, `223456.state`)
- No more display issues with filenames
- Cleaner file system

**Files Changed**:
- `core/TradingStrategies/src/InstanceStates.c`

**Migration**: 
- Old `.state` files with newlines in names should be cleaned up
- New `.state` files will have normal names
- No code changes required

### Automated Backtesting Script

**New Feature**: Created `run_btcusd_macd_860013.sh` script for automated backtesting.

**Features**:
- Command-line parameter support (`--fromdate`, `--todate`, `--logseverity`)
- Automatic cleanup of old results
- Organized file structure (`tmp/SYMBOL_STRATEGYID/`)
- Config file preservation (saves used config if parameters provided)

**Usage**:
```bash
# Run with default config
./run_btcusd_macd_860013.sh

# Run with custom parameters
./run_btcusd_macd_860013.sh --fromdate 2020-01-02 --todate 2021-01-01 --logseverity 6
```

**Files Added**:
- `ctester/run_btcusd_macd_860013.sh`

**Migration**: No migration needed. This is a new convenience script.

## Breaking Changes

**None**: All changes are backward compatible. No breaking changes in this release.

## Bug Fixes

### Log Level Not Applied
- **Fixed**: Logger now correctly uses most restrictive severity level
- **Affected**: All users using multiple loggers
- **Workaround**: None needed - fixed automatically

### Currency Conversion False Errors
- **Fixed**: No more false ERROR messages for matching currencies
- **Affected**: Users testing symbols where account currency matches symbol currency
- **Workaround**: None needed - fixed automatically

### State File Naming Issues
- **Fixed**: `.state` files now have normal filenames
- **Affected**: All users (cosmetic issue)
- **Workaround**: None needed - fixed automatically

## New Features

### Automated Backtesting Script
- **Feature**: `run_btcusd_macd_860013.sh` script
- **Purpose**: Automate backtesting with parameter support
- **Benefits**: 
  - Cleaner workflow
  - Organized results
  - Easy parameter testing

## Configuration Changes

### Log Level Configuration

**Before**: Log level might not be applied correctly if multiple loggers initialize.

**After**: Log level is always applied correctly using most restrictive setting.

**Action Required**: None. Existing config files work as expected.

### State File Location

**Before**: `.state` files had newlines in filenames (cosmetic issue).

**After**: `.state` files have normal filenames.

**Action Required**: Clean up old `.state` files if desired (optional).

## Migration Guide

### Step 1: Update Code

No code changes required. All fixes are automatic.

### Step 2: Rebuild Libraries

Rebuild the affected libraries:

```bash
cd /path/to/AutoBBS
./build.sh -c
```

This will rebuild:
- `AsirikuyCommon` (log level fix)
- `CTesterFrameworkAPI` (currency conversion fix)
- `TradingStrategies` (file naming fix)

### Step 3: Clean Up Old Files (Optional)

If you have old `.state` files with newlines in their names, you can clean them up:

```bash
cd ctester
rm -f tmp/*.state
```

The script `run_btcusd_macd_860013.sh` automatically cleans up `.state` files.

### Step 4: Verify Installation

Run a test backtest to verify everything works:

```bash
cd ctester
./run_btcusd_macd_860013.sh --fromdate 2020-01-02 --todate 2021-01-01 --logseverity 4
```

Check that:
- ✅ Log level is applied correctly (no INFO messages with severity 4)
- ✅ No false currency conversion errors
- ✅ `.state` files have normal names
- ✅ Results are organized correctly

### Step 5: Update Documentation

Review and update any custom documentation to reflect:
- Correct log level behavior
- Accurate error message expectations
- New automated backtesting script

## Compatibility

- **Python**: 3.8+ (no changes)
- **macOS**: ✅ Compatible
- **Linux**: ✅ Compatible (expected)
- **Windows**: ✅ Compatible (expected)
- **Config Files**: ✅ Backward compatible
- **API**: ✅ Backward compatible

## Testing

All changes have been tested:
- ✅ Log level handling verified
- ✅ Currency conversion logic verified
- ✅ File naming verified
- ✅ Automated script tested
- ✅ Full clean build tested
- ✅ End-to-end backtest verified

## Support

For issues or questions:
1. Check `CTESTER_TROUBLESHOOTING.md` for common issues
2. Review `CTESTER_USAGE_GUIDE.md` for usage examples
3. Check commit history for detailed change logs

## Version History

### December 2024
- Fixed log level handling
- Fixed currency conversion false errors
- Fixed instance state file naming
- Added automated backtesting script

