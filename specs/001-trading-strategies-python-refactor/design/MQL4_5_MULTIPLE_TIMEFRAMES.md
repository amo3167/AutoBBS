# MQL4/5 Multiple Timeframe Support

## Overview

MQL4/5 **fully supports multiple timeframes** through a flexible rate array system. The Framework can receive **up to 10 different timeframes** per strategy, each with its own symbol and timeframe configuration.

---

## Part 1: Architecture - Multiple Rate Arrays

### Structure: Up to 10 Rate Arrays

The Framework supports **up to 10 rate arrays** per strategy, each representing a different symbol/timeframe combination:

```mql4
// Defines.mqh
#define TOTAL_RATES_ARRAYS 10  // Maximum 10 rate arrays per strategy
```

**Rate Arrays:**
- `rates_0[]` - Rate array 0 (typically main strategy timeframe)
- `rates_1[]` - Rate array 1 (e.g., Daily for ATR calculation)
- `rates_2[]` - Rate array 2 (e.g., Weekly for trend analysis)
- `rates_3[]` through `rates_9[]` - Additional rate arrays

### Rate Information Structure

Each rate array has associated metadata in `ratesInformation[][]`:

```mql4
// ratesInformation[strategyIndex][rateArrayIndex][IDX_*]
ratesInformation[0][0][IDX_IS_ENABLED]         = true;   // Enable this rate array
ratesInformation[0][0][IDX_REQUIRED_TIMEFRAME] = 60;     // Required timeframe (1H = 60 minutes)
ratesInformation[0][0][IDX_ACTUAL_TIMEFRAME]   = 60;     // Actual timeframe used
ratesInformation[0][0][IDX_TOTAL_BARS_REQUIRED] = 500;   // Number of bars needed
ratesInformation[0][0][IDX_RATES_ARRAY_SIZE]   = 500;    // Actual array size
ratesInformation[0][0][IDX_POINT]              = 0.0001; // Point value
ratesInformation[0][0][IDX_DIGITS_RATES]       = 4;      // Decimal digits
```

**Key Fields:**
- `IDX_IS_ENABLED`: Enable/disable this rate array
- `IDX_REQUIRED_TIMEFRAME`: The timeframe the strategy needs (e.g., 60 = 1H, 240 = 4H, 1440 = Daily)
- `IDX_ACTUAL_TIMEFRAME`: The timeframe actually used (may differ for normalization)
- `IDX_TOTAL_BARS_REQUIRED`: Minimum number of bars needed
- `IDX_RATES_ARRAY_SIZE`: Actual number of bars copied

---

## Part 2: Implementation in EA Strategies

### Example: Kantu.mq4 (Multiple Timeframes)

**Kantu.mq4** implements **2 timeframes**:

```mql4
// Kantu.mq4 - initialize() function (lines 171-178)
bool initialize(...)
{
    // Rate Array 0: Main strategy timeframe (e.g., 1H, 4H, Daily)
    StringToCharArray(Symbol(), ratesSymbols[0][0].a);
    ratesInformation[0][0][IDX_IS_ENABLED] = true;
    ratesInformation[0][0][IDX_REQUIRED_TIMEFRAME] = STRATEGY_TIMEFRAME;  // e.g., 60 (1H)
    ratesInformation[0][0][IDX_TOTAL_BARS_REQUIRED] = ATR_AVERAGING_PERIOD * 2 * 1440 / STRATEGY_TIMEFRAME;
    
    // Rate Array 1: Daily timeframe (for ATR calculation)
    StringToCharArray(Symbol(), ratesSymbols[0][1].a);
    ratesInformation[0][1][IDX_IS_ENABLED] = true;
    ratesInformation[0][1][IDX_REQUIRED_TIMEFRAME] = PERIOD_D1;  // 1440 = Daily
    ratesInformation[0][1][IDX_TOTAL_BARS_REQUIRED] = 100;
    
    // Rate Arrays 2-9: Not used (disabled by default)
    // ratesInformation[0][2][IDX_IS_ENABLED] = false;
    // ...
}
```

**What This Means:**
- **rates_0[]**: Main strategy timeframe (e.g., 1H bars for EURUSD)
- **rates_1[]**: Daily timeframe (e.g., Daily bars for EURUSD)
- Framework receives both timeframes and can use them for different purposes

### Example: Teyacanani.mq4 (Multiple Timeframes)

