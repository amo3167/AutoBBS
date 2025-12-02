# MACD Strategy C++ Migration Implementation Plan

**Date**: November 2024  
**Status**: Draft  
**Related**: `MACD_CPP_MIGRATION_SPEC.md`

## Overview

This document provides a detailed, step-by-step implementation plan for converting the MACD strategy modules from C to C++. It breaks down the migration into manageable phases with specific tasks, code examples, and validation steps.

---

## Phase 1: Preparation & Infrastructure Setup

### 1.1 Build System Configuration

**Task**: Update premake4.lua to support mixed C/C++ compilation

**Changes Required**:

```lua
project "TradingStrategies"
  location("../../build/" .. _ACTION .. "/projects")
  kind "SharedLib"
  language "C++"  -- Change from "C" to "C++"
  cppdialect "C++11"  -- Add C++ standard
  targetname "trading_strategies"
  files{
    "**.h", 
    "**.c",      -- Keep C files
    "**.cpp",    -- Add C++ files
    "**.hpp",    -- Add C++ headers
    "**.cxx"
  }
  
  -- C++ specific compiler flags
  configuration { "not windows" }
    buildoptions { "-std=c++11", "-fno-exceptions" }  -- Disable exceptions for compatibility
  
  configuration { "windows" }
    buildoptions { "/std:c++11", "/EHsc" }
```

**Validation**:
- [ ] Project compiles with mixed C/C++ files
- [ ] Existing C code still compiles
- [ ] No linker errors

### 1.2 Directory Structure

**Create new directories**:

```
core/TradingStrategies/
├── include/strategies/autobbs/trend/macd/
│   ├── MACDDailyStrategy.h          (C interface - unchanged)
│   ├── MACDWeeklyStrategy.h          (C interface - unchanged)
│   ├── MACDOrderSplitting.h         (C interface - unchanged)
│   └── cpp/                          (NEW - C++ implementation headers)
│       ├── MACDSymbolConfig.hpp
│       ├── MACDSymbolConfigManager.hpp
│       ├── MACDStrategyBase.hpp
│       ├── MACDDailyStrategy.hpp
│       ├── MACDWeeklyStrategy.hpp
│       └── MACDOrderSplitter.hpp
└── src/strategies/autobbs/trend/macd/
    ├── MACDDailyStrategy.c           (Keep for reference, remove after migration)
    ├── MACDWeeklyStrategy.c          (Keep for reference, remove after migration)
    ├── MACDOrderSplitting.c           (Keep for reference, remove after migration)
    └── cpp/                           (NEW - C++ implementation)
        ├── MACDSymbolConfig.cpp
        ├── MACDSymbolConfigManager.cpp
        ├── MACDStrategyBase.cpp
        ├── MACDDailyStrategy.cpp
        ├── MACDWeeklyStrategy.cpp
        └── MACDOrderSplitter.cpp
```

**Validation**:
- [ ] Directories created
- [ ] Build system recognizes new files

### 1.3 C Interface Wrapper Skeleton

**File**: `src/strategies/autobbs/trend/macd/cpp/MACDDailyStrategy.cpp` (skeleton)

```cpp
#include "Precompiled.h"
#include "strategies/autobbs/trend/macd/MACDDailyStrategy.h"
#include "strategies/autobbs/trend/macd/cpp/MACDDailyStrategy.hpp"

extern "C" {
    AsirikuyReturnCode workoutExecutionTrend_MACD_Daily(
        StrategyParams* pParams,
        Indicators* pIndicators,
        Base_Indicators* pBase_Indicators
    ) {
        // TODO: Implement C++ strategy
        // For now, call C version
        return workoutExecutionTrend_MACD_Daily_C(pParams, pIndicators, pBase_Indicators);
    }
}
```

**Validation**:
- [ ] C interface compiles
- [ ] Can call from C code
- [ ] No breaking changes

---

## Phase 2: Symbol Configuration System

### 2.1 Create MACDSymbolConfig Class

