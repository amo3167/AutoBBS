# Order Management Strategy Migration Template

**Target Audience**: Strategies that actively manage positions with entry, modification, and exit logic.

**Examples**: TakeOver (modifies manual orders), dynamic stop loss strategies, breakeven management, partial exit strategies.

**Estimated Complexity**: Medium (3-5 hours per strategy)

---

## Migration Checklist

### Phase 1: Setup & Structure

- [ ] Create new `.cpp` file in `dev/TradingStrategies/src/`
- [ ] Create corresponding `.h` header in `dev/TradingStrategies/include/`
- [ ] Add to `premake4.lua` in TradingStrategies project
- [ ] Include required headers:
  ```cpp
  #include "Precompiled.h"
  #include "StrategyTypes.h"
  #include "AsirikuyDefines.h"
  #include "OrderManagement.h"
  #include "EasyTradeCWrapper.hpp"
  #include "StrategyUserInterface.h"
  #include "NumericLoggingHelpers.hpp"
  #include "Logging.h"
  ```

### Phase 2: Define Strategy Constants & Types

- [ ] Convert `#define` constants to typed enums or constants
- [ ] Define strategy-specific data structures

```cpp
// Constants (prefer const over #define for type safety)
constexpr BOOL USE_INTERNAL_SL = FALSE;
constexpr BOOL USE_INTERNAL_TP = TRUE;  // Recommended for backtest compliance

// Strategy parameter mapping
enum AdditionalSettings {
    STOP_LOSS_MODE = ADDITIONAL_PARAM_1,
    TAKE_PROFIT_MODE = ADDITIONAL_PARAM_2,
    TRAILING_START = ADDITIONAL_PARAM_3,
    TRAILING_DISTANCE = ADDITIONAL_PARAM_4,
    MAX_HOLDING_BARS = ADDITIONAL_PARAM_5,
    // ... up to ADDITIONAL_PARAM_40
};

// Indicator/state structure
struct StrategyIndicators {
    double dailyATR;
    double dynamicStopLoss;
    double dynamicTakeProfit;
    int currentTrend;
    double entryPrice;
    double adjustmentPoints;
    // Additional fields as needed
};
```

### Phase 3: Order Counting & State Detection

- [ ] Check current position state before taking action

```cpp
// Count open orders by type
int longCount = totalOpenOrders(pParams, BUY);
int shortCount = totalOpenOrders(pParams, SELL);

// Or use EasyTrade shortcuts
int totalOrders = totalOrdersCount();
int buyOrders = buyOrdersCount();
int sellOrders = sellOrdersCount();

// Check if any orders exist
if (hasOpenOrder() > 0) {
    // Have positions to manage
}

// Check same-day orders
BOOL isOpen = FALSE;
int sameDayCount = hasSameDayOrderEasy(pParams->currentBrokerTime, &isOpen);
```

### Phase 4: Entry Logic Patterns

#### Pattern A: Simple Open/Update (Preferred)

```cpp
// Opens new trade if none exists, updates if trade already open
// Automatically closes opposite direction
double stopLoss = 50.0;    // 50 currency units from entry
double takeProfit = 100.0; // 100 currency units from entry
double riskPercent = 0.5;  // 0.5% account risk

AsirikuyReturnCode result = openOrUpdateLongEasy(takeProfit, stopLoss, riskPercent);
if (result != SUCCESS) {
    return logAsirikuyError("openOrUpdateLongEasy", result);
}

// Short version
result = openOrUpdateShortEasy(takeProfit, stopLoss, riskPercent);
```

#### Pattern B: Single Order Placement (No Auto-Close)

```cpp
// Place single long order without affecting existing positions
double lotSize = 0.1;  // Explicit lot size (0 = calculate from risk)
result = openSingleLongEasy(takeProfit, stopLoss, lotSize, riskPercent);

// Pending orders
double entryPrice = 1.2000;
result = openSingleBuyLimitEasy(entryPrice, takeProfit, stopLoss, lotSize, riskPercent);
result = openSingleBuyStopEasy(entryPrice, takeProfit, stopLoss, lotSize);
result = openSingleSellLimitEasy(entryPrice, takeProfit, stopLoss, lotSize, riskPercent);
result = openSingleSellStopEasy(entryPrice, takeProfit, stopLoss, lotSize);
```

