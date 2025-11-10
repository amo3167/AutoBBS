# Complex Strategy Migration Template

**Target Audience**: Advanced strategies with multi-timeframe analysis, consensus logic, ATR-based volatility gating, entry de-duplication, split trade modes, and dynamic risk management.

**Examples**: TrendStrategy, SwingStrategy, multi-indicator systems with complex decision trees.

**Estimated Complexity**: High (8-16 hours per strategy)

---

## Overview

Complex strategies typically involve:
- **Multi-timeframe consensus** (Daily trend + 4H MA + MACD momentum + optional shellington flag)
- **ATR-based volatility gating** (weeklyPredictATR * 0.4 thresholds)
- **Entry de-duplication** (prevent same-day/same-price duplicate orders)
- **Split trade modes** (12+ order splitting patterns: breakout, retreat, KeyK, shellington, etc.)
- **Dynamic risk modulation** (phase-based, symbol-based, weekday-based adjustments)
- **Predictive ATR calculations** (weekly/daily max ATR for sizing and gating)

---

## Migration Checklist

### Phase 1: Setup & Multi-File Structure

- [ ] Create main strategy `.cpp` file in `dev/TradingStrategies/src/strategies/`
- [ ] Create header `.h` file in `dev/TradingStrategies/include/`
- [ ] Split helper functions into separate files if strategy is large (>2000 lines)
- [ ] Add to `premake4.lua`
- [ ] Include required headers:
  ```cpp
  #include "Precompiled.h"
  #include "StrategyTypes.h"
  #include "AsirikuyDefines.h"
  #include "OrderManagement.h"
  #include "EasyTradeCWrapper.hpp"
  #include "StrategyUserInterface.h"
  #include "NumericLoggingHelpers.hpp"
  #include "base.h"           // Base_Indicators structure
  #include "ComLib.h"         // Indicators structure  
  #include "Logging.h"
  ```

### Phase 2: Define Complex Data Structures

#### Base Indicators Structure
```cpp
// Base indicators (common across strategies)
struct Base_Indicators {
    // Daily metrics
    double dailyATR;
    double dailyHigh;
    double dailyLow;
    double dailyClose;
    double dailyOpen;
    
    // Weekly metrics  
    double weeklyATR;
    double weeklyMaxATR;
    double weeklyHigh;
    double weeklyLow;
    
    // Predictive ATR values (critical for gating)
    double pDailyATR;          // Predicted daily ATR
    double pDailyMaxATR;       // Predicted daily max ATR
    double pWeeklyPredictATR;  // Predicted weekly ATR
    double pWeeklyPredictMaxATR; // Predicted weekly max ATR
    
    // Pivot points
    double dailyPivot, dailyS1, dailyR1, dailyS2, dailyR2, dailyS3, dailyR3;
    double weeklyPivot, weeklyS1, weeklyR1, weeklyS2, weeklyR2, weeklyS3, weeklyR3;
    
    // Trend indicators
    int dailyTrendPhase;       // Daily trend phase (1=bull, -1=bear, 0=neutral)
    double ma200_4H;           // 4H 200-period MA
    double macdHist;           // MACD histogram
    double macdSignal;         // MACD signal line
};

// Strategy-specific indicators
struct StrategyIndicators {
    // Entry parameters
    double entryPrice;
    double stopLossPrice;
    double takePrice;
    int executionTrend;        // Final consensus trend (1=long, -1=short, 0=none)
    int entrySignal;           // Entry signal type
    int exitSignal;            // Exit signal type
    
    // Trade mode configuration
    int splitTradeMode;        // Split mode enum (see SplitTradeMode)
    int tpMode;                // TP mode enum (see TakeProfitMode)
    int tradeMode;             // Sub-mode for split logic
    int subTradeMode;          // Additional mode flags
    
    // Risk management
    double risk;               // Risk multiplier for this trade
    double riskCap;            // Risk cap multiplier
    double minLotSize;         // Minimum lot size
    double volumeStep;         // Lot size rounding step
    BOOL isEnableBuyMinLotSize;   // Force min lot size for buys
    BOOL isEnableSellMinLotSize;  // Force min lot size for sells
    
    // ATR envelope for gating
    double atr_euro_range;     // weeklyPredictATR * 0.4 (volatility gate)
    
    // Additional flags
    BOOL shellingtonFlag;      // Shellington pattern detected
    int bbsIndex_execution;    // BBS execution bar index
};
```

#### Enumerations
```cpp
// Already defined in StrategyTypes.h
// SplitTradeMode: STM_DEFAULT, STM_BREAKOUT, STM_LIMIT, STM_PIVOT, 
//                 STM_RETREAT, STM_KEYK, STM_SHELLINGTON, STM_SHORTTERM,
//                 STM_WEEKLY_BEGIN, STM_ATR_4H, STM_ATR_4H_ALT, STM_MACD_SHORT
// TakeProfitMode: TP_STATIC, TP_ATR_RANGE, TP_LADDER_BE

// Strategy-specific parameter mapping
enum TrendStrategyParams {
    DAILY_TREND_MA_PERIOD = ADDITIONAL_PARAM_1,
    MACD_FAST_PERIOD = ADDITIONAL_PARAM_2,
    MACD_SLOW_PERIOD = ADDITIONAL_PARAM_3,
    MACD_SIGNAL_PERIOD = ADDITIONAL_PARAM_4,
    MA_4H_PERIOD = ADDITIONAL_PARAM_5,
    SHELLINGTON_THRESHOLD = ADDITIONAL_PARAM_6,
    MIN_ATR_THRESHOLD = ADDITIONAL_PARAM_7,
    MAX_ATR_THRESHOLD = ADDITIONAL_PARAM_8,
    RISK_MODULATION_MODE = ADDITIONAL_PARAM_9,
    SPLIT_TRADE_MODE_PARAM = ADDITIONAL_PARAM_10,
    // ... up to ADDITIONAL_PARAM_40
};
```

