# Simple Strategy Migration Template

**Target Audience**: Strategies that primarily iterate over bars and emit UI values without complex order management.

**Examples**: RecordBars, simple indicator calculators, data collection strategies.

**Estimated Complexity**: Low (1-2 hours per strategy)

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
  #include "EasyTradeCWrapper.hpp"
  #include "StrategyUserInterface.h"
  #include "NumericLoggingHelpers.hpp"
  ```

### Phase 2: Function Signature Conversion

**Old C Signature:**
```c
AsirikuyReturnCode runRecordBars(StrategyParams* pParams) {
    // Implementation
}
```

**New C++ Signature:**
```cpp
extern "C" {
    AsirikuyReturnCode runRecordBarsStrategy(StrategyParams* pParams) {
        // Implementation
    }
}
```

**Key Points:**
- Keep `extern "C"` wrapper for framework compatibility
- Rename function to avoid conflicts (append "Strategy" or use namespace)
- Return type stays `AsirikuyReturnCode`
- Parameter stays `StrategyParams*`

### Phase 3: Initialization

- [ ] Initialize EasyTrade library (if using EasyTrade functions)
  ```cpp
  extern "C" {
  AsirikuyReturnCode runYourStrategy(StrategyParams* pParams) {
      // Initialize EasyTrade for MT4-style accessors
      if (initEasyTradeLibrary(pParams) != SUCCESS) {
          return FAILURE;
      }
      
      // Get bars count
      int barsTotal = barsCount(PRIMARY_RATES);
      if (barsTotal < MIN_BARS_REQUIRED) {
          return SUCCESS;  // Not enough data yet
      }
  ```

- [ ] Validate bars if using multiple timeframes
  ```cpp
      // Validate daily bars synchronized with primary
      if (validateDailyBarsEasy(pParams, PRIMARY_RATES, DAILY_RATES) != SUCCESS) {
          pantheios_logputs(PANTHEIOS_SEV_WARNING,
              (PAN_CHAR_T*)"Daily bars not synchronized");
          return FAILURE;
      }
  ```

### Phase 4: Bar Data Access Migration

**Option A: Direct Access to Rates Structure (Recommended for Performance)**

```cpp
// Get rates structure
const RatesBuffers* ratesBuffers = pParams->ratesBuffers;
const Rates& primaryRates = ratesBuffers->rates[PRIMARY_RATES];

// Access bar data
int arraySize = primaryRates.info.arraySize;
for (int i = 0; i < arraySize; i++) {
    time_t barTime = primaryRates.time[i];
    double barOpen = primaryRates.open[i];
    double barHigh = primaryRates.high[i];
    double barLow = primaryRates.low[i];
    double barClose = primaryRates.close[i];
    double barVolume = primaryRates.volume[i];
    
    // Process bar data...
}
```

**Option B: EasyTrade MT4-Style Accessors (Simpler Syntax)**

```cpp
// Current bar is shift=0, previous bar is shift=1
int barsTotal = barsCount(PRIMARY_RATES);

for (int shift = barsTotal - 1; shift >= 0; shift--) {
    time_t barTime = openTime(shift);
    double barOpen = cOpen(shift);
    double barHigh = high(shift);
    double barLow = low(shift);
    double barClose = cClose(shift);
    double barVolume = volume(shift);
    
    // Or multi-timeframe version
    double dailyClose = iClose(DAILY_RATES, shift);
    double hourlyHigh = iHigh(HOURLY_RATES, shift);
}
```

**Migration Notes:**
- **Old**: `rates->rates[0].barsTotal` → **New**: `barsCount(PRIMARY_RATES)` or `rates[PRIMARY_RATES].info.arraySize`
- **Old**: `rates->rates[0].timeArray[i]` → **New**: `rates[PRIMARY_RATES].time[i]` or `openTime(shift)`
- **Old**: `rates->rates[0].closeArray[i]` → **New**: `rates[PRIMARY_RATES].close[i]` or `cClose(shift)`

### Phase 5: Indicator Calculations

- [ ] Replace indicator calculations with EasyTrade wrappers

**Common Indicators:**

```cpp
// ATR (Average True Range)
double atr14 = iAtr(PRIMARY_RATES, 14, 1);  // 14-period ATR, previous bar

// Moving Average
double ma50 = iMA(3, PRIMARY_RATES, 50, 1);  // 50-period MA on close[1]
// MA type: 0=open, 1=high, 2=low, 3=close

// RSI
double rsi14 = iRSI(PRIMARY_RATES, 14, 1);

// MACD
double macdMain, macdSignal, macdHist;
iMACDAll(PRIMARY_RATES, 12, 26, 9, 1, &macdMain, &macdSignal, &macdHist);

// Bollinger Bands
double bbUpper = iBBands(PRIMARY_RATES, 20, 2.0, 2, 1);  // signal=2 for upper
double bbMiddle = iBBands(PRIMARY_RATES, 20, 2.0, 1, 1); // signal=1 for middle
double bbLower = iBBands(PRIMARY_RATES, 20, 2.0, 0, 1);  // signal=0 for lower

// Stochastic
double stochK = iSTO(PRIMARY_RATES, 5, 3, 3, 0, 1);  // signal=0 for K
double stochD = iSTO(PRIMARY_RATES, 5, 3, 3, 1, 1);  // signal=1 for D

// CCI
double cci14 = iCCI(PRIMARY_RATES, 14, 1);

// Standard Deviation
double stdDev = iStdev(PRIMARY_RATES, 3, 20, 1);  // 20-period stddev on close

// Range Average (custom)
double rangeAvg = iRangeAverage(PRIMARY_RATES, 10, 1);

// Pivot Points
double pivot, s1, r1, s2, r2, s3, r3;
iPivot(DAILY_RATES, 1, &pivot, &s1, &r1, &s2, &r2, &s3, &r3);
```

**Multi-Timeframe Access:**
```cpp
// Daily values from hourly chart
double dailyATR = iAtr(DAILY_RATES, 14, 1);
double weeklyATR = iAtr(DAILY_RATES, 70, 1);  // 14 periods * 5 days

// Daily ATR approximation from intraday bars
double approxDailyATR = iAtrWholeDaysSimple(PRIMARY_RATES, 14);
```

### Phase 6: UI Value Emission

- [ ] **Do NOT manually call `addValueToUI()` in strategy code**
- [ ] Rely on `harvestStrategyTelemetry()` automatic harvesting (already integrated in `runStrategy()`)
- [ ] Only emit strategy-specific diagnostic values if needed

**Standard Telemetry (Auto-Harvested):**
- `executionTrend`, `entrySignal`, `exitSignal`
- `risk`, `splitTradeMode`, `tpMode`
- `entryPrice`, `stopLossPrice`
- `atr_euro_range`, `pWeeklyPredictATR`, `pDailyMaxATR`
- `strategyInstanceId`

**Custom Diagnostic Values (Optional):**
```cpp
// Only if strategy needs additional custom metrics
using LogFmt = NumericLogFormatter;

// Log diagnostic values
pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL,
    (PAN_CHAR_T*)"RecordBars: barCount=%s, avgRange=%s",
    LogFmt::integer(barsTotal),
    LogFmt::decimal(avgRange, 4));
```

**Late Overwrite Values (Handled by Framework):**
These are automatically refreshed after order management:
- `strategyRisk`, `riskPNL`, `StrategyVolRisk`
- `weeklyATR`, `dailyATR`, `AccountRisk`
- Predictive ATR proxies

### Phase 7: Error Handling & Logging

- [ ] Use `NumericLoggingHelpers` for formatted logging

```cpp
using LogFmt = NumericLogFormatter;

// Integer logging
pantheios_logprintf(PANTHEIOS_SEV_DEBUG,
    (PAN_CHAR_T*)"Bar count: %s", LogFmt::integer(barsTotal));

// Decimal logging with precision
pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL,
    (PAN_CHAR_T*)"ATR: %s, MA: %s",
    LogFmt::decimal(atr14, 5),
    LogFmt::decimal(ma50, 5));