#### Pattern C: Lower-Level OrderManagement API

```cpp
// More control over internal vs broker SL/TP
int ratesIndex = PRIMARY_RATES;
int resultsIndex = 0;
double stopLoss = 50.0;
double takeProfit = 100.0;
double riskOverride = 0.0;  // 0 = use default from settings

result = openOrUpdateLongTrade(pParams, ratesIndex, resultsIndex,
                               stopLoss, takeProfit, riskOverride,
                               USE_INTERNAL_SL, USE_INTERNAL_TP);

result = openOrUpdateShortTrade(pParams, ratesIndex, resultsIndex,
                                stopLoss, takeProfit, riskOverride,
                                USE_INTERNAL_SL, USE_INTERNAL_TP);
```

#### Pattern D: Entry De-duplication (Critical for Pending Orders)

```cpp
// Prevent duplicate entries at same price level
double entryPrice = 1.2000;
double priceLimit = dailyATR / 4.0;  // Tolerance

// Check if pending order already exists today at this price
if (isSameDaySamePricePendingOrderEasy(entryPrice, priceLimit, 
                                       pParams->currentBrokerTime) > 0) {
    // Already have order at this price today, skip
    return SUCCESS;
}

// Place order
result = openSingleBuyLimitEasy(entryPrice, takeProfit, stopLoss, 0.0, riskPercent);
```

### Phase 5: Position Modification Patterns

#### Pattern A: Dynamic Stop Loss Adjustment

```cpp
// Modify all orders of a type
int orderTicket = -1;  // -1 or SELECT_ALL_TRADES = modify all
double newStopLoss = calculateDynamicStopLoss(pParams, indicators);
double takeProfit = -1;  // -1 = don't change TP

if (totalOpenOrders(pParams, BUY) > 0) {
    result = modifyTradeEasy(BUY, orderTicket, newStopLoss, takeProfit);
    if (result != SUCCESS) {
        return logAsirikuyError("modifyTradeEasy(BUY)", result);
    }
}

if (totalOpenOrders(pParams, SELL) > 0) {
    result = modifyTradeEasy(SELL, orderTicket, newStopLoss, takeProfit);
    if (result != SUCCESS) {
        return logAsirikuyError("modifyTradeEasy(SELL)", result);
    }
}
```

#### Pattern B: Trailing Stop

```cpp
// Automatically trail stop loss as price moves favorably
double trailStart = 50.0;    // Start trailing after 50 units profit
double trailDistance = 20.0; // Keep SL 20 units behind price

result = trailOpenTrades(pParams, PRIMARY_RATES, trailStart, trailDistance,
                        USE_INTERNAL_SL, USE_INTERNAL_TP);
```

#### Pattern C: Breakeven Management

```cpp
// Move SL to breakeven after certain profit
if (totalOpenOrders(pParams, BUY) > 0) {
    double currentProfit = caculateStrategyPNLEasy(FALSE);
    double breakevenThreshold = 30.0;  // Move to BE after 30 units profit
    
    if (currentProfit >= breakevenThreshold) {
        // Get original entry price (from order info or track separately)
        double entryPrice = pParams->orderInfo[0].openPrice;
        double beStopLoss = fabs(pParams->bidAsk.ask[0] - entryPrice);
        
        // Add small buffer to ensure SL is beyond entry
        beStopLoss += 2.0;  // 2 units buffer
        
        modifyTradeEasy(BUY, -1, beStopLoss, -1);
    }
}
```

#### Pattern D: Time-Based SL/TP Adjustment (Elliptical)

```cpp
// Tighten/loosen stops based on order age
int orderAge = getOrderAge(pParams, PRIMARY_RATES);
int maxHoldingTime = (int)parameter(MAX_HOLDING_BARS);
double targetTP = 100.0;
double zValue = 2.0;  // 98% confidence

// Calculate dynamic stops based on elliptical model
double ellipticalSL = CalculateEllipticalStopLoss(pParams, targetTP, 
                                                  maxHoldingTime, zValue, orderAge);
double ellipticalTP = CalculateEllipticalTakeProfit(pParams, targetTP,
                                                    maxHoldingTime, zValue, orderAge);

// Update trade
result = modifyTradeEasy(BUY, -1, ellipticalSL, ellipticalTP);
```

