# MACD Strategy C++ Migration Specification

**Date**: November 2024  
**Status**: Draft  
**Target**: MACD Daily, Weekly, and Order Splitting modules

## Executive Summary

This document outlines the specification and implementation plan for converting the MACD trading strategy modules from C to C++. The conversion will improve code organization, maintainability, and extensibility while maintaining full compatibility with the existing C-based framework.

**Scope**:
- `MACDDailyStrategy.c` (1,366 lines) → `MACDDailyStrategy.cpp`
- `MACDWeeklyStrategy.c` (200 lines) → `MACDWeeklyStrategy.cpp`
- `MACDOrderSplitting.c` (207 lines) → `MACDOrderSplitting.cpp`

**Total Lines**: ~1,773 lines of C code

---

## 1. Current State Analysis

### 1.1 Code Structure

**MACDDailyStrategy.c**:
- **Size**: 1,366 lines
- **Complexity**: High (extensive symbol-specific logic)
- **Key Components**:
  - `MACDSymbolConfig` struct (60+ fields)
  - `initializeSymbolConfig()` function (400+ lines, large if-else chain)
  - `workoutExecutionTrend_MACD_Daily()` main function (600+ lines)
  - 15+ symbol configurations (BTCUSD, ETHUSD, XAUUSD, etc.)
  - Complex filtering logic (volume, ATR, weekly baseline, BeiLi, etc.)

**MACDWeeklyStrategy.c**:
- **Size**: 200 lines
- **Complexity**: Medium
- **Key Components**:
  - `workoutExecutionTrend_MACD_Weekly()` function
  - Symbol-specific level thresholds
  - Volume confirmation logic

**MACDOrderSplitting.c**:
- **Size**: 207 lines
- **Complexity**: Low-Medium
- **Key Components**:
  - `splitBuyOrders_MACDDaily()` / `splitSellOrders_MACDDaily()`
  - `splitBuyOrders_MACDWeekly()` / `splitSellOrders_MACDWeekly()`
  - Order size calculation and splitting logic

### 1.2 Current Pain Points

1. **Large if-else chain** for symbol configuration (400+ lines)
2. **Repetitive code** for buy/sell signal processing
3. **Manual configuration management** (no lookup tables)
4. **Limited type safety** (lots of BOOL flags, manual validation)
5. **Hard to extend** (adding new symbols requires modifying large function)
6. **No encapsulation** (all logic in one large function)

### 1.3 Integration Points

- **Called from**: `StrategyExecution.c` (C code)
- **Headers**: Already have `extern "C"` wrappers
- **Dependencies**: All C structures (StrategyParams, Indicators, Base_Indicators)
- **Build system**: Premake4, currently `language "C"`

---

## 2. C++ Design Specification

### 2.1 Architecture Overview

```
┌─────────────────────────────────────────────────────────┐
│              C Interface (extern "C")                    │
│  workoutExecutionTrend_MACD_Daily()                      │
│  workoutExecutionTrend_MACD_Weekly()                      │
│  splitBuyOrders_MACDDaily() / splitSellOrders_MACDDaily()│
└──────────────────────┬──────────────────────────────────┘
                       │
                       ▼
┌─────────────────────────────────────────────────────────┐
│              C++ Implementation Layer                    │
│                                                          │
│  ┌──────────────────────────────────────────────┐      │
│  │  MACDStrategyBase (Abstract Base Class)       │      │
│  │  - Common MACD logic                          │      │
│  │  - Filter management                          │      │
│  │  - Volume/ATR checks                          │      │
│  └──────────────┬───────────────────────────────┘      │
│                 │                                       │
│    ┌────────────┴────────────┐                         │
│    │                         │                         │
│  ┌─▼──────────┐        ┌─────▼──────┐                 │
│  │MACDDaily   │        │MACDWeekly  │                 │
│  │Strategy    │        │Strategy    │                 │
│  └────────────┘        └────────────┘                 │
│                                                          │
│  ┌──────────────────────────────────────────────┐      │
│  │  MACDSymbolConfigManager                      │      │
│  │  - std::map<std::string, SymbolConfig>        │      │
│  │  - Configuration lookup                       │      │
│  │  - Dynamic configuration builder              │      │
│  └──────────────────────────────────────────────┘      │
│                                                          │
│  ┌──────────────────────────────────────────────┐      │
│  │  MACDOrderSplitter                            │      │
│  │  - Order size calculation                     │      │
│  │  - Split logic                                │      │
│  └──────────────────────────────────────────────┘      │
└─────────────────────────────────────────────────────────┘
```

