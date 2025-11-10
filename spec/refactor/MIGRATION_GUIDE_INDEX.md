# Strategy Migration Master Guide

## Overview

This directory contains comprehensive documentation for migrating C trading strategies to the modern C++ architecture.

**Migration Approach: Modern C++ Class-Based Architecture**

All strategies should be migrated to use **modern C++ classes** with proper encapsulation, type safety, and idiomatic patterns. The C-style function approach is deprecated.

See [CPP_CLASS_BASED_MIGRATION.md](CPP_CLASS_BASED_MIGRATION.md) for the canonical migration guide.

---

## Document Index

### Core Documentation

1. **[CPP_CLASS_BASED_MIGRATION.md](CPP_CLASS_BASED_MIGRATION.md)** - **CANONICAL MIGRATION GUIDE** ⭐
   - Modern C++ class-based architecture (RECOMMENDED)
   - StrategyContext, OrderManager, BaseStrategy abstractions
   - Complete TakeOverStrategy example with class implementation
   - Benefits: encapsulation, testability, type safety, maintainability
   - Migration process: define class, implement methods, use modern C++ features
   - Common patterns: dynamic SL, multi-condition exit, indicator caching
   - **Use this guide for all new migrations**

2. **[API_DISCOVERY.md](API_DISCOVERY.md)** - Complete API reference
   - C function catalogue (60+ functions from OrderManagement.h and EasyTradeCWrapper.hpp)
   - StrategyParams structure documentation (64 settings indices)
   - Data structure mappings (rates_t, Base_Indicators, StrategyIndicators)
   - Two-phase UI emission pattern
   - Telemetry harvesting
   - Late overwrite block mechanics
   - Common usage patterns with examples
   - Migration notes (return codes, shift conventions, de-duplication, validation)

### Legacy C-Style Templates (DEPRECATED)

**Note**: The following templates use the C-style function approach and are **deprecated**. They are kept for reference only. **Use [CPP_CLASS_BASED_MIGRATION.md](CPP_CLASS_BASED_MIGRATION.md) instead.**

2. **[MIGRATION_TEMPLATE_SIMPLE.md](MIGRATION_TEMPLATE_SIMPLE.md)** - Level 1 (DEPRECATED)
   - **Target**: Bar iteration, indicator calculations, UI values only
   - **Example Archetype**: RecordBars (no trading, pure data recording)
   - **Time Estimate**: ~2 hours
   - **Features**:
     * 10-phase migration checklist
     * Bar data access patterns (direct vs EasyTrade)
     * Indicator calculation examples (all TA-lib wrappers)
     * UI emission guidance (automatic harvesting)
     * Complete RecordBars example (~80 lines)
     * 5 common pitfalls with solutions
     * Performance tips
   - **Best For**: Data collectors, signal generators, indicators, metrics calculators
   - **Status**: DEPRECATED - Use class-based approach instead

3. **[MIGRATION_TEMPLATE_ORDER_MGMT.md](MIGRATION_TEMPLATE_ORDER_MGMT.md)** - Level 2 (DEPRECATED)
   - **Target**: Entry, modification, exit flows with dynamic stops/targets
   - **Example Archetype**: TakeOver (momentum strategy with trailing stops)
   - **Time Estimate**: ~4 hours
   - **Features**:
     * 10-phase migration checklist
     * Entry patterns (4 types: open/update, single orders, low-level API, de-duplication)
     * Modification patterns (6 types: dynamic SL, trailing, breakeven, elliptical, modify-only, advanced)
     * Exit patterns (6 types: close all, specific, conditional, internal checks, partial, pending orders)
     * Risk sizing with calculateOrderSize
     * Complete TakeOver example (~200 lines)
     * 6 indicator calculation modes for dynamic stops
     * 5 common pitfalls with solutions
     * Time estimates with complexity factors
   - **Best For**: Momentum strategies, breakout systems, mean reversion, trailing stop strategies
   - **Status**: DEPRECATED - Use class-based approach instead