#### Pattern E: Modify Only (No Entry Signal)

```cpp
// Update existing positions without triggering new entries
if (totalOpenOrders(pParams, BUY) > 0) {
    double newSL = calculateDynamicStop(indicators);
    double newTP = calculateDynamicTarget(indicators);
    
    // updateLongTrade does NOT update states.bin entry time
    result = updateLongTrade(pParams, PRIMARY_RATES, 0, newSL, newTP,
                            USE_INTERNAL_SL, USE_INTERNAL_TP);
}

if (totalOpenOrders(pParams, SELL) > 0) {
    result = updateShortTrade(pParams, PRIMARY_RATES, 0, newSL, newTP,
                             USE_INTERNAL_SL, USE_INTERNAL_TP);
}
```

### Phase 6: Exit Logic Patterns

#### Pattern A: Simple Close All

```cpp
// Close all longs
if (exitSignal == SIGNAL_CLOSE_LONG) {
    result = closeAllLongs();
    if (result != SUCCESS) {
        return logAsirikuyError("closeAllLongs", result);
    }
}

// Close all shorts
if (exitSignal == SIGNAL_CLOSE_SHORT) {
    result = closeAllShorts();
    if (result != SUCCESS) {
        return logAsirikuyError("closeAllShorts", result);
    }
}
```

#### Pattern B: Close Specific Order

```cpp
// Close by ticket
int orderTicket = pParams->orderInfo[0].ticket;
result = closeLongEasy(orderTicket);
result = closeShortEasy(orderTicket);
```

#### Pattern C: Conditional Close (Trend Reversal)

```cpp
// TakeOver pattern: close on trend reversal
if (totalOpenOrders(pParams, BUY) > 0 && indicators.trend == -1) {
    // Trend reversed to bearish, close longs
    if (pParams->bidAsk.ask[0] >= indicators.entryPrice) {
        // Only close if at or above entry (preserve profit)
        result = closeAllLongs();
    }
}

if (totalOpenOrders(pParams, SELL) > 0 && indicators.trend == 1) {
    // Trend reversed to bullish, close shorts
    if (pParams->bidAsk.bid[0] <= indicators.entryPrice) {
        result = closeAllShorts();
    }
}
```

#### Pattern D: Internal SL/TP Check

```cpp
// Check internal stop loss (if using internal SL)
double internalSL = 50.0;
result = checkInternalSL(pParams, PRIMARY_RATES, 0, internalSL);

// Check internal take profit (if using internal TP)
double internalTP = 100.0;
result = checkInternalTP(pParams, PRIMARY_RATES, 0, internalTP);

// Check timed exit (max holding period)
BOOL usingInternalSL = TRUE;
BOOL usingInternalTP = TRUE;
result = checkTimedExit(pParams, PRIMARY_RATES, 0, usingInternalSL, usingInternalTP);
```

#### Pattern E: Partial Exits / Profit Taking

```cpp
// Close winning positions above threshold
double totalPNL = caculateStrategyPNLEasy(FALSE);
double targetProfit = 200.0;

if (totalPNL >= targetProfit) {
    // Close portion of winning positions
    result = closeWinningPositionsEasy(totalPNL, targetProfit);
}
```

#### Pattern F: Close Pending Orders

```cpp
// Close all pending limit/stop orders
result = closeAllLimitAndStopOrdersEasy(pParams->currentBrokerTime);

// Type-specific pending order closes
result = closeAllBuyLimitOrdersEasy(pParams->currentBrokerTime);
result = closeAllSellLimitOrdersEasy(pParams->currentBrokerTime);
result = closeAllBuyStopOrdersEasy(pParams->currentBrokerTime);
result = closeAllSellStopOrdersEasy(pParams->currentBrokerTime);
```

### Phase 7: Risk & Position Sizing

#### Calculate Order Size

```cpp
// Method 1: Automatic calculation based on settings
double entryPrice = pParams->bidAsk.ask[0];
double stopLossPrice = entryPrice - 50.0;
double stopLossDistance = fabs(entryPrice - stopLossPrice);

double lotSize = calculateOrderSize(pParams, BUY, entryPrice, stopLossDistance);

// Method 2: Override with specific risk
double specificRisk = 0.25;  // 0.25% risk for this trade
lotSize = calculateOrderSizeWithSpecificRisk(pParams, BUY, entryPrice,
                                             stopLossDistance, specificRisk);

// Method 3: Fixed lot size
lotSize = 0.1;  // Manual override
```