**File**: `include/strategies/autobbs/trend/macd/cpp/MACDSymbolConfig.hpp`

**Implementation Steps**:

1. **Convert struct to class**:
   - Move all fields to private members
   - Add getters/setters
   - Add initialization methods

2. **Create nested structs for organization**:
   ```cpp
   class MACDSymbolConfig {
   public:
       struct MACDParams {
           int fastPeriod = 5;
           int slowPeriod = 10;
           int signalPeriod = 5;
       };
       
       struct Thresholds {
           double level = 0.0;
           double maxLevel = 0.0;
           double histLevel = 0.01;
           double nextMACDRange = 0.0;
       };
       
       // ... more nested structs
   };
   ```

3. **Add filter management**:
   ```cpp
   enum class FilterType {
       VolumeControl = 0,
       BeiLi = 1,
       SlowLine = 2,
       ATR = 3,
       CMFVolume = 4,
       CMFVolumeGap = 5,
       MaxLevelBuy = 6,
       MaxLevelSell = 7,
       WeeklyBaseLine = 8,
       // ... more filters
   };
   
   bool isFilterEnabled(FilterType filter) const {
       return filterFlags_.test(static_cast<size_t>(filter));
   }
   
   void setFilter(FilterType filter, bool enabled) {
       filterFlags_.set(static_cast<size_t>(filter), enabled);
   }
   ```

**Validation**:
- [ ] Class compiles
- [ ] Can create instances
- [ ] Getters/setters work
- [ ] Filter flags work correctly

### 2.2 Create MACDSymbolConfigManager

**File**: `include/strategies/autobbs/trend/macd/cpp/MACDSymbolConfigManager.hpp`

**Implementation Steps**:

1. **Create singleton pattern**:
   ```cpp
   class MACDSymbolConfigManager {
   public:
       static MACDSymbolConfigManager& getInstance() {
           static MACDSymbolConfigManager instance;
           return instance;
       }
       
       MACDSymbolConfig getConfig(
           const std::string& symbol,
           double price,
           bool isBacktesting,
           const std::tm& date
       );
       
   private:
       MACDSymbolConfigManager();
       // ... builder methods
   };
   ```

2. **Create symbol lookup map**:
   ```cpp
   private:
       using ConfigBuilder = std::function<MACDSymbolConfig(
           const std::string&, double, bool, const std::tm&
       )>;
       
       std::map<std::string, ConfigBuilder> exactMatchBuilders_;
       std::vector<std::pair<std::string, ConfigBuilder>> patternBuilders_;
       
       void initializeBuilders();
   ```

3. **Implement builder methods**:
   ```cpp
   MACDSymbolConfig buildCryptoConfig(
       const std::string& symbol,
       double price,
       bool isBacktesting,
       const std::tm& date
   );
   
   MACDSymbolConfig buildOilConfig(
       const std::string& symbol,
       double price
   );
   
   // ... more builders
   ```

4. **Implement lookup logic**:
   ```cpp
   MACDSymbolConfig MACDSymbolConfigManager::getConfig(
       const std::string& symbol,
       double price,
       bool isBacktesting,
       const std::tm& date
   ) {
       // Try exact match
       auto it = exactMatchBuilders_.find(symbol);
       if (it != exactMatchBuilders_.end()) {
           return it->second(symbol, price, isBacktesting, date);
       }
       
       // Try pattern matching
       for (const auto& [pattern, builder] : patternBuilders_) {
           if (symbol.find(pattern) != std::string::npos) {
               return builder(symbol, price, isBacktesting, date);
           }
       }
       
       // Default
       return buildDefaultConfig();
   }
   ```

**Validation**:
- [ ] All symbols produce correct config
- [ ] Lookup is fast (O(log n) or better)
- [ ] Pattern matching works
- [ ] Default config works

### 2.3 Unit Tests for Configuration

**File**: `tests/MACDSymbolConfigTests.cpp`

**Test Cases**:

