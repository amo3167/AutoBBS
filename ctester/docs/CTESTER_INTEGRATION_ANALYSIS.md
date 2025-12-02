# CTester Integration Analysis

**Date**: December 2024  
**Status**: Complete  
**Task**: T020 - Review existing CTester Python 3 codebase

## Executive Summary

This document provides a comprehensive analysis of the CTester Python 3 codebase, documenting how it currently calls trading strategies, its data structures, interfaces, dependencies, and integration points for connecting with AsirikuyFrameworkAPI.

## 1. CTester Architecture Overview

### 1.1 System Purpose
CTester is a Python-based backtesting framework that:
- Loads historical market data from CSV files
- Executes trading strategies via C library calls
- Supports both single strategy and portfolio testing
- Provides optimization capabilities (genetic algorithm and brute force)
- Generates test results, statistics, and visualizations

### 1.2 Main Entry Point
- **File**: `ctester/asirikuy_strategy_tester.py`
- **Function**: `main()`
- **Command Line**: Supports config file, output files, version flags
- **Python Version**: Python 3.8+ (migrated from Python 2.7)

### 1.3 Key Components

1. **Configuration Management** (`include/mt.py`)
   - Reads `.set` files (strategy parameters)
   - Reads `.config` files (test configuration)
   - Parses strategy settings and optimization parameters

2. **Data Loading** (`include/asirikuy.py`)
   - Loads historical rates from CSV files
   - Handles multiple symbols and timeframes
   - Manages rate coherence across symbols
   - Adds swap data to rates

3. **Library Interface** (`include/asirikuy.py`)
   - Loads `CTesterFrameworkAPI` shared library
   - Defines ctypes structures for C/C++ interop
   - Provides callback function types

4. **Graphics & Reporting** (`include/graphics.py`)
   - Generates plots and charts
   - Creates HTML reports
   - Visualizes test results

## 2. How CTester Calls Trading Strategies

### 2.1 Library Loading

CTester loads the C library dynamically based on platform:

```python
system = platform.system()
if (system == "Windows"):
    astdll = loadLibrary('CTesterFrameworkAPI')
elif (system == "Linux"):
    astdll = loadLibrary('libCTesterFrameworkAPI.so')
elif (system == "Darwin"):
    astdll = loadLibrary('libCTesterFrameworkAPI.dylib')
```

**Location**: `asirikuy_strategy_tester.py:63-72`

### 2.2 Framework Initialization

```python
astdll.initCTesterFramework(asirikuyCtesterLogPath, config.getint("misc", "logSeverity"))
```

**Location**: `asirikuy_strategy_tester.py:188`

### 2.3 Main Execution Functions

CTester uses two main C functions:

#### 2.3.1 Portfolio Test Execution
```python
astdll.runPortfolioTest.restype = TestResult
result = astdll.runPortfolioTest(
    c_int(numSystemsInPortfolio),
    ctypes.pointer(settings),
    ctypes.pointer(symbols),
    accountCurrency,
    brokerName,
    refBrokerName,
    ctypes.pointer(accountInfo),
    ctypes.pointer(testSettings),
    ctypes.pointer(ratesInfoArray),
    c_int(numCandles),
    c_int(numSystemsInPortfolio),
    ctypes.pointer(ratesArray),
    c_double(minLotSize),
    testUpdate_c,      # Callback for trade updates
    testFinished_c,    # Callback for test completion (optional)
    signalUpdate_c     # Callback for trade signals
)
```

**Location**: `asirikuy_strategy_tester.py:573-578, 616-621`

**Parameters**:
- `numSystemsInPortfolio`: Number of strategies in portfolio
- `settings`: Array of strategy settings (64 doubles per strategy)
- `symbols`: Array of symbol names (c_char_p)
- `accountCurrency`: Account currency string
- `brokerName`: Broker name string
- `refBrokerName`: Reference broker name string
- `accountInfo`: Account information array
- `testSettings`: Test configuration (spread, dates, etc.)
- `ratesInfoArray`: Rate metadata array
- `numCandles`: Number of candles in historical data
- `ratesArray`: Historical rate data
- `minLotSize`: Minimum lot size
- Callbacks: Function pointers for updates