#### Margin Validation

```cpp
// Always check margin before placing orders
if (!isEnoughFreeMargin(pParams, BUY, lotSize)) {
    pantheios_logprintf(PANTHEIOS_SEV_WARNING,
        (PAN_CHAR_T*)"Insufficient margin for lotSize=%s",
        LogFmt::decimal(lotSize, 2));
    return SUCCESS;  // Skip trade
}

// Or check free margin directly
double freeMargin = caculateFreeMarginEasy();
if (freeMargin < MIN_MARGIN_THRESHOLD) {
    return SUCCESS;  // Skip trade
}
```

#### Risk Monitoring

```cpp
// Calculate current portfolio risk
double totalRisk = caculateStrategyRiskEasy(FALSE);  // Include locked profit
double totalRiskNLP = caculateStrategyRiskEasy(TRUE); // Exclude locked profit

// Calculate current P&L
double totalPNL = caculateStrategyPNLEasy(FALSE);
double totalPNLNLP = caculateStrategyPNLEasy(TRUE);

// Volatility-adjusted risk
double dailyATR = iAtr(DAILY_RATES, 14, 1);
double volRisk = caculateStrategyVolRiskEasy(dailyATR);
double volRiskNoTP = caculateStrategyVolRiskForNoTPOrdersEasy(dailyATR);

// Max risk checks
double accountRiskPercent = parameter(ACCOUNT_RISK_PERCENT);
double maxRisk = pParams->accountInfo.balance * (accountRiskPercent / 100.0);

if (totalRisk > maxRisk) {
    pantheios_logputs(PANTHEIOS_SEV_WARNING,
        (PAN_CHAR_T*)"Max risk exceeded, skipping new entries");
    return SUCCESS;
}
```

### Phase 8: Order Validation & Checks

```cpp
// Verify SL/TP are set correctly on all orders
double defaultStopLoss = 50.0;
double defaultTakeProfit = 100.0;

result = checkOrders(defaultTakeProfit, defaultStopLoss);
if (result != SUCCESS) {
    // Some orders missing SL/TP, they've been assigned defaults
    pantheios_logputs(PANTHEIOS_SEV_WARNING,
        (PAN_CHAR_T*)"Orders updated with default SL/TP");
}

// Validate orders are correct
if (!areOrdersCorrect(pParams, defaultStopLoss, defaultTakeProfit)) {
    pantheios_logputs(PANTHEIOS_SEV_ERROR,
        (PAN_CHAR_T*)"Order validation failed");
    return FAILURE;
}
```

### Phase 9: Indicator Calculation for Exits

