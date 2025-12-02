# AutoBBS Dispatcher Routing Specification

## Overview

AutoBBS (Strategy IDs: 29, 30) is a **dispatcher strategy** that routes execution to different `workoutExecutionTrend_*` functions based on the `AUTOBBS_TREND_MODE` parameter (also referred to as `strategy_mode`).

This document maps all routing logic to facilitate C++ migration where each workoutExecutionTrend_* function will become its own C++ strategy class.

**Source File**: `dev/TradingStrategies/src/strategies/AutoBBS.c`  
**Routing Parameter**: `pParams->settings[AUTOBBS_TREND_MODE]`  
**Total Routes**: ~25 active functions (some removed during cleanup)

---

## Routing Logic

### Main Dispatcher Switch Statement

Located in `runAutoBBS()` around line 203-295:

```c
switch ((int)parameter(AUTOBBS_TREND_MODE)) {
    case MODE_VALUE:
        workoutExecutionTrend_FunctionName(pParams, pIndicators, pBase_Indicators);
        break;
    // ... etc
}
```

---

## Complete Routing Table

| Mode Value | Function Name | Status | Strategy Type | Notes |
|------------|--------------|--------|---------------|-------|
| 0 | `workoutExecutionTrend_4HBBS_Swing` | ✅ Active | 4H BBS Swing | Bollinger Band Squeeze on 4H |
| 2 | `workoutExecutionTrend_Limit` | ✅ Active | Limit Orders | Basic limit order strategy |
| 3 | `workoutExecutionTrend_Limit_BreakOutOnPivot` | ✅ Active | Limit + Pivot | Breakout on pivot points |
| 9 | `workoutExecutionTrend_Limit_BBS` | ✅ Active | Limit + BBS | BBS with limit orders |
| 10 | `workoutExecutionTrend_WeeklyAuto` | ✅ Active | Weekly Auto | Weekly timeframe auto trading |
| 13 | ~~`workoutExecutionTrend_Weekly_Swing_New`~~ | ❌ Removed | - | Function removed during cleanup |
| 14 | `workoutExecutionTrend_Auto_Hedge` | ✅ Active | Auto Hedge | Hedging strategy |
| 15 | `workoutExecutionTrend_XAUUSD_DayTrading` | ✅ Active | Gold Day Trading | Specific to XAUUSD (Gold) |
| 16 | ~~`workoutExecutionTrend_GBPJPY_DayTrading_Ver2`~~ | ❌ Removed | - | Function removed |
| 17 | ~~`workoutExecutionTrend_WeeklyATR_Prediction`~~ | ❌ Removed | - | Function removed |
| 18 | `workoutExecutionTrend_4HBBS_Swing_BoDuan` | ✅ Active | 4H BBS Swing | BoDuan variant |
| 19 | ~~`workoutExecutionTrend_DayTrading_ExecutionOnly`~~ | ❌ Removed | - | Function removed |
| 20 | `workoutExecutionTrend_4HBBS_Swing_XAUUSD_BoDuan` | ✅ Active | 4H BBS XAUUSD | Gold-specific BBS |
| 21 | ~~`workoutExecutionTrend_MultipleDay`~~ | ❌ Removed | - | Function removed |
| 22 | ~~`workoutExecutionTrend_MultipleDay_V2`~~ | ❌ Removed | - | Function removed |
| 23 | `workoutExecutionTrend_MACD_Daily` | ✅ Active | MACD Daily | MACD on daily timeframe |
| 24 | `workoutExecutionTrend_MACD_Weekly` | ✅ Active | MACD Weekly | MACD on weekly timeframe |
| 25 | `workoutExecutionTrend_Auto` | ✅ Active | Auto Trading | General auto trading (tradeMode=1) |
| 26 | `workoutExecutionTrend_Limit_BBS_LongTerm` | ✅ Active | Limit BBS Long | Long-term BBS with limits |
| 27 | `workoutExecutionTrend_Ichimoko_Daily_Index` | ✅ Active | Ichimoku Daily Index | Ichimoku cloud strategy |
| 28 | `workoutExecutionTrend_Ichimoko_Daily_Index` | ✅ Active | Ichimoku Daily Index | (Duplicate of mode 27) |
| 30 | `workoutExecutionTrend_4H_Shellington` | ✅ Active | 4H Shellington | Shellington method on 4H |
| 31 | `workoutExecutionTrend_Ichimoko_Daily_New` | ✅ Active | Ichimoku Daily New | Updated Ichimoku variant |
| 32 | `workoutExecutionTrend_MACD_Daily_New` | ✅ Active | MACD Daily New | Updated MACD daily |
| 33 | `workoutExecutionTrend_MACD_BEILI` | ✅ Active | MACD BEILI | MACD with BEILI divergence |
| 34 | `workoutExecutionTrend_ShortTerm` | ✅ Active | Short Term | Short-term trading |
| 35 | `workoutExecutionTrend_Ichimoko_Weekly_Index` | ✅ Active | Ichimoku Weekly | Ichimoku on weekly |
| 36 | - | 🔴 Empty | - | No function assigned |
| 101 | `workoutExecutionTrend_Ichimoko_Daily_Index_Regression_Test` | ✅ Active | Ichimoku Test | Regression testing version |
| 102 | `workoutExecutionTrend_ASI` | ✅ Active | ASI | Accumulation Swing Index |