### Phase 3: Multi-Timeframe Data Validation

**Critical**: Always validate multi-timeframe bar synchronization before proceeding.

```cpp
AsirikuyReturnCode validateMultiTimeframeData(StrategyParams* pParams) {
    using LogFmt = NumericLogFormatter;
    
    // Validate daily bars
    AsirikuyReturnCode result = validateDailyBarsEasy(pParams, PRIMARY_RATES, DAILY_RATES);
    if (result != SUCCESS) {
        pantheios_logprintf(PANTHEIOS_SEV_ERROR,
            (PAN_CHAR_T*)"Daily bars validation failed: %s",
            LogFmt::integer(result));
        return result;
    }
    
    // Validate hourly bars
    result = validateHourlyBarsEasy(pParams, PRIMARY_RATES, HOURLY_RATES);
    if (result != SUCCESS) {
        pantheios_logprintf(PANTHEIOS_SEV_ERROR,
            (PAN_CHAR_T*)"Hourly bars validation failed: %s",
            LogFmt::integer(result));
        return result;
    }
    
    // Validate 4H bars (if using secondary timeframe)
    result = validateSecondaryBarsEasy(pParams, PRIMARY_RATES, DAILY_RATES, 240, 5);
    if (result != SUCCESS) {
        pantheios_logprintf(PANTHEIOS_SEV_WARNING,
            (PAN_CHAR_T*)"4H bars validation warning: %s",
            LogFmt::integer(result));
        // Continue anyway, non-critical
    }
    
    // Validate current time
    result = validateCurrentTimeEasy(pParams, PRIMARY_RATES);
    if (result != SUCCESS) {
        return result;
    }
    
    return SUCCESS;
}
```

### Phase 4: Base Indicators Calculation

```cpp
Base_Indicators loadBaseIndicators(StrategyParams* pParams) {
    Base_Indicators base = {};
    
    // Daily OHLC
    base.dailyHigh = iHigh(DAILY_RATES, 1);
    base.dailyLow = iLow(DAILY_RATES, 1);
    base.dailyClose = iClose(DAILY_RATES, 1);
    base.dailyOpen = iOpen(DAILY_RATES, 1);
    
    // Daily ATR (standard and whole-days approximation)
    base.dailyATR = iAtr(DAILY_RATES, 14, 1);
    base.pDailyATR = iAtrWholeDaysSimple(PRIMARY_RATES, 14);  // Predictive
    base.pDailyMaxATR = iAtrWholeDaysSimple(PRIMARY_RATES, 20);  // Max for caps
    
    // Weekly ATR (14 periods * 5 days = 70 daily bars)
    base.weeklyATR = iAtr(DAILY_RATES, 70, 1);
    base.pWeeklyPredictATR = iAtrWholeDaysSimple(PRIMARY_RATES, 70);
    base.pWeeklyPredictMaxATR = iAtrWholeDaysSimple(PRIMARY_RATES, 100);
    
    // Pivot points (daily)
    iPivot(DAILY_RATES, 1, &base.dailyPivot,
           &base.dailyS1, &base.dailyR1,
           &base.dailyS2, &base.dailyR2,
           &base.dailyS3, &base.dailyR3);
    
    // Pivot points (weekly - use daily bars, find weekly boundaries)
    // Requires custom logic to identify weekly high/low/close
    // Simplified: use last 5 days for approximation
    double weeklyHigh = iHigh(DAILY_RATES, 1);
    double weeklyLow = iLow(DAILY_RATES, 1);
    for (int i = 2; i <= 5; i++) {
        weeklyHigh = fmax(weeklyHigh, iHigh(DAILY_RATES, i));
        weeklyLow = fmin(weeklyLow, iLow(DAILY_RATES, i));
    }
    double weeklyClose = iClose(DAILY_RATES, 1);
    
    base.weeklyPivot = (weeklyHigh + weeklyLow + weeklyClose) / 3.0;
    base.weeklyS1 = 2.0 * base.weeklyPivot - weeklyHigh;
    base.weeklyR1 = 2.0 * base.weeklyPivot - weeklyLow;
    base.weeklyS2 = base.weeklyPivot - (weeklyHigh - weeklyLow);
    base.weeklyR2 = base.weeklyPivot + (weeklyHigh - weeklyLow);
    
    // Daily trend phase (custom indicator - example using MA crossover)
    double dailyMA50 = iMA(3, DAILY_RATES, 50, 1);  // 50-period MA on close
    double dailyMA200 = iMA(3, DAILY_RATES, 200, 1);  // 200-period MA on close
    
    if (base.dailyClose > dailyMA50 && dailyMA50 > dailyMA200) {
        base.dailyTrendPhase = 1;  // Bullish
    } else if (base.dailyClose < dailyMA50 && dailyMA50 < dailyMA200) {
        base.dailyTrendPhase = -1;  // Bearish
    } else {
        base.dailyTrendPhase = 0;  // Neutral/transitioning
    }
    
    // 4H 200-period MA (for trend filter)
    base.ma200_4H = iMA(3, FOURHOURLY_RATES, 200, 1);
    
    // MACD on primary timeframe
    double macdMain;
    iMACDAll(PRIMARY_RATES, 12, 26, 9, 1,
             &macdMain, &base.macdSignal, &base.macdHist);
    
    return base;
}
```

