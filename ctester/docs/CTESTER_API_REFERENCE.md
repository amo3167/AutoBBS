# CTester API Reference

**Date**: December 2024  
**Status**: Complete  
**Task**: T065 - Create comprehensive integration documentation  
**Version**: Python 3.8+

## Table of Contents

1. [Overview](#overview)
2. [CTesterFrameworkAPI Functions](#ctesterframeworkapi-functions)
3. [Data Structures](#data-structures)
4. [Error Codes](#error-codes)
5. [Logging API](#logging-api)
6. [Recent Changes (December 2024)](#recent-changes-december-2024)

## Overview

CTester uses `CTesterFrameworkAPI` (C library) for strategy execution. The Python layer handles configuration, data loading, and result processing.

### Library Location

- **macOS**: `bin/gmake/x64/Debug/libCTesterFrameworkAPI.dylib`
- **Linux**: `bin/gmake/x64/Debug/libCTesterFrameworkAPI.so`
- **Windows**: `bin/gmake/x64/Debug/CTesterFrameworkAPI.dll`

## CTesterFrameworkAPI Functions

### Initialization

#### `initCTesterFramework`

```c
int __stdcall initCTesterFramework(char* pAsirikuyTesterLog, int severityLevel);
```

**Purpose**: Initialize the CTester framework

**Parameters**:
- `pAsirikuyTesterLog`: Path to log file (UTF-8 encoded string)
- `severityLevel`: Log severity level (0-7)
  - `0` = Emergency
  - `1` = Alert
  - `2` = Critical
  - `3` = Error
  - `4` = Warning
  - `5` = Notice
  - `6` = Info
  - `7` = Debug

**Returns**: Error code (0 = success)

**Python Usage**:
```python
astdll.initCTesterFramework.argtypes = [c_char_p, c_int]
astdll.initCTesterFramework.restype = c_int

log_path = './log/AsirikuyCTester.log'.encode('utf-8')
severity = 4  # Warning level
result = astdll.initCTesterFramework(log_path, severity)
```

**Notes** (December 2024):
- Log level now uses most restrictive (lowest) severity when multiple loggers initialize
- If CTester sets severity 4 and Framework sets severity 7, the logger uses severity 4

#### `getCTesterFrameworkVersion`

```c
void __stdcall getCTesterFrameworkVersion(int* pMajor, int* pMinor, int* pBugfix);
```

**Purpose**: Get framework version

**Python Usage**:
```python
major = c_int()
minor = c_int()
bugfix = c_int()
astdll.getCTesterFrameworkVersion(byref(major), byref(minor), byref(bugfix))
print(f"Version: {major.value}.{minor.value}.{bugfix.value}")
```

### Portfolio Testing

#### `runPortfolioTest`

```c
TestResult __stdcall runPortfolioTest(
    int              testId,
    double**         pInSettings,
    char**           pInTradeSymbol,
    char*            pInAccountCurrency,
    char*            pInBrokerName,
    char*            pInRefBrokerName,
    double**         pInAccountInfo,
    TestSettings*    testSettings,
    CRatesInfo**     pRatesInfo,
    int              numCandles,
    int              numSystems,
    ASTRates***      pRates,
    double           minLotSize,
    void (*testUpdate)(int testId, double percentageOfTestCompleted, COrderInfo lastOrder, double currentBalance, char* symbol),
    void (*testFinished)(TestResult testResults),
    void (*signalUpdate)(TradeSignal signal)
);
```

**Purpose**: Execute portfolio test with multiple strategies

**Returns**: `TestResult` structure

**Python Usage**: See `CTESTER_INTEGRATION_GUIDE.md` for detailed examples

**Notes** (December 2024):
- Currency conversion logic improved
- No false errors when account currency matches symbol currency
- Optimized conversion symbol checking

### Symbol Analysis

#### `C_getConversionSymbols`

```c
int C_getConversionSymbols(
    char* pSymbol,
    char* pAccountCurrency,
    char* pBaseConversionSymbol,
    char* pQuoteConversionSymbol
);
```

**Purpose**: Get conversion symbols for currency conversion

**Parameters**:
- `pSymbol`: Trade symbol (e.g., "BTCUSD")
- `pAccountCurrency`: Account currency (e.g., "USD")
- `pBaseConversionSymbol`: Output buffer for base conversion symbol
- `pQuoteConversionSymbol`: Output buffer for quote conversion symbol

**Returns**: 
- `SUCCESS` (0): Conversion symbols found or not needed
- `NO_CONVERSION_SYMBOLS`: Conversion needed but symbols not found

**Notes** (December 2024):
- Returns `SUCCESS` with empty strings when no conversion is needed
- No false errors for matching currencies

## Data Structures

### TestResult

```c
typedef struct TestResult_t {
    double totalReturn;
    double maxDDDepth;
    double maxDDLength;
    double pf;              // Profit factor
    double cagr;            // CAGR
    double sharpe;          // Sharpe ratio
    int totalTrades;
    int numLongs;
    int numShorts;
    double r2;
    double winning;           // Win rate
    double risk_reward;
    char symbol[MAX_SYMBOL_LENGTH];
    int testId;
} TestResult;
```

### COrderInfo

```c
typedef struct COrderInfo_t {
    int orderNumber;
    int orderType;          // BUY=0, SELL=1
    int openTime;
    int closeTime;
    double openPrice;
    double closePrice;
    double lots;
    double profit;
    double sl;              // Stop loss
    double tp;              // Take profit
    double balance;
    int id;
    char pair[MAX_SYMBOL_LENGTH];
    double swap;
} COrderInfo;
```

### CRates

```c
typedef struct CRates_t {
    double open;
    double high;
    double low;
    double close;
    int time;
    long volume;
    int spread;
} CRates;
```

### CRatesInfo

```c
typedef struct CRatesInfo_t {
    int symbolDigits;
    int timeframe;
    int numBars;
    int startTime;
    int endTime;
} CRatesInfo;
```

## Error Codes

| Code | Name | Description |
|------|------|-------------|
| 0 | SUCCESS | Operation successful |
| 1 | FAILURE | General failure |
| 3000 | SYMBOL_NOT_FOUND | Symbol not found |
| 3001 | INVALID_SYMBOL | Invalid symbol format |
| 3002 | SYMBOL_TOO_SHORT | Symbol name too short |
| 3003 | PARSE_SYMBOL_FAILED | Failed to parse symbol |
| 3004 | NO_CONVERSION_SYMBOLS | No conversion symbols found |
| 3005 | NULL_POINTER | Null pointer passed |
| 3006 | INVALID_STRATEGY | Invalid strategy ID |
| 3007 | NOT_ENOUGH_MARGIN | Insufficient margin |
| 3008 | SPREAD_TOO_WIDE | Spread exceeds limit |
| 3009 | NOT_ENOUGH_RATES_DATA | Insufficient rate data |
| 3010 | WORST_CASE_SCENARIO | Worst case scenario |
| 3011 | NORMALIZE_BARS_FAILED | Failed to normalize bars |
| 3012 | INIT_LOG_FAILED | Log initialization failed |
| 3013 | DEINIT_LOG_FAILED | Log deinitialization failed |
| 3014 | ZERO_DIVIDE | Division by zero |
| 3015 | TA_LIB_ERROR | Technical analysis library error |
| 3016 | INVALID_TIME_OFFSET | Invalid time offset |
| 3017 | INVALID_PARAMETER | Invalid parameter value |
| 3018 | NN_TRAINING_FAILED | Neural network training failed |
| 3019 | UNKNOWN_TIMEZONE | Unknown timezone |
| 3020 | LOCAL_TZ_MISMATCH | Local timezone mismatch |
| 3021 | BROKER_TZ_MISMATCH | Broker timezone mismatch |
| 3022 | TOO_MANY_INSTANCES | Too many instances |
| 3023 | INVALID_CONFIG | Invalid configuration |
| 3024 | MISSING_CONFIG | Missing configuration |
| 3025 | INIT_XML_FAILED | XML initialization failed |
| 3026 | UNKNOWN_INSTANCE_ID | Unknown instance ID |
| 3027 | INSUFFICIENT_MEMORY | Insufficient memory |
| 3028 | WAIT_FOR_INIT | Wait for initialization |

## Logging API

### Log Severity Levels

The logger uses severity levels where **lower numbers = more restrictive**:

- **0 - EMERGENCY**: System is unusable
- **1 - ALERT**: Action must be taken immediately
- **2 - CRITICAL**: Critical conditions
- **3 - ERROR**: Error conditions
- **4 - WARNING**: Warning conditions
- **5 - NOTICE**: Normal but significant conditions
- **6 - INFO**: Informational messages
- **7 - DEBUG**: Debug-level messages

### Log Level Behavior (December 2024)

When multiple loggers initialize:
- **Before**: Used highest (least restrictive) severity level
- **After**: Uses lowest (most restrictive) severity level

**Example**:
- CTester initializes with severity 4 (WARNING)
- Framework initializes with severity 7 (DEBUG)
- **Result**: Logger uses severity 4 (WARNING) - only WARNING, ERROR, CRITICAL, ALERT, EMERGENCY are logged

### Log Files

- **CTester Log**: `log/AsirikuyCTester.log`
- **Framework Log**: `log/AsirikuyFramework.log`

Both logs use the same severity level (most restrictive).

## Recent Changes (December 2024)

### Log Level Handling

**Change**: Logger now uses most restrictive severity when multiple loggers initialize.

**Impact**: Log level settings are now correctly applied.

**Files**: `core/AsirikuyCommon/src/AsirikuyLogger.c`

### Currency Conversion

**Change**: Improved detection of when currency conversion is not needed.

**Impact**: No false ERROR messages for matching currencies.

**Files**: `core/CTesterFrameworkAPI/src/tester.c`

### File Naming

**Change**: Fixed `.state` file naming to remove literal newlines.

**Impact**: Normal filenames for state files.

**Files**: `core/TradingStrategies/src/InstanceStates.c`

## Python Integration

### Library Loading

```python
from include.asirikuy import loadLibrary
import platform

system = platform.system()
if system == "Darwin":
    astdll = loadLibrary('libCTesterFrameworkAPI.dylib')
elif system == "Linux":
    astdll = loadLibrary('libCTesterFrameworkAPI.so')
elif system == "Windows":
    astdll = loadLibrary('CTesterFrameworkAPI')
```

### Function Signatures

All functions use `__stdcall` calling convention (ignored on macOS/Linux).

### String Encoding

All string parameters must be UTF-8 encoded bytes:

```python
symbol = 'BTCUSD'.encode('utf-8')
log_path = './log/test.log'.encode('utf-8')
```

## See Also

- `CTESTER_INTEGRATION_GUIDE.md` - Detailed integration guide
- `CTESTER_USAGE_GUIDE.md` - Usage examples
- `CTESTER_TROUBLESHOOTING.md` - Troubleshooting guide
- `CTESTER_MIGRATION_NOTES.md` - Migration notes
- `CTESTER_INTERFACE_DOCUMENTATION.md` - Interface documentation