**Teyacanani.mq4** also uses **2 timeframes**:

```mql4
// Teyacanani.mq4 - initialize() function
// Rate Array 0: Main strategy timeframe
ratesInformation[0][0][IDX_REQUIRED_TIMEFRAME] = STRATEGY_TIMEFRAME;

// Rate Array 1: Daily timeframe (for ATR)
ratesInformation[0][1][IDX_REQUIRED_TIMEFRAME] = PERIOD_D1;
ratesInformation[0][1][IDX_TOTAL_BARS_REQUIRED] = ATR_AVERAGING_PERIOD + 10;
```

### Example: WatukushayFE_RSI.mq4 (Multiple Timeframes)

**WatukushayFE_RSI.mq4** uses **2 timeframes**:

```mql4
// WatukushayFE_RSI.mq4 - initialize() function
// Rate Array 0: Main strategy timeframe
ratesInformation[0][0][IDX_REQUIRED_TIMEFRAME] = STRATEGY_TIMEFRAME;

// Rate Array 1: Daily timeframe
ratesInformation[0][1][IDX_REQUIRED_TIMEFRAME] = PERIOD_D1;
ratesInformation[0][1][IDX_TOTAL_BARS_REQUIRED] = ATR_AVERAGING_PERIOD + 2;
```

### Common Pattern: Main Timeframe + Daily

**Most strategies use:**
- **rates_0[]**: Main strategy timeframe (1H, 4H, Daily, etc.)
- **rates_1[]**: Daily timeframe (for ATR calculation, volatility analysis)

**Why Daily?**
- ATR (Average True Range) is often calculated on Daily bars
- Volatility analysis requires Daily data
- Risk management uses Daily ATR for stop loss/take profit calculations

---

## Part 3: How Multiple Timeframes Are Passed to Framework

### Step 1: Copy Rates for Each Timeframe

**MQL4/5 copies rates for each enabled timeframe:**

```mql4
// Common.mq4 - c_copyRatesArrays() (lines 93-152)
void c_copyRatesArrays(int strategyIndex, 
                       charArray& ratesSymbols[][TOTAL_RATES_ARRAYS], 
                       double& ratesInformation[][TOTAL_RATES_ARRAYS][RATES_INFO_ARRAY_SIZE], 
                       MqlRates& rates_0[], 
                       MqlRates& rates_1[], 
                       // ... rates_2 through rates_9 ...
                       MqlRates& rates_9[])
{
    // Copy rates for array 0 (if enabled)
    if(ratesInformation[strategyIndex][0][IDX_IS_ENABLED] != false)
    {
        // Get total bars available for this timeframe
        ratesInformation[strategyIndex][0][IDX_RATES_ARRAY_SIZE] = 
            iBars(CharArrayToString(ratesSymbols[strategyIndex][0].a), 
                  ratesInformation[strategyIndex][0][IDX_ACTUAL_TIMEFRAME]);
        
        // Copy ALL bars for this symbol/timeframe
        CopyRates(CharArrayToString(ratesSymbols[strategyIndex][0].a), 
                  ratesInformation[strategyIndex][0][IDX_ACTUAL_TIMEFRAME], 
                  0,  // Start from most recent
                  ratesInformation[strategyIndex][0][IDX_RATES_ARRAY_SIZE],  // Copy all bars
                  rates_0);
    }
    
    // Copy rates for array 1 (if enabled)
    if(ratesInformation[strategyIndex][1][IDX_IS_ENABLED] != false)
    {
        ratesInformation[strategyIndex][1][IDX_RATES_ARRAY_SIZE] = 
            iBars(CharArrayToString(ratesSymbols[strategyIndex][1].a), 
                  ratesInformation[strategyIndex][1][IDX_ACTUAL_TIMEFRAME]);
        
        CopyRates(CharArrayToString(ratesSymbols[strategyIndex][1].a), 
                  ratesInformation[strategyIndex][1][IDX_ACTUAL_TIMEFRAME], 
                  0, 
                  ratesInformation[strategyIndex][1][IDX_RATES_ARRAY_SIZE], 
                  rates_1);
    }
    
    // Repeat for rates_2 through rates_9...
}
```

### Step 2: Pass All Rate Arrays to Framework

**All rate arrays are passed to Framework in `mql5_runStrategy()`:**