---

## Active Functions Summary

**Total Active Functions**: 23 (after cleanup removed 6 functions)

### By Category

#### MACD Strategies (4)
1. `workoutExecutionTrend_MACD_Daily` (mode 23)
2. `workoutExecutionTrend_MACD_Weekly` (mode 24)
3. `workoutExecutionTrend_MACD_Daily_New` (mode 32)
4. `workoutExecutionTrend_MACD_BEILI` (mode 33)

#### BBS (Bollinger Band Squeeze) Strategies (5)
1. `workoutExecutionTrend_4HBBS_Swing` (mode 0)
2. `workoutExecutionTrend_Limit_BBS` (mode 9)
3. `workoutExecutionTrend_4HBBS_Swing_BoDuan` (mode 18)
4. `workoutExecutionTrend_4HBBS_Swing_XAUUSD_BoDuan` (mode 20)
5. `workoutExecutionTrend_Limit_BBS_LongTerm` (mode 26)

#### Ichimoku Strategies (5)
1. `workoutExecutionTrend_Ichimoko_Daily_Index` (mode 27, 28)
2. `workoutExecutionTrend_Ichimoko_Daily_New` (mode 31)
3. `workoutExecutionTrend_Ichimoko_Weekly_Index` (mode 35)
4. `workoutExecutionTrend_Ichimoko_Daily_Index_Regression_Test` (mode 101)

#### Limit Order Strategies (3)
1. `workoutExecutionTrend_Limit` (mode 2)
2. `workoutExecutionTrend_Limit_BreakOutOnPivot` (mode 3)
3. `workoutExecutionTrend_Limit_BBS` (mode 9) - also in BBS category

#### Other Strategies (6)
1. `workoutExecutionTrend_WeeklyAuto` (mode 10)
2. `workoutExecutionTrend_Auto_Hedge` (mode 14)
3. `workoutExecutionTrend_XAUUSD_DayTrading` (mode 15)
4. `workoutExecutionTrend_Auto` (mode 25)
5. `workoutExecutionTrend_4H_Shellington` (mode 30)
6. `workoutExecutionTrend_ShortTerm` (mode 34)
7. `workoutExecutionTrend_ASI` (mode 102)

---

## Order Splitting Routing

AutoBBS also dispatches order splitting based on `pIndicators->splitTradeMode`:

### splitBuyOrders() Dispatcher (lines 14-100)

