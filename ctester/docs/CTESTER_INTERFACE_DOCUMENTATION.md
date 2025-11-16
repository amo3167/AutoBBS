# CTester Interface Documentation

**Date**: December 2024  
**Status**: Complete  
**Task**: T021 - Document CTester interfaces

## Executive Summary

This document provides detailed interface documentation for CTester, mapping current CTester structures to CTesterDefines.h structures, documenting API differences, and identifying integration points for AsirikuyFrameworkAPI.

## 1. Current CTester API (CTesterFrameworkAPI)

### 1.1 Library Functions

#### Initialization
```c
int __stdcall initCTesterFramework(char* pAsirikuyTesterLog, int severityLevel);
```
- **Purpose**: Initialize the CTester framework
- **Parameters**:
  - `pAsirikuyTesterLog`: Path to log file
  - `severityLevel`: Log severity level
- **Returns**: Error code (0 = success)
- **Location**: `core/CTesterFrameworkAPI/include/CTesterFrameworkAPI.h:17`
- **Python Usage**: `asirikuy_strategy_tester.py:188`

#### Version
```c
void __stdcall getCTesterFrameworkVersion(int* pMajor, int* pMinor, int* pBugfix);
```
- **Purpose**: Get framework version
- **Location**: `core/CTesterFrameworkAPI/include/CTesterFrameworkAPI.h:16`

#### Portfolio Test
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
- **Purpose**: Execute portfolio test with multiple strategies
- **Returns**: `TestResult` structure
- **Location**: `core/CTesterFrameworkAPI/include/tester.h:82-99`
- **Python Usage**: `asirikuy_strategy_tester.py:573-578, 616-621`

#### Optimization
```c
int __stdcall runOptimizationMultipleSymbols(
    OptimizationParam* optimizationParams,
    int                numOptimizationParams,
    int                optimizationType,
    GeneticOptimizationSettings optimizationSettings,
    int                numCores,
    double*            pInSettings,
    char**             pInTradeSymbol,
    char*              pInAccountCurrency,
    char*              pInBrokerName,
    char*              pInRefBrokerName,
    double*            pInAccountInfo,
    TestSettings*      testSettings,
    CRatesInfo**       pRatesInfo,
    int                numCandles,
    int                numPairs,
    ASTRates***        pRates,
    double             minLotSize,
    void (*optimizationUpdate)(TestResult testResults, double* settings, int numSettings),
    void (*optimizationFinished)(),
    char**             error
);
```
- **Purpose**: Run optimization (genetic algorithm or brute force)
- **Returns**: Error code (0 = success)
- **Location**: `core/CTesterFrameworkAPI/include/optimizer.h:55`
- **Python Usage**: `asirikuy_strategy_tester.py:501-522`

#### Stop Optimization
```c
void __stdcall stopOptimization();
```
- **Purpose**: Stop running optimization
- **Location**: `core/CTesterFrameworkAPI/include/optimizer.h:52`

### 1.2 Current CTester Structures (Python ctypes)

#### Rate Structure
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
- **Location**: `ctester/include/asirikuy.py:389-399`
- **Size**: 8 fields
- **Note**: Includes swap data (shortSwap, longSwap)

#### OrderInfo Structure
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
- **Location**: `ctester/include/asirikuy.py:401-418`
- **Size**: 15 fields
- **Note**: All fields are `c_double` (including times and boolean)

#### RateInfo Structure
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
- **Location**: `ctester/include/asirikuy.py:420-429`
- **Size**: 7 fields
- **Note**: All fields are `c_double`

#### TestResult Structure
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
- **Location**: `ctester/include/asirikuy.py:344-364`
- **Size**: 18 fields

#### TestSettings Structure
```python
class TestSettings(Structure):
    _fields_ = [
        ("spread", c_double),
        ("fromDate", c_int),
        ("toDate", c_int),
        ("is_calculate_expectancy", c_int)
    ]
```
- **Location**: `ctester/include/asirikuy.py:381-387`
- **Size**: 4 fields

#### TradeSignal Structure
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
- **Location**: `ctester/include/asirikuy.py:329-342`
- **Size**: 11 fields

### 1.3 Settings Array

**Type**: `c_double * 64`  
**Definition**: `SettingsType = c_double * 64`  
**Location**: `ctester/include/asirikuy.py:512`