```cpp
TEST(MACDSymbolConfig, BTCUSD_Configuration) {
    auto& manager = MACDSymbolConfigManager::getInstance();
    std::tm date = {};
    date.tm_year = 2022 - 1900;  // After cutoff date
    
    auto config = manager.getConfig("BTCUSD", 50000.0, false, date);
    
    EXPECT_EQ(config.getMACDParams().fastPeriod, 7);
    EXPECT_EQ(config.getMACDParams().slowPeriod, 14);
    EXPECT_TRUE(config.isFilterEnabled(MACDSymbolConfig::FilterType::SlowLine));
    // ... more assertions
}

TEST(MACDSymbolConfig, BacktestMode) {
    auto& manager = MACDSymbolConfigManager::getInstance();
    std::tm date = {};
    date.tm_year = 2020 - 1900;  // Before cutoff date
    
    // Backtesting before cutoff: should use 5/10/5
    auto config1 = manager.getConfig("BTCUSD", 50000.0, true, date);
    EXPECT_EQ(config1.getMACDParams().fastPeriod, 5);
    
    // Live trading: should use 7/14/7
    auto config2 = manager.getConfig("BTCUSD", 50000.0, false, date);
    EXPECT_EQ(config2.getMACDParams().fastPeriod, 7);
}
```

**Validation**:
- [ ] All tests pass
- [ ] Coverage > 90% for configuration code

---

## Phase 3: Order Splitting

### 3.1 Create MACDOrderSplitter Class

**File**: `include/strategies/autobbs/trend/macd/cpp/MACDOrderSplitter.hpp`

**Implementation**:

```cpp
class MACDOrderSplitter {
public:
    void splitBuyOrders(
        StrategyParams* pParams,
        Indicators* pIndicators,
        Base_Indicators* pBase_Indicators,
        double atr,
        double stopLoss
    );
    
    void splitSellOrders(
        StrategyParams* pParams,
        Indicators* pIndicators,
        Base_Indicators* pBase_Indicators,
        double atr,
        double stopLoss
    );
    
    // Weekly versions
    void splitBuyOrdersWeekly(...);
    void splitSellOrdersWeekly(...);
    
private:
    struct OrderCalculation {
        double takePrice;
        double lots;
        bool shouldSplit;
        double mainLots;
        double secondaryLots;
    };
    
    OrderCalculation calculateOrder(
        StrategyParams* pParams,
        Indicators* pIndicators,
        Base_Indicators* pBase_Indicators,
        OrderType orderType,
        double atr,
        double stopLoss,
        bool isWeekly
    ) const;
    
    static constexpr double TP_MULTIPLIER_DAILY_ATR = 1.5;
    static constexpr int MIN_LOT_SIZE_THRESHOLD = 5;
};
```

**Key Improvements**:
- Extract common calculation logic
- Use struct for return values
- Template for buy/sell if possible

**Validation**:
- [ ] Order splitting produces same results
- [ ] All trade modes work
- [ ] Lot size calculations correct

---

## Phase 4: Weekly Strategy (Pilot)

### 4.1 Create MACDStrategyBase

**File**: `include/strategies/autobbs/trend/macd/cpp/MACDStrategyBase.hpp`

**Implementation**:

```cpp
class MACDStrategyBase {
public:
    virtual ~MACDStrategyBase() = default;
    virtual AsirikuyReturnCode execute(
        StrategyParams* pParams,
        Indicators* pIndicators,
        Base_Indicators* pBase_Indicators
    ) = 0;
    
protected:
    // Common filter methods
    bool checkVolumeFilter(
        const MACDSymbolConfig& config,
        const Indicators* pIndicators,
        bool isBuy
    ) const;
    
    bool checkLevelThreshold(
        const MACDSymbolConfig& config,
        double fast,
        bool isBuy
    ) const;
    
    // Helper methods
    void logMACDValues(
        const StrategyParams* pParams,
        const Indicators* pIndicators,
        const char* timeString
    ) const;
};
```

### 4.2 Implement MACDWeeklyStrategy

**File**: `include/strategies/autobbs/trend/macd/cpp/MACDWeeklyStrategy.hpp`