```cpp
// Dynamic stop loss indicators
struct StrategyIndicators {
    double dailyATR;
    double yesterdayHigh;
    double yesterdayLow;
    double twoDayHigh;
    double twoDayLow;
    double ma200;
    double bbStopPrice;
    int bbTrend;
};

StrategyIndicators loadIndicators(StrategyParams* pParams) {
    StrategyIndicators ind;
    
    // ATR for volatility-based stops
    ind.dailyATR = iAtr(DAILY_RATES, 14, 1);
    
    // Previous day high/low
    ind.yesterdayHigh = iHigh(DAILY_RATES, 1);
    ind.yesterdayLow = iLow(DAILY_RATES, 1);
    
    // Two-day high/low
    ind.twoDayHigh = fmax(iHigh(DAILY_RATES, 1), iHigh(DAILY_RATES, 2));
    ind.twoDayLow = fmin(iLow(DAILY_RATES, 1), iLow(DAILY_RATES, 2));
    
    // Moving average for dynamic stop
    ind.ma200 = iMA(3, HOURLY_RATES, 200, 1);
    
    // Bollinger Band stop (custom indicator)
    int bbIndex;
    iBBandStop(PRIMARY_RATES, 20, 2.0, &ind.bbTrend, &ind.bbStopPrice, &bbIndex);
    
    return ind;
}

// Calculate dynamic stop loss based on mode
double calculateDynamicStopLoss(StrategyParams* pParams, 
                               StrategyIndicators& ind, OrderType type) {
    int stopMode = (int)parameter(STOP_LOSS_MODE);
    double stopPrice = 0.0;
    
    switch (stopMode) {
        case 0:  // Smart mode (max of multiple indicators)
            if (type == BUY) {
                stopPrice = ind.twoDayLow;
                if (ind.ma200 > stopPrice) stopPrice = ind.ma200;
            } else {
                stopPrice = ind.twoDayHigh;
                if (ind.ma200 < stopPrice) stopPrice = ind.ma200;
            }
            break;
            
        case 1:  // Yesterday high/low
            stopPrice = (type == BUY) ? ind.yesterdayLow : ind.yesterdayHigh;
            break;
            
        case 2:  // Two-day high/low
            stopPrice = (type == BUY) ? ind.twoDayLow : ind.twoDayHigh;
            break;
            
        case 3:  // MA200
            stopPrice = ind.ma200;
            break;
            
        case 4:  // Bollinger Band stop
            stopPrice = ind.bbStopPrice;
            break;
            
        case 5:  // ATR-based
            stopPrice = ind.dailyATR;
            break;
            
        default:
            return -1;  // No change
    }
    
    // Convert price to distance from current price
    if (type == BUY) {
        return fabs(pParams->bidAsk.ask[0] - stopPrice);
    } else {
        return fabs(stopPrice - pParams->bidAsk.bid[0]);
    }
}
```

### Phase 10: Complete Strategy Structure

```cpp
extern "C" {

AsirikuyReturnCode runOrderMgmtStrategy(StrategyParams* pParams) {
    using LogFmt = NumericLogFormatter;
    
    // Null check
    if (pParams == NULL) {
        pantheios_logputs(PANTHEIOS_SEV_CRITICAL,
            (PAN_CHAR_T*)"runOrderMgmtStrategy: pParams = NULL");
        return NULL_POINTER;
    }
    
    // Initialize EasyTrade
    if (initEasyTradeLibrary(pParams) != SUCCESS) {
        return FAILURE;
    }
    
    // Load indicators
    StrategyIndicators indicators = loadIndicators(pParams);
    
    // Log current state
    int instanceId = (int)pParams->settings[STRATEGY_INSTANCE_ID];
    pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL,
        (PAN_CHAR_T*)"OrderMgmt[%s]: dailyATR=%s, buyOrders=%s, sellOrders=%s",
        LogFmt::integer(instanceId),
        LogFmt::decimal(indicators.dailyATR, 5),
        LogFmt::integer(buyOrdersCount()),
        LogFmt::integer(sellOrdersCount()));
    
    // Exit logic (if applicable)
    AsirikuyReturnCode result = handleExits(pParams, indicators);
    if (result != SUCCESS) {
        return result;
    }
    
    // Modification logic
    result = modifyPositions(pParams, indicators);
    if (result != SUCCESS) {
        return result;
    }
    
    // Entry logic (if strategy also enters)
    result = handleEntries(pParams, indicators);
    if (result != SUCCESS) {
        return result;
    }
    
    return SUCCESS;
}

} // extern "C"
```

---

## Complete Example: TakeOver Strategy (Dynamic Stop Loss)