#### 2.3.2 Optimization Execution
```python
astdll.runOptimizationMultipleSymbols.restype = c_int
if not astdll.runOptimizationMultipleSymbols(
    ctypes.pointer(optimizationParams[0]),
    c_int(numOptimizationParams[0]),
    c_int(optimizationType),
    optimizationSettings,
    c_int(numCores),
    settings[0],
    ctypes.pointer(symbols),
    accountCurrency,
    brokerName,
    refBrokerName,
    accountInfo[0],
    ctypes.pointer(testSettings),
    ctypes.pointer(ratesInfoArray),
    c_int(numCandles),
    c_int(numPairs),
    ctypes.pointer(ratesArray),
    c_double(minLotSize),
    optimizationUpdate_c,
    optimizationFinished_c,
    byref(error_c)
):
    print("Error executing framework: " + str(error_c.value))
```

**Location**: `asirikuy_strategy_tester.py:501-522`

**Parameters**:
- `optimizationParams`: Array of optimization parameters
- `numOptimizationParams`: Number of parameters to optimize
- `optimizationType`: Type of optimization (genetic/brute force)
- `optimizationSettings`: Genetic algorithm settings
- `numCores`: Number of CPU cores to use
- Other parameters similar to `runPortfolioTest`

### 2.4 Callback Functions

CTester uses callback functions to receive updates from the C library:

#### Test Update Callback
```python
TEST_UPDATE = CFUNCTYPE(c_void_p, c_int, c_double, OrderInfo, c_double, c_char_p)
testUpdate_c = TEST_UPDATE(testUpdate)

def testUpdate(testId, percentageOfTestCompleted, lastTrade, currentBalance, symbol):
    # Writes trade information to CSV file
    f.write("%d,%s,%s,%s,%.5lf,%.5lf,%lf,%.2lf,%.5lf,%.5lf,%.2lf,%d,%s,%.2lf\n" % (
        int(lastTrade.ticket), opType[int(lastTrade.type)], 
        strftime("%d/%m/%Y %H:%M", gmtime(lastTrade.openTime)), 
        strftime("%d/%m/%Y %H:%M", gmtime(lastTrade.closeTime)),
        lastTrade.openPrice, lastTrade.closePrice, lastTrade.lots, 
        lastTrade.profit, lastTrade.stopLoss, lastTrade.takeProfit, 
        currentBalance, testId, symbol, lastTrade.swap)
    )
```

**Location**: `asirikuy_strategy_tester.py:541-542, 718-727`

#### Signal Update Callback
```python
SIGNAL_UPDATE = CFUNCTYPE(c_void_p, TradeSignal)
signalUpdate_c = SIGNAL_UPDATE(signalUpdate)

def signalUpdate(tradeSignal):
    # Writes trade signals to XML file
    xmlTrade = ET.SubElement(xmlTrades, 'Trade')
    xmlTrade.set('instance', str(int(tradeSignal.testId)))
    xmlTrade.set('no', str(int(tradeSignal.no)))
    xmlTrade.set('time', datetime.datetime.fromtimestamp(tradeSignal.time).isoformat())
    # ... more XML fields
```

**Location**: `asirikuy_strategy_tester.py:544-545, 729-748`

#### Optimization Update Callback
```python
OPTIMIZATION_UPDATE = CFUNCTYPE(c_void_p, TestResult, POINTER(c_double), c_int)
optimizationUpdate_c = OPTIMIZATION_UPDATE(optimizationUpdate)

def optimizationUpdate(testResults, settings, numSettings):
    # Processes optimization iteration results
    # Writes to optimization output file
```

**Location**: `asirikuy_strategy_tester.py:477-478, 671-713`

## 3. Data Structures

### 3.1 Core Structures (ctypes)

All structures are defined in `include/asirikuy.py` using ctypes.

#### 3.1.1 Rate Structure
```python
class Rate(Structure):
    _fields_ = [
        ("open", c_double),
        ("high", c_double),
        ("low", c_double),
        ("close", c_double),
        ("volume", c_double),
        ("shortSwap", c_double),
        ("longSwap", c_double),
        ("time", c_int)
    ]
```

**Purpose**: Represents a single OHLCV candle with swap data  
**Location**: `include/asirikuy.py:389-399`

#### 3.1.2 RateInfo Structure
```python
class RateInfo(Structure):
    _fields_ = [
        ("isEnabled", c_double),
        ("requiredTimeFrame", c_double),
        ("totalBarsRequired", c_double),
        ("actualTimeFrame", c_double),
        ("ratesArraySize", c_double),
        ("point", c_double),
        ("digits", c_double)
    ]
```

