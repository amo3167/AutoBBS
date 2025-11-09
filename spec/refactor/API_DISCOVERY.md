# Phase 2 API Discovery Documentation

## Overview

This document captures the **actual** Phase 2 API surface discovered during the migration attempt on 2025-11-09, contrasting it with what was **expected** based on initial design assumptions.

**Purpose**: Prevent future migration attempts from making the same assumptions. Document what actually exists vs what strategies need.

---

## Critical Finding

**The Phase 2 infrastructure components (OrderBuilder, Indicators, OrderManager) exist and are well-tested, but they may not expose all the convenience methods that strategies expect.**

Strategy migrations cannot proceed until we:
1. Understand actual data access patterns in existing C strategies
2. Validate what APIs are actually available in Phase 2
3. Design compatibility layers or extensions as needed

---

## API Gap Analysis

### 1. rates_t Structure

#### Expected (Based on Strategy Requirements)
```cpp
struct rates_t {
    int barsTotal;              // Total number of bars available
    double* timeArray;          // Array of bar timestamps
    double* openArray;          // Array of open prices
    double* highArray;          // Array of high prices
    double* lowArray;           // Array of low prices
    double* closeArray;         // Array of close prices
    double* volumeArray;        // Array of volumes
    // ... other fields
};
```

**Usage Pattern in Attempted Migration**:
```cpp
// RecordBarsStrategy.cpp (attempted)
int barsTotal = rates->rates[B_PRIMARY_RATES].barsTotal;
for (int i = 0; i < barsTotal; i++) {
    double time = rates->rates[B_PRIMARY_RATES].timeArray[i];
    double open = rates->rates[B_PRIMARY_RATES].openArray[i];
    // ... etc
}
```

#### Actual (From AsirikuyDefines.h)
**Status**: ✅ **DOCUMENTED** - Structure found and documented

**Actual Definition** (from `dev/AsirikuyCommon/include/AsirikuyDefines.h`):
```cpp
typedef struct ratesInfo_t {
    BOOL     isEnabled;
    BOOL     isBufferFull;
    int      timeframe;
    int      arraySize;        // ✅ This is "barsTotal"!
    double   point;
    int      digits;
} RatesInfo;

typedef struct rates_t {
    RatesInfo info;            // Contains arraySize and other metadata
    time_t*   time;            // ✅ Time array (not "timeArray")
    double*   open;            // ✅ Open array (not "openArray")
    double*   high;            // ✅ High array (not "highArray")
    double*   low;             // ✅ Low array (not "lowArray")
    double*   close;           // ✅ Close array (not "closeArray")
    double*   volume;          // ✅ Volume array (not "volumeArray")
} Rates;

typedef struct ratesBuffers_t {
    int    instanceId;
    int    bufferOffsets[MAX_RATES_BUFFERS];
    Rates  rates[MAX_RATES_BUFFERS];
} RatesBuffers;
```

**Actual Usage in C Code** (from `RecordBars.c`):
```cpp
AsirikuyReturnCode runRecordBars(StrategyParams* pParams) {
    // Access array size (not barsTotal!)
    int shift1Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 2;
    int shift0Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 1;
    
    // Access OHLCV arrays (not timeArray, openArray, etc!)
    time_t barTime = pParams->ratesBuffers->rates[PRIMARY_RATES].time[shift1Index];
    double barOpen = pParams->ratesBuffers->rates[PRIMARY_RATES].open[shift1Index];
    double barHigh = pParams->ratesBuffers->rates[PRIMARY_RATES].high[shift1Index];
    double barLow = pParams->ratesBuffers->rates[PRIMARY_RATES].low[shift1Index];
    double barClose = pParams->ratesBuffers->rates[PRIMARY_RATES].close[shift1Index];
    double barVolume = pParams->ratesBuffers->rates[PRIMARY_RATES].volume[shift1Index];
}
```

**Key Differences**:
| Expected | Actual | Notes |
|----------|--------|-------|
| `barsTotal` | `info.arraySize` | Bar count is in nested info struct |
| `timeArray` | `time` | Simple pointer, not "Array" suffix |
| `openArray` | `open` | Simple pointer, not "Array" suffix |
| `highArray` | `high` | Simple pointer, not "Array" suffix |
| `lowArray` | `low` | Simple pointer, not "Array" suffix |
| `closeArray` | `close` | Simple pointer, not "Array" suffix |
| `volumeArray` | `volume` | Simple pointer, not "Array" suffix |