**Key Index Constants**:
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
- `ORDERINFO_ARRAY_SIZE = 63`
- `ADDITIONAL_PARAM_8` through `ADDITIONAL_PARAM_41` (0-40)

### 1.4 AccountInfo Array

**Type**: `c_double * 10`  
**Definition**: `accountInfoType = c_double * 10`  
**Location**: `ctester/include/asirikuy.py:720`

**Index Constants**:
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

## 2. Target API (AsirikuyFrameworkAPI)

### 2.1 Library Functions

#### Initialization
```c
int __stdcall initInstanceC(int instanceId, int isTesting, char* pAsirikuyConfig, char* pAccountName);
```
- **Purpose**: Initialize a strategy instance
- **Parameters**:
  - `instanceId`: Instance ID
  - `isTesting`: 1 for backtesting, 0 for live
  - `pAsirikuyConfig`: Path to config file
  - `pAccountName`: Account name
- **Returns**: `AsirikuyReturnCode` (0 = SUCCESS)
- **Location**: `core/AsirikuyFrameworkAPI/include/AsirikuyFrameworkAPI.h:118`

#### Deinitialization
```c
void __stdcall deinitInstance(int instanceId);
```
- **Purpose**: Clean up instance
- **Location**: `core/AsirikuyFrameworkAPI/include/AsirikuyFrameworkAPI.h:96`

#### Run Strategy
```c
int __stdcall c_runStrategy(
    double*       pInSettings,
    char*         pInTradeSymbol,
    char*         pInAccountCurrency,
    char*         pInBrokerName,
    char*         pInRefBrokerName,
    int*          pInCurrentBrokerTime,
    int*          pInOpenOrdersCount,
    COrderInfo*   pInOrderInfo,
    double*       pInAccountInfo,
    double*       pInBidAsk,
    CRatesInfo*   pInRatesInfo,
    CRates*       pInRates_0,
    CRates*       pInRates_1,
    CRates*       pInRates_2,
    CRates*       pInRates_3,
    CRates*       pInRates_4,
    CRates*       pInRates_5,
    CRates*       pInRates_6,
    CRates*       pInRates_7,
    CRates*       pInRates_8,
    CRates*       pInRates_9,
    double*       pOutResults
);
```
- **Purpose**: Execute strategy for one bar/candle
- **Returns**: `AsirikuyReturnCode` (0 = SUCCESS)
- **Location**: `core/AsirikuyFrameworkAPI/include/CTester/CTesterTradingStrategiesAPI.h:57-79`
- **Note**: This is called per-bar, not for entire test

#### Get Framework Version
```c
void __stdcall getFrameworkVersion(int* pMajor, int* pMinor, int* pBugfix);
```
- **Purpose**: Get framework version
- **Location**: `core/AsirikuyFrameworkAPI/include/AsirikuyFrameworkAPI.h:101`

### 2.2 Target Structures (CTesterDefines.h)

#### CRates Structure
```c
typedef struct CRates_t
{
    double open;
    double high;
    double low;
    double close;
    double volume;
    int    time;
} CRates;
```
- **Location**: `core/AsirikuyFrameworkAPI/include/CTester/CTesterDefines.h:45-53`
- **Size**: 6 fields
- **Note**: Single candle, no swap data

#### COrderInfo Structure
```c
typedef struct COrderInfo_t
{
    double ticket;
    double instanceId;
    double type;
    double openTime;
    double closeTime;
    double stopLoss;
    double takeProfit;
    double expiriation;
    double openPrice;
    double closePrice;
    double lots;
    double profit;
    double commission;
    double swap;
    double isOpen;
} COrderInfo;
```
- **Location**: `core/AsirikuyFrameworkAPI/include/CTester/CTesterDefines.h:55-72`
- **Size**: 15 fields
- **Note**: Matches Python OrderInfo structure

#### CRatesInfo Structure
```c
typedef struct CRatesInfo_t
{
    double isEnabled;
    double requiredTimeframe;
    double totalBarsRequired;
    double actualTimeframe;
    double ratesArraySize;
    double point;
    double digits;
} CRatesInfo;
```
- **Location**: `core/AsirikuyFrameworkAPI/include/CTester/CTesterDefines.h:74-83`
- **Size**: 7 fields
- **Note**: Field name difference: `requiredTimeframe` vs `requiredTimeFrame`