**Purpose**: Metadata about rate arrays (timeframe, size, precision)  
**Location**: `include/asirikuy.py:420-429`

#### 3.1.3 OrderInfo Structure
```python
class OrderInfo(Structure):
    _fields_ = [
        ("ticket", c_double),
        ("instanceId", c_double),
        ("type", c_double),
        ("openTime", c_double),
        ("closeTime", c_double),
        ("stopLoss", c_double),
        ("takeProfit", c_double),
        ("expiration", c_double),
        ("openPrice", c_double),
        ("closePrice", c_double),
        ("lots", c_double),
        ("profit", c_double),
        ("commission", c_double),
        ("swap", c_double),
        ("isOpen", c_double)
    ]
```

**Purpose**: Represents a trading order/trade  
**Location**: `include/asirikuy.py:401-418`

#### 3.1.4 TestResult Structure
```python
class TestResult(Structure):
    _fields_ = [
        ("testId", c_int),
        ("totalTrades", c_int),
        ("cagr", c_double),
        ("sharpe", c_double),
        ("martin", c_double),
        ("risk_reward", c_double),
        ("winning", c_double),
        ("finalBalance", c_double),
        ("maxDDDepth", c_double),
        ("maxDDLength", c_double),
        ("pf", c_double),
        ("r2", c_double),
        ("ulcerIndex", c_double),
        ("avgTradeDuration", c_double),
        ("numShorts", c_int),
        ("numLongs", c_int),
        ("yearsTraded", c_double),
        ("symbol", c_char*5000)
    ]
```

**Purpose**: Contains test execution results and statistics  
**Location**: `include/asirikuy.py:344-364`

#### 3.1.5 TestSettings Structure
```python
class TestSettings(Structure):
    _fields_ = [
        ("spread", c_double),
        ("fromDate", c_int),
        ("toDate", c_int),
        ("is_calculate_expectancy", c_int)
    ]
```

**Purpose**: Test configuration (spread, date range, expectancy calculation)  
**Location**: `include/asirikuy.py:381-387`

#### 3.1.6 TradeSignal Structure
```python
class TradeSignal(Structure):
    _fields_ = [
        ("testId", c_int),
        ("no", c_int),
        ("time", c_int),
        ("type", c_int),
        ("orderId", c_int),
        ("lots", c_double),
        ("price", c_double),
        ("sl", c_double),
        ("tp", c_double),
        ("profit", c_double),
        ("balance", c_double)
    ]
```

**Purpose**: Represents a trade signal (buy/sell/modify/close)  
**Location**: `include/asirikuy.py:329-342`

### 3.2 Settings Array

**Type**: `c_double * 64` (64-element array of doubles)  
**Definition**: `SettingsType = c_double * 64`  
**Location**: `include/asirikuy.py:512`

**Index Constants** (from `include/asirikuy.py:600-691`):
- `IS_BACKTESTING = 46`
- `DISABLE_COMPOUNDING = 47`
- `TIMED_EXIT_BARS = 48`
- `ORIGINAL_EQUITY = 49`
- `OPERATIONAL_MODE = 50`
- `STRATEGY_INSTANCE_ID = 51`
- `INTERNAL_STRATEGY_ID = 52`
- `TIMEFRAME = 53`
- `RUN_EVERY_TICK = 43`
- `MAX_OPEN_ORDERS = 45`
- `ACCOUNT_RISK_PERCENT = 57`
- `MAX_DRAWDOWN_PERCENT = 58`
- `MAX_SPREAD = 59`
- `SL_ATR_MULTIPLIER = 60`
- `TP_ATR_MULTIPLIER = 61`
- `ATR_AVERAGING_PERIOD = 62`
- `ADDITIONAL_PARAM_8` through `ADDITIONAL_PARAM_41` (0-40)

**Usage Example**:
```python
settings[i][IS_BACKTESTING] = True
settings[i][INTERNAL_STRATEGY_ID] = strategyIDs[i]
settings[i][TIMEFRAME] = float(sets[i].mainParams["STRATEGY_TIMEFRAME"]['value'])
```

**Location**: `asirikuy_strategy_tester.py:247-270`

### 3.3 AccountInfo Array