**Correct C++ Migration Pattern**:
```cpp
// RecordBarsStrategy.cpp (corrected)
const rates_t* rates = context.getRates();
int arraySize = rates->rates[PRIMARY_RATES].info.arraySize;

for (int i = 0; i < arraySize; i++) {
    time_t barTime = rates->rates[PRIMARY_RATES].time[i];
    double barOpen = rates->rates[PRIMARY_RATES].open[i];
    double barHigh = rates->rates[PRIMARY_RATES].high[i];
    double barLow = rates->rates[PRIMARY_RATES].low[i];
    double barClose = rates->rates[PRIMARY_RATES].close[i];
    double barVolume = rates->rates[PRIMARY_RATES].volume[i];
}
```

**Resolution**: ✅ **RESOLVED** - Field names documented, migration pattern established

---

### 2. StrategyContext Accessor Methods

#### Expected (Based on Strategy Requirements)
```cpp
class StrategyContext {
public:
    // Bar count
    int getBarsTotal() const;
    
    // OHLCV accessors by index
    double getTime(int index) const;
    double getOpen(int index) const;
    double getHigh(int index) const;
    double getLow(int index) const;
    double getClose(int index) const;
    double getVolume(int index) const;
    
    // Existing methods...
};
```

**Usage Pattern in Attempted Migration**:
```cpp
// RecordBarsStrategy.cpp (attempted)
int barsTotal = context.getBarsTotal();
for (int i = 0; i < barsTotal; i++) {
    double time = context.getTime(i);
    double open = context.getOpen(i);
    // ... etc
}
```

#### Actual (From Phase 2)
**Status**: ✅ **DOCUMENTED** - These convenience methods do NOT exist, but not needed

**Actual StrategyContext API** (from `StrategyContext.hpp`):
```cpp
class StrategyContext {
public:
    // Raw access to rates buffers
    RatesBuffers* getRatesBuffers() const;
    
    // Access specific timeframe rates
    const Rates& getRates(BaseRatesIndexes index) const;
    
    // No getBarsTotal(), getTime(), getOpen/High/Low/Close/Volume()!
};
```

**Why Convenience Methods Aren't Needed**:

Strategies can access bar data directly through the rates structure:
```cpp
// Correct pattern using actual API
const RatesBuffers* ratesBuffers = context.getRatesBuffers();
const Rates& primaryRates = ratesBuffers->rates[PRIMARY_RATES];

// Get bar count
int arraySize = primaryRates.info.arraySize;

// Access OHLCV data
for (int i = 0; i < arraySize; i++) {
    time_t barTime = primaryRates.time[i];
    double barOpen = primaryRates.open[i];
    double barHigh = primaryRates.high[i];
    double barLow = primaryRates.low[i];
    double barClose = primaryRates.close[i];
    double barVolume = primaryRates.volume[i];
}
```

**Alternative: Use Indicators Class** (if indicator wrappers are needed):

The Indicators class provides wrapped access to technical indicators that internally access bar data:
```cpp
Indicators& indicators = context.getIndicators();
double high = indicators.getHigh(PRIMARY_RATES, 1);  // High of bar at index 1
double low = indicators.getLow(PRIMARY_RATES, 1);
double close = indicators.getClose(PRIMARY_RATES, 1);
```

**Decision**: ✅ **No changes needed** to StrategyContext. Strategies should access rates directly or use Indicators class.

---

### 3. OrderManager Methods

#### Expected (Based on Strategy Requirements)
```cpp
class OrderManager {
public:
    // Singleton pattern
    static OrderManager& getInstance();
    
    // High-level order operations
    bool modifyTradeEasy(int ticket, double sl, double tp);
    bool closeAllLongs();
    bool closeAllShorts();
    int totalOpenOrders();
    
    // Other methods...
};
```

**Usage Pattern in Attempted Migration**:
```cpp
// TakeOverStrategy.cpp (attempted)
OrderManager& orderMgr = OrderManager::getInstance();
int openOrders = orderMgr.totalOpenOrders();
if (needToClose) {
    orderMgr.closeAllLongs();
}
```

#### Actual (From Phase 2)
**Status**: ✅ **DOCUMENTED** - OrderManager exists but with different API

**Actual OrderManager API** (from `OrderManager.hpp`):
```cpp
class OrderManager {
public:
    // Constructor (not singleton - access via StrategyContext)
    explicit OrderManager(StrategyContext& context);
    
    // Order counting
    int getTotalOpenOrders(OrderType orderType) const;
    int getTotalClosedOrders(OrderType orderType) const;
    
    // Order sizing and risk management
    double calculateOrderSize(OrderType orderType, double entryPrice, double stopLoss) const;
    double calculateOrderSizeWithRisk(OrderType orderType, double entryPrice, double stopLoss, double risk) const;
    bool hasEnoughFreeMargin(OrderType orderType, double lotSize) const;
    double getMaxLossPerLot(OrderType orderType, double entryPrice, double stopLoss) const;
    
    // Order lifecycle
    bool openOrUpdateLongTrade(int ratesIndex, int resultsIndex, double stopLoss, double takeProfit, double risk, bool useInternalSL, bool useInternalTP);
    bool openOrUpdateShortTrade(int ratesIndex, int resultsIndex, double stopLoss, double takeProfit, double risk, bool useInternalSL, bool useInternalTP);
    
    // Note: No getInstance(), modifyTradeEasy(), closeAllLongs(), closeAllShorts()!
};
```