## 3. Structure Mapping

### 3.1 Rate → CRates

| CTester (Python) | CTesterDefines.h (C) | Notes |
|------------------|----------------------|-------|
| `Rate` | `CRates` | Structure name |
| `open` (c_double) | `open` (double) | ✅ Direct match |
| `high` (c_double) | `high` (double) | ✅ Direct match |
| `low` (c_double) | `low` (double) | ✅ Direct match |
| `close` (c_double) | `close` (double) | ✅ Direct match |
| `volume` (c_double) | `volume` (double) | ✅ Direct match |
| `time` (c_int) | `time` (int) | ✅ Direct match |
| `shortSwap` (c_double) | ❌ Not in CRates | ⚠️ Missing in target |
| `longSwap` (c_double) | ❌ Not in CRates | ⚠️ Missing in target |

**Key Differences**:
- CTester `Rate` includes swap data (`shortSwap`, `longSwap`)
- `CRates` does not include swap data
- **Solution**: Swap data must be handled separately or added to bid/ask

### 3.2 OrderInfo → COrderInfo

| CTester (Python) | CTesterDefines.h (C) | Notes |
|------------------|----------------------|-------|
| `OrderInfo` | `COrderInfo` | Structure name |
| `ticket` (c_double) | `ticket` (double) | ✅ Direct match |
| `instanceId` (c_double) | `instanceId` (double) | ✅ Direct match |
| `type` (c_double) | `type` (double) | ✅ Direct match |
| `openTime` (c_double) | `openTime` (double) | ✅ Direct match |
| `closeTime` (c_double) | `closeTime` (double) | ✅ Direct match |
| `stopLoss` (c_double) | `stopLoss` (double) | ✅ Direct match |
| `takeProfit` (c_double) | `takeProfit` (double) | ✅ Direct match |
| `expiration` (c_double) | `expiriation` (double) | ⚠️ Typo in C: "expiriation" |
| `openPrice` (c_double) | `openPrice` (double) | ✅ Direct match |
| `closePrice` (c_double) | `closePrice` (double) | ✅ Direct match |
| `lots` (c_double) | `lots` (double) | ✅ Direct match |
| `profit` (c_double) | `profit` (double) | ✅ Direct match |
| `commission` (c_double) | `commission` (double) | ✅ Direct match |
| `swap` (c_double) | `swap` (double) | ✅ Direct match |
| `isOpen` (c_double) | `isOpen` (double) | ✅ Direct match |

**Key Differences**:
- Field order matches exactly
- Typo in C structure: `expiriation` vs `expiration`
- **Solution**: Use `expiriation` when mapping to C structure

### 3.3 RateInfo → CRatesInfo

| CTester (Python) | CTesterDefines.h (C) | Notes |
|------------------|----------------------|-------|
| `RateInfo` | `CRatesInfo` | Structure name |
| `isEnabled` (c_double) | `isEnabled` (double) | ✅ Direct match |
| `requiredTimeFrame` (c_double) | `requiredTimeframe` (double) | ⚠️ Case difference |
| `totalBarsRequired` (c_double) | `totalBarsRequired` (double) | ✅ Direct match |
| `actualTimeFrame` (c_double) | `actualTimeframe` (double) | ⚠️ Case difference |
| `ratesArraySize` (c_double) | `ratesArraySize` (double) | ✅ Direct match |
| `point` (c_double) | `point` (double) | ✅ Direct match |
| `digits` (c_double) | `digits` (double) | ✅ Direct match |

**Key Differences**:
- Field name case: `requiredTimeFrame` vs `requiredTimeframe`
- Field name case: `actualTimeFrame` vs `actualTimeframe`
- **Solution**: Map field names correctly (case-sensitive)

### 3.4 Settings Array

**CTester**: `c_double * 64`  
**AsirikuyFrameworkAPI**: `double*` (64 elements)

**Mapping**: Direct - same structure, same indices  
**Location**: Settings indices are defined in `AsirikuyDefines.h` and match CTester

### 3.5 AccountInfo Array

**CTester**: `c_double * 10`  
**AsirikuyFrameworkAPI**: `double*` (10 elements)

**Mapping**: Direct - same structure, same indices