4. **[MIGRATION_TEMPLATE_COMPLEX.md](MIGRATION_TEMPLATE_COMPLEX.md)** - Level 3 (DEPRECATED)
   - **Target**: Multi-timeframe, consensus logic, ATR gating, split trades, dynamic risk
   - **Example Archetype**: TrendStrategy (multi-timeframe with 12+ split trade modes)
   - **Time Estimate**: ~14-16 hours
   - **Features**:
     * 12-phase migration checklist
     * Multi-timeframe data validation (daily, 4H, weekly)
     * Base indicators calculation (ATR, pivots, trend phase, MACD)
     * Multi-timeframe consensus logic (4 components: daily trend + 4H MA + MACD + shellington)
     * ATR-based volatility gating (weeklyPredictATR * 0.4 thresholds)
     * Entry de-duplication (dailyATR/4 price tolerance, time-based blocking)
     * Dynamic risk modulation (5 modes: phase-based, ATR-based, weekday, symbol-specific, performance-based)
     * Split trade mode dispatcher (10+ patterns)
     * Complete split mode implementations:
       - KeyK: 3-part split (1x/2x/runner)
       - Shellington: Risk-capped with min lot size
       - ShortTerm: Gap-based (1/3 daily range)
       - WeeklyBeginning: Pivot-filtered, no TP
       - ATR4H: ATR-based ladder
     * Complete TrendStrategy example (~400 lines)
     * 5 common pitfalls with solutions
     * Testing strategy (unit/integration/paper trading)
     * Performance considerations
     * Time estimates with complexity factors
   - **Best For**: Multi-timeframe systems, trend following, volatility-adaptive strategies, portfolio strategies
   - **Status**: DEPRECATED - Use class-based approach instead

---

## Quick Start Guide (Modern C++ Approach)

### Step 1: Read the Canonical Guide

**Start here**: [CPP_CLASS_BASED_MIGRATION.md](CPP_CLASS_BASED_MIGRATION.md)

This guide covers:
- Architecture overview (StrategyContext, OrderManager, BaseStrategy)
- Complete class-based migration process
- Real TakeOverStrategy example
- Benefits of class-based approach
- Common patterns and best practices

### Step 2: Understand Your Strategy

Assess what your strategy does:
- **Indicator-only**: Calculate and display values (no trading)
- **Order management**: Place, modify, and close orders
- **Complex multi-timeframe**: Consensus logic, ATR gating, split trades

All types use the **same class-based architecture** with varying complexity.

### Step 3: Implement Your Strategy Class

```cpp
class MyStrategy : public BaseStrategy {
public:
    MyStrategy();
    
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
    // Helper methods
    void loadMyIndicators(const StrategyContext& context);
    bool shouldEnter(const StrategyContext& context);
    bool shouldExit(const StrategyContext& context);
};
```

### Step 4: Test and Deploy

- Write unit tests for individual methods
- Run backtest to validate behavior
- Deploy to paper trading
- Monitor and iterate

---

## Architecture Decision

**Why Class-Based > C-Style Functions?**

| Aspect | C-Style | Class-Based |
|--------|---------|-------------|
| Encapsulation | ❌ Global | ✅ Private methods |
| Testability | ⚠️ Mock C API | ✅ Mock classes |
| Type Safety | ⚠️ Loose | ✅ Strong |
| Maintainability | ⚠️ Scattered | ✅ Self-contained |
| Reusability | ❌ Copy-paste | ✅ Inheritance |
| Modern C++ | ❌ No | ✅ Yes |

**Decision**: Modern C++ class-based is canonical. C-style templates are deprecated.

---

## Legacy Section: C-Style Quick Start (DEPRECATED)

**⚠️ Do not use for new migrations. Use class-based approach above.**

### Step 1: Assess Your Strategy Complexity (DEPRECATED)

**Simple Strategy Indicators:**
- Primary focus: recording data or calculating indicators
- No trade management logic
- Simple bar iteration
- UI value emission only
- Examples: RecordBars, IndicatorCalculator, DataLogger

**Order Management Strategy Indicators:**
- Entry/exit logic with dynamic stops
- Trailing stop management
- Single timeframe or simple multi-timeframe
- Risk sizing
- Moderate indicator complexity
- Examples: TakeOver, BreakoutStrategy, MomentumTrader