**Underlying C Functions Available**:

From `OrderManagement.h` and `EasyTradeCWrapper.hpp`:
```c
// Order counting (OrderManagement.h)
int totalOpenOrders(StrategyParams* pParams, OrderType orderType);

// Order closing (EasyTradeCWrapper.hpp)
AsirikuyReturnCode closeAllLongs();
AsirikuyReturnCode closeAllShorts();
```

**Correct C++ Migration Patterns**:

1. **Access OrderManager via StrategyContext** (not singleton):
```cpp
// TakeOverStrategy.cpp (corrected)
OrderManager& orderMgr = context.getOrderManager();
```

2. **Count orders by type**:
```cpp
// Get count of BUY orders
int buyOrders = orderMgr.getTotalOpenOrders(BUY);

// Get count of SELL orders  
int sellOrders = orderMgr.getTotalOpenOrders(SELL);

// Get total count (BUY + SELL)
int totalOrders = orderMgr.getTotalOpenOrders(BUY) + orderMgr.getTotalOpenOrders(SELL);
```

3. **Close all positions** (use C functions directly):
```cpp
// Close operations not wrapped in OrderManager, use C functions
#include "EasyTradeCWrapper.hpp"

if (needToCloseLongs) {
    AsirikuyReturnCode result = closeAllLongs();
    if (result != SUCCESS) {
        // Handle error
    }
}

if (needToCloseShorts) {
    AsirikuyReturnCode result = closeAllShorts();
    if (result != SUCCESS) {
        // Handle error
    }
}
```

**Design Note**: OrderManager is a wrapper around C `OrderManagement.h` functions, not a singleton. It provides order sizing and lifecycle management but doesn't wrap all C functions. For operations like `closeAllLongs/Shorts`, strategies should call the C functions directly from `EasyTradeCWrapper.hpp`.

**Resolution**: ✅ **RESOLVED** - Use OrderManager via context, call C functions directly for closeAll operations

---

### 4. Logging API (pantheios)

#### Expected (Based on Strategy Requirements)
```cpp
// Expected helper functions for formatted logging
pantheios::log_NOTICE("Value: ", pantheios::integer(value));
pantheios::log_NOTICE("Price: ", pantheios::real(price));
```

**Usage Pattern in Attempted Migration**:
```cpp
// TakeOverStrategy.cpp (attempted)
pantheios::log_NOTICE("Open orders: ", pantheios::integer(openOrders));
pantheios::log_NOTICE("Current bid: ", pantheios::real(context.getBid()));
```

#### Actual (From Phase 2)
**Status**: ✅ **KNOWN** - pantheios lacks these helpers

**Actual pantheios API**:
```cpp
// pantheios only accepts strings and string concatenation
pantheios::log_NOTICE("Message text");
pantheios::log_NOTICE("Symbol: ", symbol);
// No integer() or real() functions
```

**Workarounds**:
- Convert numbers to strings manually using sprintf/snprintf/stringstream
- Create utility functions: `toString(int)`, `toString(double)`
- Use C++11 std::to_string (if upgrading to C++11)
- Format strings before passing to pantheios

**Example Workaround**:
```cpp
char buffer[64];
sprintf(buffer, "%d", value);
pantheios::log_NOTICE("Value: ", buffer);
```

**Required Investigation**:
- [ ] Check if pantheios has any number formatting utilities
- [ ] Decide on standard approach for numeric logging
- [ ] Consider creating helper utilities in a common header

---

### 5. Type Mismatches

#### Issue: StrategyResult.code Type

**Expected**:
```cpp
struct StrategyResult {
    AsirikuyReturnCode code;  // Expected type
    // ... other fields
};
```

**Actual**:
```cpp
struct StrategyResult {
    StrategyErrorCode code;   // Actual type
    // ... other fields
};
```

**Problem**: The two enum types are incompatible, causing compilation errors when strategies try to return standard Asirikuy return codes.

**Possible Solutions**:
1. **Change StrategyResult to use AsirikuyReturnCode** (breaking change for existing code)
2. **Add conversion function**: `AsirikuyReturnCode toAsirikuyReturnCode(StrategyErrorCode code)`
3. **Add type aliases**: Make StrategyErrorCode values compatible with AsirikuyReturnCode
4. **Use union or variant**: Store either type