| Split Mode | Function | Status |
|------------|----------|--------|
| 0 | `splitBuyOrders_ShortTerm` | ✅ Active |
| 3 | `splitBuyOrders_LongTerm` | ✅ Active |
| 4 | `splitBuyOrders_Limit` | ✅ Active |
| 8 | `splitBuyOrders_WeeklyTrading` | ✅ Active |
| 9 | `splitBuyOrders_KeyK` | ✅ Active |
| 10 | `splitBuyOrders_Weekly_Beginning` | ✅ Active |
| 11 | `splitBuyOrders_Weekly_ShortTerm` | ✅ Active |
| 12 | `splitBuyOrders_ATR` | ✅ Active |
| 14 | `splitBuyOrders_ShortTerm_Hedge` | ✅ Active |
| 15 | `splitBuyOrders_ShortTerm_ATR_Hedge` | ✅ Active |
| 16 | `splitBuyOrders_Daily_Swing` | ✅ Active |
| 19 | `splitBuyOrders_4HSwing` | ✅ Active |
| 20 | `splitBuyOrders_4HSwing_100P` | ✅ Active |
| 24 | `splitBuyOrders_MACDDaily` | ✅ Active |
| 25 | `splitBuyOrders_MACDWeekly` | ✅ Active |
| 26 | `splitBuyOrders_Ichimoko_Daily` | ✅ Active |
| 27 | `splitBuyOrders_4HSwing_Shellington` | ✅ Active |
| 31 | `splitBuyOrders_ShortTerm_New` | ✅ Active |
| 32 | `splitBuyRangeOrders` | ✅ Active |
| 33 | `splitBuyOrders_Ichimoko_Weekly` | ✅ Active |

### splitSellOrders() Dispatcher (lines 102-180)

Similar routing for sell orders (same modes, different functions).

**Total Order Split Functions**: ~20 buy + ~20 sell = **~40 split functions**

---

## C++ Migration Strategy

### Approach 1: Keep Dispatcher Pattern

AutoBBS becomes a dispatcher class that creates sub-strategies:

```cpp
class AutoBBSStrategy : public BaseStrategy {
public:
    AsirikuyReturnCode execute(const StrategyContext& ctx) override {
        int trendMode = (int)ctx.getSetting(AUTOBBS_TREND_MODE);
        
        // Use factory to create sub-strategy
        auto subStrategy = TrendStrategyFactory::create(trendMode);
        if (!subStrategy) {
            return INVALID_PARAMETER;
        }
        
        return subStrategy->execute(ctx);
    }
};

// Separate factory for trend sub-strategies
class TrendStrategyFactory {
public:
    static std::unique_ptr<ITrendStrategy> create(int mode) {
        switch(mode) {
            case 0: return std::make_unique<FourHourBBSSwingStrategy>();
            case 2: return std::make_unique<LimitStrategy>();
            case 23: return std::make_unique<MACDDailyStrategy>();
            // ... etc
        }
    }
};
```

### Approach 2: Direct Mapping

Remove AutoBBS dispatcher entirely, map MQL settings directly to strategy IDs:

- Mode 23 → MACD_DAILY strategy (new ID)
- Mode 24 → MACD_WEEKLY strategy (new ID)
- etc.

**Recommendation**: Approach 1 (keep dispatcher) for backward compatibility.

---

## Implementation Classes (Phase 2)

Each active function becomes a C++ class:

### MACD Classes
- `MACDDailyStrategy` ← `workoutExecutionTrend_MACD_Daily`
- `MACDWeeklyStrategy` ← `workoutExecutionTrend_MACD_Weekly`
- `MACDDailyNewStrategy` ← `workoutExecutionTrend_MACD_Daily_New`
- `MACDBEILIStrategy` ← `workoutExecutionTrend_MACD_BEILI`

### BBS Classes
- `FourHourBBSSwingStrategy` ← `workoutExecutionTrend_4HBBS_Swing`
- `LimitBBSStrategy` ← `workoutExecutionTrend_Limit_BBS`
- `FourHourBBSBoDuanStrategy` ← `workoutExecutionTrend_4HBBS_Swing_BoDuan`
- `FourHourBBSXAUUSDBoDuanStrategy` ← `workoutExecutionTrend_4HBBS_Swing_XAUUSD_BoDuan`
- `LimitBBSLongTermStrategy` ← `workoutExecutionTrend_Limit_BBS_LongTerm`