### Phase 5: Multi-Timeframe Consensus Logic

```cpp
int calculateExecutionTrend(StrategyParams* pParams,
                            const Base_Indicators& base,
                            StrategyIndicators& indicators) {
    using LogFmt = NumericLogFormatter;
    
    // Component 1: Daily trend phase
    int dailyTrend = base.dailyTrendPhase;
    
    // Component 2: 4H MA trend
    double current4HClose = iClose(FOURHOURLY_RATES, 0);
    int ma4HTrend = (current4HClose > base.ma200_4H) ? 1 : -1;
    
    // Component 3: MACD momentum
    int macdTrend = (base.macdHist > 0) ? 1 : -1;
    
    // Component 4: Optional shellington pattern flag
    // (Defined by strategy-specific logic, e.g., specific candle patterns)
    BOOL shellington = indicators.shellingtonFlag;
    
    // Consensus: All components must agree (strict)
    int executionTrend = 0;
    
    if (dailyTrend == 1 && ma4HTrend == 1 && macdTrend == 1) {
        executionTrend = 1;  // Strong bullish consensus
        
        // Shellington can boost confidence or require additional confirmation
        if (shellington) {
            indicators.splitTradeMode = STM_SHELLINGTON;
        }
    }
    else if (dailyTrend == -1 && ma4HTrend == -1 && macdTrend == -1) {
        executionTrend = -1;  // Strong bearish consensus
        
        if (shellington) {
            indicators.splitTradeMode = STM_SHELLINGTON;
        }
    }
    
    // Log consensus breakdown
    pantheios_logprintf(PANTHEIOS_SEV_DEBUG,
        (PAN_CHAR_T*)"Consensus: daily=%s, 4H_MA=%s, MACD=%s, shellington=%s → execution=%s",
        LogFmt::integer(dailyTrend),
        LogFmt::integer(ma4HTrend),
        LogFmt::integer(macdTrend),
        LogFmt::boolean(shellington),
        LogFmt::integer(executionTrend));
    
    return executionTrend;
}
```

### Phase 6: ATR-Based Volatility Gating

```cpp
BOOL passesVolatilityGate(const Base_Indicators& base,
                          StrategyIndicators& indicators) {
    using LogFmt = NumericLogFormatter;
    
    // Calculate ATR euro range (weeklyPredictATR * 0.4)
    indicators.atr_euro_range = base.pWeeklyPredictATR * 0.4;
    
    // Get thresholds from settings
    double minThreshold = parameter(MIN_ATR_THRESHOLD);  // e.g., 30.0
    double maxThreshold = parameter(MAX_ATR_THRESHOLD);  // e.g., 150.0
    
    // Gate: ATR must be within acceptable range
    if (indicators.atr_euro_range < minThreshold) {
        pantheios_logprintf(PANTHEIOS_SEV_DEBUG,
            (PAN_CHAR_T*)"ATR gate FAIL: %s < min %s (low volatility)",
            LogFmt::decimal(indicators.atr_euro_range, 2),
            LogFmt::decimal(minThreshold, 2));
        return FALSE;
    }
    
    if (indicators.atr_euro_range > maxThreshold) {
        pantheios_logprintf(PANTHEIOS_SEV_DEBUG,
            (PAN_CHAR_T*)"ATR gate FAIL: %s > max %s (excessive volatility)",
            LogFmt::decimal(indicators.atr_euro_range, 2),
            LogFmt::decimal(maxThreshold, 2));
        return FALSE;
    }
    
    pantheios_logprintf(PANTHEIOS_SEV_DEBUG,
        (PAN_CHAR_T*)"ATR gate PASS: %s ∈ [%s, %s]",
        LogFmt::decimal(indicators.atr_euro_range, 2),
        LogFmt::decimal(minThreshold, 2),
        LogFmt::decimal(maxThreshold, 2));
    
    return TRUE;
}
```

### Phase 7: Entry De-duplication (Critical)