### 2.2 Core Classes

#### 2.2.1 `MACDSymbolConfig` (Class)

**Purpose**: Encapsulate symbol-specific configuration with better type safety and initialization.

```cpp
class MACDSymbolConfig {
public:
    // MACD Parameters
    struct MACDParams {
        int fastPeriod = 5;
        int slowPeriod = 10;
        int signalPeriod = 5;
    };
    
    // Entry Thresholds
    struct Thresholds {
        double level = 0.0;
        double maxLevel = 0.0;
        double histLevel = 0.01;
        double nextMACDRange = 0.0;
    };
    
    // Risk Management
    struct RiskParams {
        double stopLossMultiplier = 1.0;
        double maxRisk = 2.0;
        int range = 10000;
    };
    
    // Filter Flags (using enum class for type safety)
    enum class FilterType {
        VolumeControl,
        BeiLi,
        SlowLine,
        ATR,
        CMFVolume,
        CMFVolumeGap,
        MaxLevelBuy,
        MaxLevelSell,
        WeeklyBaseLine
    };
    
    // Configuration methods
    void setSymbol(const std::string& symbol);
    void setPrice(double price);
    void setBacktestMode(bool isBacktesting);
    void setDate(const std::tm& date);
    
    // Getters
    MACDParams getMACDParams() const { return macdParams_; }
    Thresholds getThresholds() const { return thresholds_; }
    RiskParams getRiskParams() const { return riskParams_; }
    bool isFilterEnabled(FilterType filter) const;
    
    // Dynamic level calculation
    void calculateDynamicLevels(double price);
    
private:
    MACDParams macdParams_;
    Thresholds thresholds_;
    RiskParams riskParams_;
    std::bitset<16> filterFlags_;  // Type-safe flag storage
    std::string symbol_;
    bool isBacktesting_ = false;
    
    // Helper methods
    void initializeDefaults();
    void configureForSymbol();
};
```

#### 2.2.2 `MACDSymbolConfigManager` (Class)

**Purpose**: Replace large if-else chain with lookup table and builder pattern.

```cpp
class MACDSymbolConfigManager {
public:
    static MACDSymbolConfigManager& getInstance();
    
    // Get configuration for symbol
    MACDSymbolConfig getConfig(
        const std::string& symbol,
        double price,
        bool isBacktesting,
        const std::tm& date
    );
    
    // Register custom symbol configuration
    void registerSymbol(const std::string& symbol, 
                       const MACDSymbolConfig& config);
    
private:
    MACDSymbolConfigManager();
    
    // Symbol configuration builders
    MACDSymbolConfig buildCryptoConfig(const std::string& symbol, 
                                      double price, 
                                      bool isBacktesting,
                                      const std::tm& date);
    MACDSymbolConfig buildOilConfig(const std::string& symbol, double price);
    MACDSymbolConfig buildGoldConfig(const std::string& symbol, double price);
    MACDSymbolConfig buildForexConfig(const std::string& symbol);
    MACDSymbolConfig buildDefaultConfig();
    
    // Symbol pattern matching
    bool matchesPattern(const std::string& symbol, 
                       const std::string& pattern) const;
    
    std::map<std::string, std::function<MACDSymbolConfig(double, bool, const std::tm&)>> 
        configBuilders_;
};
```

#### 2.2.3 `MACDStrategyBase` (Abstract Base Class)

**Purpose**: Common logic for Daily and Weekly strategies.