```cpp
/* TakeOverStrategy.cpp - Manages manual orders with dynamic stops */
#include "Precompiled.h"
#include "StrategyTypes.h"
#include "AsirikuyDefines.h"
#include "OrderManagement.h"
#include "EasyTradeCWrapper.hpp"
#include "NumericLoggingHelpers.hpp"
#include "Logging.h"

constexpr BOOL USE_INTERNAL_SL = FALSE;  // Use broker SL
constexpr BOOL USE_INTERNAL_TP = FALSE;  // Use broker TP

enum AdditionalSettings {
    BBS_PERIOD = ADDITIONAL_PARAM_1,
    BBS_DEVIATION = ADDITIONAL_PARAM_2,
    BBS_ADJUST_POINTS = ADDITIONAL_PARAM_3,
    POSITION = ADDITIONAL_PARAM_4,
    DSL_TYPE = ADDITIONAL_PARAM_7
};

enum ExitDslTypes {
    EXIT_DSL_SMART = 0,      // Max of 2-day low and MA200
    EXIT_DSL_1DAY_HL = 1,    // Yesterday high/low
    EXIT_DSL_2DAY_HL = 2,    // 2-day high/low
    EXIT_DSL_MA200 = 3,      // Hourly 200 MA
    EXIT_DSL_BBS = 4,        // Bollinger Band stop
    EXIT_DSL_ATR = 5         // Daily ATR
};

struct TakeOverIndicators {
    double position;
    int bbsTrend;
    double bbsStopPrice;
    int bbsIndex;
    double dailyATR;
    double yesterdayHigh;
    double yesterdayLow;
    double yesterdayClose;
    double twoDayHigh;
    double twoDayLow;
    double ma200;
    double buyStopLossPrice;
    double sellStopLossPrice;
    double adjustPoints;
    int dslType;
};

static TakeOverIndicators loadIndicators(StrategyParams* pParams) {
    TakeOverIndicators ind = {};
    
    // ATR
    ind.dailyATR = iAtr(DAILY_RATES, (int)parameter(ATR_AVERAGING_PERIOD), 1);
    
    // Bollinger Band stop
    iBBandStop(PRIMARY_RATES, (int)parameter(BBS_PERIOD),
               parameter(BBS_DEVIATION), &ind.bbsTrend,
               &ind.bbsStopPrice, &ind.bbsIndex);
    
    // Position tracking
    ind.position = parameter(POSITION);
    
    // Daily levels
    ind.yesterdayHigh = iHigh(DAILY_RATES, 1);
    ind.yesterdayLow = iLow(DAILY_RATES, 1);
    ind.yesterdayClose = iClose(DAILY_RATES, 1);
    
    // 2-day high/low
    ind.twoDayHigh = fmax(iHigh(DAILY_RATES, 1), iHigh(DAILY_RATES, 2));
    ind.twoDayLow = fmin(iLow(DAILY_RATES, 1), iLow(DAILY_RATES, 2));
    
    // Moving average
    ind.ma200 = iMA(3, HOURLY_RATES, 200, 1);
    
    // Settings
    ind.adjustPoints = parameter(BBS_ADJUST_POINTS);
    ind.dslType = (int)parameter(DSL_TYPE);
    
    // Calculate dynamic stops based on mode
    switch (ind.dslType) {
        case EXIT_DSL_SMART:
            ind.buyStopLossPrice = ind.twoDayLow;
            if (ind.ma200 > ind.buyStopLossPrice)
                ind.buyStopLossPrice = ind.ma200;
            
            ind.sellStopLossPrice = ind.twoDayHigh;
            if (ind.ma200 < ind.sellStopLossPrice)
                ind.sellStopLossPrice = ind.ma200;
            break;
            
        case EXIT_DSL_1DAY_HL:
            ind.buyStopLossPrice = ind.yesterdayLow;
            ind.sellStopLossPrice = ind.yesterdayHigh;
            break;
            
        case EXIT_DSL_2DAY_HL:
            ind.buyStopLossPrice = ind.twoDayLow;
            ind.sellStopLossPrice = ind.twoDayHigh;
            break;
            
        case EXIT_DSL_MA200:
            ind.buyStopLossPrice = ind.sellStopLossPrice = ind.ma200;
            break;
            
        case EXIT_DSL_BBS:
            ind.buyStopLossPrice = ind.sellStopLossPrice = ind.bbsStopPrice;
            break;
            
        case EXIT_DSL_ATR:
            ind.buyStopLossPrice = ind.sellStopLossPrice = ind.dailyATR;
            break;
            
        default:
            ind.buyStopLossPrice = ind.sellStopLossPrice = -1;  // No change
            break;
    }
    
    return ind;
}

static AsirikuyReturnCode modifyPositions(StrategyParams* pParams,
                                         const TakeOverIndicators& ind) {
    using LogFmt = NumericLogFormatter;
    
    // Modify long positions
    if (totalOpenOrders(pParams, BUY) > 0 && ind.buyStopLossPrice > 0) {
        double stopLoss = fabs(pParams->bidAsk.ask[0] - ind.buyStopLossPrice 
                              + ind.adjustPoints);
        
        AsirikuyReturnCode result = modifyTradeEasy(BUY, -1, stopLoss, -1);
        if (result != SUCCESS) {
            return logAsirikuyError("modifyTradeEasy(BUY)", result);
        }
        
        pantheios_logprintf(PANTHEIOS_SEV_DEBUG,
            (PAN_CHAR_T*)"Modified BUY SL: %s",
            LogFmt::decimal(stopLoss, 5));
    }
    
    // Modify short positions
    if (totalOpenOrders(pParams, SELL) > 0 && ind.sellStopLossPrice > 0) {
        double stopLoss = fabs(ind.sellStopLossPrice - pParams->bidAsk.bid[0]
                              + ind.adjustPoints);
        
        AsirikuyReturnCode result = modifyTradeEasy(SELL, -1, stopLoss, -1);
        if (result != SUCCESS) {
            return logAsirikuyError("modifyTradeEasy(SELL)", result);
        }
        
        pantheios_logprintf(PANTHEIOS_SEV_DEBUG,
            (PAN_CHAR_T*)"Modified SELL SL: %s",
            LogFmt::decimal(stopLoss, 5));
    }
    
    return SUCCESS;
}

static AsirikuyReturnCode handleTrendReversalExits(StrategyParams* pParams,
                                                   const TakeOverIndicators& ind) {
    // Exit on trend reversal (1M BBS only)
    if (ind.dslType != EXIT_DSL_BBS) {
        return SUCCESS;
    }
    
    if ((int)pParams->settings[TIMEFRAME] != 1) {
        return SUCCESS;  // Only for 1-minute charts
    }
    
    int shift1Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 2;
    
    // Close longs on bearish reversal
    if (totalOpenOrders(pParams, BUY) > 0 && 
        ind.bbsTrend == -1 && 
        ind.bbsIndex == shift1Index &&
        pParams->bidAsk.ask[0] >= ind.position) {
        
        AsirikuyReturnCode result = closeAllLongs();
        if (result != SUCCESS) {
            return logAsirikuyError("closeAllLongs", result);
        }
        
        pantheios_logputs(PANTHEIOS_SEV_INFORMATIONAL,
            (PAN_CHAR_T*)"Closed longs on BBS reversal");
        return SUCCESS;
    }
    
    // Close shorts on bullish reversal
    if (totalOpenOrders(pParams, SELL) > 0 &&
        ind.bbsTrend == 1 &&
        ind.bbsIndex == shift1Index &&
        pParams->bidAsk.bid[0] <= ind.position) {
        
        AsirikuyReturnCode result = closeAllShorts();
        if (result != SUCCESS) {
            return logAsirikuyError("closeAllShorts", result);
        }
        
        pantheios_logputs(PANTHEIOS_SEV_INFORMATIONAL,
            (PAN_CHAR_T*)"Closed shorts on BBS reversal");
        return SUCCESS;
    }
    
    return SUCCESS;
}

extern "C" {

AsirikuyReturnCode runTakeOverStrategy(StrategyParams* pParams) {
    using LogFmt = NumericLogFormatter;
    
    if (pParams == NULL) {
        pantheios_logputs(PANTHEIOS_SEV_CRITICAL,
            (PAN_CHAR_T*)"runTakeOverStrategy: pParams = NULL");
        return NULL_POINTER;
    }
    
    // Initialize EasyTrade
    if (initEasyTradeLibrary(pParams) != SUCCESS) {
        return FAILURE;
    }
    
    // Load indicators
    TakeOverIndicators indicators = loadIndicators(pParams);
    
    // Log state
    int instanceId = (int)pParams->settings[STRATEGY_INSTANCE_ID];
    pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL,
        (PAN_CHAR_T*)"TakeOver[%s]: BBSTrend=%s, BBSStop=%s, DSL=%s",
        LogFmt::integer(instanceId),
        LogFmt::integer(indicators.bbsTrend),
        LogFmt::decimal(indicators.bbsStopPrice, 5),
        LogFmt::integer(indicators.dslType));
    
    pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL,
        (PAN_CHAR_T*)"TakeOver[%s]: Buy=%s, Sell=%s, BuySLP=%s, SellSLP=%s",
        LogFmt::integer(instanceId),
        LogFmt::integer(buyOrdersCount()),
        LogFmt::integer(sellOrdersCount()),
        LogFmt::decimal(indicators.buyStopLossPrice, 5),
        LogFmt::decimal(indicators.sellStopLossPrice, 5));
    
    // Check for trend reversal exits (1M BBS only)
    AsirikuyReturnCode result = handleTrendReversalExits(pParams, indicators);
    if (result != SUCCESS) {
        return result;
    }
    
    // Modify positions with dynamic stops
    result = modifyPositions(pParams, indicators);
    if (result != SUCCESS) {
        return result;
    }
    
    return SUCCESS;
}

} // extern "C"
```