```cpp
BOOL isDuplicateEntry(StrategyParams* pParams,
                     const Base_Indicators& base,
                     double entryPrice,
                     OrderType orderType) {
    using LogFmt = NumericLogFormatter;
    
    // Use dailyATR / 4 as price tolerance
    double priceLimit = base.pDailyATR / 4.0;
    
    // Check for same-day same-price pending orders
    double duplicateCount = isSameDaySamePricePendingOrderEasy(
        entryPrice, priceLimit, pParams->currentBrokerTime
    );
    
    if (duplicateCount > 0) {
        pantheios_logprintf(PANTHEIOS_SEV_WARNING,
            (PAN_CHAR_T*)"De-dup BLOCK: %s pending order(s) at price %s ± %s today",
            LogFmt::integer((int)duplicateCount),
            LogFmt::decimal(entryPrice, 5),
            LogFmt::decimal(priceLimit, 5));
        return TRUE;
    }
    
    // Additional check: orders based on dailyMaxATR threshold
    // Some strategies use pDailyMaxATR / 3 for stricter filtering
    double strictLimit = base.pDailyMaxATR / 3.0;
    
    // Type-specific checks if needed
    if (orderType == BUY) {
        double buyLimitCount = isSamePriceBuyLimitOrderEasy(entryPrice,
                                   pParams->currentBrokerTime, strictLimit);
        if (buyLimitCount > 0) {
            return TRUE;
        }
    } else {
        double sellLimitCount = isSamePriceSellLimitOrderEasy(entryPrice,
                                    pParams->currentBrokerTime, strictLimit);
        if (sellLimitCount > 0) {
            return TRUE;
        }
    }
    
    pantheios_logprintf(PANTHEIOS_SEV_DEBUG,
        (PAN_CHAR_T*)"De-dup PASS: No duplicate orders at %s",
        LogFmt::decimal(entryPrice, 5));
    
    return FALSE;
}
```

### Phase 8: Risk Modulation Logic

```cpp
double calculateDynamicRisk(StrategyParams* pParams,
                           const Base_Indicators& base,
                           StrategyIndicators& indicators) {
    using LogFmt = NumericLogFormatter;
    
    double baseRisk = parameter(ACCOUNT_RISK_PERCENT) / 100.0;  // e.g., 0.005 (0.5%)
    double modulator = 1.0;
    
    int riskMode = (int)parameter(RISK_MODULATION_MODE);
    
    switch (riskMode) {
        case 0:  // No modulation
            break;
            
        case 1:  // Phase-based modulation
            if (base.dailyTrendPhase == 1 || base.dailyTrendPhase == -1) {
                modulator = 1.5;  // Increase risk in strong trends
            } else {
                modulator = 0.75;  // Reduce risk in transitioning phases
            }
            break;
            
        case 2:  // ATR-based modulation
            // Reduce risk in high volatility
            if (indicators.atr_euro_range > 100.0) {
                modulator = 0.5;
            } else if (indicators.atr_euro_range < 50.0) {
                modulator = 1.25;  // Increase risk in low volatility
            }
            break;
            
        case 3:  // Weekday-based modulation
            int dayOfWeek = dayOfWeek();
            if (dayOfWeek == 1 || dayOfWeek == 2) {
                modulator = 1.2;  // Mon-Tue: higher risk
            } else if (dayOfWeek == 5) {
                modulator = 0.6;  // Friday: reduce risk
            }
            break;
            
        case 4:  // Symbol-specific modulation
            // Example: Different risk for volatile symbols
            const char* symbol = pParams->tradeSymbol;
            if (strcmp(symbol, "XAUUSD") == 0) {
                modulator = 0.75;  // Gold: reduce risk
            } else if (strcmp(symbol, "GBPJPY") == 0) {
                modulator = 0.6;  // GBPJPY: reduce risk (high volatility)
            }
            break;
            
        case 5:  // Recent performance-based
            // Reduce risk after losses
            double totalLostPips = 0.0;
            int lossCount = getLossTimesInDayEasy(pParams->currentBrokerTime,
                                                  &totalLostPips);
            if (lossCount > 0) {
                modulator = fmax(0.5, 1.0 - (lossCount * 0.2));  // 20% per loss
            }
            break;
    }
    
    double finalRisk = baseRisk * modulator;
    
    // Cap risk between 0.1% and 2.0%
    finalRisk = fmax(0.001, fmin(0.02, finalRisk));
    
    pantheios_logprintf(PANTHEIOS_SEV_DEBUG,
        (PAN_CHAR_T*)"Risk: base=%s, mode=%s, modulator=%s → final=%s",
        LogFmt::decimal(baseRisk * 100, 2),
        LogFmt::integer(riskMode),
        LogFmt::decimal(modulator, 2),
        LogFmt::decimal(finalRisk * 100, 2));
    
    return finalRisk;
}
```

### Phase 9: Split Trade Mode Dispatcher

