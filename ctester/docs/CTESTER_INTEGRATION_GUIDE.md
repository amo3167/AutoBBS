# CTester Integration Guide

**Date**: December 2024  
**Status**: Complete  
**Task**: T035 - Create CTester integration documentation  
**Version**: Python 3.8+

## Table of Contents

1. [Introduction](#introduction)
2. [Architecture Overview](#architecture-overview)
3. [API Reference](#api-reference)
4. [Data Structures](#data-structures)
5. [Integration Points](#integration-points)
6. [Code Examples](#code-examples)
7. [Extension Guide](#extension-guide)
8. [Best Practices](#best-practices)

## Introduction

This guide is for developers who want to:
- Integrate CTester into their own applications
- Extend CTester functionality
- Understand the internal architecture
- Modify or customize CTester behavior

### Prerequisites

- Python 3.8+ programming experience
- Understanding of `ctypes` for C library interop
- Familiarity with trading/backtesting concepts
- Basic knowledge of C/C++ data structures

## Architecture Overview

### System Components

```
┌─────────────────────────────────────────────────────────┐
│                    Python Layer                         │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐ │
│  │   Config     │  │     Data     │  │   Graphics   │ │
│  │  Management  │  │   Loading    │  │   & Reports  │ │
│  └──────────────┘  └──────────────┘  └──────────────┘ │
│                                                         │
│  ┌──────────────────────────────────────────────────┐  │
│  │         asirikuy_strategy_tester.py              │  │
│  │         (Main Orchestration)                      │  │
│  └──────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────┘
                          │
                          │ ctypes
                          ▼
┌─────────────────────────────────────────────────────────┐
│              CTesterFrameworkAPI (C Library)             │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐ │
│  │   Strategy   │  │  Portfolio   │  │ Optimization │ │
│  │  Execution   │  │   Testing     │  │   Engine     │ │
│  └──────────────┘  └──────────────┘  └──────────────┘ │
└─────────────────────────────────────────────────────────┘
```

### Key Files

| File | Purpose |
|------|---------|
| `asirikuy_strategy_tester.py` | Main entry point, test orchestration |
| `include/asirikuy.py` | C library interface, ctypes definitions |
| `include/mt.py` | Set file parsing, configuration management |
| `include/graphics.py` | Plotting and visualization |
| `include/auto_installer.py` | Dependency management |
| `include/misc.py` | Utility functions |

### Data Flow

1. **Configuration Loading**
   - Read `.config` file → `configparser`
   - Read `.set` files → `MT4Set` class
   - Parse strategy parameters

2. **Historical Data Loading**
   - Load CSV files from `history/` directory
   - Convert to `ASTRates` structures
   - Build `CRatesInfo` metadata

3. **Library Initialization**
   - Load `CTesterFrameworkAPI` shared library
   - Initialize framework with log path
   - Set up callback functions

4. **Test Execution**
   - Call `runPortfolioTest()` or `runOptimizationMultipleSymbols()`
   - Process callbacks for progress updates
   - Collect results

5. **Result Processing**
   - Convert C structures to Python objects
   - Generate XML output
   - Create plots and HTML reports

## API Reference

### Library Loading

**Function**: `loadLibrary(library_name)`

**Location**: `include/asirikuy.py`

**Description**: Loads the CTesterFrameworkAPI shared library, searching common build directories.

**Example**:
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

**Search Paths**:
- Current directory
- `../bin/gmake/x64/Debug/`
- `../bin/gmake/x64/Release/`
- `./bin/gmake/x64/Debug/`
- `./bin/gmake/x64/Release/`

### Initialization

**Function**: `initCTesterFramework(log_path, severity_level)`

**C Signature**:
```c
int __stdcall initCTesterFramework(char* pAsirikuyTesterLog, int severityLevel);
```

**Python Usage**:
```python
astdll.initCTesterFramework.argtypes = [c_char_p, c_int]
astdll.initCTesterFramework.restype = c_int

log_path = './log/AsirikuyCTester.log'.encode('utf-8')
severity = 6  # Info level
result = astdll.initCTesterFramework(log_path, severity)
if result != 0:
    print(f"Initialization failed with error code: {result}")
```

**Parameters**:
- `log_path`: Path to log file (bytes, UTF-8 encoded)
- `severity_level`: Log severity (0-7, see error codes)

**Returns**: Error code (0 = success)

### Portfolio Test

**Function**: `runPortfolioTest(...)`

**C Signature**:
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
    char**           pError
);
```

**Python Usage**:
```python
from include.asirikuy import *

# Define callback function
TEST_UPDATE = CFUNCTYPE(c_void_p, c_int, c_double, COrderInfo, c_double, c_char_p)
def testUpdate(testId, percentage, lastOrder, balance, symbol):
    print(f"Test {testId}: {percentage:.1f}% complete, Balance: {balance:.2f}")

testUpdate_c = TEST_UPDATE(testUpdate)

# Prepare data structures
settings_array = (POINTER(c_double) * numSystems)()
symbols_array = (c_char_p * numSystems)()
rates_info_array = (POINTER(CRatesInfo) * numSystems)()
rates_array = (POINTER(POINTER(ASTRates)) * numSystems)()

# ... populate arrays ...

# Call function
astdll.runPortfolioTest.argtypes = [
    c_int,                          # testId
    POINTER(POINTER(c_double)),     # pInSettings
    POINTER(c_char_p),              # pInTradeSymbol
    c_char_p,                       # pInAccountCurrency
    c_char_p,                       # pInBrokerName
    c_char_p,                       # pInRefBrokerName
    POINTER(POINTER(c_double)),     # pInAccountInfo
    POINTER(TestSettings),          # testSettings
    POINTER(POINTER(CRatesInfo)),  # pRatesInfo
    c_int,                          # numCandles
    c_int,                          # numSystems
    POINTER(POINTER(POINTER(ASTRates))),  # pRates
    c_double,                       # minLotSize
    TEST_UPDATE,                    # testUpdate callback
    POINTER(c_char_p)               # pError
]

result = astdll.runPortfolioTest(
    testId,
    settings_array,
    symbols_array,
    account_currency.encode('utf-8'),
    broker_name.encode('utf-8'),
    ref_broker_name.encode('utf-8'),
    account_info_array,
    test_settings,
    rates_info_array,
    num_candles,
    num_systems,
    rates_array,
    min_lot_size,
    testUpdate_c,
    error_ptr
)
```

### Optimization

**Function**: `runOptimizationMultipleSymbols(...)`

**C Signature**:
```c
int __stdcall runOptimizationMultipleSymbols(
    int                              testId,
    double**                         pInSettings,
    char**                           pInTradeSymbol,
    char*                            pInAccountCurrency,
    char*                            pInBrokerName,
    char*                            pInRefBrokerName,
    double**                         pInAccountInfo,
    TestSettings*                    testSettings,
    CRatesInfo**                     pRatesInfo,
    int                              numCandles,
    int                              numSystems,
    ASTRates***                      pRates,
    double                           minLotSize,
    GeneticOptimizationSettings*     optimizationSettings,
    void (*optimizationUpdate)(TestResult result, double* pSettings, int iteration),
    void (*optimizationFinished)(),
    char**                           pError
);
```

**Python Usage**:
```python
# Define callbacks
OPTIMIZATION_UPDATE = CFUNCTYPE(c_void_p, TestResult, POINTER(c_double), c_int)
OPTIMIZATION_FINISHED = CFUNCTYPE(c_void_p)

def optimizationUpdate(result, settings, iteration):
    print(f"Iteration {iteration}: Profit={result.profit:.2f}, DD={result.maxDD:.2f}")

def optimizationFinished():
    print("Optimization complete!")

optimizationUpdate_c = OPTIMIZATION_UPDATE(optimizationUpdate)
optimizationFinished_c = OPTIMIZATION_FINISHED(optimizationFinished)

# Prepare optimization settings
opt_settings = GeneticOptimizationSettings()
opt_settings.population = 25
opt_settings.maxGenerations = 100
opt_settings.optimizationGoal = 0  # Profit
# ... set other parameters ...

# Call function
result = astdll.runOptimizationMultipleSymbols(
    testId,
    settings_array,
    symbols_array,
    account_currency.encode('utf-8'),
    broker_name.encode('utf-8'),
    ref_broker_name.encode('utf-8'),
    account_info_array,
    test_settings,
    rates_info_array,
    num_candles,
    num_systems,
    rates_array,
    min_lot_size,
    opt_settings,
    optimizationUpdate_c,
    optimizationFinished_c,
    error_ptr
)
```

## Data Structures

### Python ctypes Structures

All structures are defined in `include/asirikuy.py`. Key structures:

#### `ASTRates`
```python
class ASTRates(Structure):
    _fields_ = [
        ("time", c_int),
        ("open", c_double),
        ("high", c_double),
        ("low", c_double),
        ("close", c_double),
        ("volume", c_double),
        ("swapLong", c_double),
        ("swapShort", c_double)
    ]
```

#### `COrderInfo`
```python
class COrderInfo(Structure):
    _fields_ = [
        ("order", c_int),
        ("type", c_int),
        ("symbol", c_char * 32),
        ("lots", c_double),
        ("openPrice", c_double),
        ("openTime", c_int),
        ("closePrice", c_double),
        ("closeTime", c_int),
        ("profit", c_double),
        ("swap", c_double),
        ("comment", c_char * 128)
    ]
```

#### `TestResult`
```python
class TestResult(Structure):
    _fields_ = [
        ("numTrades", c_int),
        ("profit", c_double),
        ("maxDD", c_double),
        ("maxDDLength", c_int),
        ("PF", c_double),
        ("R2", c_double),
        ("ulcerIndex", c_double),
        ("sharpe", c_double),
        ("CAGR", c_double),
        ("CAGRToMaxDD", c_double),
        ("numShorts", c_int),
        ("numLongs", c_int)
    ]
```

#### `CRatesInfo`
```python
class CRatesInfo(Structure):
    _fields_ = [
        ("isEnabled", c_int),
        ("actualTimeFrame", c_int),
        ("requiredTimeFrame", c_int),
        ("totalBarsRequired", c_int),
        ("ratesArraySize", c_int),
        ("digits", c_double),
        ("point", c_double)
    ]
```

#### `TestSettings`
```python
class TestSettings(Structure):
    _fields_ = [
        ("fromDate", c_int),
        ("toDate", c_int)
    ]
```

#### `GeneticOptimizationSettings`
```python
class GeneticOptimizationSettings(Structure):
    _fields_ = [
        ("population", c_int),
        ("crossoverProbability", c_double),
        ("mutationProbability", c_double),
        ("migrationProbability", c_double),
        ("evolutionaryMode", c_int),
        ("elitismMode", c_int),
        ("mutationMode", c_int),
        ("crossoverMode", c_int),
        ("maxGenerations", c_int),
        ("stopIfConverged", c_int),
        ("discardAssymetricSets", c_int),
        ("minTradesAYear", c_int),
        ("optimizationGoal", c_int)
    ]
```

### Settings Array Indices

Strategy settings are passed as `double[64]` arrays. Key indices (defined in `include/asirikuy.py`):

| Index | Constant | Description |
|-------|----------|-------------|
| 0 | ORIGINAL_EQUITY | Starting balance |
| 1 | ACCOUNT_RISK_PERCENT | Risk percentage |
| 2 | MAX_DRAWDOWN_PERCENT | Max drawdown limit |
| 3 | MAX_SPREAD_PIPS | Maximum spread |
| 4 | TIMEFRAME | Strategy timeframe |
| 5 | OPERATIONAL_MODE | Operational mode |
| ... | ... | (See `include/asirikuy.py` for full list) |

## Integration Points

### 1. Custom Callback Functions

You can customize progress callbacks:

```python
def customTestUpdate(testId, percentage, lastOrder, balance, symbol):
    # Custom logging
    logger.info(f"Test {testId}: {percentage:.1f}% - Balance: {balance:.2f}")
    
    # Custom notifications
    if percentage > 0 and percentage % 10 == 0:
        send_notification(f"Test {testId} is {percentage:.0f}% complete")
    
    # Custom data collection
    save_intermediate_result(testId, balance, lastOrder)

TEST_UPDATE = CFUNCTYPE(c_void_p, c_int, c_double, COrderInfo, c_double, c_char_p)
custom_callback = TEST_UPDATE(customTestUpdate)
```

### 2. Custom Data Loading

Extend data loading for different formats:

```python
def loadRatesFromDatabase(symbol, timeframe, from_date, to_date):
    """Load rates from database instead of CSV"""
    # Query database
    rates = query_rates(symbol, timeframe, from_date, to_date)
    
    # Convert to ASTRates array
    num_candles = len(rates)
    rates_array = (ASTRates * num_candles)()
    
    for i, rate in enumerate(rates):
        rates_array[i].time = int(rate.timestamp)
        rates_array[i].open = rate.open
        rates_array[i].high = rate.high
        rates_array[i].low = rate.low
        rates_array[i].close = rate.close
        rates_array[i].volume = rate.volume
        rates_array[i].swapLong = rate.swap_long
        rates_array[i].swapShort = rate.swap_short
    
    return {'rates': rates_array, 'numCandles': num_candles}
```

### 3. Custom Result Processing

Process results differently:

```python
def processResultsCustom(result, xml_root):
    """Custom result processing"""
    # Extract key metrics
    metrics = {
        'profit': result.profit,
        'max_dd': result.maxDD,
        'sharpe': result.sharpe,
        'cagr': result.CAGR
    }
    
    # Store in database
    save_to_database(metrics)
    
    # Send to API
    send_to_api(metrics)
    
    # Generate custom report
    generate_custom_report(result, xml_root)
```

### 4. Custom Optimization Goals

Implement custom optimization objectives:

```python
def customOptimizationGoal(result, settings):
    """Custom optimization goal calculation"""
    # Combine multiple metrics
    score = (
        result.profit * 0.4 +
        (1.0 / (1.0 + result.maxDD)) * 0.3 +
        result.sharpe * 0.2 +
        result.CAGR * 0.1
    )
    return score
```

## Code Examples

### Example 1: Minimal Test Execution

```python
#!/usr/bin/env python3
import ctypes
from ctypes import *
from include.asirikuy import *

# Load library
astdll = loadLibrary('libCTesterFrameworkAPI.dylib')

# Initialize
log_path = './log/test.log'.encode('utf-8')
astdll.initCTesterFramework(log_path, 6)

# Prepare settings
settings = (c_double * 64)()
settings[ORIGINAL_EQUITY] = 100000.0
settings[ACCOUNT_RISK_PERCENT] = 1.0
settings[TIMEFRAME] = 60.0

# Prepare symbol
symbol = b'USDJPY1987'

# Prepare rates (simplified - normally load from CSV)
num_candles = 1000
rates = (ASTRates * num_candles)()
# ... populate rates ...

# Prepare rates info
rates_info = CRatesInfo()
rates_info.isEnabled = 1
rates_info.actualTimeFrame = 60
rates_info.requiredTimeFrame = 60
rates_info.totalBarsRequired = 1000
rates_info.ratesArraySize = 1000
rates_info.digits = 3.0
rates_info.point = 0.001

# Prepare test settings
test_settings = TestSettings()
test_settings.fromDate = 0  # Unix timestamp
test_settings.toDate = 999999999  # Unix timestamp

# Callback
def testUpdate(testId, percentage, lastOrder, balance, symbol):
    print(f"Progress: {percentage:.1f}%")

TEST_UPDATE = CFUNCTYPE(c_void_p, c_int, c_double, COrderInfo, c_double, c_char_p)
testUpdate_c = TEST_UPDATE(testUpdate)

# Error pointer
error_ptr = POINTER(c_char_p)()

# Run test
result = astdll.runPortfolioTest(
    1,  # testId
    byref(settings),
    byref(symbol),
    b'USD',
    b'Test Broker',
    b'Test Broker',
    None,  # account info
    byref(test_settings),
    byref(rates_info),
    num_candles,
    1,  # numSystems
    byref(rates),
    0.00001,  # minLotSize
    testUpdate_c,
    error_ptr
)

print(f"Profit: {result.profit:.2f}")
print(f"Max DD: {result.maxDD:.2f}")
print(f"Trades: {result.numTrades}")
```

### Example 2: Portfolio Test

```python
from include.asirikuy import *
from include.mt import readSetFile

# Load multiple strategies
num_systems = 3
set_files = ['strategy1.set', 'strategy2.set', 'strategy3.set']
symbols = [b'USDJPY1987', b'EURUSD1987', b'GBPUSD1987']
strategy_ids = [22, 22, 22]

# Load set files
settings_list = []
for set_file in set_files:
    mt_set = readSetFile(set_file)
    settings = (c_double * 64)()
    # ... populate settings from mt_set ...
    settings_list.append(settings)

# Prepare arrays
settings_array = (POINTER(c_double) * num_systems)()
symbols_array = (c_char_p * num_systems)()
rates_info_array = (POINTER(CRatesInfo) * num_systems)()
rates_array = (POINTER(POINTER(ASTRates)) * num_systems)()

for i in range(num_systems):
    settings_array[i] = settings_list[i]
    symbols_array[i] = symbols[i]
    # ... prepare rates_info_array[i] and rates_array[i] ...

# Run portfolio test
result = astdll.runPortfolioTest(
    1,
    settings_array,
    symbols_array,
    b'USD',
    b'Test Broker',
    b'Test Broker',
    None,
    test_settings,
    rates_info_array,
    num_candles,
    num_systems,
    rates_array,
    0.00001,
    testUpdate_c,
    error_ptr
)
```

### Example 3: Custom Result Handler

```python
class CustomResultHandler:
    def __init__(self):
        self.results = []
        self.trades = []
    
    def handleTestUpdate(self, testId, percentage, lastOrder, balance, symbol):
        """Store intermediate results"""
        if lastOrder.order > 0:  # Valid order
            self.trades.append({
                'order': lastOrder.order,
                'type': 'BUY' if lastOrder.type == 0 else 'SELL',
                'symbol': lastOrder.symbol.decode('utf-8'),
                'profit': lastOrder.profit,
                'open_time': lastOrder.openTime,
                'close_time': lastOrder.closeTime
            })
    
    def handleTestComplete(self, result):
        """Process final results"""
        self.results.append({
            'profit': result.profit,
            'max_dd': result.maxDD,
            'sharpe': result.sharpe,
            'cagr': result.CAGR,
            'num_trades': result.numTrades
        })
        
        # Save to database
        self.saveToDatabase()
        
        # Generate report
        self.generateReport()
    
    def saveToDatabase(self):
        """Save results to database"""
        # Implementation here
        pass
    
    def generateReport(self):
        """Generate custom report"""
        # Implementation here
        pass

# Usage
handler = CustomResultHandler()

def testUpdate(testId, percentage, lastOrder, balance, symbol):
    handler.handleTestUpdate(testId, percentage, lastOrder, balance, symbol)

TEST_UPDATE = CFUNCTYPE(c_void_p, c_int, c_double, COrderInfo, c_double, c_char_p)
testUpdate_c = TEST_UPDATE(testUpdate)

# ... run test ...

result = astdll.runPortfolioTest(...)
handler.handleTestComplete(result)
```

## Extension Guide

### Adding New Strategy Support

1. **Update Strategy ID List** (`include/asirikuy.py`):
```python
strategies = [
    # ... existing strategies ...
    'MY_NEW_STRATEGY',  # Add new strategy
]
```

2. **Add Strategy-Specific Settings**:
```python
# In settings array, add new indices if needed
MY_STRATEGY_PARAM_1 = 50
MY_STRATEGY_PARAM_2 = 51
```

3. **Update Set File Parser** (`include/mt.py`):
```python
# Add parsing for new strategy parameters
if strategy_id == MY_STRATEGY_ID:
    settings[MY_STRATEGY_PARAM_1] = float(mt_set.get('MY_PARAM_1'))
    settings[MY_STRATEGY_PARAM_2] = float(mt_set.get('MY_PARAM_2'))
```

### Adding New Output Formats

1. **Create Output Generator**:
```python
def generateJSONOutput(result, trades, filename):
    """Generate JSON output"""
    import json
    
    data = {
        'result': {
            'profit': result.profit,
            'max_dd': result.maxDD,
            'sharpe': result.sharpe,
            # ... other metrics ...
        },
        'trades': [
            {
                'order': t.order,
                'type': 'BUY' if t.type == 0 else 'SELL',
                'profit': t.profit,
                # ... other fields ...
            }
            for t in trades
        ]
    }
    
    with open(filename, 'w') as f:
        json.dump(data, f, indent=2)
```

2. **Integrate into Main Script**:
```python
# In asirikuy_strategy_tester.py, after test completion
if args.output_format == 'json':
    generateJSONOutput(result, trades, output_file + '.json')
```

### Adding Custom Metrics

1. **Extend TestResult Processing**:
```python
def calculateCustomMetrics(result, trades):
    """Calculate custom metrics"""
    # Calculate win rate
    winning_trades = [t for t in trades if t.profit > 0]
    win_rate = len(winning_trades) / len(trades) if trades else 0
    
    # Calculate average win/loss
    avg_win = sum(t.profit for t in winning_trades) / len(winning_trades) if winning_trades else 0
    losing_trades = [t for t in trades if t.profit < 0]
    avg_loss = sum(t.profit for t in losing_trades) / len(losing_trades) if losing_trades else 0
    
    return {
        'win_rate': win_rate,
        'avg_win': avg_win,
        'avg_loss': avg_loss,
        'profit_factor': abs(avg_win / avg_loss) if avg_loss != 0 else 0
    }
```

## Best Practices

### 1. Error Handling

Always check return values and error pointers:

```python
error_ptr = POINTER(c_char_p)()
result = astdll.runPortfolioTest(...)

if result.numTrades < 0:  # Error indicator
    if error_ptr.contents:
        error_msg = error_ptr.contents.value.decode('utf-8')
        raise RuntimeError(f"Test failed: {error_msg}")
    else:
        raise RuntimeError("Test failed with unknown error")
```

### 2. Memory Management

Properly manage ctypes arrays:

```python
# Allocate arrays
rates_array = (ASTRates * num_candles)()

# Populate
for i in range(num_candles):
    rates_array[i].time = timestamps[i]
    # ... set other fields ...

# Use in function call
result = astdll.runPortfolioTest(..., rates_array, ...)

# Arrays are automatically freed when out of scope
```

### 3. String Encoding

Always encode strings to bytes for C functions:

```python
# Correct
symbol = 'USDJPY1987'.encode('utf-8')
log_path = './log/test.log'.encode('utf-8')

# Incorrect (will cause TypeError)
symbol = 'USDJPY1987'  # String, not bytes
```

### 4. Callback Thread Safety

Callbacks may be called from C threads. Ensure thread safety:

```python
import threading

class ThreadSafeHandler:
    def __init__(self):
        self.lock = threading.Lock()
        self.results = []
    
    def handleUpdate(self, testId, percentage, lastOrder, balance, symbol):
        with self.lock:
            self.results.append({
                'testId': testId,
                'percentage': percentage,
                'balance': balance
            })
```

### 5. Configuration Validation

Validate configuration before running tests:

```python
def validateConfig(config):
    """Validate configuration"""
    errors = []
    
    # Check required sections
    if not config.has_section('strategy'):
        errors.append("Missing [strategy] section")
    
    # Check required options
    if not config.has_option('strategy', 'pair'):
        errors.append("Missing 'pair' in [strategy] section")
    
    # Validate values
    try:
        balance = config.getfloat('account', 'balance')
        if balance <= 0:
            errors.append("Balance must be positive")
    except ValueError:
        errors.append("Invalid balance value")
    
    if errors:
        raise ValueError("Configuration errors:\n" + "\n".join(errors))
```

### 6. Logging

Use proper logging instead of print statements:

```python
import logging

logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    handlers=[
        logging.FileHandler('ctester.log'),
        logging.StreamHandler()
    ]
)

logger = logging.getLogger('ctester')

# In callbacks
def testUpdate(testId, percentage, lastOrder, balance, symbol):
    logger.info(f"Test {testId}: {percentage:.1f}% - Balance: {balance:.2f}")
```

## Next Steps

- See [CTester Usage Guide](CTESTER_USAGE_GUIDE.md) for user documentation
- See [CTester Troubleshooting Guide](CTESTER_TROUBLESHOOTING.md) for common issues
- See [CTester Interface Documentation](CTESTER_INTERFACE_DOCUMENTATION.md) for detailed API reference