```cpp
class MACDStrategyBase {
public:
    virtual ~MACDStrategyBase() = default;
    
    // Main execution (to be implemented by derived classes)
    virtual AsirikuyReturnCode execute(
        StrategyParams* pParams,
        Indicators* pIndicators,
        Base_Indicators* pBase_Indicators
    ) = 0;
    
protected:
    // Common filter methods
    bool checkVolumeFilters(const MACDSymbolConfig& config,
                           const Indicators* pIndicators) const;
    bool checkATRFilter(const MACDSymbolConfig& config,
                       double atr5, double entryPrice, double atrRange) const;
    bool checkWeeklyBaseLine(const MACDSymbolConfig& config,
                             double preWeeklyClose, double weeklyBaseline) const;
    bool checkBeiLiFilter(const MACDSymbolConfig& config,
                          bool isMACDBeili, double minPoint, 
                          double level, int range) const;
    bool checkMaxLevelFilter(const MACDSymbolConfig& config,
                            double fast, double maxLevel,
                            bool isBuy, int orderIndex,
                            const StrategyParams* pParams) const;
    
    // Risk adjustment
    void adjustRiskForVolume(const MACDSymbolConfig& config,
                            Indicators* pIndicators,
                            double maxRisk) const;
    
    // Helper methods
    bool isLateEntry(const MACDSymbolConfig& config,
                    const std::vector<double>& preHist,
                    const std::vector<double>& fast,
                    double level, double histLevel,
                    int orderIndex, OrderType orderType,
                    const StrategyParams* pParams) const;
};
```

#### 2.2.4 `MACDDailyStrategy` (Class)

**Purpose**: Daily strategy implementation.

```cpp
class MACDDailyStrategy : public MACDStrategyBase {
public:
    AsirikuyReturnCode execute(
        StrategyParams* pParams,
        Indicators* pIndicators,
        Base_Indicators* pBase_Indicators
    ) override;
    
private:
    // Entry signal processing
    void processBuySignal(
        StrategyParams* pParams,
        Indicators* pIndicators,
        Base_Indicators* pBase_Indicators,
        const MACDSymbolConfig& config,
        double preDailyClose,
        double dailyBaseLine,
        double macdLimit,
        int orderIndex
    );
    
    void processSellSignal(
        StrategyParams* pParams,
        Indicators* pIndicators,
        Base_Indicators* pBase_Indicators,
        const MACDSymbolConfig& config,
        double preDailyClose,
        double dailyBaseLine,
        double macdLimit,
        int orderIndex
    );
    
    // Stop loss calculation
    double calculateStopLoss(
        const MACDSymbolConfig& config,
        Base_Indicators* pBase_Indicators,
        double highHourlyClose,
        double lowHourlyClose,
        double entryPrice,
        bool isBuy
    ) const;
    
    // Entry filters
    bool applyEntryFilters(
        const MACDSymbolConfig& config,
        Indicators* pIndicators,
        Base_Indicators* pBase_Indicators,
        double preDailyClose,
        double dailyBaseLine,
        double weeklyBaseline,
        int orderIndex,
        bool isBuy,
        const StrategyParams* pParams
    ) const;
    
    // MACD indicator loading
    struct MACDData {
        double fast, slow, preFast, preSlow;
        std::vector<double> preHist;  // Historical values
        std::vector<double> fastHist;  // Fast line history
    };
    
    MACDData loadMACDIndicators(
        int fastPeriod, int slowPeriod, int signalPeriod,
        int startShift
    ) const;
};
```

#### 2.2.5 `MACDWeeklyStrategy` (Class)

**Purpose**: Weekly strategy implementation.

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
};
```

#### 2.2.6 `MACDOrderSplitter` (Class)

**Purpose**: Order splitting logic.

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
    
    // Weekly order splitting
    void splitBuyOrdersWeekly(...);
    void splitSellOrdersWeekly(...);
    
private:
    struct OrderSplitResult {
        double takePrice;
        double lots;
        bool shouldSplit;
        double mainLots;
        double secondaryLots;
    };
    
    OrderSplitResult calculateOrderSplit(
        StrategyParams* pParams,
        Indicators* pIndicators,
        Base_Indicators* pBase_Indicators,
        OrderType orderType,
        double atr,
        double stopLoss
    ) const;
    
    static constexpr double TP_MULTIPLIER_DAILY_ATR = 1.5;
    static constexpr int MIN_LOT_SIZE_THRESHOLD = 5;
};
```