```cpp
void executeSplitTradeMode(StrategyParams* pParams,
                          StrategyIndicators& indicators,
                          const Base_Indicators& base,
                          OrderType orderType,
                          double stopLoss) {
    using LogFmt = NumericLogFormatter;
    
    pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL,
        (PAN_CHAR_T*)"Executing split mode: %s for %s orders",
        splitTradeModeToString(indicators.splitTradeMode),
        (orderType == BUY) ? "BUY" : "SELL");
    
    switch (indicators.splitTradeMode) {
        case STM_DEFAULT:
            // Single order with full risk
            if (orderType == BUY) {
                openSingleLongEasy(indicators.takePrice, stopLoss, 0.0, indicators.risk);
            } else {
                openSingleShortEasy(indicators.takePrice, stopLoss, 0.0, indicators.risk);
            }
            break;
            
        case STM_KEYK:
            // 3-part split: 1x, 2x, no-TP (1/3 risk each)
            if (orderType == BUY) {
                splitBuyOrders_KeyK(pParams, &indicators, &base, indicators.takePrice, stopLoss);
            } else {
                splitSellOrders_KeyK(pParams, &indicators, &base, indicators.takePrice, stopLoss);
            }
            break;
            
        case STM_SHELLINGTON:
            // Advanced split with risk cap and min lot size
            if (orderType == BUY) {
                splitBuyOrders_Shellington(pParams, &indicators, &base, stopLoss);
            } else {
                splitSellOrders_Shellington(pParams, &indicators, &base, stopLoss);
            }
            break;
            
        case STM_SHORTTERM:
            // Gap-based split (1/3 of daily range)
            if (orderType == BUY) {
                splitBuyOrders_ShortTerm(pParams, &indicators, &base, indicators.takePrice, stopLoss);
            } else {
                splitSellOrders_ShortTerm(pParams, &indicators, &base, indicators.takePrice, stopLoss);
            }
            break;
            
        case STM_WEEKLY_BEGIN:
            // Week beginning strategy (no TP, check pivot proximity)
            if (orderType == BUY) {
                splitBuyOrders_WeeklyBeginning(pParams, &indicators, &base, stopLoss);
            } else {
                splitSellOrders_WeeklyBeginning(pParams, &indicators, &base, stopLoss);
            }
            break;
            
        case STM_ATR_4H:
        case STM_ATR_4H_ALT:
            // ATR-based ladder (multiple orders at different levels)
            if (orderType == BUY) {
                splitBuyOrders_ATR4H(pParams, &indicators, &base, stopLoss);
            } else {
                splitSellOrders_ATR4H(pParams, &indicators, &base, stopLoss);
            }
            break;
            
        // ... additional split modes
            
        default:
            pantheios_logprintf(PANTHEIOS_SEV_WARNING,
                (PAN_CHAR_T*)"Unknown split mode: %s, using default",
                LogFmt::integer(indicators.splitTradeMode));
            
            if (orderType == BUY) {
                openSingleLongEasy(indicators.takePrice, stopLoss, 0.0, indicators.risk);
            } else {
                openSingleShortEasy(indicators.takePrice, stopLoss, 0.0, indicators.risk);
            }
            break;
    }
}
```

### Phase 10: Split Trade Mode Implementations

#### KeyK Pattern (3-part split)
```cpp
void splitBuyOrders_KeyK(StrategyParams* pParams,
                        StrategyIndicators* pIndicators,
                        const Base_Indicators* pBase,
                        double takePricePrimary,
                        double stopLoss) {
    double takePrice;
    
    // 1x reward (1/3 risk)
    takePrice = takePricePrimary;
    openSingleLongEasy(takePrice, stopLoss, 0.0, pIndicators->risk / 3.0);
    
    // 2x reward (1/3 risk)
    takePrice = 2.0 * takePricePrimary;
    openSingleLongEasy(takePrice, stopLoss, 0.0, pIndicators->risk / 3.0);
    
    // No TP - runner (1/3 risk)
    takePrice = 0.0;
    openSingleLongEasy(takePrice, stopLoss, 0.0, pIndicators->risk / 3.0);
}

void splitSellOrders_KeyK(StrategyParams* pParams,
                         StrategyIndicators* pIndicators,
                         const Base_Indicators* pBase,
                         double takePricePrimary,
                         double stopLoss) {
    double takePrice;
    
    takePrice = takePricePrimary;
    openSingleShortEasy(takePrice, stopLoss, 0.0, pIndicators->risk / 3.0);
    
    takePrice = 2.0 * takePricePrimary;
    openSingleShortEasy(takePrice, stopLoss, 0.0, pIndicators->risk / 3.0);
    
    takePrice = 0.0;
    openSingleShortEasy(takePrice, stopLoss, 0.0, pIndicators->risk / 3.0);
}
```

#### Shellington Pattern (Risk-capped with min lot size)
```cpp
void splitBuyOrders_Shellington(StrategyParams* pParams,
                               StrategyIndicators* pIndicators,
                               const Base_Indicators* pBase,
                               double stopLoss) {
    double takePrice;
    double lots;
    
    // Use min lot size if enabled, else calculate
    if (pIndicators->isEnableBuyMinLotSize) {
        lots = pIndicators->minLotSize;
    } else {
        // Use max of stopLoss or 1.5x dailyATR for sizing
        double sizingDistance = fmax(stopLoss, pBase->dailyATR * 1.5);
        lots = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, sizingDistance);
        lots *= pIndicators->risk;
        
        // Round up to volume step
        lots = roundUp(lots, pIndicators->volumeStep);
    }
    
    // Take profit is max of calculated TP or risk-capped SL
    takePrice = fmax(pIndicators->takePrice, pIndicators->riskCap * stopLoss);
    
    openSingleLongEasy(takePrice, stopLoss, lots, 0.0);
}

void splitSellOrders_Shellington(StrategyParams* pParams,
                                StrategyIndicators* pIndicators,
                                const Base_Indicators* pBase,
                                double stopLoss) {
    double takePrice;
    double lots;
    
    if (pIndicators->isEnableSellMinLotSize) {
        lots = pIndicators->minLotSize;
    } else {
        double sizingDistance = fmax(stopLoss, pBase->dailyATR * 1.5);
        lots = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, sizingDistance);
        lots *= pIndicators->risk;
        lots = roundUp(lots, pIndicators->volumeStep);
    }
    
    takePrice = fmax(pIndicators->takePrice, pIndicators->riskCap * stopLoss);
    
    openSingleShortEasy(takePrice, stopLoss, lots, 0.0);
}
```