---

## Common Pitfalls & Solutions

### Pitfall 1: Modifying Non-Existent Orders

**Problem:**
```cpp
// Always trying to modify without checking if orders exist
modifyTradeEasy(BUY, -1, newStopLoss, -1);  // Fails if no longs
```

**Solution:**
```cpp
if (totalOpenOrders(pParams, BUY) > 0) {
    modifyTradeEasy(BUY, -1, newStopLoss, -1);
}
```

### Pitfall 2: Wrong Stop Loss Calculation

**Problem:**
```cpp
// Using absolute price instead of distance
double stopLossPrice = 1.1950;
modifyTradeEasy(BUY, -1, stopLossPrice, -1);  // WRONG!
```

**Solution:**
```cpp
// Calculate distance from current price
double stopLossPrice = 1.1950;
double currentPrice = pParams->bidAsk.ask[0];
double stopLossDistance = fabs(currentPrice - stopLossPrice);
modifyTradeEasy(BUY, -1, stopLossDistance, -1);  // CORRECT
```

### Pitfall 3: Ignoring Spread for SELL Orders

**Problem:**
```cpp
// Using ASK price for SELL stop calculation
double stopLoss = fabs(stopLossPrice - pParams->bidAsk.ask[0]);  // WRONG
```

**Solution:**
```cpp
// Use BID for SELL orders, ASK for BUY orders
if (orderType == BUY) {
    stopLoss = fabs(pParams->bidAsk.ask[0] - stopLossPrice);
} else {
    stopLoss = fabs(stopLossPrice - pParams->bidAsk.bid[0]);
}
```