### 2.3 C Interface Wrappers

**Purpose**: Maintain compatibility with existing C code.

```cpp
extern "C" {
    AsirikuyReturnCode workoutExecutionTrend_MACD_Daily(
        StrategyParams* pParams,
        Indicators* pIndicators,
        Base_Indicators* pBase_Indicators
    ) {
        static MACDDailyStrategy strategy;
        return strategy.execute(pParams, pIndicators, pBase_Indicators);
    }
    
    AsirikuyReturnCode workoutExecutionTrend_MACD_Weekly(
        StrategyParams* pParams,
        Indicators* pIndicators,
        Base_Indicators* pBase_Indicators
    ) {
        static MACDWeeklyStrategy strategy;
        return strategy.execute(pParams, pIndicators, pBase_Indicators);
    }
    
    void splitBuyOrders_MACDDaily(
        StrategyParams* pParams,
        Indicators* pIndicators,
        Base_Indicators* pBase_Indicators,
        double atr,
        double stopLoss
    ) {
        static MACDOrderSplitter splitter;
        splitter.splitBuyOrders(pParams, pIndicators, pBase_Indicators, atr, stopLoss);
    }
    
    // ... other order splitting functions
}
```

---

## 3. Benefits of C++ Conversion

### 3.1 Code Organization

1. **Eliminate large if-else chain**: Use `std::map` for symbol lookup
2. **Reduce duplication**: Template functions for buy/sell logic
3. **Better encapsulation**: Private methods, member variables
4. **Type safety**: `enum class` instead of BOOL flags
5. **RAII**: Automatic resource management

### 3.2 Maintainability

1. **Easier to add symbols**: Register in map, no code changes
2. **Clearer structure**: Classes separate concerns
3. **Better documentation**: C++ features (const, override, etc.)
4. **Easier testing**: Mockable classes, unit testable

### 3.3 Performance

1. **No performance penalty**: C++ compiles to same assembly
2. **Possible optimizations**: Inline functions, constexpr
3. **Better compiler optimizations**: More type information

### 3.4 Extensibility

1. **Inheritance**: Easy to create strategy variants
2. **Polymorphism**: Strategy pattern for different implementations
3. **Templates**: Generic filter functions
4. **STL containers**: Efficient data structures

---

## 4. Migration Strategy

### 4.1 Phase 1: Preparation (Week 1)

**Goals**: Set up infrastructure, create C++ skeleton

1. **Update build system**:
   - Modify `premake4.lua` to support mixed C/C++
   - Add C++ files to build
   - Ensure C++ standard (C++11 or C++14)

2. **Create header structure**:
   - `MACDSymbolConfig.hpp`
   - `MACDSymbolConfigManager.hpp`
   - `MACDStrategyBase.hpp`
   - `MACDDailyStrategy.hpp`
   - `MACDWeeklyStrategy.hpp`
   - `MACDOrderSplitter.hpp`

3. **Create C interface wrappers**:
   - Maintain existing function signatures
   - Add `extern "C"` wrappers

### 4.2 Phase 2: Symbol Configuration (Week 2)

**Goals**: Convert configuration management

1. **Implement `MACDSymbolConfig` class**:
   - Convert struct to class
   - Add getters/setters
   - Implement initialization logic

2. **Implement `MACDSymbolConfigManager`**:
   - Create symbol lookup map
   - Implement builder functions
   - Replace if-else chain

3. **Testing**:
   - Unit tests for each symbol configuration
   - Verify all symbols produce same config as C version

### 4.3 Phase 3: Order Splitting (Week 2-3)

**Goals**: Convert order splitting logic

1. **Implement `MACDOrderSplitter` class**:
   - Convert functions to methods
   - Extract common logic
   - Add helper methods

2. **Testing**:
   - Unit tests for order splitting
   - Verify same results as C version

### 4.4 Phase 4: Weekly Strategy (Week 3)

**Goals**: Convert weekly strategy (simpler, good test case)

1. **Implement `MACDWeeklyStrategy` class**:
   - Inherit from `MACDStrategyBase`
   - Implement execute method
   - Extract common filters to base class