### Ichimoku Classes
- `IchimokuDailyIndexStrategy` ← `workoutExecutionTrend_Ichimoko_Daily_Index`
- `IchimokuDailyNewStrategy` ← `workoutExecutionTrend_Ichimoko_Daily_New`
- `IchimokuWeeklyIndexStrategy` ← `workoutExecutionTrend_Ichimoko_Weekly_Index`
- `IchimokuDailyIndexRegressionStrategy` ← `workoutExecutionTrend_Ichimoko_Daily_Index_Regression_Test`

### Limit Classes
- `LimitStrategy` ← `workoutExecutionTrend_Limit`
- `LimitBreakOutOnPivotStrategy` ← `workoutExecutionTrend_Limit_BreakOutOnPivot`

### Other Classes
- `WeeklyAutoStrategy` ← `workoutExecutionTrend_WeeklyAuto`
- `AutoHedgeStrategy` ← `workoutExecutionTrend_Auto_Hedge`
- `XAUUSDDayTradingStrategy` ← `workoutExecutionTrend_XAUUSD_DayTrading`
- `AutoStrategy` ← `workoutExecutionTrend_Auto`
- `FourHourShellingtonStrategy` ← `workoutExecutionTrend_4H_Shellington`
- `ShortTermStrategy` ← `workoutExecutionTrend_ShortTerm`
- `ASIStrategy` ← `workoutExecutionTrend_ASI`

**Total Classes**: 23 strategy classes

---

## Order Splitting Migration

Order splitting functions should be migrated to **OrderBuilder** pattern:

```cpp
class OrderBuilder {
    OrderBuilder& setSplitMode(int mode);
    OrderBuilder& splitForMACD();
    OrderBuilder& splitForBBS();
    OrderBuilder& splitForIchimoku();
    // ... etc
    std::vector<Order> build();
};
```

Each strategy uses OrderBuilder instead of calling split functions directly.

---

## Testing Strategy

### Side-by-Side Testing

For each mode value:
1. Run C version: `runAutoBBS()` with `AUTOBBS_TREND_MODE = N`
2. Run C++ version: `AutoBBSStrategy::execute()` with same mode
3. Compare outputs (orders, results, signals)
4. Validate identical behavior

### Test Coverage

- [ ] Test all 23 active modes
- [ ] Test removed modes (should return error/no-op)
- [ ] Test invalid modes
- [ ] Test order splitting for each mode
- [ ] Test with different market conditions

---

## Migration Checklist

### Phase 2 Tasks

- [ ] Create `AutoBBSStrategy` dispatcher class
- [ ] Create `TrendStrategyFactory` for sub-strategies
- [ ] Migrate each of 23 `workoutExecutionTrend_*` functions to C++ classes
- [ ] Migrate order splitting functions to OrderBuilder
- [ ] Register all sub-strategies in factory
- [ ] Test each mode value
- [ ] Validate backtesting results match C version
- [ ] Remove C implementations after validation

---

## Notes

1. **Removed Functions**: 6 functions removed during cleanup (modes 13, 16, 17, 19, 21, 22)
   - Migration should handle these gracefully (return error or log)

2. **Duplicate Mode 27/28**: Both route to `workoutExecutionTrend_Ichimoko_Daily_Index`
   - Consider consolidating in C++ version

3. **Mode 25 Special Handling**: Sets `pIndicators->tradeMode = 1` before calling function
   - This context-specific setup needs to be preserved

4. **Order Splitting Complexity**: ~40 split functions need careful migration
   - Consider creating split strategy pattern or builder pattern

5. **TrendStrategy.c Size**: Contains all 23 functions, ~9,286 lines
   - Breaking this down is a major Phase 2 task

---

**Document Version**: 1.0  
**Status**: Complete - Ready for Phase 2  
**Last Updated**: November 9, 2025  
**Owner**: Phase 2 Implementation Team