### Pitfall 4: Not Checking Return Codes

**Problem:**
```cpp
modifyTradeEasy(BUY, -1, stopLoss, -1);  // Ignoring errors
```

**Solution:**
```cpp
AsirikuyReturnCode result = modifyTradeEasy(BUY, -1, stopLoss, -1);
if (result != SUCCESS) {
    return logAsirikuyError("modifyTradeEasy(BUY)", result);
}
```

### Pitfall 5: Moving Stop Loss Backward

**Problem:**
```cpp
// Always updating SL without checking if it's favorable
modifyTradeEasy(BUY, -1, newStopLoss, -1);  // Might move SL away from price
```

**Solution:**
```cpp
// Use stopMovingbackSL parameter in advanced modify functions
modifyTradeEasy_new(BUY, orderTicket, stopLoss, takeProfit, tpMode, TRUE);
// TRUE = prevent moving SL backward (away from profit)
```

---

## Performance Considerations

1. **Check Order Count First:** Always check if orders exist before calling modify functions
2. **Batch Modifications:** Use `orderTicket=-1` to modify all orders at once
3. **Avoid Excessive Logging:** Use DEBUG level for per-tick logs
4. **Cache Indicator Values:** Calculate indicators once per tick, not per order

---

## Migration Time Estimate

**Order Management Strategy (TakeOver-like):**
- Setup & structure: 15 minutes
- Indicator calculation: 45 minutes
- Entry logic: 30 minutes
- Modification logic: 1 hour
- Exit logic: 45 minutes
- Testing & validation: 1 hour
- **Total: ~4 hours**

**Factors That Increase Time:**
- Complex entry conditions (+1 hour)
- Multiple exit strategies (+30 min per strategy)
- Partial exit logic (+1 hour)
- Breakeven management (+30 min)
- Elliptical stops (+45 min)

---

## Next Steps After Migration

1. **Test Modification Logic:** Verify SL/TP updates happen correctly
2. **Validate Risk Calculations:** Ensure position sizing matches expected risk
3. **Monitor Slippage:** Track actual vs expected execution prices
4. **Backtest Extensively:** Compare with original C strategy results
5. **Paper Trade First:** Test in demo before live deployment
