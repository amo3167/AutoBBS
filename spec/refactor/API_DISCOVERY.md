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

#### Actual (From Phase 2)
**Status**: ❌ **UNKNOWN** - Need to analyze actual rates_t definition

**Required Investigation**:
- [ ] Find actual rates_t structure definition
- [ ] Document actual fields available
- [ ] Understand how C strategies currently access bar data
- [ ] Determine if arrays exist but are named differently
- [ ] Check if data access uses a different pattern (e.g., buffer access)

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
**Status**: ✅ **KNOWN** - These methods do NOT exist in StrategyContext

**Actual StrategyContext API** (from StrategyContext.hpp):
```cpp
class StrategyContext {
public:
    // Constructor
    StrategyContext(StrategyParams* params);
    
    // Getters for core data
    const char* getSymbol() const;
    double getBid() const;
    double getAsk() const;
    const SymbolInfo* getSymbolInfo() const;
    const AccountInfo* getAccountInfo() const;
    const rates_t* getRates() const;
    const Base_Indicators* getIndicators() const;
    OrderManager& getOrderManager();
    const OrderManager& getOrderManager() const;
    
    // Results
    StrategyResult& getResults();
    
    // No getBarsTotal(), getTime(), getOpen/High/Low/Close/Volume()!
};
```

**Gap**: Strategies need to either:
- Access rates structure directly (but rates_t fields are unknown)
- Use Indicators wrapper methods (if they provide bar access)
- Add convenience methods to StrategyContext
- Use a different pattern entirely

**Required Investigation**:
- [ ] How do existing C strategies access bar data?
- [ ] Can Indicators class provide bar access?
- [ ] Should we add convenience methods to StrategyContext?
- [ ] Is there a RatesBuffer or similar helper class?

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
**Status**: ✅ **PARTIALLY KNOWN** - OrderManager exists but has different API

**Actual OrderManager API** (from OrderManager.hpp):
```cpp
class OrderManager {
public:
    // Constructor (not singleton)
    OrderManager(StrategyParams* params);
    
    // Order counting
    int openBuyOrders(const char* symbol) const;
    int openSellOrders(const char* symbol) const;
    int totalOrders(const char* symbol) const;
    
    // Order operations (wrap C functions)
    int openTrade(/* ... many parameters ... */);
    bool updateTrade(int ticket, double sl, double tp);
    bool closeTrade(int ticket);
    
    // Position sizing
    double calculateOrderSize(/* ... */);
    
    // No getInstance(), modifyTradeEasy(), closeAllLongs/Shorts()!
};
```

**Gap**: OrderManager is a wrapper around C functions, not a singleton with high-level operations.

**Workarounds**:
- Access OrderManager via StrategyContext: `context.getOrderManager()`
- Use lower-level methods: `updateTrade()` instead of `modifyTradeEasy()`
- Implement closeAllLongs/Shorts logic manually using order counting and closeTrade()
- totalOpenOrders() can be calculated: `openBuyOrders() + openSellOrders()`

**Required Investigation**:
- [ ] Document the C functions that OrderManager wraps
- [ ] Determine if modifyTradeEasy() exists in C layer
- [ ] Check if there are helper functions in OrderManagement.h
- [ ] Decide if high-level convenience methods should be added

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