#### Short-Term Pattern (Gap-based 1/3 daily range)
```cpp
void splitBuyOrders_ShortTerm(StrategyParams* pParams,
                             StrategyIndicators* pIndicators,
                             const Base_Indicators* pBase,
                             double takePricePrimary,
                             double stopLoss) {
    double pATR = pBase->pDailyATR;
    double pHigh = pBase->pDailyHigh;
    double gap = pHigh - pIndicators->entryPrice;
    
    time_t currentTime = pParams->currentBrokerTime;
    
    // Only trade if gap >= pATR / 2 and yesterday's close range < dailyATR
    double yesterdayCloseRange = fabs(iClose(DAILY_RATES, 1) - iClose(DAILY_RATES, 2));
    
    if (gap >= pATR / 2.0 && yesterdayCloseRange < pBase->dailyATR) {
        int orderCountToday = getOrderCountTodayEasy(currentTime);
        
        if (orderCountToday == 0) {  // First order of the day
            // Take 1/3 of gap as TP
            double takePrice = gap / 3.0;
            
            // Calculate lot size
            double lots = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, takePrice);
            lots *= pIndicators->risk;
            
            // Cap to max risk (7x account risk percent)
            double lotsMax = calculateOrderSizeWithSpecificRisk(pParams, BUY,
                                pIndicators->entryPrice, stopLoss,
                                pParams->settings[ACCOUNT_RISK_PERCENT] * 7.0);
            lots = fmin(lotsMax, lots);
            
            openSingleLongEasy(takePrice, stopLoss, lots, 0.0);
        }
    }
}

void splitSellOrders_ShortTerm(StrategyParams* pParams,
                              StrategyIndicators* pIndicators,
                              const Base_Indicators* pBase,
                              double takePricePrimary,
                              double stopLoss) {
    double pATR = pBase->pDailyATR;
    double pLow = pBase->pDailyLow;
    double gap = pIndicators->entryPrice - pLow;
    
    time_t currentTime = pParams->currentBrokerTime;
    
    double yesterdayCloseRange = fabs(iClose(DAILY_RATES, 1) - iClose(DAILY_RATES, 2));
    
    if (gap >= pATR / 2.0 && yesterdayCloseRange < pBase->dailyATR) {
        int orderCountToday = getOrderCountTodayEasy(currentTime);
        
        if (orderCountToday == 0) {
            double takePrice = gap / 3.0;
            
            double lots = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, takePrice);
            lots *= pIndicators->risk;
            
            double lotsMax = calculateOrderSizeWithSpecificRisk(pParams, SELL,
                                pIndicators->entryPrice, stopLoss,
                                pParams->settings[ACCOUNT_RISK_PERCENT] * 7.0);
            lots = fmin(lotsMax, lots);
            
            openSingleShortEasy(takePrice, stopLoss, lots, 0.0);
        }
    }
}
```

#### Weekly Beginning Pattern (Pivot-based, no TP)
```cpp
void splitBuyOrders_WeeklyBeginning(StrategyParams* pParams,
                                   StrategyIndicators* pIndicators,
                                   const Base_Indicators* pBase,
                                   double stopLoss) {
    // Only enter if entry price is below weekly R1
    if (pIndicators->entryPrice <= pBase->weeklyR1) {
        // No TP (runner strategy)
        openSingleLongEasy(0.0, stopLoss, 0.0, pIndicators->risk);
    }
}

void splitSellOrders_WeeklyBeginning(StrategyParams* pParams,
                                    StrategyIndicators* pIndicators,
                                    const Base_Indicators* pBase,
                                    double stopLoss) {
    // Only enter if entry price is above weekly S1
    if (pIndicators->entryPrice >= pBase->weeklyS1) {
        openSingleShortEasy(0.0, stopLoss, 0.0, pIndicators->risk);
    }
}
```

### Phase 11: Complete Strategy Orchestration