**Type**: `c_double * 10` (10-element array of doubles)  
**Definition**: `accountInfoType = c_double * 10`  
**Location**: `include/asirikuy.py:720`

**Index Constants** (from `include/asirikuy.py:721-730`):
- `IDX_ACCOUNT_NUMBER = 0`
- `IDX_BALANCE = 1`
- `IDX_EQUITY = 2`
- `IDX_MARGIN = 3`
- `IDX_LEVERAGE = 4`
- `IDX_CONTRACT_SIZE = 5`
- `IDX_MINIMUM_STOP = 6`
- `IDX_STOPOUT_PERCENT = 7`
- `IDX_TOTAL_OPEN_TRADE_RISK_PERCENT = 8`
- `IDX_LARGEST_DRAWDOWN_PERCENT = 9`

**Usage Example**:
```python
accountInfo[i][IDX_BALANCE] = config.getfloat("account", "balance")
accountInfo[i][IDX_LEVERAGE] = config.getfloat("account", "leverage")
accountInfo[i][IDX_MINIMUM_STOP] = float(minimumStops[i])
```

**Location**: `asirikuy_strategy_tester.py:306-318`

### 3.4 Array Types

#### Rates Arrays
```python
RatesType = Rate * numCandles                    # Single rate array
RatesArrayType = 10 * ctypes.POINTER(Rate)       # Array of 10 rate pointers
MasterRatesArrayType = numPairs * ctypes.POINTER(ctypes.POINTER(Rate))  # Per-pair arrays
```

**Purpose**: Multi-level arrays for managing rates across symbols and timeframes  
**Location**: `asirikuy_strategy_tester.py:347-350`

#### RatesInfo Arrays
```python
RatesInfoType = RateInfo * MAX_RATES_BUFFERS     # MAX_RATES_BUFFERS = 10
RatesInfoArrayType = numPairs * ctypes.POINTER(RateInfo)  # Per-pair info
```

**Location**: `include/asirikuy.py:432-433`, `asirikuy_strategy_tester.py:351-352`

#### Settings Arrays
```python
SettingsArrayType = numPairs * ctypes.POINTER(c_double)  # Per-strategy settings
```

**Location**: `asirikuy_strategy_tester.py:245-246`

## 4. Data Flow

### 4.1 Configuration Loading

1. **Config File** (`.config` file)
   - Read via `readConfigFile()` → `configparser.RawConfigParser`
   - Contains: account settings, strategy settings, optimization settings
   - **Location**: `asirikuy_strategy_tester.py:100-105`

2. **Set File** (`.set` file)
   - Read via `readSetFile()` → `MT4Set` class
   - Contains: strategy parameters, optimization ranges
   - **Location**: `asirikuy_strategy_tester.py:191-196`, `include/mt.py:32-86`

### 4.2 Historical Data Loading

1. **Rate File Discovery**
   - Pattern: `{symbol}_{timeframe}.csv`
   - Location: `./history/`
   - **Location**: `asirikuy_strategy_tester.py:342-345`

2. **Rate Loading Process**
   ```python
   for i in range(numPairs):
       # Load main rate file
       result = loadRates(historyFilePaths[i], numCandles, symbols[i], False)
       ratesArray[i][j] = result['rates']
       endingDate.append(result['endingDate'])
   ```
   - **Location**: `asirikuy_strategy_tester.py:374-437`

3. **Rate File Format** (CSV)
   - Columns: `Date, Open, High, Low, Close, Volume, [ShortSwap, LongSwap]`
   - Date format: `DD/MM/YY HH:MM`
   - **Location**: `include/asirikuy.py:200-218`

4. **Additional Rates**
   - Strategies may require additional symbols/timeframes
   - Loaded based on `rateRequirements`, `symbolRequirements`, `timeframeRequirements`
   - **Location**: `asirikuy_strategy_tester.py:388-430`

### 4.3 Strategy Execution Flow

```
1. Load Configuration
   ↓
2. Load Historical Data
   ↓
3. Prepare Data Structures
   - Settings arrays
   - Account info arrays
   - Rate arrays
   - RateInfo arrays
   ↓
4. Initialize Framework
   - initCTesterFramework()
   ↓
5. Execute Test/Optimization
   - runPortfolioTest() OR runOptimizationMultipleSymbols()
   ↓
6. Process Results
   - Callbacks receive updates
   - TestResult returned
   ↓
7. Generate Output
   - CSV files
   - XML files (optional)
   - Plots (optional)
   - HTML reports (optional)
```