// Boolean logging
pantheios_logprintf(PANTHEIOS_SEV_DEBUG,
    (PAN_CHAR_T*)"Is backtest: %s",
    LogFmt::boolean(pParams->settings[IS_BACKTESTING]));
```

- [ ] Check return codes from all functions
```cpp
AsirikuyReturnCode result = validateDailyBarsEasy(pParams, PRIMARY_RATES, DAILY_RATES);
if (result != SUCCESS) {
    pantheios_logprintf(PANTHEIOS_SEV_ERROR,
        (PAN_CHAR_T*)"Bar validation failed: %s",
        LogFmt::integer(result));
    return result;
}
```

### Phase 8: Parameter Access

- [ ] Use settings array with enum indices

```cpp
// Access strategy parameters
int instanceId = (int)pParams->settings[STRATEGY_INSTANCE_ID];
int timeframe = (int)pParams->settings[TIMEFRAME];
BOOL isBacktest = (BOOL)pParams->settings[IS_BACKTESTING];

// Risk management
double accountRisk = pParams->settings[ACCOUNT_RISK_PERCENT];
int maxOrders = (int)pParams->settings[MAX_OPEN_ORDERS];

// Strategy-specific parameters (indices 0-39)
double customParam1 = pParams->settings[ADDITIONAL_PARAM_1];
double customParam2 = pParams->settings[ADDITIONAL_PARAM_2];
// ... up to ADDITIONAL_PARAM_40

