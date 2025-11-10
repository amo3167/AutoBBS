# Modern C++ Class-Based Strategy Migration Guide

## Philosophy

**The canonical migration approach uses modern C++ with classes, encapsulation, RAII, and idiomatic patterns.** This approach prioritizes:

- **Maintainability**: Clear class interfaces, separation of concerns
- **Testability**: Easy to mock dependencies, unit test individual methods
- **Type Safety**: Strong typing, compile-time checks
- **Modern C++**: Smart pointers, RAII, standard library containers
- **Readability**: Self-documenting code with clear abstractions

The C-style function approach (using `StrategyParams*` directly) is **deprecated** for new migrations.

---

## Architecture Overview

### Core Components

```
┌─────────────────────────────────────────────────────┐
│              Strategy (Pure Virtual)                │
│  - executeStrategy(context, indicators) -> result   │
│  - loadIndicators(context) -> Indicators*          │
│  - updateResults(context, result)                   │
└─────────────────────────────────────────────────────┘
                          ▲
                          │ inherits
                          │
┌─────────────────────────────────────────────────────┐
│           TakeOverStrategy (Concrete)               │
│  - Indicator loading logic                          │
│  - Entry/exit decision logic                        │
│  - Order modification logic                         │
│  - Strategy-specific helpers                        │
└─────────────────────────────────────────────────────┘
                          │
                          │ uses
                          ▼
┌──────────────────┐  ┌──────────────────┐  ┌──────────────────┐
│ StrategyContext  │  │  OrderManager    │  │   Indicators     │
│ - Bar data       │  │  - Place orders  │  │   - Calculate    │
│ - Settings       │  │  - Modify orders │  │   - Cache values │
│ - Bid/Ask        │  │  - Close orders  │  │   - Multi-TF     │
└──────────────────┘  └──────────────────┘  └──────────────────┘
```

### Key Classes

#### 1. StrategyContext
Provides clean abstraction over raw `StrategyParams*`:
```cpp
class StrategyContext {
public:
    // Bar data access
    double getClose(int ratesIndex, int shift = 0) const;
    double getHigh(int ratesIndex, int shift = 0) const;
    double getLow(int ratesIndex, int shift = 0) const;
    double getOpen(int ratesIndex, int shift = 0) const;
    time_t getTime(int ratesIndex, int shift = 0) const;
    int getBarsTotal(int ratesIndex) const;
    
    // Market data
    double getBid(int index = 0) const;
    double getAsk(int index = 0) const;
    
    // Settings
    double getSetting(SettingIndex index) const;
    const char* getSymbol() const;
    time_t getCurrentTime() const;
};
```

#### 2. OrderManager
Singleton managing all order operations:
```cpp
class OrderManager {
public:
    static OrderManager& getInstance();
    
    // Order placement
    bool openSingleLong(double tp, double sl, double lots, double risk);
    bool openSingleShort(double tp, double sl, double lots, double risk);
    bool openOrUpdateLong(double tp, double sl, double lots, double risk);
    bool openOrUpdateShort(double tp, double sl, double lots, double risk);
    
    // Order modification
    bool modifyTradeEasy(int orderType, double tp, double sl, double lots);
    bool trailOpenTrades(int orderType, double trailDistance);
    
    // Order closure
    bool closeAllLongs();
    bool closeAllShorts();
    bool closeLongTrade(int index);
    bool closeShortTrade(int index);
    
    // Query
    int totalOpenOrders(int orderType) const;
    int totalClosedOrders() const;
};
```

#### 3. BaseStrategy
Abstract base class for all strategies:
```cpp
class BaseStrategy {
public:
    virtual ~BaseStrategy() = default;
    
    // Pure virtual methods
    virtual StrategyResult executeStrategy(
        const StrategyContext& context,
        Indicators* indicators) = 0;
    
    virtual Indicators* loadIndicators(
        const StrategyContext& context) = 0;
    
    virtual void updateResults(
        const StrategyContext& context,
        const StrategyResult& result) = 0;
    
    virtual std::string getName() const = 0;
    
protected:
    BaseStrategy(StrategyType type);
    StrategyType m_type;
};
```

---

## Migration Process

### Phase 1: Define Strategy Class