**Required Investigation**:
- [ ] Analyze where StrategyResult.code is used
- [ ] Check if StrategyErrorCode values overlap with AsirikuyReturnCode
- [ ] Determine best solution for compatibility
- [ ] Document conversion strategy

---

### 6. Missing Utility Functions

#### Issue: addValueToUI()

**Expected**:
```cpp
// Utility function to add key-value pairs to UI
void addValueToUI(StrategyParams* params, const char* key, const char* value);
```

**Usage Pattern in Attempted Migration**:
```cpp
// TakeOverStrategy.cpp (attempted)
char buffer[64];
sprintf(buffer, "%d", openOrders);
addValueToUI(params, "OpenOrders", buffer);
```

**Actual**:
**Status**: ❌ **UNKNOWN** - Function not found during compilation

**Required Investigation**:
- [ ] Search codebase for addValueToUI or similar functions
- [ ] Check if functionality exists under different name
- [ ] Determine if this is a legacy function that was removed
- [ ] Check how existing C strategies add values to UI
- [ ] Decide if function needs to be created or alternative exists

---

## Recommended Next Steps

### Phase 1: Understand Existing C Strategies

Before migrating strategies, understand how they currently work:

1. **Analyze RecordBars.c**:
   - [ ] How does it access bar data (time, OHLCV)?
   - [ ] What structure fields does it use?
   - [ ] How does it iterate through bars?

2. **Analyze TakeOver.c**:
   - [ ] How does it query open orders?
   - [ ] How does it modify trades?
   - [ ] How does it close positions?
   - [ ] How does it log information?

3. **Analyze TrendStrategy.c**:
   - [ ] Common patterns for data access
   - [ ] Common patterns for order management
   - [ ] Helper functions that strategies rely on

### Phase 2: Document Actual APIs

Create comprehensive API documentation:

1. **rates_t Structure**:
   - [ ] Find definition in headers
   - [ ] Document all fields
   - [ ] Show example usage from C code

2. **StrategyParams Structure**:
   - [ ] Document all fields
   - [ ] Show how to access results
   - [ ] Show how to add UI values

3. **C Function APIs**:
   - [ ] Document OrderManagement.h functions
   - [ ] Document EasyTrade indicator functions
   - [ ] Document any utility functions

### Phase 3: Design Compatibility Layer

Based on findings, design appropriate abstractions:

1. **Data Access**:
   - [ ] Decide if StrategyContext should add convenience methods
   - [ ] Or if strategies should access rates directly
   - [ ] Or if a RatesBuffer helper class is needed

2. **Order Management**:
   - [ ] Decide if OrderManager should add high-level methods
   - [ ] Or if strategies should use lower-level API
   - [ ] Document patterns for common operations

3. **Logging**:
   - [ ] Create numeric logging helpers
   - [ ] Document standard approach

4. **Type Compatibility**:
   - [ ] Resolve StrategyErrorCode vs AsirikuyReturnCode
   - [ ] Create conversion utilities if needed

### Phase 4: Update Strategy Templates

Create new templates based on actual APIs:

1. **Simple Strategy Template** (for RecordBars-like strategies)
2. **Order Management Strategy Template** (for TakeOver-like strategies)
3. **Complex Strategy Template** (for TrendStrategy-like strategies)

---

## Lessons Learned

### 1. Don't Assume APIs Exist
- Infrastructure components existing ≠ convenience methods existing
- Always validate actual API surface before writing code

### 2. Study Existing Code First
- C strategies show actual usage patterns
- Patterns reveal what's possible vs what's convenient

### 3. Infrastructure vs Strategy API
- Infrastructure provides low-level primitives (OrderManager wraps C functions)
- Strategy API needs higher-level convenience methods
- Gap between the two must be bridged

### 4. C/C++ Boundary is Complex
- Type conversions matter (enums, return codes)
- C function signatures constrain C++ wrappers
- Need clear understanding of what crosses the boundary

### 5. Start with Discovery
- API discovery phase should precede implementation
- Document actual capabilities before designing abstractions
- Validate assumptions early with small experiments

---

## Document Status

**Version**: 1.0  
**Created**: 2025-11-09  
**Status**: ⚠️ INCOMPLETE - Requires investigation to fill gaps

**Completion Checklist**:
- [ ] Document actual rates_t structure
- [ ] Document actual StrategyParams fields
- [ ] Document C OrderManagement functions
- [ ] Document C EasyTrade indicator functions
- [ ] Resolve type compatibility issues
- [ ] Find or create missing utility functions
- [ ] Create example code using actual APIs
- [ ] Design recommended compatibility layer
- [ ] Update strategy templates

**Next Action**: Analyze RecordBars.c to understand actual data access patterns