// Or use EasyTrade shorthand
double param1 = parameter(ADDITIONAL_PARAM_1);
```

### Phase 9: Time Functions

```cpp
// Current bar time components
int currentHour = hour();
int currentMinute = minute();
int currentDayOfWeek = dayOfWeek();  // 0=Sunday, 1=Monday, etc.
int currentMonth = month();
int currentYear = year();

// Time-based filtering
if (dayOfWeek() == 5 && hour() >= 15) {
    // Friday after 3 PM - avoid new entries
    return SUCCESS;
}

// Symbol properties
int period = Period();      // Timeframe in minutes
int digits = Digits();      // Decimal places
double currentSpread = spread();
```

### Phase 10: Testing & Validation

- [ ] Compile strategy
  ```powershell
  cd e:\workspace\AutoBBS
  premake4 vs2010
  # Open build/vs2010/AsirikuyFramework.sln
  # Build TradingStrategies project
  ```

- [ ] Create unit test (optional but recommended)
  ```cpp
  // In dev/TradingStrategies/tests/YourStrategyTests.cpp
  #include "Precompiled.h"
  #include "YourStrategy.h"
  
  bool testBasicExecution() {
      // Create stub StrategyParams
      // Call strategy function
      // Assert expected outcomes
      return true;
  }
  ```

- [ ] Test in backtest mode first
- [ ] Verify UI values match expected outputs
- [ ] Compare results with original C strategy

---

## Complete Example: RecordBars Strategy

```cpp
/* RecordBarsStrategy.cpp - Simple bar recording strategy */
#include "Precompiled.h"
#include "StrategyTypes.h"
#include "AsirikuyDefines.h"
#include "EasyTradeCWrapper.hpp"
#include "NumericLoggingHelpers.hpp"