```cpp
// TakeOverStrategy.hpp
#pragma once
#include "BaseStrategy.hpp"

namespace trading {

// Strategy-specific indicator structure
struct TakeOverIndicators {
    double dailyATR;
    double bbsStopPrice;
    int bbsTrend;
    int bbsIndex;
    double preHigh;
    double preLow;
    double preClose;
    double pre2DaysHigh;
    double pre2DaysLow;
    double movingAverage200M;
    double buyStopLossPrice;
    double sellStopLossPrice;
    double position;
    double adjust;
    DslType dslType;
};

class TakeOverStrategy : public BaseStrategy {
public:
    TakeOverStrategy();
    virtual ~TakeOverStrategy() = default;
    
    // Implement base class interface
    StrategyResult executeStrategy(
        const StrategyContext& context,
        Indicators* indicators) override;
    
    Indicators* loadIndicators(
        const StrategyContext& context) override;
    
    void updateResults(
        const StrategyContext& context,
        const StrategyResult& result) override;
    
    std::string getName() const override;

private:
    // Strategy-specific helpers
    void loadTakeOverIndicators(
        const StrategyContext& context,
        TakeOverIndicators& indicators) const;
    
    bool modifyOrders(
        const StrategyContext& context,
        const TakeOverIndicators& indicators,
        int orderType) const;
    
    bool shouldExitOnBBSTrend(
        const StrategyContext& context,
        const TakeOverIndicators& indicators) const;
};

} // namespace trading
```

### Phase 2: Implement Indicator Loading

```cpp
void TakeOverStrategy::loadTakeOverIndicators(
    const StrategyContext& context,
    TakeOverIndicators& indicators) const {
    
    // Get indicator values from C functions
    indicators.dailyATR = iAtr(DAILY_RATES, 
        static_cast<int>(context.getSetting(ATR_AVERAGING_PERIOD)), 1);
    
    // Get Bollinger Band stop
    iBBandStop(PRIMARY_RATES,
        static_cast<int>(context.getSetting(ADDITIONAL_PARAM_1)),
        static_cast<int>(context.getSetting(ADDITIONAL_PARAM_2)),
        &indicators.bbsTrend,
        &indicators.bbsStopPrice,
        &indicators.bbsIndex);
    
    // Get daily price data
    indicators.preHigh = iHigh(DAILY_RATES, 1);
    indicators.preLow = iLow(DAILY_RATES, 1);
    indicators.preClose = iClose(DAILY_RATES, 1);
    
    // Calculate 2-day high/low
    indicators.pre2DaysHigh = std::max(indicators.preHigh, iHigh(DAILY_RATES, 2));
    indicators.pre2DaysLow = std::min(indicators.preLow, iLow(DAILY_RATES, 2));
    
    // Get 1H MA200
    indicators.movingAverage200M = iMA(3, HOURLY_RATES, 200, 1);
    
    // Calculate stop loss based on DSL type
    indicators.dslType = static_cast<DslType>(
        static_cast<int>(context.getSetting(ADDITIONAL_PARAM_7))
    );
    
    switch (indicators.dslType) {
        case EXIT_DSL_NONE:
            // Smart auto DSL: use closer of 2-day HL or 1H MA200
            indicators.buyStopLossPrice = std::max(
                indicators.pre2DaysLow,
                indicators.movingAverage200M
            );
            indicators.sellStopLossPrice = std::min(
                indicators.pre2DaysHigh,
                indicators.movingAverage200M
            );
            break;
            
        case EXIT_DSL_1_DAY_HL:
            indicators.buyStopLossPrice = indicators.preLow;
            indicators.sellStopLossPrice = indicators.preHigh;
            break;
            
        case EXIT_DSL_BBS:
            indicators.buyStopLossPrice = indicators.bbsStopPrice;
            indicators.sellStopLossPrice = indicators.bbsStopPrice;
            break;
            
        // ... other cases
    }
    
    // Set UI values
    addValueToUI("BBSStopPrice", indicators.bbsStopPrice);
    addValueToUI("DailyATR", indicators.dailyATR);
    addValueToUI("BuySLP", indicators.buyStopLossPrice);
    addValueToUI("SellSLP", indicators.sellStopLossPrice);
}
```

### Phase 3: Implement Strategy Logic