## 4. API Differences

### 4.1 Execution Model

#### Current (CTesterFrameworkAPI)
- **Model**: Batch execution
- **Function**: `runPortfolioTest()` - executes entire test
- **Callbacks**: Receives updates during execution
- **Returns**: Complete `TestResult` at end

#### Target (AsirikuyFrameworkAPI)
- **Model**: Per-bar execution
- **Function**: `c_runStrategy()` - executes one bar/candle
- **Callbacks**: None (returns results directly)
- **Returns**: `AsirikuyReturnCode` + output in `pOutResults`

**Key Difference**: AsirikuyFrameworkAPI requires a loop to execute bar-by-bar, while CTesterFrameworkAPI executes the entire test in one call.

### 4.2 Initialization

#### Current
```python
astdll.initCTesterFramework(logPath, severityLevel)
```
- Single initialization for entire framework
- Global initialization

#### Target
```c
initInstanceC(instanceId, isTesting, configPath, accountName)
```
- Per-instance initialization
- Each strategy instance needs initialization
- Requires `deinitInstance()` for cleanup

### 4.3 Function Signatures

#### runPortfolioTest vs c_runStrategy

**Current (runPortfolioTest)**:
- Takes arrays of settings, symbols, account info
- Takes array of rate arrays (per symbol)
- Executes entire test
- Uses callbacks for updates

**Target (c_runStrategy)**:
- Takes single settings array
- Takes single symbol
- Takes single rate array (current bar)
- Executes one bar
- Returns results in output parameter

**Key Differences**:
1. **Batch vs Per-Bar**: Current executes entire test, target executes one bar
2. **Multi-Strategy**: Current supports portfolio, target is single strategy per call
3. **Callbacks**: Current uses callbacks, target returns results directly
4. **Rate Arrays**: Current takes arrays of arrays, target takes individual rate arrays

### 4.4 Rate Array Structure

#### Current
- `Rate` structure includes swap data
- Arrays of `Rate` structures
- Multiple rate buffers per symbol (up to 10)

#### Target
- `CRates` structure does not include swap data
- Single `CRates` structure per call (current bar)
- Multiple rate buffers (pInRates_0 through pInRates_9)

**Key Difference**: 
- Current: Array of Rate structures (all bars)
- Target: Single CRates structure (current bar) + separate arrays for each buffer

## 5. Integration Points

### 5.1 Library Loading

**Current**:
```python
if system == "Darwin":
    astdll = loadLibrary('libCTesterFrameworkAPI.dylib')
```

**Target**:
```python
if system == "Darwin":
    asirikuy_dll = loadLibrary('libAsirikuyFrameworkAPI.dylib')
```

**Integration Point**: Change library name in `asirikuy_strategy_tester.py:63-72`

### 5.2 Initialization

**Current**:
```python
astdll.initCTesterFramework(logPath, severityLevel)
```

**Target**:
```python
# Per-instance initialization
for i in range(numSystemsInPortfolio):
    result = asirikuy_dll.initInstanceC(
        c_int(instanceId),
        c_int(1),  # isTesting = 1
        configPath.encode('utf-8'),
        accountName.encode('utf-8')
    )
```

**Integration Point**: Replace initialization in `asirikuy_strategy_tester.py:188`

### 5.3 Test Execution

**Current**:
```python
result = astdll.runPortfolioTest(
    c_int(numSystemsInPortfolio),
    ctypes.pointer(settings),
    # ... many parameters
    testUpdate_c,
    testFinished_c,
    signalUpdate_c
)
```

**Target**:
```python
# Loop through bars
for bar_index in range(numCandles):
    # Prepare current bar data
    current_rates = [ratesArray[i][0][bar_index] for i in range(numPairs)]
    
    # Call strategy for each system
    for system_id in range(numSystemsInPortfolio):
        result = asirikuy_dll.c_runStrategy(
            settings[system_id],
            symbols[system_id].encode('utf-8'),
            # ... parameters for current bar
            pOutResults
        )
        # Process results
        # Update orders, balance, etc.
```

**Integration Point**: Major refactoring in `asirikuy_strategy_tester.py:573-621`

### 5.4 Callback Handling

**Current**: Uses callbacks for updates  
**Target**: No callbacks, returns results directly