2. **Testing**:
   - Backtest comparison with C version
   - Verify identical results

### 4.5 Phase 5: Daily Strategy (Week 4-5)

**Goals**: Convert complex daily strategy

1. **Implement `MACDStrategyBase`**:
   - Extract common filter methods
   - Implement shared logic

2. **Implement `MACDDailyStrategy`**:
   - Break down large function into methods
   - Extract buy/sell signal processing
   - Extract filter application
   - Extract stop loss calculation

3. **Refactoring**:
   - Eliminate code duplication
   - Use templates where appropriate
   - Improve readability

4. **Testing**:
   - Comprehensive backtest comparison
   - Verify all symbols work correctly
   - Performance testing

### 4.6 Phase 6: Integration & Testing (Week 6)

**Goals**: Final integration and validation

1. **Integration testing**:
   - Test with all calling code
   - Verify C interface compatibility
   - Test with MQL4/5, Python, CTester

2. **Performance validation**:
   - Compare execution time
   - Memory usage analysis
   - Binary size comparison

3. **Documentation**:
   - Update code comments
   - Create migration guide
   - Document new architecture

---

## 5. Implementation Details

### 5.1 Build System Changes

**premake4.lua modifications**:

```lua
project "TradingStrategies"
  location("../../build/" .. _ACTION .. "/projects")
  kind "SharedLib"
  language "C++"  -- Change from "C" to "C++"
  cppdialect "C++11"  -- Or C++14
  targetname "trading_strategies"
  files{
    "**.h", 
    "**.c", 
    "**.cpp",  -- Add C++ files
    "**.hpp",  -- Add C++ headers
    "**.cxx"
  }
  
  -- C++ specific settings
  configuration { "not windows" }
    buildoptions { "-std=c++11" }
  
  configuration { "windows" }
    buildoptions { "/std:c++11" }
```

### 5.2 Header File Structure

```
core/TradingStrategies/include/strategies/autobbs/trend/macd/
├── MACDDailyStrategy.h          (C interface, unchanged)
├── MACDWeeklyStrategy.h          (C interface, unchanged)
├── MACDOrderSplitting.h         (C interface, unchanged)
└── cpp/
    ├── MACDSymbolConfig.hpp
    ├── MACDSymbolConfigManager.hpp
    ├── MACDStrategyBase.hpp
    ├── MACDDailyStrategy.hpp
    ├── MACDWeeklyStrategy.hpp
    └── MACDOrderSplitter.hpp
```

### 5.3 Symbol Configuration Map

**Example implementation**:

```cpp
class MACDSymbolConfigManager {
private:
    using ConfigBuilder = std::function<MACDSymbolConfig(double, bool, const std::tm&)>;
    
    std::map<std::string, ConfigBuilder> symbolBuilders_ = {
        {"BTCUSD", [](double price, bool backtest, const std::tm& date) {
            return buildCryptoConfig("BTCUSD", price, backtest, date);
        }},
        {"ETHUSD", [](double price, bool backtest, const std::tm& date) {
            return buildCryptoConfig("ETHUSD", price, backtest, date);
        }},
        {"SpotCrudeUSD", [](double price, bool, const std::tm&) {
            return buildOilConfig("SpotCrudeUSD", price);
        }},
        {"XTIUSD", [](double price, bool, const std::tm&) {
            return buildOilConfig("XTIUSD", price);
        }},
        // ... more symbols
    };
    
    // Pattern matching for symbol groups
    std::vector<std::pair<std::string, ConfigBuilder>> patternBuilders_ = {
        {"XAU", [](double price, bool, const std::tm&) {
            return buildGoldConfig("XAUUSD", price);
        }},
        {"JPY", [](double, bool, const std::tm&) {
            return buildForexConfig("JPY");
        }},
        // ... more patterns
    };
};
```

### 5.4 Template Functions for Buy/Sell Logic

**Example**:

```cpp
template<OrderType OrderType>
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
    // Common logic for both buy and sell
    // Use template specialization for differences
}

// Specialization for BUY
template<>
void processSignal<BUY>(...) {
    // Buy-specific logic
}

// Specialization for SELL
template<>
void processSignal<SELL>(...) {
    // Sell-specific logic
}
```