```cpp
StrategyResult TakeOverStrategy::executeStrategy(
    const StrategyContext& context,
    Indicators* indicators) {
    
    StrategyResult result;
    result.code = SUCCESS;
    result.generatedOrders = 0;
    result.pnlImpact = 0.0;
    
    // Load indicators
    TakeOverIndicators tkIndicators;
    loadTakeOverIndicators(context, tkIndicators);
    
    // Log state
    int instanceId = static_cast<int>(context.getSetting(STRATEGY_INSTANCE_ID));
    pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL,
        (PAN_CHAR_T*)"TakeOver[%d]: BBSTrend=%d, BuySLP=%.5f, SellSLP=%.5f",
        instanceId, tkIndicators.bbsTrend,
        tkIndicators.buyStopLossPrice, tkIndicators.sellStopLossPrice);
    
    // Check BBS exit condition
    if (shouldExitOnBBSTrend(context, tkIndicators)) {
        return result;
    }
    
    // Get order manager
    OrderManager& orderMgr = OrderManager::getInstance();
    
    // Modify existing orders
    if (orderMgr.totalOpenOrders(BUY) > 0) {
        if (!modifyOrders(context, tkIndicators, BUY)) {
            result.code = STRATEGY_FAILED_TO_MODIFY_ORDERS;
            pantheios_logputs(PANTHEIOS_SEV_ERROR,
                (PAN_CHAR_T*)"TakeOver: Failed to modify buy orders");
        }
    }
    
    if (orderMgr.totalOpenOrders(SELL) > 0) {
        if (!modifyOrders(context, tkIndicators, SELL)) {
            result.code = STRATEGY_FAILED_TO_MODIFY_ORDERS;
            pantheios_logputs(PANTHEIOS_SEV_ERROR,
                (PAN_CHAR_T*)"TakeOver: Failed to modify sell orders");
        }
    }
    
    return result;
}
```

### Phase 4: Implement Helper Methods

```cpp
bool TakeOverStrategy::modifyOrders(
    const StrategyContext& context,
    const TakeOverIndicators& indicators,
    int orderType) const {
    
    OrderManager& orderMgr = OrderManager::getInstance();
    
    if (orderType == BUY && indicators.buyStopLossPrice > 0) {
        double stopLoss = std::abs(
            context.getAsk(0) - indicators.buyStopLossPrice + indicators.adjust
        );
        return orderMgr.modifyTradeEasy(BUY, -1, stopLoss, -1);
    }
    else if (orderType == SELL && indicators.sellStopLossPrice > 0) {
        double stopLoss = std::abs(
            indicators.sellStopLossPrice - context.getBid(0) + indicators.adjust
        );
        return orderMgr.modifyTradeEasy(SELL, -1, stopLoss, -1);
    }
    
    return true;
}

bool TakeOverStrategy::shouldExitOnBBSTrend(
    const StrategyContext& context,
    const TakeOverIndicators& indicators) const {
    
    // Only check BBS exit for 1M timeframe
    if (indicators.dslType != EXIT_DSL_BBS ||
        static_cast<int>(context.getSetting(TIMEFRAME)) != 1) {
        return false;
    }
    
    OrderManager& orderMgr = OrderManager::getInstance();
    int shift1Index = context.getBarsTotal(0) - 2;
    
    // Exit longs on BBS reversal to down
    if (orderMgr.totalOpenOrders(BUY) > 0 &&
        indicators.bbsTrend == DOWN &&
        indicators.bbsIndex == shift1Index &&
        context.getAsk(0) >= indicators.position) {
        
        orderMgr.closeAllLongs();
        pantheios_logputs(PANTHEIOS_SEV_INFORMATIONAL,
            (PAN_CHAR_T*)"TakeOver: Closed longs on BBS reversal");
        return true;
    }
    
    // Exit shorts on BBS reversal to up
    if (orderMgr.totalOpenOrders(SELL) > 0 &&
        indicators.bbsTrend == UP &&
        indicators.bbsIndex == shift1Index &&
        context.getBid(0) <= indicators.position) {
        
        orderMgr.closeAllShorts();
        pantheios_logputs(PANTHEIOS_SEV_INFORMATIONAL,
            (PAN_CHAR_T*)"TakeOver: Closed shorts on BBS reversal");
        return true;
    }
    
    return false;
}
```

---

## Benefits of Class-Based Approach

### 1. Encapsulation
- Strategy logic is self-contained within the class
- Private helper methods keep implementation details hidden
- Clear public interface for framework interaction

### 2. Testability
```cpp
// Easy to unit test individual methods
TEST(TakeOverStrategy, ModifyOrdersBuy) {
    TakeOverStrategy strategy;
    MockStrategyContext context;
    TakeOverIndicators indicators;
    indicators.buyStopLossPrice = 1.2000;
    
    bool result = strategy.modifyOrders(context, indicators, BUY);
    EXPECT_TRUE(result);
}
```

### 3. Reusability
- Common logic can be extracted to base class
- Strategy-specific helpers are isolated
- Easy to compose and extend behavior

### 4. Type Safety
- Compile-time checks for method signatures
- Strong typing for indicator structures
- Clear interfaces reduce runtime errors