**Complex Strategy Indicators:**
- Multi-timeframe consensus required
- ATR-based volatility filtering
- Multiple split trade modes
- Dynamic risk modulation
- Entry de-duplication
- Phase-based logic
- 10+ configuration parameters
- Examples: TrendStrategy, SwingStrategy, PortfolioManager

### Step 2: Select Appropriate Template

| Complexity | Template | Time | When to Use |
|-----------|----------|------|-------------|
| **Low** | MIGRATION_TEMPLATE_SIMPLE.md | 2 hrs | No trading, data only, simple logic |
| **Medium** | MIGRATION_TEMPLATE_ORDER_MGMT.md | 4 hrs | Trade management, dynamic stops, moderate indicators |
| **High** | MIGRATION_TEMPLATE_COMPLEX.md | 14-16 hrs | Multi-timeframe, ATR gating, split modes, consensus |

### Step 3: Read API_DISCOVERY.md Relevant Sections

Before starting migration:
1. Review **C Function API Catalogue** (section mapping to your needs)
2. Study **Common Usage Patterns** (4 examples)
3. Read **Migration Notes** (return codes, shift conventions, validation)
4. Understand **Two-Phase UI Emission** pattern
5. Review **StrategyParams Structure** documentation

### Step 4: Follow Template Checklist

Each template provides:
- Phase-by-phase checklist (10-12 phases)
- Code examples for each phase
- Complete working example (~80-400 lines)
- Common pitfalls with solutions
- Testing guidance
- Performance tips

### Step 5: Test Thoroughly

**All strategies must pass:**
- [ ] Compilation without errors/warnings
- [ ] Unit tests (if applicable)
- [ ] Backtesting validation (compare results to original C strategy ± 5%)
- [ ] Order analysis (count, sizing, timing)
- [ ] Paper trading (minimum 2 weeks)
- [ ] Log review (no errors, warnings within acceptable levels)

---

## Common Migration Patterns

### Pattern 1: Multi-Timeframe ATR Gating
```cpp
// Calculate predictive ATR
double weeklyATR = iAtrWholeDaysSimple(PRIMARY_RATES, 70);
double atrRange = weeklyATR * 0.4;

// Gate entry
if (atrRange < 30.0 || atrRange > 150.0) {
    return SUCCESS;  // Skip trade
}
```

**Used in**: Complex strategies with volatility filtering  
**See**: MIGRATION_TEMPLATE_COMPLEX.md Phase 6

### Pattern 2: Risk-Based Position Sizing
```cpp
// Calculate position size
double stopDistance = fabs(entryPrice - stopLossPrice);
double lots = calculateOrderSize(pParams, BUY, entryPrice, stopDistance);
lots *= riskMultiplier;  // Apply risk modulation

// Cap to maximum risk
double lotsMax = calculateOrderSizeWithSpecificRisk(pParams, BUY,
                     entryPrice, stopDistance, maxRiskPercent);
lots = fmin(lots, lotsMax);
```

**Used in**: All order management strategies  
**See**: MIGRATION_TEMPLATE_ORDER_MGMT.md Phase 7

### Pattern 3: Internal TP/SL Checks
```cpp
// Check internal stops (executed per tick)
checkInternalSL(pParams);
checkInternalTP(pParams);

// These execute before broker stop loss
// Useful for tighter risk management
```

**Used in**: Strategies requiring precise exit control  
**See**: MIGRATION_TEMPLATE_ORDER_MGMT.md Phase 6

### Pattern 4: Multi-Timeframe Consensus
```cpp
// Component 1: Daily trend phase
int dailyTrend = base.dailyTrendPhase;  // 1=bull, -1=bear, 0=neutral

// Component 2: 4H MA trend
int ma4HTrend = (iClose(FOURHOURLY_RATES, 0) > ma200_4H) ? 1 : -1;

// Component 3: MACD momentum
int macdTrend = (macdHist > 0) ? 1 : -1;

// Consensus: all must agree
int executionTrend = 0;
if (dailyTrend == 1 && ma4HTrend == 1 && macdTrend == 1) {
    executionTrend = 1;  // Strong bullish consensus
}
```