---

## 6. Risk Assessment

### 6.1 Technical Risks

| Risk | Probability | Impact | Mitigation |
|------|------------|--------|------------|
| Build system issues | Medium | High | Test build early, maintain C fallback |
| Performance regression | Low | Medium | Benchmark before/after, optimize if needed |
| Integration issues | Low | High | Maintain C interface, extensive testing |
| Memory issues | Low | High | Use RAII, avoid manual memory management |
| Compiler compatibility | Medium | Medium | Test on all target platforms |

### 6.2 Compatibility Risks

| Risk | Probability | Impact | Mitigation |
|------|------------|--------|------------|
| C interface changes | Low | High | Keep exact same function signatures |
| Binary compatibility | Low | High | Test with all callers (MQL, Python, CTester) |
| Data structure changes | Low | High | Use same C structures, no changes |

### 6.3 Migration Risks

| Risk | Probability | Impact | Mitigation |
|------|------------|--------|------------|
| Incomplete conversion | Medium | High | Phased approach, test each phase |
| Logic errors | Medium | High | Extensive backtesting, comparison tests |
| Timeline overrun | Medium | Low | Phased approach allows partial completion |

---

## 7. Testing Strategy

### 7.1 Unit Testing

**Framework**: Google Test or Catch2

**Test Cases**:
1. **Symbol Configuration**:
   - Test each symbol produces correct config
   - Test dynamic level calculation
   - Test backtest mode logic
   - Test date cutoff logic

2. **Filter Logic**:
   - Test each filter independently
   - Test filter combinations
   - Test edge cases

3. **Order Splitting**:
   - Test lot size calculations
   - Test split logic
   - Test different trade modes

### 7.2 Integration Testing

1. **Backtest Comparison**:
   - Run same backtests with C and C++ versions
   - Compare results (trades, P&L, signals)
   - Verify identical behavior

2. **Live Trading Simulation**:
   - Test with live data feeds
   - Verify no crashes or errors
   - Monitor performance

### 7.3 Regression Testing

1. **All Symbols**:
   - Test each supported symbol
   - Verify entry/exit signals match
   - Verify stop loss calculations

2. **Edge Cases**:
   - Missing data
   - Invalid inputs
   - Boundary conditions

---

## 8. Success Criteria

### 8.1 Functional Requirements

- ✅ All symbols produce identical configuration as C version
- ✅ All entry/exit signals match C version exactly
- ✅ All order splitting produces same results
- ✅ All filters work identically
- ✅ Backtest results match C version (same trades, P&L)

### 8.2 Non-Functional Requirements

- ✅ No performance regression (< 5% overhead acceptable)
- ✅ Binary size increase < 10%
- ✅ Memory usage similar or better
- ✅ Compiles on all target platforms
- ✅ No breaking changes to C interface

### 8.3 Code Quality

- ✅ Reduced code duplication
- ✅ Improved readability
- ✅ Better maintainability
- ✅ Comprehensive documentation
- ✅ Unit test coverage > 80%

---

## 9. Timeline Estimate

| Phase | Duration | Dependencies |
|-------|----------|---------------|
| Phase 1: Preparation | 1 week | None |
| Phase 2: Symbol Config | 1 week | Phase 1 |
| Phase 3: Order Splitting | 1 week | Phase 1 |
| Phase 4: Weekly Strategy | 1 week | Phase 2, 3 |
| Phase 5: Daily Strategy | 2 weeks | Phase 2, 3, 4 |
| Phase 6: Integration & Testing | 1 week | Phase 5 |

**Total Estimate**: 7 weeks

**Buffer**: +2 weeks for unexpected issues

**Total with Buffer**: 9 weeks

---

## 10. Rollback Plan

If issues arise:

1. **Keep C version**: Maintain C files alongside C++ during migration
2. **Feature flag**: Use compile-time flag to switch between C/C++
3. **Gradual rollout**: Test with subset of symbols first
4. **Quick revert**: Can revert to C version in < 1 hour

---

## 11. Future Enhancements

After successful migration:

1. **Add new symbols**: Easy registration in map
2. **Strategy variants**: Use inheritance for variations
3. **Advanced features**: 
   - Configuration from files
   - Runtime symbol registration
   - Strategy composition
4. **Performance optimizations**:
   - SIMD for MACD calculations
   - Parallel filter evaluation
   - Caching strategies

---

## 12. Decision Points

### 12.1 C++ Standard

**Recommendation**: C++11 (widely supported, stable)

**Alternatives**:
- C++14: More features, slightly less support
- C++17: Modern features, may have compatibility issues

### 12.2 Testing Framework

**Recommendation**: Google Test (mature, widely used)

**Alternatives**:
- Catch2: Header-only, simpler
- Boost.Test: If already using Boost

### 12.3 Symbol Lookup Strategy

**Option A**: Exact match map (fastest, requires all symbols)
**Option B**: Pattern matching (flexible, slower)
**Option C**: Hybrid (exact match + patterns)

**Recommendation**: Option C (best of both worlds)

---

## 13. Open Questions

1. **C++ standard version**: C++11, C++14, or C++17?
2. **Testing framework**: Google Test, Catch2, or custom?
3. **Symbol registration**: Compile-time or runtime?
4. **Template usage**: How aggressive with templates?
5. **Exception handling**: Use exceptions or error codes?

---

## 14. Next Steps

1. **Review and approve** this specification
2. **Set up development branch**: `feature/macd-cpp-migration`
3. **Create initial C++ skeleton**: Phase 1 implementation
4. **Set up testing framework**: Unit test infrastructure
5. **Begin Phase 1**: Build system and header structure

---

## Appendix A: Code Size Comparison

**Current (C)**:
- MACDDailyStrategy.c: 1,366 lines
- MACDWeeklyStrategy.c: 200 lines
- MACDOrderSplitting.c: 207 lines
- **Total**: ~1,773 lines

**Estimated (C++)**:
- MACDDailyStrategy.cpp: ~800 lines (reduced duplication)
- MACDWeeklyStrategy.cpp: ~150 lines
- MACDOrderSplitting.cpp: ~150 lines
- MACDSymbolConfigManager.cpp: ~300 lines
- MACDStrategyBase.cpp: ~200 lines
- Headers: ~400 lines
- **Total**: ~2,000 lines (but better organized)

**Net Change**: +~200 lines, but significantly better structure

---

## Appendix B: Example Code Snippets

### B.1 Symbol Configuration Lookup

```cpp
MACDSymbolConfig MACDSymbolConfigManager::getConfig(
    const std::string& symbol,
    double price,
    bool isBacktesting,
    const std::tm& date
) {
    // Try exact match first
    auto it = symbolBuilders_.find(symbol);
    if (it != symbolBuilders_.end()) {
        return it->second(price, isBacktesting, date);
    }
    
    // Try pattern matching
    for (const auto& [pattern, builder] : patternBuilders_) {
        if (symbol.find(pattern) != std::string::npos) {
            return builder(price, isBacktesting, date);
        }
    }
    
    // Default configuration
    return buildDefaultConfig();
}
```

### B.2 Filter Application

```cpp
bool MACDStrategyBase::applyEntryFilters(
    const MACDSymbolConfig& config,
    Indicators* pIndicators,
    Base_Indicators* pBase_Indicators,
    double preDailyClose,
    double dailyBaseLine,
    double weeklyBaseline,
    int orderIndex,
    bool isBuy,
    const StrategyParams* pParams
) const {
    // Apply filters in order, short-circuit on failure
    if (config.isFilterEnabled(MACDSymbolConfig::FilterType::SlowLine)) {
        if (!checkSlowLineFilter(config, pIndicators, isBuy)) {
            return false;
        }
    }
    
    if (config.isFilterEnabled(MACDSymbolConfig::FilterType::VolumeControl)) {
        if (!checkVolumeFilters(config, pIndicators)) {
            return false;
        }
    }
    
    // ... more filters
    
    return true;
}
```

---

**Document Version**: 1.0  
**Last Updated**: November 2024  
**Author**: AI Assistant  
**Status**: Draft - Pending Review