**Implementation**:

```cpp
class MACDWeeklyStrategy : public MACDStrategyBase {
public:
    AsirikuyReturnCode execute(
        StrategyParams* pParams,
        Indicators* pIndicators,
        Base_Indicators* pBase_Indicators
    ) override;
    
private:
    double getLevelThreshold(const std::string& symbol) const;
    void processBuySignal(...);
    void processSellSignal(...);
    void processExitSignals(...);
};
```

**Key Changes**:
- Break down large function into smaller methods
- Use base class for common logic
- Extract level threshold logic

**Validation**:
- [ ] Weekly strategy produces identical results
- [ ] All entry/exit signals match
- [ ] Performance is acceptable

---

## Phase 5: Daily Strategy (Main Migration)

### 5.1 Break Down Large Function

**Current**: 600+ line function  
**Target**: Multiple focused methods

**Method Breakdown**:

```cpp
class MACDDailyStrategy : public MACDStrategyBase {
public:
    AsirikuyReturnCode execute(...) override;
    
private:
    // Configuration
    MACDSymbolConfig loadConfiguration(...);
    
    // Indicator loading
    struct MACDIndicators {
        double fast, slow, preFast, preSlow;
        std::vector<double> preHist;
        std::vector<double> fastHist;
    };
    MACDIndicators loadMACDData(...);
    
    // Signal processing
    void processBuySignal(...);
    void processSellSignal(...);
    
    // Filter application
    bool applyEntryFilters(
        const MACDSymbolConfig& config,
        Indicators* pIndicators,
        bool isBuy,
        // ... other params
    ) const;
    
    // Stop loss calculation
    double calculateStopLoss(...) const;
    
    // Special handling
    void handleNoStopLossMode(...);
    void handleEODEntry(...);
};
```

### 5.2 Extract Filter Logic

**Create filter methods in base class**:

```cpp
class MACDStrategyBase {
protected:
    bool checkSlowLineFilter(
        const MACDSymbolConfig& config,
        const Indicators* pIndicators,
        bool isBuy
    ) const {
        if (!config.isFilterEnabled(MACDSymbolConfig::FilterType::SlowLine)) {
            return true;  // Filter not enabled, pass
        }
        
        if (isBuy) {
            return pIndicators->slow > 0;
        } else {
            return pIndicators->slow < 0;
        }
    }
    
    bool checkVolumeFilters(...) const;
    bool checkATRFilter(...) const;
    bool checkWeeklyBaseLine(...) const;
    bool checkBeiLiFilter(...) const;
    bool checkMaxLevelFilter(...) const;
    bool checkCMFVolumeFilter(...) const;
    bool checkNextDayBarFilter(...) const;
};
```

### 5.3 Template for Buy/Sell Logic

**Extract common patterns**:

```cpp
template<bool IsBuy>
void processSignal(
    StrategyParams* pParams,
    Indicators* pIndicators,
    Base_Indicators* pBase_Indicators,
    const MACDSymbolConfig& config,
    double preDailyClose,
    double dailyBaseLine,
    double macdLimit,
    int orderIndex
) {
    // Common logic
    if constexpr (IsBuy) {
        // Buy-specific
        pIndicators->entryPrice = pParams->bidAsk.ask[0];
    } else {
        // Sell-specific
        pIndicators->entryPrice = pParams->bidAsk.bid[0];
    }
    
    // ... rest of common logic
}

// Usage
processSignal<true>(...);   // Buy
processSignal<false>(...);  // Sell
```

### 5.4 Implementation Steps

1. **Create class skeleton** with all method declarations
2. **Move configuration loading** to separate method
3. **Move indicator loading** to separate method
4. **Extract buy signal processing** to method
5. **Extract sell signal processing** to method
6. **Move filter application** to base class
7. **Move stop loss calculation** to method
8. **Refactor and optimize**

**Validation**:
- [ ] Each method is < 100 lines
- [ ] No code duplication
- [ ] All tests pass
- [ ] Backtest results match exactly

---

## Phase 6: Integration & Testing