### 5. Modern C++ Features
```cpp
// Use standard library
std::vector<Order> orders;
std::unique_ptr<Indicators> indicators;
std::string strategyName = getName();

// RAII for resource management
class IndicatorCache {
    std::map<std::string, double> cache;
public:
    ~IndicatorCache() { /* automatic cleanup */ }
};
```

---

## Migration Checklist

### For Each Strategy:

- [ ] Create strategy class inheriting from `BaseStrategy`
- [ ] Define strategy-specific indicator structure
- [ ] Implement `loadIndicators()` method
- [ ] Implement `executeStrategy()` method with main logic
- [ ] Implement `updateResults()` if strategy places orders
- [ ] Add private helper methods for complex logic
- [ ] Use `StrategyContext` for all bar/settings access
- [ ] Use `OrderManager` singleton for all order operations
- [ ] Add UI values via `addValueToUI()`
- [ ] Write unit tests for key methods
- [ ] Update strategy registry/factory

---

## Comparison: C-Style vs Class-Based

| Aspect | C-Style Function | Class-Based (Modern) |
|--------|------------------|---------------------|
| **Encapsulation** | ❌ Global functions | ✅ Private methods |
| **Testability** | ⚠️ Requires mocking C API | ✅ Easy to mock classes |
| **Type Safety** | ⚠️ Loose (void*, casts) | ✅ Strong typing |
| **Maintainability** | ⚠️ Scattered logic | ✅ Self-contained |
| **Reusability** | ❌ Copy-paste | ✅ Inheritance |
| **Modern C++** | ❌ Not applicable | ✅ Full support |
| **Complexity** | ⚠️ Simple but verbose | ✅ Clear abstraction |

---

## Common Patterns

### Pattern 1: Dynamic Stop Loss Calculation
```cpp
double TakeOverStrategy::calculateDynamicStopLoss(
    const StrategyContext& context,
    const TakeOverIndicators& indicators,
    int orderType) const {
    
    switch (indicators.dslType) {
        case EXIT_DSL_1_DAY_HL:
            return (orderType == BUY) ? 
                indicators.preLow : indicators.preHigh;
        
        case EXIT_DSL_BBS:
            return indicators.bbsStopPrice;
        
        case EXIT_DSL_DAILY_ATR:
            return indicators.dailyATR;
        
        default:
            return 0.0; // No change
    }
}
```

### Pattern 2: Multi-Condition Exit
```cpp
bool TakeOverStrategy::shouldExit(
    const StrategyContext& context,
    const TakeOverIndicators& indicators,
    int orderType) const {
    
    // Condition 1: BBS trend reversal
    if (shouldExitOnBBSTrend(context, indicators)) {
        return true;
    }
    
    // Condition 2: Time-based exit
    if (isEndOfWeek(context.getCurrentTime())) {
        return true;
    }
    
    // Condition 3: Profit target reached
    OrderManager& orderMgr = OrderManager::getInstance();
    if (orderMgr.getPnL(orderType) > indicators.dailyATR * 3.0) {
        return true;
    }
    
    return false;
}
```

### Pattern 3: Indicator Caching
```cpp
class TakeOverStrategy : public BaseStrategy {
private:
    // Cache indicators to avoid recalculation
    mutable std::optional<TakeOverIndicators> m_cachedIndicators;
    mutable time_t m_lastUpdateTime = 0;
    
    const TakeOverIndicators& getIndicators(
        const StrategyContext& context) const {
        
        time_t currentTime = context.getCurrentTime();
        if (!m_cachedIndicators || currentTime != m_lastUpdateTime) {
            TakeOverIndicators indicators;
            loadTakeOverIndicators(context, indicators);
            m_cachedIndicators = indicators;
            m_lastUpdateTime = currentTime;
        }
        
        return *m_cachedIndicators;
    }
};
```

---

## Next Steps

1. **Review existing class-based implementation** (TakeOverStrategy.cpp is already done!)
2. **Document StrategyContext API** if not already complete
3. **Document OrderManager API** if not already complete
4. **Create unit test framework** for strategy classes
5. **Migrate remaining strategies** one by one using this pattern
6. **Update build system** to compile all strategy classes
7. **Create strategy registry** to instantiate strategies at runtime

---

## Summary

The modern C++ class-based approach is the **canonical migration path** for all strategies. It provides:
- Clear abstraction through classes
- Better maintainability and testability
- Full access to C++ features
- Type safety and compile-time checks
- Idiomatic C++ patterns

The C-style function approach is **deprecated** and should not be used for new migrations.