**Used in**: Complex multi-timeframe strategies  
**See**: MIGRATION_TEMPLATE_COMPLEX.md Phase 5

---

## API Quick Reference

### Bar Data Access
```cpp
// Current bar
double currentClose = cClose(PRIMARY_RATES);
double currentHigh = high(PRIMARY_RATES);

// Historical bar (shift=1 is last completed bar)
double yesterdayClose = iClose(DAILY_RATES, 1);
double lastHigh = iHigh(PRIMARY_RATES, 1);

// Bar count
int barsTotal = barsCount(PRIMARY_RATES);
```

### Indicator Calculation
```cpp
// ATR (14-period, shift=1)
double atr = iAtr(DAILY_RATES, 14, 1);

// Predictive ATR (whole days approximation)
double predictATR = iAtrWholeDaysSimple(PRIMARY_RATES, 14);

// Moving Average (close price, 50-period, shift=1)
double ma = iMA(3, DAILY_RATES, 50, 1);

// MACD (all components)
double macdMain, macdSignal, macdHist;
iMACDAll(PRIMARY_RATES, 12, 26, 9, 1, &macdMain, &macdSignal, &macdHist);
```

### Order Placement
```cpp
// Simple long order (TP in pips, SL in pips, lots OR risk percent)
openSingleLongEasy(takeProfitPips, stopLossPips, lots, riskPercent);

// Open or update existing long
openOrUpdateLongEasy(takeProfitPips, stopLossPips, lots, riskPercent);

// De-duplication check
double priceLimit = dailyATR / 4.0;
double duplicateCount = isSameDaySamePricePendingOrderEasy(
    entryPrice, priceLimit, currentTime
);
```

### Position Sizing
```cpp
// Calculate lots for risk percent
double lots = calculateOrderSize(pParams, orderType, entryPrice, stopDistance);
lots *= riskPercent;

// Calculate lots for specific risk
double lotsSpecific = calculateOrderSizeWithSpecificRisk(
    pParams, orderType, entryPrice, stopDistance, maxRiskPercent
);
```

### Multi-Timeframe Validation
```cpp
// Always validate before using multi-timeframe data
AsirikuyReturnCode result = validateDailyBarsEasy(pParams, PRIMARY_RATES, DAILY_RATES);
if (result != SUCCESS) return result;

result = validateHourlyBarsEasy(pParams, PRIMARY_RATES, HOURLY_RATES);
if (result != SUCCESS) return result;
```

---

## Troubleshooting Guide

### Issue: "Bars not aligned" Error

**Symptoms**: validateDailyBarsEasy() returns FAILURE  
**Cause**: Primary and daily timeframes out of sync  
**Solution**: Check broker time configuration, ensure daily bars start at midnight broker time

### Issue: Duplicate Orders Being Placed

**Symptoms**: Multiple orders at same price/time  
**Cause**: Missing de-duplication check  
**Solution**: Add isDuplicateEntry() check before order placement (see MIGRATION_TEMPLATE_COMPLEX.md Phase 7)

### Issue: Position Size Too Large

**Symptoms**: Orders exceed margin or risk limits  
**Cause**: Missing risk cap logic  
**Solution**: Use calculateOrderSizeWithSpecificRisk() to cap maximum lots (see Pattern 2 above)

### Issue: ATR Values Incorrect

**Symptoms**: ATR too large, includes forming bar  
**Cause**: Using shift=0 on incomplete bar  
**Solution**: Use shift=1 for completed bars, or iAtrWholeDaysSimple() for predictive values

### Issue: Strategy Not Entering Trades

**Symptoms**: Consensus logic always returns 0  
**Cause**: Overly strict consensus requirements  
**Solution**: Log each consensus component separately, relax one or more conditions (see MIGRATION_TEMPLATE_COMPLEX.md Phase 5)

### Issue: Trailing Stop Not Working

**Symptoms**: Stop loss not updating as price moves  
**Cause**: Missing trailOpenTrades() call or incorrect parameters  
**Solution**: Call trailOpenTrades() per tick with correct trail distance (see MIGRATION_TEMPLATE_ORDER_MGMT.md Phase 5)