```cpp
extern "C" {

AsirikuyReturnCode runComplexStrategy(StrategyParams* pParams) {
    using LogFmt = NumericLogFormatter;
    
    // Null check
    if (pParams == NULL) {
        pantheios_logputs(PANTHEIOS_SEV_CRITICAL,
            (PAN_CHAR_T*)"runComplexStrategy: pParams = NULL");
        return NULL_POINTER;
    }
    
    // Initialize EasyTrade
    if (initEasyTradeLibrary(pParams) != SUCCESS) {
        return FAILURE;
    }
    
    // Validate multi-timeframe data
    AsirikuyReturnCode result = validateMultiTimeframeData(pParams);
    if (result != SUCCESS) {
        return result;
    }
    
    // Check minimum bars
    int barsTotal = barsCount(PRIMARY_RATES);
    if (barsTotal < 200) {
        return SUCCESS;  // Not enough data
    }
    
    // Load base indicators
    Base_Indicators base = loadBaseIndicators(pParams);
    
    // Initialize strategy indicators
    StrategyIndicators indicators = {};
    indicators.splitTradeMode = (int)parameter(SPLIT_TRADE_MODE_PARAM);
    indicators.tpMode = (int)parameter(ADDITIONAL_PARAM_11);
    indicators.minLotSize = 0.01;
    indicators.volumeStep = 0.01;
    
    // Calculate execution trend (multi-timeframe consensus)
    indicators.executionTrend = calculateExecutionTrend(pParams, base, indicators);
    
    // Log consensus state
    int instanceId = (int)pParams->settings[STRATEGY_INSTANCE_ID];
    pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL,
        (PAN_CHAR_T*)"ComplexStrategy[%s]: executionTrend=%s, dailyPhase=%s, atrRange=%s",
        LogFmt::integer(instanceId),
        LogFmt::integer(indicators.executionTrend),
        LogFmt::integer(base.dailyTrendPhase),
        LogFmt::decimal(indicators.atr_euro_range, 2));
    
    // Early exit if no consensus
    if (indicators.executionTrend == 0) {
        pantheios_logputs(PANTHEIOS_SEV_DEBUG,
            (PAN_CHAR_T*)"No consensus, skipping entry");
        return SUCCESS;
    }
    
    // Volatility gating
    if (!passesVolatilityGate(base, indicators)) {
        return SUCCESS;  // ATR outside acceptable range
    }
    
    // Calculate dynamic risk
    indicators.risk = calculateDynamicRisk(pParams, base, indicators);
    indicators.riskCap = 2.0;  // Risk cap multiplier
    
    // Determine entry parameters
    if (indicators.executionTrend == 1) {
        // Long entry
        indicators.entryPrice = pParams->bidAsk.ask[0];
        indicators.stopLossPrice = base.dailyLow;  // Example: yesterday's low
        indicators.takePrice = base.dailyATR * 2.0;  // 2x ATR target
        indicators.entrySignal = 1;
        
        // Entry de-duplication check
        if (isDuplicateEntry(pParams, base, indicators.entryPrice, BUY)) {
            return SUCCESS;  // Duplicate entry blocked
        }
        
        // Calculate stop loss distance
        double stopLoss = fabs(indicators.entryPrice - indicators.stopLossPrice);
        
        // Execute split trade mode
        executeSplitTradeMode(pParams, indicators, base, BUY, stopLoss);
        
        pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL,
            (PAN_CHAR_T*)"ComplexStrategy[%s]: LONG entry at %s, SL=%s, TP=%s, mode=%s",
            LogFmt::integer(instanceId),
            LogFmt::decimal(indicators.entryPrice, 5),
            LogFmt::decimal(indicators.stopLossPrice, 5),
            LogFmt::decimal(indicators.takePrice, 2),
            splitTradeModeToString(indicators.splitTradeMode));
    }
    else if (indicators.executionTrend == -1) {
        // Short entry
        indicators.entryPrice = pParams->bidAsk.bid[0];
        indicators.stopLossPrice = base.dailyHigh;  // Example: yesterday's high
        indicators.takePrice = base.dailyATR * 2.0;
        indicators.entrySignal = -1;
        
        // Entry de-duplication check
        if (isDuplicateEntry(pParams, base, indicators.entryPrice, SELL)) {
            return SUCCESS;  // Duplicate entry blocked
        }
        
        // Calculate stop loss distance
        double stopLoss = fabs(indicators.stopLossPrice - indicators.entryPrice);
        
        // Execute split trade mode
        executeSplitTradeMode(pParams, indicators, base, SELL, stopLoss);
        
        pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL,
            (PAN_CHAR_T*)"ComplexStrategy[%s]: SHORT entry at %s, SL=%s, TP=%s, mode=%s",
            LogFmt::integer(instanceId),
            LogFmt::decimal(indicators.entryPrice, 5),
            LogFmt::decimal(indicators.stopLossPrice, 5),
            LogFmt::decimal(indicators.takePrice, 2),
            splitTradeModeToString(indicators.splitTradeMode));
    }
    
    // Note: Telemetry harvesting is automatic (handled by framework)
    // Note: Late overwrite block (risk/ATR recalc) is automatic
    
    return SUCCESS;
}

} // extern "C"
```

---

## Common Pitfalls & Solutions

### Pitfall 1: Missing Bar Validation

**Problem:**
```cpp
// Accessing multi-timeframe data without validation
double dailyClose = iClose(DAILY_RATES, 1);  // May be misaligned
```

**Solution:**
```cpp
// Always validate first
if (validateDailyBarsEasy(pParams, PRIMARY_RATES, DAILY_RATES) != SUCCESS) {
    return FAILURE;
}
double dailyClose = iClose(DAILY_RATES, 1);
```

### Pitfall 2: Ignoring De-duplication

**Problem:**
```cpp
// Placing order without checking duplicates
openSingleLongEasy(takePrice, stopLoss, 0.0, risk);  // May duplicate
```

**Solution:**
```cpp
if (isDuplicateEntry(pParams, base, entryPrice, BUY)) {
    return SUCCESS;  // Block duplicate
}
openSingleLongEasy(takePrice, stopLoss, 0.0, risk);
```

### Pitfall 3: Incorrect ATR Calculation

**Problem:**
```cpp
// Using standard ATR on incomplete bars
double weeklyATR = iAtr(DAILY_RATES, 70, 0);  // Includes forming bar
```

**Solution:**
```cpp
// Use shift=1 or predictive ATR
double weeklyATR = iAtr(DAILY_RATES, 70, 1);  // Completed bars only
// Or use predictive for current state
double weeklyATR = iAtrWholeDaysSimple(PRIMARY_RATES, 70);
```