## 5. Integration Points

### 5.1 Current Integration

CTester currently integrates with **CTesterFrameworkAPI**, which is a wrapper around the strategy execution engine. The integration happens through:

1. **Library Loading**: Dynamic loading of shared library
2. **Function Calls**: Direct ctypes function calls
3. **Data Passing**: ctypes structures and arrays
4. **Callbacks**: Python callback functions passed to C

### 5.2 Integration Points for AsirikuyFrameworkAPI

To integrate with AsirikuyFrameworkAPI, we need to:

1. **Replace Library Loading**
   - Current: `CTesterFrameworkAPI.dll/so/dylib`
   - New: `libAsirikuyFrameworkAPI.dylib` (or equivalent)

2. **Map Function Calls**
   - Current: `runPortfolioTest()` → New: `c_runStrategy()` (or equivalent)
   - Current: `runOptimizationMultipleSymbols()` → New: Optimization API
   - Need to map CTester structures to AsirikuyFrameworkAPI structures

3. **Structure Mapping**
   - `Rate` → `CRates` (in CTesterDefines.h)
   - `OrderInfo` → `COrderInfo` (in CTesterDefines.h)
   - `RateInfo` → `CRatesInfo` (in CTesterDefines.h)
   - `Settings` array → Strategy parameters structure
   - `AccountInfo` array → Account information structure

4. **Callback Adaptation**
   - Current callbacks may need adaptation
   - AsirikuyFrameworkAPI may have different callback signatures

### 5.3 Key Integration Challenges

1. **Structure Differences**
   - CTester uses ctypes structures
   - AsirikuyFrameworkAPI uses C structures (CTesterDefines.h)
   - Need conversion layer

2. **Function Signature Differences**
   - `runPortfolioTest()` vs `c_runStrategy()`
   - Parameter order and types may differ
   - Return types may differ

3. **Initialization Differences**
   - Current: `initCTesterFramework()`
   - New: May need `initInstanceC()` per strategy instance

4. **Multi-Strategy Support**
   - CTester supports portfolio testing
   - Need to map to AsirikuyFrameworkAPI's instance management

## 6. Dependencies

### 6.1 Python Dependencies

- **ctypes**: For C library interop (built-in)
- **configparser**: For configuration file parsing (built-in)
- **csv**: For CSV file handling (built-in)
- **datetime**: For date/time handling (built-in)
- **calendar**: For date conversions (built-in)
- **argparse**: For command-line arguments (built-in)
- **xml.etree.ElementTree**: For XML generation (built-in)
- **numpy**: For numerical operations
- **matplotlib**: For plotting
- **fastcsv**: For fast CSV reading
- **requests**: For HTTP requests (auto-installer)
- **colorama**: For colored terminal output

### 6.2 External Dependencies

- **CTesterFrameworkAPI**: C shared library (current)
- **AsirikuyFrameworkAPI**: C shared library (target)
- **Historical Data**: CSV files in `./history/` directory
- **Configuration Files**: `.config` and `.set` files

### 6.3 System Dependencies

- **Python 3.8+**: Runtime environment
- **Platform-specific**: Library loading (Windows/Linux/macOS)
- **MPI**: Optional, for distributed optimization

## 7. Configuration Files

### 7.1 Config File Format (`.config`)

**Sections**:
- `[account]`: Balance, leverage, contract size, spread, etc.
- `[strategy]`: Pairs, strategy IDs, timeframes, requirements
- `[optimization]`: Optimization settings (type, population, etc.)
- `[misc]`: Log severity, etc.

**Example**:
```ini
[account]
balance = 10000
leverage = 100
contractSize = 100000
spread = 2.0

[strategy]
pair = EURUSD,GBPUSD
strategyID = 0,1
passedtimeframe = 60,60
```

**Location**: `config/` directory

### 7.2 Set File Format (`.set`)

**Sections**:
- `[main]`: Main strategy parameters
- `[additional]`: Additional parameters
- Optimization flags: `PARAM_NAME,F`, `PARAM_NAME,1`, `PARAM_NAME,2`, `PARAM_NAME,3`