```mql4
// Common.mq4 - c_runStrategy() (lines 959-1030)
void c_runStrategy(int strategyIndex, ...)
{
    // ... prepare other data ...
    
    // Copy rates for all enabled timeframes
    c_copyRatesArrays(strategyIndex, ratesSymbols, ratesInformation, 
                      rates_0, rates_1, rates_2, rates_3, rates_4, 
                      rates_5, rates_6, rates_7, rates_8, rates_9);
    
    // Call Framework with all rate arrays
    mql5_runStrategy(
        settings,
        symbol,
        currency,
        brokerName,
        refBrokerName,
        brokerTime,
        ordersCount,
        orderInfo,
        accountInfo,
        bidAsk,
        ratesInfo,
        rates_0,  // Rate array 0 (main timeframe)
        rates_1,  // Rate array 1 (e.g., Daily)
        rates_2,  // Rate array 2 (e.g., Weekly)
        rates_3,  // Rate array 3
        rates_4,  // Rate array 4
        rates_5,  // Rate array 5
        rates_6,  // Rate array 6
        rates_7,  // Rate array 7
        rates_8,  // Rate array 8
        rates_9,  // Rate array 9
        results
    );
}
```

### Framework Function Signature

**Framework receives all rate arrays:**

```c
// AsirikuyFramework.mqh
int mql5_runStrategy(
    double&  inSettings[],
    charArray&  inTradeSymbol[],
    charArray&  inAccountCurrency[],
    charArray&  inBrokerName[],
    charArray&  inReferenceBrokerName[],
    int&     inCurrentBrokerTime[],
    int&     inOpenOrdersCount[],
    double&  inOrderInfo[][TOTAL_ORDER_INFO_INDEXES],
    double&  inAccountInfo[],
    double&  inBidAsk[],
    double&  inRatesInfo[][RATES_INFO_ARRAY_SIZE],
    MqlRates&  inRates_0[],  // Rate array 0
    MqlRates&  inRates_1[],  // Rate array 1
    MqlRates&  inRates_2[],  // Rate array 2
    MqlRates&  inRates_3[],  // Rate array 3
    MqlRates&  inRates_4[],  // Rate array 4
    MqlRates&  inRates_5[],  // Rate array 5
    MqlRates&  inRates_6[],  // Rate array 6
    MqlRates&  inRates_7[],  // Rate array 7
    MqlRates&  inRates_8[],  // Rate array 8
    MqlRates&  inRates_9[],  // Rate array 9
    double&  outResults[][RESULTS_ARRAY_SIZE]
);
```

---

## Part 4: Timeframe Normalization

### Automatic Timeframe Adjustment

**For timeframes greater than 1H, Framework uses 1H and normalizes:**

```mql4
// Common.mq4 - c_setCommonRatesInfoSettings() (lines 250-281)
void c_setCommonRatesInfoSettings(...)
{
    for(int i = 0; i < totalInternalStrategies; i++)
    {
        for(int j = 0; j < TOTAL_RATES_ARRAYS; j++)
        {
            if(ratesInformation[i][j][IDX_IS_ENABLED] == false)
                continue;
            
            // For strategy timeframes greater than 1 hour, 
            // the rates timeframe is reduced to 1 hour to enable normalization
            if(ratesInformation[i][j][IDX_REQUIRED_TIMEFRAME] > PERIOD_H1)  // > 60 minutes
            {
                ratesInformation[i][j][IDX_ACTUAL_TIMEFRAME] = PERIOD_H1;  // Use 1H instead
            }
            else
            {
                ratesInformation[i][j][IDX_ACTUAL_TIMEFRAME] = 
                    ratesInformation[i][j][IDX_REQUIRED_TIMEFRAME];
            }
        }
    }
}
```

**What This Means:**
- If strategy requires **4H** timeframe, Framework uses **1H** bars and normalizes them to 4H
- If strategy requires **Daily** timeframe, Framework uses **1H** bars and normalizes them to Daily
- Framework handles the normalization internally

**Why?**
- Ensures consistent bar alignment
- Handles weekend bars correctly
- Allows Framework to normalize to any required timeframe

---

## Part 5: Supported Timeframes

### Standard Timeframes

**MQL4/5 supports all standard timeframes:**