### Pitfall 4: Not Capping Risk

**Problem:**
```cpp
// Calculated risk exceeds reasonable limits
double lots = calculateOrderSize(pParams, BUY, entry, stopLoss) * risk;
openSingleLongEasy(takePrice, stopLoss, lots, 0.0);  // May be huge
```

**Solution:**
```cpp
double lots = calculateOrderSize(pParams, BUY, entry, stopLoss) * risk;

// Cap to maximum risk (e.g., 7x account risk)
double lotsMax = calculateOrderSizeWithSpecificRisk(pParams, BUY, entry, stopLoss,
                     pParams->settings[ACCOUNT_RISK_PERCENT] * 7.0);
lots = fmin(lotsMax, lots);

openSingleLongEasy(takePrice, stopLoss, lots, 0.0);
```

### Pitfall 5: Hardcoded Split Mode Logic

**Problem:**
```cpp
// Always using same split pattern
splitBuyOrders_KeyK(pParams, &indicators, &base, takePrice, stopLoss);
```

**Solution:**
```cpp
// Use dispatcher with mode enum
executeSplitTradeMode(pParams, indicators, base, BUY, stopLoss);
// Mode determined by parameter/consensus/market conditions
```

---

## Performance Considerations

1. **Indicator Caching**: Calculate ATR values once per tick, store in Base_Indicators
2. **Early Exits**: Check consensus first, exit early if no agreement (avoid expensive calculations)
3. **De-dup Efficiency**: Use indexed price comparisons rather than full order iteration
4. **Logging Volume**: Use DEBUG level for per-tick consensus breakdown, INFO for entries only
5. **Split Mode Complexity**: Simple modes (DEFAULT, KEYK) are faster than complex (SHELLINGTON, ATR_4H)

---

## Migration Time Estimate

**Complex Strategy (TrendStrategy-like):**
- Setup & multi-file structure: 30 minutes
- Data structures & enums: 1 hour
- Multi-timeframe validation: 30 minutes
- Base indicators calculation: 1.5 hours
- Consensus logic: 2 hours
- Volatility gating: 30 minutes
- De-duplication logic: 1 hour
- Risk modulation: 1.5 hours
- Split mode dispatcher: 1 hour
- Split mode implementations: 3-4 hours (12+ patterns)
- Testing & validation: 2-3 hours
- **Total: ~14-16 hours**

**Factors That Increase Time:**
- Custom indicators (+2 hours per indicator)
- Additional split modes (+30 min per mode)
- Complex risk modulation (+1 hour)
- Symbol-specific logic (+1 hour)
- Backtesting validation (+4 hours)

---

## Testing Strategy

### Unit Testing
```cpp
// Test consensus logic
bool testMultiTimeframeConsensus() {
    Base_Indicators base = createMockBase(1, 1, 1);  // All bullish
    StrategyIndicators ind = {};
    
    int trend = calculateExecutionTrend(mockParams, base, ind);
    assert(trend == 1);  // Should be bullish
    
    return true;
}

// Test volatility gate
bool testVolatilityGate() {
    Base_Indicators base = {};
    base.pWeeklyPredictATR = 100.0;  // ATR = 100
    
    StrategyIndicators ind = {};
    
    // Should pass: 40 (100*0.4) is within [30, 150]
    assert(passesVolatilityGate(base, ind) == TRUE);
    
    return true;
}
```

### Integration Testing
1. **Backtest**: Run strategy on historical data (6-12 months)
2. **Compare Results**: Verify P&L matches original C strategy ± 5%
3. **Order Analysis**: Check order count, average size, win rate
4. **Slippage Check**: Monitor execution prices vs expected
5. **Risk Validation**: Ensure no trades exceed max risk

### Paper Trading Checklist
- [ ] Deploy to demo account
- [ ] Monitor for 2 weeks minimum
- [ ] Verify consensus logic triggering correctly
- [ ] Check ATR gate effectiveness
- [ ] Validate de-duplication working
- [ ] Confirm split modes executing as expected
- [ ] Review logs for errors/warnings

---

## Next Steps After Migration

1. **Parameter Optimization**: Tune ATR thresholds, risk modulators, split mode selection
2. **Symbol Expansion**: Test strategy on multiple symbols, adjust parameters per symbol
3. **Performance Analysis**: Compare vs benchmark, identify improvement opportunities
4. **Documentation**: Create strategy-specific guide with parameter descriptions
5. **Monitoring Dashboard**: Track key metrics (consensus rate, gate pass rate, de-dup blocks, P&L)
6. **Live Deployment**: Start with minimum position size, scale up gradually

---

## Summary

Complex strategies require careful orchestration of multiple components:
- **Multi-timeframe consensus** ensures high-probability setups
- **ATR-based gating** filters out unsuitable market conditions
- **De-duplication** prevents overtrading
- **Split modes** allow flexible position management
- **Dynamic risk** adapts to market conditions and performance

The key to successful migration is:
1. **Validate data integrity** at every timeframe
2. **Test consensus logic** thoroughly with edge cases
3. **Implement de-duplication** rigorously (prevents costly duplicates)
4. **Start simple** then add complexity incrementally
5. **Monitor extensively** during paper trading phase

This template provides the foundation for migrating even the most sophisticated trading strategies while maintaining their original logic and performance characteristics.