### 6.1 C Interface Implementation

**File**: `src/strategies/autobbs/trend/macd/cpp/MACDDailyStrategy.cpp`

```cpp
#include "Precompiled.h"
#include "strategies/autobbs/trend/macd/MACDDailyStrategy.h"
#include "strategies/autobbs/trend/macd/cpp/MACDDailyStrategy.hpp"

extern "C" {
    AsirikuyReturnCode workoutExecutionTrend_MACD_Daily(
        StrategyParams* pParams,
        Indicators* pIndicators,
        Base_Indicators* pBase_Indicators
    ) {
        static MACDDailyStrategy strategy;
        return strategy.execute(pParams, pIndicators, pBase_Indicators);
    }
}
```

### 6.2 Backtest Comparison

**Test Plan**:

1. **Run backtests with C version**:
   - Save all entry/exit signals
   - Save all configuration values
   - Save all filter decisions

2. **Run backtests with C++ version**:
   - Compare signals
   - Compare configurations
   - Compare filter decisions

3. **Validation**:
   - [ ] All signals match exactly
   - [ ] All configurations match
   - [ ] All filter decisions match
   - [ ] Final P&L matches
   - [ ] Trade count matches

### 6.3 Performance Testing

**Benchmarks**:

1. **Execution time**:
   - Measure C version execution time
   - Measure C++ version execution time
   - Compare (should be < 5% difference)

2. **Memory usage**:
   - Measure C version memory
   - Measure C++ version memory
   - Compare (should be similar)

3. **Binary size**:
   - Compare library sizes
   - Should be < 10% increase

### 6.4 Integration Testing

**Test with all callers**:

1. **MQL4/5**:
   - Test DLL loading
   - Test function calls
   - Test with live data

2. **Python (CTester)**:
   - Test ctypes interface
   - Test with backtests
   - Test with optimization

3. **C code**:
   - Test from StrategyExecution.c
   - Test from other strategies
   - Test with all symbols

---

## Code Examples

### Example 1: Symbol Configuration Lookup

**Before (C)**:
```c
if (strstr(pParams->tradeSymbol, "BTCUSD") != NULL || 
    strstr(pParams->tradeSymbol, "ETHUSD") != NULL) {
    // 50+ lines of configuration
} else if (strstr(pParams->tradeSymbol, "SpotCrudeUSD") != NULL) {
    // 30+ lines of configuration
}
// ... 15 more else-if blocks
```

**After (C++)**:
```cpp
auto& manager = MACDSymbolConfigManager::getInstance();
auto config = manager.getConfig(
    std::string(pParams->tradeSymbol),
    pParams->bidAsk.ask[0],
    (bool)pParams->settings[IS_BACKTESTING],
    timeInfo
);
```

### Example 2: Filter Application

**Before (C)**:
```c
if (isEnableSlow == TRUE &&
    pIndicators->entrySignal != 0 &&
    pIndicators->slow <= 0)
{
    // ... error handling
    pIndicators->entrySignal = 0;
}

if (isEnableCMFVolume == TRUE &&
    pIndicators->entrySignal != 0 &&
    pIndicators->cmfVolume <= 0)
{
    // ... error handling
    pIndicators->entrySignal = 0;
}
// ... 10+ more similar blocks
```

**After (C++)**:
```cpp
if (!applyEntryFilters(config, pIndicators, pBase_Indicators, 
                      preDailyClose, dailyBaseLine, weeklyBaseline,
                      orderIndex, isBuy, pParams)) {
    pIndicators->entrySignal = 0;
    return;  // Early exit
}
```

### Example 3: Buy/Sell Signal Processing

**Before (C)**:
```c
// Buy signal (200+ lines)
if (pIndicators->fast > 0 && ...) {
    // ... buy logic
}

// Sell signal (200+ lines, mostly duplicated)
if (pIndicators->fast < 0 && ...) {
    // ... sell logic (almost identical)
}
```