---

## Best Practices

### 1. Start Simple
- Migrate simplest strategy first to validate toolchain
- Use MIGRATION_TEMPLATE_SIMPLE.md for proof of concept
- Progressively tackle more complex strategies

### 2. Validate Early, Validate Often
- Multi-timeframe validation is **mandatory**
- Check bar synchronization before every indicator calculation
- Log validation failures with context

### 3. Log Liberally During Migration
- Use NumericLogFormatter for numeric values (thread-safe)
- Log consensus components separately for debugging
- DEBUG level for per-tick details, INFO for entries/exits only
- Review logs after backtesting to catch issues

### 4. Test Incrementally
- Compile after each phase
- Unit test individual components (consensus, gating, de-dup)
- Backtest after completing full migration
- Compare results to original C strategy (± 5% tolerance)

### 5. Document Strategy-Specific Logic
- Capture parameter meanings in code comments
- Document consensus logic decisions
- Note any deviations from template patterns
- Create strategy-specific README

### 6. Performance Matters
- Cache indicator calculations (Base_Indicators pattern)
- Exit early when consensus fails (before expensive calculations)
- Use indexed de-duplication checks (not full order iteration)
- Consider split mode complexity impact

---

## Support & Contribution

### Getting Help

1. **Check template first**: 95% of issues covered in "Common Pitfalls" sections
2. **Review API_DISCOVERY.md**: Function signatures, usage patterns, migration notes
3. **Compare to examples**: RecordBars (simple), TakeOver (order mgmt), TrendStrategy (complex)
4. **Log analysis**: Enable DEBUG logging to see actual data flow

### Contributing

Template improvements welcome:
- Additional split trade mode examples
- New consensus pattern variants
- Performance optimization tips
- Testing automation scripts
- Migration tooling/scripts

---

## Version History

**v1.0** (2025-11-10) - Initial complete template set
- Simple strategy template (RecordBars)
- Order management template (TakeOver)
- Complex strategy template (TrendStrategy)
- Complete API catalogue (60+ functions)
- StrategyParams documentation (64 settings)
- NumericLoggingHelpers implementation
- Two-phase UI emission pattern
- Telemetry harvesting mechanics
- Late overwrite block validation

---

## Template Comparison Matrix

| Feature | Simple | Order Mgmt | Complex |
|---------|--------|------------|---------|
| **Bar Iteration** | ✅ Primary | ✅ Primary | ✅ Multi-TF |
| **Indicator Calc** | ✅ Basic | ✅ Moderate | ✅ Advanced |
| **UI Emission** | ✅ Auto | ✅ Auto | ✅ Auto |
| **Order Placement** | ❌ None | ✅ Yes | ✅ Yes |
| **Dynamic Stops** | ❌ N/A | ✅ 6 types | ✅ 6+ types |
| **Risk Sizing** | ❌ N/A | ✅ Basic | ✅ Dynamic |
| **De-duplication** | ❌ N/A | ⚠️ Optional | ✅ Required |
| **Multi-TF Consensus** | ❌ No | ⚠️ Optional | ✅ Yes |
| **ATR Gating** | ❌ No | ⚠️ Optional | ✅ Yes |
| **Split Trade Modes** | ❌ No | ❌ No | ✅ 10+ modes |
| **Risk Modulation** | ❌ No | ⚠️ Optional | ✅ 5 modes |
| **Time Estimate** | ~2 hrs | ~4 hrs | ~14-16 hrs |
| **Complexity** | Low | Medium | High |
| **Lines of Code** | ~80 | ~200 | ~400 |

---

## Next Steps

1. **Read API_DISCOVERY.md** - Understand available functions and patterns
2. **Assess your strategy** - Determine complexity level (Simple/Order/Complex)
3. **Choose template** - Select matching template
4. **Follow checklist** - Complete phases sequentially
5. **Test thoroughly** - Unit → Backtest → Paper → Live (gradual)
6. **Document** - Capture strategy-specific decisions and deviations

**All documentation is production-ready. Begin migration now.**