extern "C" {

AsirikuyReturnCode runRecordBarsStrategy(StrategyParams* pParams) {
    using LogFmt = NumericLogFormatter;
    
    // Initialize EasyTrade library
    if (initEasyTradeLibrary(pParams) != SUCCESS) {
        pantheios_logputs(PANTHEIOS_SEV_ERROR,
            (PAN_CHAR_T*)"Failed to initialize EasyTrade library");
        return FAILURE;
    }
    
    // Get bar count
    int barsTotal = barsCount(PRIMARY_RATES);
    if (barsTotal < 10) {
        // Not enough data yet
        return SUCCESS;
    }
    
    // Calculate statistics over last 100 bars
    int lookback = (barsTotal < 100) ? barsTotal : 100;
    double sumRange = 0.0;
    double sumBody = 0.0;
    int bullishCount = 0;
    int bearishCount = 0;
    
    for (int shift = 1; shift <= lookback; shift++) {
        double barOpen = cOpen(shift);
        double barHigh = high(shift);
        double barLow = low(shift);
        double barClose = cClose(shift);
        
        // Calculate range
        double barRange = barHigh - barLow;
        sumRange += barRange;
        
        // Calculate body
        double barBody = barClose - barOpen;
        sumBody += fabs(barBody);
        
        // Count direction
        if (barClose > barOpen) {
            bullishCount++;
        } else if (barClose < barOpen) {
            bearishCount++;
        }
    }
    
    // Calculate averages
    double avgRange = sumRange / lookback;
    double avgBody = sumBody / lookback;
    double bullishPercent = (bullishCount * 100.0) / lookback;
    
    // Calculate ATR for comparison
    double atr14 = iAtr(PRIMARY_RATES, 14, 1);
    
    // Log statistics (telemetry harvesting will handle UI emission)
    pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL,
        (PAN_CHAR_T*)"RecordBars[%s]: bars=%s, avgRange=%s, avgBody=%s, bullish=%s%%, ATR14=%s",
        LogFmt::integer((int)pParams->settings[STRATEGY_INSTANCE_ID]),
        LogFmt::integer(barsTotal),
        LogFmt::decimal(avgRange, 5),
        LogFmt::decimal(avgBody, 5),
        LogFmt::decimal(bullishPercent, 1),
        LogFmt::decimal(atr14, 5));
    
    // No trading signals - this is data collection only
    pParams->results[0].tradingSignals = NO_SIGNAL;
    
    return SUCCESS;
}

} // extern "C"
```

---

## Common Pitfalls & Solutions

### Pitfall 1: Accessing Forming Bar Data

**Problem:**
```cpp
// DON'T: Using shift=0 for signal calculations (repainting risk)
double currentClose = cClose(0);
if (currentClose > threshold) {
    // Signal generated, but will change as bar forms
}
```

**Solution:**
```cpp
// DO: Use shift=1 for completed bars
double previousClose = cClose(1);
if (previousClose > threshold) {
    // Signal based on closed bar (stable)
}
```

### Pitfall 2: Incorrect Rate Index

**Problem:**
```cpp
// Using wrong constant or hardcoded index
double dailyClose = iClose(1, 0);  // Assumes DAILY_RATES=1
```

**Solution:**
```cpp
// Use named constants
double dailyClose = iClose(DAILY_RATES, 1);  // Explicit, readable
```

### Pitfall 3: Not Checking Bar Count

**Problem:**
```cpp
// Accessing bars without checking count
double ma50 = iMA(3, PRIMARY_RATES, 50, 1);  // Crashes if < 51 bars
```

**Solution:**
```cpp
int barsTotal = barsCount(PRIMARY_RATES);
if (barsTotal < 51) {
    return SUCCESS;  // Not enough data
}
double ma50 = iMA(3, PRIMARY_RATES, 50, 1);
```

### Pitfall 4: Manual UI Emission

**Problem:**
```cpp
// Old pattern: manually emitting every value
addValueToUI("avgRange", avgRange);
addValueToUI("avgBody", avgBody);
// ... repeating for every metric
```

**Solution:**
```cpp
// Framework handles standard telemetry automatically
// Only emit truly custom diagnostic values if needed
// Most strategies don't need manual UI calls anymore
```

### Pitfall 5: Ignoring Return Codes

**Problem:**
```cpp
initEasyTradeLibrary(pParams);  // Ignoring return value
```

**Solution:**
```cpp
if (initEasyTradeLibrary(pParams) != SUCCESS) {
    return FAILURE;
}
```

---

## Performance Considerations

1. **Direct Array Access vs EasyTrade Functions:**
   - Direct: `rates[PRIMARY_RATES].close[i]` - Fastest (no function call overhead)
   - EasyTrade: `cClose(shift)` - Slightly slower but more readable

2. **Indicator Caching:**
   ```cpp
   // DON'T: Recalculate indicator every bar in loop
   for (int shift = 1; shift <= 100; shift++) {
       double atr = iAtr(PRIMARY_RATES, 14, shift);  // Expensive!
   }
   
   // DO: Calculate once if value doesn't change
   double currentATR = iAtr(PRIMARY_RATES, 14, 1);
   // Use cached value in loop
   ```

3. **Logging Volume:**
   - Use `PANTHEIOS_SEV_DEBUG` for per-tick logs (can be disabled in production)
   - Use `PANTHEIOS_SEV_INFORMATIONAL` for significant events only
   - Avoid excessive logging in tight loops

---

## Migration Time Estimate

**Simple Strategy (RecordBars-like):**
- Setup & structure: 15 minutes
- Bar data access migration: 30 minutes
- Indicator calculations: 30 minutes
- Testing & validation: 45 minutes
- **Total: ~2 hours**

**Factors That Increase Time:**
- Multiple timeframes (+30 min)
- Custom indicators (+1 hour per indicator)
- Complex statistical calculations (+1 hour)
- Extensive logging requirements (+30 min)

---

## Next Steps After Migration

1. **Add to Strategy Registry** (in `AsirikuyStrategies.c`):
   ```c
   case STRATEGY_RECORDBARS:
       result = runRecordBarsStrategy(pParams);
       break;
   ```

2. **Update Configuration** (in `AsirikuyConfig.xml`):
   ```xml
   <Strategy>
       <Name>RecordBars</Name>
       <StrategyId>STRATEGY_RECORDBARS</StrategyId>
       <!-- ... parameters -->
   </Strategy>
   ```

3. **Create Documentation** for strategy-specific parameters

4. **Backtest** against historical data to verify behavior matches original

5. **Code Review** before deploying to live/demo environments