```mql4
// Timeframe constants (in minutes)
PERIOD_M1   = 1      // 1 minute
PERIOD_M5   = 5      // 5 minutes
PERIOD_M15  = 15     // 15 minutes
PERIOD_M30  = 30     // 30 minutes
PERIOD_H1   = 60     // 1 hour
PERIOD_H4   = 240    // 4 hours
PERIOD_D1   = 1440   // Daily
PERIOD_W1   = 10080  // Weekly
PERIOD_MN1  = 43200  // Monthly
```

### Timeframe Migration (MQL5)

**MQL5 uses `TFMigrate()` to convert MQL4 timeframe codes to MQL5 enums:**

```mql5
// Common.mq5 - TFMigrate() (lines 56-89)
ENUM_TIMEFRAMES TFMigrate(int tf)
{
    switch(tf)
    {
        case 0:   return(PERIOD_CURRENT);
        case 1:   return(PERIOD_M1);
        case 5:   return(PERIOD_M5);
        case 15:  return(PERIOD_M15);
        case 30:  return(PERIOD_M30);
        case 60:  return(PERIOD_H1);
        case 240: return(PERIOD_H4);
        case 1440: return(PERIOD_D1);
        case 10080: return(PERIOD_W1);
        case 43200: return(PERIOD_MN1);
        // ... additional timeframes ...
        default: return(PERIOD_CURRENT);
    }
}
```

---

## Part 6: Common Use Cases

### Use Case 1: Main Timeframe + Daily ATR

**Most common pattern:**

```mql4
// Rate Array 0: Main strategy timeframe (e.g., 1H)
ratesInformation[0][0][IDX_REQUIRED_TIMEFRAME] = STRATEGY_TIMEFRAME;  // 60 (1H)
ratesInformation[0][0][IDX_TOTAL_BARS_REQUIRED] = 500;

// Rate Array 1: Daily for ATR calculation
ratesInformation[0][1][IDX_REQUIRED_TIMEFRAME] = PERIOD_D1;  // 1440 (Daily)
ratesInformation[0][1][IDX_TOTAL_BARS_REQUIRED] = 100;
```

**Framework Usage:**
- `rates_0[]`: Main strategy logic (entry/exit signals)
- `rates_1[]`: Daily ATR for stop loss/take profit calculation

### Use Case 2: Multiple Timeframes for Trend Analysis

**Advanced pattern (hypothetical):**

```mql4
// Rate Array 0: 1H (main timeframe)
ratesInformation[0][0][IDX_REQUIRED_TIMEFRAME] = 60;  // 1H

// Rate Array 1: 4H (trend filter)
ratesInformation[0][1][IDX_REQUIRED_TIMEFRAME] = 240;  // 4H

// Rate Array 2: Daily (long-term trend)
ratesInformation[0][2][IDX_REQUIRED_TIMEFRAME] = 1440;  // Daily

// Rate Array 3: Weekly (very long-term trend)
ratesInformation[0][3][IDX_REQUIRED_TIMEFRAME] = 10080;  // Weekly
```

**Framework Usage:**
- `rates_0[]`: Entry/exit signals (1H)
- `rates_1[]`: 4H trend filter
- `rates_2[]`: Daily trend confirmation
- `rates_3[]`: Weekly trend context

### Use Case 3: Different Symbols

**Each rate array can use a different symbol:**

```mql4
// Rate Array 0: EURUSD 1H
StringToCharArray("EURUSD", ratesSymbols[0][0].a);
ratesInformation[0][0][IDX_REQUIRED_TIMEFRAME] = 60;

// Rate Array 1: USDJPY Daily (correlation analysis)
StringToCharArray("USDJPY", ratesSymbols[0][1].a);
ratesInformation[0][1][IDX_REQUIRED_TIMEFRAME] = 1440;
```

**Framework Usage:**
- `rates_0[]`: Main trading symbol (EURUSD)
- `rates_1[]`: Correlation symbol (USDJPY) for analysis

---

## Part 7: Validation and Requirements

### Bar Count Validation

**Framework validates that enough bars are available:**