**Integration Point**: Need to implement update logic in Python loop instead of callbacks

### 5.5 Structure Conversion

**Integration Points**:
1. Convert `Rate` array to `CRates` structures (per bar)
2. Convert `OrderInfo` to `COrderInfo` (field name mapping)
3. Convert `RateInfo` to `CRatesInfo` (field name case)
4. Handle swap data separately (not in CRates)

## 6. Conversion Functions Needed

### 6.1 Rate Conversion

```python
def convertRateToCRates(rate_array, bar_index):
    """Convert Rate array to CRates structure for specific bar"""
    crates = CRates()
    crates.open = rate_array[bar_index].open
    crates.high = rate_array[bar_index].high
    crates.low = rate_array[bar_index].low
    crates.close = rate_array[bar_index].close
    crates.volume = rate_array[bar_index].volume
    crates.time = rate_array[bar_index].time
    # Note: swap data handled separately
    return crates
```

### 6.2 OrderInfo Conversion

```python
def convertOrderInfoToCOrderInfo(order_info):
    """Convert OrderInfo to COrderInfo"""
    c_order = COrderInfo()
    c_order.ticket = order_info.ticket
    c_order.instanceId = order_info.instanceId
    c_order.type = order_info.type
    c_order.openTime = order_info.openTime
    c_order.closeTime = order_info.closeTime
    c_order.stopLoss = order_info.stopLoss
    c_order.takeProfit = order_info.takeProfit
    c_order.expiriation = order_info.expiration  # Note: typo in C
    c_order.openPrice = order_info.openPrice
    c_order.closePrice = order_info.closePrice
    c_order.lots = order_info.lots
    c_order.profit = order_info.profit
    c_order.commission = order_info.commission
    c_order.swap = order_info.swap
    c_order.isOpen = order_info.isOpen
    return c_order
```

### 6.3 RateInfo Conversion

```python
def convertRateInfoToCRatesInfo(rate_info):
    """Convert RateInfo to CRatesInfo"""
    c_rate_info = CRatesInfo()
    c_rate_info.isEnabled = rate_info.isEnabled
    c_rate_info.requiredTimeframe = rate_info.requiredTimeFrame  # Case difference
    c_rate_info.totalBarsRequired = rate_info.totalBarsRequired
    c_rate_info.actualTimeframe = rate_info.actualTimeFrame  # Case difference
    c_rate_info.ratesArraySize = rate_info.ratesArraySize
    c_rate_info.point = rate_info.point
    c_rate_info.digits = rate_info.digits
    return c_rate_info
```

## 7. Execution Flow Comparison

### 7.1 Current Flow (CTesterFrameworkAPI)

```
1. Load Library
2. initCTesterFramework()
3. Prepare Data Structures
4. runPortfolioTest()  ← Single call, executes entire test
   ├─ Callback: testUpdate() (per trade)
   ├─ Callback: signalUpdate() (per signal)
   └─ Callback: testFinished() (at end)
5. Process TestResult
6. Generate Output
```

### 7.2 Target Flow (AsirikuyFrameworkAPI)

```
1. Load Library
2. For each strategy instance:
   initInstanceC(instanceId, ...)
3. Prepare Data Structures
4. For each bar in historical data:
   For each strategy instance:
     a. Prepare current bar CRates structures
     b. c_runStrategy()  ← Per-bar call
     c. Process results (pOutResults)
     d. Update orders, balance, etc.
     e. Handle trade signals
5. For each strategy instance:
   deinitInstance(instanceId)
6. Aggregate results
7. Generate Output
```

## 8. Integration Challenges

### 8.1 Batch vs Per-Bar Execution

**Challenge**: CTesterFrameworkAPI executes entire test in one call, AsirikuyFrameworkAPI requires per-bar execution.

**Solution**: 
- Implement execution loop in Python
- Manage state between bar calls
- Track orders, balance, statistics

### 8.2 Multi-Strategy Support

**Challenge**: CTester supports portfolio testing (multiple strategies), AsirikuyFrameworkAPI is per-instance.

**Solution**:
- Initialize multiple instances (one per strategy)
- Call `c_runStrategy()` for each instance per bar
- Aggregate results

### 8.3 Callback vs Direct Returns

**Challenge**: CTester uses callbacks, AsirikuyFrameworkAPI returns results directly.