**After (C++)**:
```cpp
if (pIndicators->fast > 0 && ...) {
    processSignal<BUY>(pParams, pIndicators, pBase_Indicators,
                      config, preDailyClose, dailyBaseLine,
                      macdLimit, orderIndex);
}

if (pIndicators->fast < 0 && ...) {
    processSignal<SELL>(pParams, pIndicators, pBase_Indicators,
                        config, preDailyClose, dailyBaseLine,
                        macdLimit, orderIndex);
}
```

---

## Migration Checklist

### Phase 1: Preparation
- [ ] Update premake4.lua for C++
- [ ] Create directory structure
- [ ] Create C interface wrappers
- [ ] Verify build system works

### Phase 2: Symbol Configuration
- [ ] Create MACDSymbolConfig class
- [ ] Create MACDSymbolConfigManager
- [ ] Implement all symbol builders
- [ ] Unit tests for all symbols
- [ ] Verify configurations match C version

### Phase 3: Order Splitting
- [ ] Create MACDOrderSplitter class
- [ ] Implement all splitting methods
- [ ] Unit tests for order splitting
- [ ] Verify results match C version

### Phase 4: Weekly Strategy
- [ ] Create MACDStrategyBase
- [ ] Implement MACDWeeklyStrategy
- [ ] Extract common filters
- [ ] Backtest comparison
- [ ] Verify identical results

### Phase 5: Daily Strategy
- [ ] Create MACDDailyStrategy class
- [ ] Break down large function
- [ ] Extract filter methods
- [ ] Extract signal processing
- [ ] Refactor and optimize
- [ ] Comprehensive backtest comparison

### Phase 6: Integration
- [ ] C interface implementation
- [ ] Integration with all callers
- [ ] Performance testing
- [ ] Documentation
- [ ] Remove old C files

---

## Risk Mitigation

### Risk 1: Build System Issues

**Mitigation**:
- Test build system changes early
- Keep C files until migration complete
- Use feature flag to switch between C/C++

### Risk 2: Logic Errors

**Mitigation**:
- Extensive unit testing
- Backtest comparison for every change
- Code review for each phase
- Keep C version as reference

### Risk 3: Performance Regression

**Mitigation**:
- Benchmark before/after
- Profile hot paths
- Use inline functions where appropriate
- Avoid unnecessary allocations

### Risk 4: Integration Issues

**Mitigation**:
- Maintain exact C interface
- Test with all callers early
- Gradual rollout (test with one symbol first)

---

## Success Metrics

### Code Quality
- ✅ Reduced code duplication by > 50%
- ✅ Average function length < 100 lines
- ✅ Cyclomatic complexity reduced
- ✅ Unit test coverage > 80%

### Functionality
- ✅ All symbols work correctly
- ✅ All backtests produce identical results
- ✅ All filters work identically
- ✅ No regression in features

### Performance
- ✅ Execution time within 5% of C version
- ✅ Memory usage similar or better
- ✅ Binary size increase < 10%

### Maintainability
- ✅ Adding new symbol takes < 30 minutes
- ✅ Code is easier to understand
- ✅ Better documentation

---

## Timeline

| Week | Phase | Deliverables |
|------|-------|--------------|
| 1 | Preparation | Build system, directory structure, C wrappers |
| 2 | Symbol Config | MACDSymbolConfig class, Manager, unit tests |
| 3 | Order Splitting | MACDOrderSplitter class, tests |
| 4 | Weekly Strategy | MACDStrategyBase, MACDWeeklyStrategy, tests |
| 5-6 | Daily Strategy | MACDDailyStrategy, refactoring, tests |
| 7 | Integration | C interface, integration tests, docs |
| 8-9 | Buffer | Bug fixes, optimization, final testing |

**Total**: 9 weeks (with 2-week buffer)

---

## Next Steps

1. **Review and approve** this plan
2. **Set up development branch**: `feature/macd-cpp-migration`
3. **Create Phase 1 tasks** in project management
4. **Begin implementation** with Phase 1

---

**Document Version**: 1.0  
**Last Updated**: November 2024  
**Status**: Ready for Review