```mql4
// Common.mq4 - c_validateSystemSettings() (lines 796-840)
bool c_validateSystemSettings(...)
{
    for(int i = 0; i < TOTAL_RATES_ARRAYS; i++)
    {
        if(ratesInformation[strategyIndex][i][IDX_IS_ENABLED] == false)
            continue;
        
        // Calculate required bars (accounting for upconversion and weekend bars)
        requiredBars = ratesInformation[strategyIndex][i][IDX_TOTAL_BARS_REQUIRED] * 
                       WEEKEND_BAR_MULTIPLIER * 
                       ratesInformation[strategyIndex][i][IDX_REQUIRED_TIMEFRAME] / 
                       ratesInformation[strategyIndex][i][IDX_ACTUAL_TIMEFRAME];
        
        // Check if enough bars are available
        if(iBars(CharArrayToString(ratesSymbols[strategyIndex][i].a), 
                 ratesInformation[strategyIndex][i][IDX_ACTUAL_TIMEFRAME]) < requiredBars)
        {
            Alert("Setup error: Insufficient bars");
            return(false);
        }
    }
    
    return(true);
}
```

**What This Checks:**
- Enough historical bars are available for each timeframe
- Accounts for weekend bar multiplier (1.2x)
- Accounts for timeframe upconversion (e.g., 1H → 4H needs 4x bars)

---

## Part 8: Comparison with Python Solution

### MQL4/5: Multiple Timeframes

**Advantages:**
- ✅ Up to 10 timeframes per strategy
- ✅ Direct access to any timeframe (no API calls)
- ✅ All timeframes available instantly
- ✅ Framework handles normalization automatically
- ✅ Can use different symbols per timeframe

**Implementation:**
- Configure `ratesInformation[][]` array
- Copy rates for each enabled timeframe
- Pass all rate arrays to Framework

### Python Solution: Multiple Timeframes

**Current Implementation:**
- ⚠️ Limited to strategy's rate requirements
- ⚠️ Must fetch each timeframe via API
- ⚠️ API rate limits apply
- ⚠️ Must handle normalization manually (if needed)

**How Python Handles It:**

```python
# strategy.py - run() method
for i in range(0, 9):  # Up to 10 rate arrays
    if rateRequirements[i].isEnabled:
        symbol = rateRequirements[i].symbol
        timeframe = rateRequirements[i].timeframe
        
        # Fetch candles for this symbol/timeframe
        rates = account.getCandles(
            numCandles=rateRequirements[i].totalBarsRequired,
            pair=symbol,
            timeframe=timeframe,
            allSymbolsLoaded=allSymbolsLoaded,
            allRatesLoaded=allRatesLoaded
        )
        
        # Pass to Framework
        ratesInfo[i] = rates
```

**Key Differences:**
- Python must make API calls for each timeframe
- Python must cache data to reduce API calls
- Python has rate limiting constraints
- MQL4/5 has instant access to all timeframes

---

## Summary

### MQL4/5 Multiple Timeframe Support

**Architecture:**
- ✅ Up to **10 rate arrays** per strategy (`TOTAL_RATES_ARRAYS = 10`)
- ✅ Each rate array can have **different symbol and timeframe**
- ✅ All rate arrays passed to Framework simultaneously

**Implementation:**
- ✅ Configure `ratesInformation[][]` array in EA's `initialize()` function
- ✅ Set `IDX_REQUIRED_TIMEFRAME` for each rate array
- ✅ Set `IDX_IS_ENABLED = true` to enable rate array
- ✅ Framework automatically copies rates for all enabled timeframes

**Common Pattern:**
- **rates_0[]**: Main strategy timeframe (1H, 4H, Daily, etc.)
- **rates_1[]**: Daily timeframe (for ATR calculation)
- **rates_2[]** through **rates_9[]**: Additional timeframes as needed

**Timeframe Normalization:**
- ✅ Framework automatically normalizes timeframes > 1H to 1H
- ✅ Framework handles weekend bar alignment
- ✅ Framework converts 1H bars to required timeframe internally

**Your EA Strategies:**
- ✅ **Kantu.mq4**: Uses 2 timeframes (main + Daily)
- ✅ **Teyacanani.mq4**: Uses 2 timeframes (main + Daily)
- ✅ **WatukushayFE_RSI.mq4**: Uses 2 timeframes (main + Daily)
- ✅ All strategies follow the same pattern: main timeframe + Daily for ATR

**Answer to Your Question:**
- ✅ **Yes, multiple timeframes ARE implemented** in your EA strategies
- ✅ Most strategies use **2 timeframes**: main strategy timeframe + Daily
- ✅ Framework **requires and supports** multiple timeframes (1H, 4H, Daily, Weekly, etc.)
- ✅ Framework can handle **up to 10 different timeframes** per strategy

---

**Document Status**: Complete
**Last Updated**: December 2024