**Solution**:
- Process results in Python loop
- Implement update logic equivalent to callbacks
- Generate CSV/XML output incrementally

### 8.4 Swap Data Handling

**Challenge**: CTester `Rate` includes swap data, `CRates` does not.

**Solution**:
- Handle swap separately
- Apply swap when processing orders
- May need to add to bid/ask or handle in order processing

### 8.5 TestResult Generation

**Challenge**: CTesterFrameworkAPI returns complete `TestResult`, AsirikuyFrameworkAPI returns per-bar results.

**Solution**:
- Accumulate statistics during execution
- Calculate final metrics (CAGR, Sharpe, etc.) in Python
- Generate `TestResult` structure at end

## 9. Wrapper Design

### 9.1 Wrapper Structure

```
python3_wrapper/
├── ctester_wrapper.py          # Main wrapper
├── structures.py               # ctypes structure definitions
├── converters.py               # Conversion functions
└── execution_engine.py         # Per-bar execution loop
```

### 9.2 Wrapper Functions

#### High-Level API (Compatible with CTester)
```python
def runPortfolioTest(
    settings, symbols, accountInfo, testSettings,
    ratesInfoArray, ratesArray, minLotSize,
    testUpdate_callback, testFinished_callback, signalUpdate_callback
):
    """Wrapper that provides CTester-compatible interface"""
    # Initialize instances
    # Execute per-bar loop
    # Process results
    # Return TestResult
```

#### Low-Level API (Direct AsirikuyFrameworkAPI)
```python
def c_runStrategy(
    settings, symbol, accountInfo, ratesInfo, rates,
    currentBrokerTime, openOrdersCount, orderInfo, bidAsk, results
):
    """Direct wrapper for c_runStrategy"""
    # Convert structures
    # Call C function
    # Return results
```

## 10. Testing Strategy

### 10.1 Unit Tests

1. **Structure Conversion Tests**
   - Test Rate → CRates conversion
   - Test OrderInfo → COrderInfo conversion
   - Test RateInfo → CRatesInfo conversion

2. **Function Binding Tests**
   - Test library loading
   - Test initInstanceC binding
   - Test c_runStrategy binding

### 10.2 Integration Tests

1. **Single Strategy Test**
   - Test with one symbol, one strategy
   - Compare results with CTesterFrameworkAPI

2. **Portfolio Test**
   - Test with multiple strategies
   - Verify instance management

3. **Optimization Test**
   - Test optimization workflow
   - Verify parameter iteration

### 10.3 Validation

- Compare trade-by-trade results
- Verify statistical metrics match
- Check performance (should be similar)

## 11. Implementation Notes

### 11.1 Memory Management

- CTester structures are managed by Python
- AsirikuyFrameworkAPI may allocate memory internally
- Need to ensure proper cleanup with `deinitInstance()`

### 11.2 Error Handling

- AsirikuyFrameworkAPI returns `AsirikuyReturnCode`
- Need to map error codes to CTester error handling
- Handle errors gracefully in execution loop

### 11.3 Thread Safety

- Current CTester may use MPI for parallel optimization
- AsirikuyFrameworkAPI per-bar execution may need synchronization
- Consider thread safety for multi-instance execution

## 12. References

- **CTesterDefines.h**: `core/AsirikuyFrameworkAPI/include/CTester/CTesterDefines.h`
- **CTesterTradingStrategiesAPI.h**: `core/AsirikuyFrameworkAPI/include/CTester/CTesterTradingStrategiesAPI.h`
- **AsirikuyFrameworkAPI.h**: `core/AsirikuyFrameworkAPI/include/AsirikuyFrameworkAPI.h`
- **Current CTester Structures**: `ctester/include/asirikuy.py`
- **Current CTester API**: `core/CTesterFrameworkAPI/include/tester.h`

## 13. Next Steps

1. **T025**: Create `python3_wrapper/ctester_wrapper.py` skeleton
2. **T026**: Implement ctypes bindings for AsirikuyFrameworkAPI
3. **T027**: Implement structure conversion functions
4. **T028**: Implement `c_runStrategy` function binding
5. **T030-T032**: Integration implementation

---

**Document Status**: Complete  
**Next Task**: T025 - Create Python 3 wrapper skeleton