**Example**:
```ini
[main]
STRATEGY_TIMEFRAME=240
DISABLE_COMPOUNDING=0
ACCOUNT_RISK_PERCENT=2.0

[additional]
PARAM1=10.0
PARAM1,F=1
PARAM1,1=5.0
PARAM1,2=1.0
PARAM1,3=20.0
```

**Location**: `sets/` directory

## 8. Output Files

### 8.1 Test Results (`.txt`)

**Format**: CSV
**Columns**: Order Number, Order Type, Open Time, Close Time, Open Price, Close Price, Lots, Profit, SL, TP, Balance, ID, Pair, Swap

**Location**: Generated in current directory or specified output path

### 8.2 Optimization Results (`.txt`)

**Format**: CSV
**Columns**: Iteration, Symbol, NumTrades, Profit, maxDD, maxDDLength, PF, R2, ulcerIndex, Sharpe, CAGR, CAGR to Max DD, numShorts, numLongs, Set Parameters

**Location**: Generated in current directory or specified output path

### 8.3 XML Output (`.xml`)

**Format**: XML with XSL stylesheet
**Contains**: Portfolio information, trade signals, statistics

**Location**: Generated when `write_xml = True` in config

## 9. Error Handling

### 9.1 Error Types

CTester defines error constants in `include/asirikuy.py:479-499`:
- `SUCCESS = 0`
- `INVALID_CURRENCY = 3000`
- `UNKNOWN_SYMBOL = 3001`
- `NOT_ENOUGH_RATES_DATA = 3009`
- etc.

### 9.2 Error Propagation

- C library returns error codes via `error_c` parameter
- Python checks return values and prints error messages
- **Location**: `asirikuy_strategy_tester.py:523, 580, 624`

## 10. Integration Requirements

### 10.1 What Needs to Be Created

1. **Python 3 Wrapper** (`python3_wrapper/ctester_wrapper.py`)
   - Load `libAsirikuyFrameworkAPI.dylib`
   - Map CTester structures to CTesterDefines.h structures
   - Implement conversion functions
   - Provide compatible function signatures

2. **Structure Conversion Functions**
   - `Rate` → `CRates`
   - `OrderInfo` → `COrderInfo`
   - `RateInfo` → `CRatesInfo`
   - Settings array → Strategy parameters
   - AccountInfo array → Account structure

3. **Function Mapping**
   - Map `runPortfolioTest()` to AsirikuyFrameworkAPI calls
   - Map `runOptimizationMultipleSymbols()` to optimization API
   - Handle initialization (`initInstanceC()`, `deinitInstance()`)

4. **Callback Adaptation**
   - Adapt existing callbacks to new signatures
   - Handle any callback differences

### 10.2 Backward Compatibility

- Maintain existing CTester interface
- Existing config files should work
- Existing set files should work
- Output formats should remain compatible

## 11. Testing Strategy

### 11.1 Test Cases

1. **Single Strategy Test**
   - Test with one symbol, one strategy
   - Verify results match baseline

2. **Portfolio Test**
   - Test with multiple strategies
   - Verify portfolio execution

3. **Optimization Test**
   - Test genetic algorithm optimization
   - Test brute force optimization
   - Verify optimization results

4. **Multi-Symbol Test**
   - Test with multiple symbols
   - Verify rate handling

### 11.2 Validation

- Compare results with existing CTester output
- Verify statistical metrics match
- Check trade-by-trade accuracy
- Performance benchmarking

## 12. Next Steps

1. **T021**: Document CTester interfaces in detail
2. **T025**: Create Python 3 wrapper skeleton
3. **T026**: Implement ctypes bindings
4. **T027**: Map data structures
5. **T028**: Implement function bindings
6. **T030-T032**: Integration implementation
7. **T033-T034**: Testing and validation

## 13. References

- **Main File**: `ctester/asirikuy_strategy_tester.py`
- **Structures**: `ctester/include/asirikuy.py`
- **Configuration**: `ctester/include/mt.py`
- **Graphics**: `ctester/include/graphics.py`
- **CTesterDefines.h**: C header with target structures (to be reviewed)

## 14. Notes

- CTester supports MPI for distributed optimization
- CTester supports both single and portfolio testing
- Historical data must be in specific CSV format
- Multiple rate buffers (up to 10) can be loaded per symbol
- Strategy parameters are stored in 64-element settings array
- Account information is stored in 10-element array

---

**Document Status**: Complete  
**Next Task**: T021 - Document CTester interfaces

