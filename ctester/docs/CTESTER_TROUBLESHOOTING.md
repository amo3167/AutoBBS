# CTester Troubleshooting Guide

**Date**: December 2024  
**Status**: Complete  
**Task**: T035 - Create CTester integration documentation  
**Version**: Python 3.8+

## Table of Contents

1. [Common Issues](#common-issues)
2. [Error Messages](#error-messages)
3. [Debugging Tips](#debugging-tips)
4. [Performance Optimization](#performance-optimization)
5. [Platform-Specific Issues](#platform-specific-issues)

## Common Issues

### Issue 0: Log Level Not Applied (Fixed December 2024)

**Symptoms**:
- INFO messages appear despite setting `logseverity = 4` (WARNING)
- Log level setting seems to be ignored

**Status**: ✅ **FIXED** in December 2024

**Cause**: When multiple loggers initialize (CTester and Framework), the logger was using the highest severity level instead of the most restrictive one.

**Solution**: 
- **Fixed automatically** - No action needed
- The logger now correctly uses the most restrictive (lowest) severity level
- If CTester sets severity 4 (WARNING) and Framework sets severity 7 (DEBUG), the logger uses severity 4 (WARNING)

**Verification**:
```bash
# Set logseverity = 4 in config
# Run backtest
# Check log file - should only see WARNING, ERROR, CRITICAL, ALERT, EMERGENCY
# No INFO or DEBUG messages should appear
```

### Issue 0a: Currency Conversion False Errors (Fixed December 2024)

**Symptoms**:
```
[ERROR] For system No.0, no quote conversion symbol found for trade symbol BTCUSD with account currency USD.
[ERROR] For system No.0, no base conversion symbol found for trade symbol BTCUSD with account currency USD.
```

**Status**: ✅ **FIXED** in December 2024

**Cause**: The system was logging ERROR messages when no currency conversion was needed (e.g., BTCUSD with USD account currency).

**Solution**:
- **Fixed automatically** - No action needed
- The system now correctly detects when no conversion is needed
- Logs INFO message instead of ERROR when account currency matches symbol currency

**Verification**:
```bash
# Test with BTCUSD and USD account currency
# Should see INFO message: "no conversion symbols needed for trade symbol BTCUSD with account currency USD"
# No ERROR messages should appear
```

### Issue 0b: State Files with Newlines in Names (Fixed December 2024)

**Symptoms**:
- `.state` files appear with blank lines in their names when listing files
- Filenames look corrupted or have strange formatting

**Status**: ✅ **FIXED** in December 2024

**Cause**: Literal newline characters (`\n\n\n\n\n`) were being added to file paths in `InstanceStates.c`.

**Solution**:
- **Fixed automatically** - No action needed
- `.state` files now have normal filenames (e.g., `1.state`, `223456.state`)
- Old files with newlines can be cleaned up (optional)

**Cleanup** (optional):
```bash
cd ctester
rm -f tmp/*.state
```

### Issue 1: Library Not Found

**Symptoms**:
```
OSError: dlopen(libCTesterFrameworkAPI.dylib, 0x0006): tried: 'libCTesterFrameworkAPI.dylib' (no such file)
```

**Causes**:
- Library not built
- Library in wrong location
- Wrong library name for platform

**Solutions**:

1. **Build the library**:
```bash
cd core/CTesterFrameworkAPI
premake4 gmake
cd build/gmake
make
```

2. **Check library location**:
   - macOS: `bin/gmake/x64/Debug/libCTesterFrameworkAPI.dylib`
   - Linux: `bin/gmake/x64/Debug/libCTesterFrameworkAPI.so`
   - Windows: `bin/gmake/x64/Debug/CTesterFrameworkAPI.dll`

3. **Verify library loading**:
   The `loadLibrary()` function searches these paths automatically:
   - Current directory
   - `../bin/gmake/x64/Debug/`
   - `../bin/gmake/x64/Release/`
   - `./bin/gmake/x64/Debug/`
   - `./bin/gmake/x64/Release/`

4. **Check platform**:
```python
import platform
print(platform.system())  # Should be 'Darwin', 'Linux', or 'Windows'
```

### Issue 2: Historical Data Not Found

**Symptoms**:
```
FileNotFoundError: [Errno 2] No such file or directory: './history/USDJPY1987_60.csv'
```

**Causes**:
- CSV file missing
- Wrong file name format
- Wrong directory

**Solutions**:

1. **Check file naming**:
   - Format: `SYMBOL_TIMEFRAME.csv`
   - Example: `USDJPY1987_60.csv` (USDJPY1987, 60-minute timeframe)
   - Must match `pair` value in config file

2. **Check file location**:
   - Default: `./history/` directory
   - Verify path in config: `pair = USDJPY1987` → `history/USDJPY1987_60.csv`

3. **Check CSV format**:
   - Required columns: `Time,Open,High,Low,Close,Volume`
   - Time format: Unix timestamp (integer)
   - Example:
   ```
   Time,Open,High,Low,Close,Volume
   504921600,138.50,138.75,138.25,138.60,1000
   ```

4. **Verify file exists**:
```bash
ls -la history/USDJPY1987_60.csv
```

### Issue 3: Config File Parsing Errors

**Symptoms**:
```
configparser.NoOptionError: No option 'set' in section: 'strategy'
ValueError: invalid literal for int() with base 10: '6 \t\t\t; 0 = Emergency...'
```

**Causes**:
- Missing config options
- Inline comments in values
- Wrong section names

**Solutions**:

1. **Use correct option name**:
   - Use `setFile` instead of `set` (or vice versa for backward compatibility)
   - Code checks both: `setFile` first, then `set`

2. **Remove inline comments from values**:
```ini
# Wrong
logSeverity = 6 ; 0 = Emergency, 1 = Alert, ...

# Correct
logSeverity = 6
; 0 = Emergency, 1 = Alert, ...
```

3. **Check section names**:
   - Must be: `[misc]`, `[account]`, `[strategy]`, `[optimization]`
   - Case-sensitive

4. **Validate config file**:
```python
import configparser
config = configparser.ConfigParser()
config.read('config/ast.config')
print(config.sections())  # Should show all sections
```

### Issue 4: String/Bytes Encoding Errors

**Symptoms**:
```
TypeError: bytes or integer address expected instead of str instance
```

**Causes**:
- Python 3 strings not encoded to bytes
- ctypes expects bytes for `c_char_p`

**Solutions**:

1. **Encode strings to bytes**:
```python
# Wrong
symbol = 'USDJPY1987'
astdll.runPortfolioTest(..., symbol, ...)

# Correct
symbol = 'USDJPY1987'.encode('utf-8')
astdll.runPortfolioTest(..., symbol, ...)
```

2. **Check all string parameters**:
   - `pInTradeSymbol`: Must be bytes
   - `pInAccountCurrency`: Must be bytes
   - `pInBrokerName`: Must be bytes
   - `pInRefBrokerName`: Must be bytes
   - Log paths: Must be bytes

3. **Use consistent encoding**:
```python
# Always use UTF-8
symbol = symbol_string.encode('utf-8')
log_path = log_path_string.encode('utf-8')
```

### Issue 5: Set File Not Found

**Symptoms**:
```
FileNotFoundError: [Errno 2] No such file or directory: './sets/30002_USDJPY.set'
```

**Causes**:
- Set file missing
- Wrong path in config
- File in different directory

**Solutions**:

1. **Check set file location**:
   - Default: `./sets/` directory
   - Can use absolute paths in config

2. **Verify config path**:
```ini
[strategy]
setFile = 30002_USDJPY.set  # Relative to ./sets/
# OR
setFile = /absolute/path/to/30002_USDJPY.set  # Absolute path
```

3. **Check file exists**:
```bash
ls -la sets/30002_USDJPY.set
```

4. **List available set files**:
```bash
ls sets/*.set
```

### Issue 6: Import Errors

**Symptoms**:
```
ModuleNotFoundError: No module named 'include.asirikuy'
ImportError: cannot import name 'loadLibrary' from 'include.asirikuy'
```

**Causes**:
- Wrong import path
- Module not in Python path
- Circular imports

**Solutions**:

1. **Use correct import**:
```python
# Correct (from ctester directory)
from include.asirikuy import *

# Wrong
from .include.asirikuy import *  # Relative import (only for packages)
```

2. **Check Python path**:
```python
import sys
print(sys.path)  # Should include ctester directory
```

3. **Run from correct directory**:
```bash
cd ctester
python3 asirikuy_strategy_tester.py -c config/ast.config
```

4. **Check __init__.py**:
   - Ensure `include/__init__.py` exists
   - Can be empty file

### Issue 7: MPI Execution Errors

**Symptoms**:
```
ModuleNotFoundError: No module named 'mpi4py'
```

**Causes**:
- mpi4py not installed
- MPI not configured

**Solutions**:

1. **Install mpi4py** (optional):
```bash
pip3 install mpi4py
```

2. **MPI is optional**:
   - CTester works without MPI
   - MPI only needed for parallel optimization
   - Single-process execution works fine

3. **Check MPI environment**:
```bash
# Check if running under MPI
echo $PMI_RANK
echo $OMPI_COMM_WORLD_RANK

# If empty, not running under MPI (OK)
```

### Issue 8: Plot Generation Fails

**Symptoms**:
```
ModuleNotFoundError: No module named 'matplotlib'
```

**Causes**:
- matplotlib not installed
- Graphics dependencies missing

**Solutions**:

1. **Install dependencies**:
```bash
pip3 install matplotlib numpy
```

2. **Auto-installer should handle this**:
   - `auto_installer.py` installs missing packages
   - Check if it's working

3. **Disable plots**:
```ini
[account]
generate_plot = 0  # Skip plot generation
```

### Issue 9: Optimization Not Converging

**Symptoms**:
- Optimization runs indefinitely
- No improvement in results
- Population not evolving

**Causes**:
- Poor parameter ranges
- Wrong optimization settings
- Insufficient generations

**Solutions**:

1. **Adjust optimization parameters**:
```ini
[optimization]
population = 50  # Increase population
maxGenerations = 200  # Increase generations
stopIfConverged = 1  # Stop when converged
```

2. **Check parameter ranges**:
   - Ensure set file has reasonable parameter ranges
   - Too wide ranges slow convergence
   - Too narrow ranges limit exploration

3. **Try different optimization goals**:
```ini
optimizationGoal = 0  # Profit
# OR
optimizationGoal = 1  # Max DD
# OR
optimizationGoal = 3  # Profit Factor
```

4. **Monitor progress**:
   - Check optimization output file
   - Look for improvement trends
   - Stop if no improvement after many generations

### Issue 10: Memory Issues

**Symptoms**:
```
MemoryError: Unable to allocate array
OSError: [Errno 12] Cannot allocate memory
```

**Causes**:
- Too much historical data
- Too many systems in portfolio
- Insufficient system memory

**Solutions**:

1. **Reduce data size**:
   - Shorter date ranges
   - Fewer symbols
   - Lower timeframe requirements

2. **Reduce portfolio size**:
   - Fewer systems in portfolio
   - Test systems individually first

3. **Optimize memory usage**:
   - Use smaller `rateRequirements` values
   - Process in batches
   - Clear unused arrays

4. **Check system memory**:
```bash
# macOS/Linux
free -h
# OR
vm_stat  # macOS
```

## Error Messages

### C Library Error Codes

Error codes defined in `include/asirikuy.py`:

| Code | Constant | Description | Solution |
|------|----------|-------------|----------|
| 0 | SUCCESS | Operation successful | - |
| 3000 | INVALID_CURRENCY | Invalid currency code | Check currency in config |
| 3001 | UNKNOWN_SYMBOL | Symbol not recognized | Check symbol name format |
| 3002 | SYMBOL_TOO_SHORT | Symbol name too short | Use full symbol name |
| 3003 | PARSE_SYMBOL_FAILED | Failed to parse symbol | Check symbol format |
| 3004 | NO_CONVERSION_SYMBOLS | No conversion symbols | Check broker configuration |
| 3005 | NULL_POINTER | Null pointer passed | Check function parameters |
| 3006 | INVALID_STRATEGY | Invalid strategy ID | Check strategy ID (0-30) |
| 3007 | NOT_ENOUGH_MARGIN | Insufficient margin | Increase balance or reduce risk |
| 3008 | SPREAD_TOO_WIDE | Spread exceeds limit | Reduce max spread or check data |
| 3009 | NOT_ENOUGH_RATES_DATA | Insufficient rate data | Check historical data file |
| 3010 | WORST_CASE_SCENARIO | Worst case scenario | Check strategy parameters |
| 3011 | NORMALIZE_BARS_FAILED | Failed to normalize bars | Check timeframe requirements |
| 3012 | INIT_LOG_FAILED | Log initialization failed | Check log directory permissions |
| 3013 | DEINIT_LOG_FAILED | Log deinitialization failed | Check log file access |
| 3014 | ZERO_DIVIDE | Division by zero | Check calculation parameters |
| 3015 | TA_LIB_ERROR | Technical analysis library error | Check indicator parameters |
| 3016 | INVALID_TIME_OFFSET | Invalid time offset | Check timezone settings |
| 3017 | INVALID_PARAMETER | Invalid parameter value | Check all parameter values |
| 3018 | NN_TRAINING_FAILED | Neural network training failed | Check NN parameters |
| 3019 | UNKNOWN_TIMEZONE | Unknown timezone | Check timezone configuration |
| 3020 | LOCAL_TZ_MISMATCH | Local timezone mismatch | Check system timezone |
| 3021 | BROKER_TZ_MISMATCH | Broker timezone mismatch | Check broker timezone |
| 3022 | TOO_MANY_INSTANCES | Too many instances | Reduce portfolio size |
| 3023 | INVALID_CONFIG | Invalid configuration | Check config file format |
| 3024 | MISSING_CONFIG | Missing configuration | Check required config options |
| 3025 | INIT_XML_FAILED | XML initialization failed | Check XML output path |
| 3026 | UNKNOWN_INSTANCE_ID | Unknown instance ID | Check instance ID values |
| 3027 | INSUFFICIENT_MEMORY | Insufficient memory | Reduce data size or portfolio |
| 3028 | WAIT_FOR_INIT | Wait for initialization | Ensure library is initialized |

### Python Error Messages

#### `TypeError: expected c_double, got float`
**Cause**: Type mismatch in ctypes  
**Solution**: Use `c_double(value)` or ensure values are floats

#### `AttributeError: 'NoneType' object has no attribute 'value'`
**Cause**: Error pointer not initialized  
**Solution**: Initialize error pointer: `error_ptr = POINTER(c_char_p)()`

#### `OSError: [Errno 2] No such file or directory`
**Cause**: File not found  
**Solution**: Check file paths, verify files exist

#### `ValueError: invalid literal for int()`
**Cause**: String cannot be converted to int  
**Solution**: Check value format, remove inline comments

## Debugging Tips

### 1. Enable Verbose Logging

Set high log severity in config:

```ini
[misc]
logSeverity = 7  ; Debug level (most verbose)
```

Check log file:
```bash
tail -f log/AsirikuyCTester.log
```

### 2. Add Debug Print Statements

Add print statements at key points:

```python
print(f"Loading rates for {symbol}, timeframe {timeframe}")
print(f"Number of candles: {num_candles}")
print(f"Settings: {settings[ORIGINAL_EQUITY]}")
```

### 3. Validate Data Structures

Check data before passing to C library:

```python
# Check rates array
print(f"Rates array size: {len(rates_array)}")
print(f"First rate: time={rates_array[0].time}, open={rates_array[0].open}")

# Check settings
print(f"Settings array: {[settings[i] for i in range(10)]}")

# Check symbols
print(f"Symbols: {[s.decode('utf-8') for s in symbols_array]}")
```

### 4. Use Python Debugger

Run with debugger:

```bash
python3 -m pdb asirikuy_strategy_tester.py -c config/ast.config
```

Set breakpoints:
```python
import pdb; pdb.set_trace()  # Breakpoint
```

### 5. Check Callback Execution

Verify callbacks are called:

```python
def testUpdate(testId, percentage, lastOrder, balance, symbol):
    print(f"CALLBACK: testId={testId}, percentage={percentage}, balance={balance}")
    # ... rest of callback ...
```

### 6. Validate Configuration

Create config validator:

```python
def validateConfig(config_path):
    import configparser
    config = configparser.ConfigParser()
    config.read(config_path)
    
    # Check sections
    required_sections = ['misc', 'account', 'strategy', 'optimization']
    for section in required_sections:
        if not config.has_section(section):
            print(f"ERROR: Missing section [{section}]")
    
    # Check options
    required_options = {
        'strategy': ['setFile', 'pair', 'strategyID'],
        'account': ['balance', 'currency']
    }
    for section, options in required_options.items():
        for option in options:
            if not config.has_option(section, option):
                print(f"ERROR: Missing option {section}.{option}")
    
    print("Configuration validation complete")
```

### 7. Test with Minimal Configuration

Start with simplest possible config:

```ini
[misc]
logSeverity = 6

[account]
currency = USD
brokerName = Test
refBrokerName = Test
balance = 10000
leverage = 100
contractSize = 100000
digits = 3
stopOutPercent = 1
minimumStop = 0.05
spread = 0.03
minLotSize = 0.00001
generate_plot = 0

[strategy]
setFile = simple.set
pair = USDJPY1987
strategyID = 22
passedTimeFrame = 60
fromDate = 01/01/20
toDate = 01/01/21
rateRequirements = 1000
symbolRequirements = D
timeframeRequirements = 0

[optimization]
optimize = 0
```

### 8. Check Library Version

Verify library version:

```python
major = c_int()
minor = c_int()
bugfix = c_int()
astdll.getCTesterFrameworkVersion(byref(major), byref(minor), byref(bugfix))
print(f"Library version: {major.value}.{minor.value}.{bugfix.value}")
```

## Performance Optimization

### 1. Reduce Historical Data Size

- Use shorter date ranges
- Use higher timeframes (fewer bars)
- Reduce `rateRequirements` values

### 2. Optimize Portfolio Size

- Test systems individually first
- Combine only proven systems
- Limit to 3-5 systems per portfolio

### 3. Use MPI for Parallel Optimization

```bash
mpirun -np 4 python3 asirikuy_strategy_tester.py -c config/optimize.config -oo results
```

### 4. Disable Unnecessary Features

```ini
[account]
generate_plot = 0  # Skip plot generation

[misc]
logSeverity = 3  # Error level only (less logging overhead)
```

### 5. Optimize Set File Parameters

- Use reasonable parameter ranges
- Avoid too many parameters
- Focus on key parameters

### 6. Cache Historical Data

Load data once and reuse:

```python
# Load and cache
cached_rates = {}

def loadRatesCached(symbol, timeframe):
    key = f"{symbol}_{timeframe}"
    if key not in cached_rates:
        cached_rates[key] = loadRates(symbol, timeframe)
    return cached_rates[key]
```

## Platform-Specific Issues

### macOS

**Issue**: Library loading with `.dylib` extension  
**Solution**: Use `libCTesterFrameworkAPI.dylib` (with `lib` prefix)

**Issue**: Code signing requirements  
**Solution**: Disable code signing for development:
```bash
codesign --remove-signature libCTesterFrameworkAPI.dylib
```

### Linux

**Issue**: Library dependencies  
**Solution**: Install required system libraries:
```bash
sudo apt-get install libgomp1  # OpenMP
```

**Issue**: Library path  
**Solution**: Add to `LD_LIBRARY_PATH`:
```bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./bin/gmake/x64/Debug
```

### Windows

**Issue**: DLL dependencies  
**Solution**: Ensure all DLL dependencies are in PATH or same directory

**Issue**: Path separators  
**Solution**: Use forward slashes or raw strings:
```python
path = r"C:\path\to\file"  # Raw string
# OR
path = "C:/path/to/file"  # Forward slashes work too
```

## Getting Help

1. **Check Logs**: Always check `log/AsirikuyCTester.log` first
2. **Run Diagnostics**: Run `python3 ast_diagnostics.py` to verify installation
3. **Review Documentation**: See [CTester Usage Guide](CTESTER_USAGE_GUIDE.md) and [CTester Integration Guide](CTESTER_INTEGRATION_GUIDE.md)
4. **Check Error Codes**: Refer to error code table above
5. **Validate Configuration**: Use minimal config to isolate issues

## Next Steps

- See [CTester Usage Guide](CTESTER_USAGE_GUIDE.md) for usage instructions
- See [CTester Integration Guide](CTESTER_INTEGRATION_GUIDE.md) for developer documentation
- See [CTester Interface Documentation](CTESTER_INTERFACE_DOCUMENTATION.md) for API reference

