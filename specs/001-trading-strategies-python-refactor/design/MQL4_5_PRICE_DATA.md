# MQL4/5 Price Data Handling

## Overview

MQL4/5 has **direct access** to all price data through MetaTrader's built-in functions. Unlike the Python solution which must fetch data from broker APIs, MQL4/5 accesses data that is **already stored and maintained by the MetaTrader platform**.

---

## Part 1: Historical Candle Data (OHLCV)

### MQL4: Direct Function Access

MQL4 uses individual functions to access historical candle data:

```mql4
// MQL4 Functions for Historical Data
int iBars(string symbol, int timeframe)           // Get total number of bars
double iOpen(string symbol, int timeframe, int shift)   // Open price at bar index
double iHigh(string symbol, int timeframe, int shift)   // High price at bar index
double iLow(string symbol, int timeframe, int shift)    // Low price at bar index
double iClose(string symbol, int timeframe, int shift) // Close price at bar index
int iTime(string symbol, int timeframe, int shift)      // Time at bar index
double iVolume(string symbol, int timeframe, int shift) // Volume at bar index
```

**Example Usage:**

```mql4
// Get number of bars available
int totalBars = iBars("EURUSD", PERIOD_H1);

// Get OHLCV data for bar at index 0 (most recent completed bar)
double open = iOpen("EURUSD", PERIOD_H1, 0);
double high = iHigh("EURUSD", PERIOD_H1, 0);
double low = iLow("EURUSD", PERIOD_H1, 0);
double close = iClose("EURUSD", PERIOD_H1, 0);
int time = iTime("EURUSD", PERIOD_H1, 0);
double volume = iVolume("EURUSD", PERIOD_H1, 0);

// Get historical bar (index 1 = previous bar)
double prevClose = iClose("EURUSD", PERIOD_H1, 1);
```

### MQL5: CopyRates() Function

MQL5 uses `CopyRates()` to copy entire arrays of historical data:

```mql5
// MQL5 Structure for Rate Data
struct MqlRates
{
    datetime time;      // Bar time
    double   open;      // Open price
    double   high;      // High price
    double   low;       // Low price
    double   close;     // Close price
    long     tick_volume;  // Tick volume
    int      spread;    // Spread
    long     real_volume;  // Real volume (if available)
};

// MQL5 Function to Copy Historical Data
int CopyRates(
    string symbol,              // Symbol name
    ENUM_TIMEFRAMES timeframe,   // Timeframe
    int start_pos,               // Start position (0 = most recent)
    int count,                   // Number of bars to copy
    MqlRates& rates[]            // Destination array
);
```

**Example Usage:**

```mql5
// Declare rates array
MqlRates rates[];

// Get total number of bars
int totalBars = Bars("EURUSD", PERIOD_H1);

// Copy last 500 bars
int copied = CopyRates("EURUSD", PERIOD_H1, 0, 500, rates);

// Access data
datetime barTime = rates[0].time;    // Most recent bar time
double open = rates[0].open;          // Most recent open
double high = rates[0].high;          // Most recent high
double low = rates[0].low;            // Most recent low
double close = rates[0].close;        // Most recent close
long volume = rates[0].tick_volume;   // Most recent volume

// Access historical bar
double prevClose = rates[1].close;    // Previous bar close
```

### How MQL4/5 Prepares Rates for Framework

**MQL5 Implementation (`Common.mq5`):**

```mql5
// Common.mq5 - c_copyRatesArrays() (lines 174-238)
void c_copyRatesArrays(int strategyIndex, 
                       charArray& ratesSymbols[][TOTAL_RATES_ARRAYS], 
                       double& ratesInformation[][TOTAL_RATES_ARRAYS][RATES_INFO_ARRAY_SIZE], 
                       MqlRates& rates_0[], 
                       MqlRates& rates_1[], 
                       // ... rates_2 through rates_9 ...
                       MqlRates& rates_9[]) export
{
    // For each enabled rate array (up to 10 arrays per strategy)
    if(ratesInformation[strategyIndex][0][IDX_IS_ENABLED] != false)
    {
        // Get total number of bars available
        ratesInformation[strategyIndex][0][IDX_RATES_ARRAY_SIZE] = 
            iBars(CharArrayToString(ratesSymbols[strategyIndex][0].a), 
                  ratesInformation[strategyIndex][0][IDX_ACTUAL_TIMEFRAME]);
        
        // Copy ALL available bars into rates_0 array
        CopyRates(CharArrayToString(ratesSymbols[strategyIndex][0].a), 
                  TFMigrate(ratesInformation[strategyIndex][0][IDX_ACTUAL_TIMEFRAME]), 
                  0,  // Start from most recent bar
                  ratesInformation[strategyIndex][0][IDX_RATES_ARRAY_SIZE],  // Copy all bars
                  rates_0);
        
        barHours[0] = TimeHour(rates_0[0].time);
    }
    
    // Repeat for rates_1 through rates_9...
}
```

**Key Points:**

1. **Direct Access**: No API calls needed - data is already in MetaTrader
2. **All Bars Available**: `iBars()` returns total number of bars available (can be thousands)
3. **Copy All Bars**: `CopyRates()` copies ALL available bars, not just a subset
4. **Multiple Symbols/Timeframes**: Up to 10 rate arrays per strategy (different symbols/timeframes)
5. **Real-time Updates**: MetaTrader automatically updates historical data as new bars form

### Data Source: MetaTrader Platform

**Where Data Comes From:**

```
MetaTrader Terminal
│
├── Historical Data (Stored Locally)
│   ├── Downloaded from broker server
│   ├── Stored in terminal's data folder
│   ├── Automatically updated on connection
│   └── Available offline (cached)
│
└── Current Data (Real-time)
    ├── Streamed from broker server
    ├── Updated on every tick
    └── Available only when connected
```

**Data Persistence:**

- Historical data is **cached locally** by MetaTrader
- Data is stored in terminal's data directory (e.g., `MQL4/History/`)
- Can be accessed even when offline (for backtesting)
- Automatically synchronized with broker server when online

---

## Part 2: Current Bid/Ask Prices

### MQL4: MarketInfo() Function

```mql4
// MQL4 Function for Current Prices
double MarketInfo(string symbol, int type);

// Common types:
#define MODE_BID    1  // Current bid price
#define MODE_ASK    2  // Current ask price
#define MODE_POINT  11 // Point size
#define MODE_DIGITS  12 // Number of digits after decimal
#define MODE_SPREAD  13 // Current spread
```

**Example Usage:**

```mql4
// Get current bid/ask prices
double bid = MarketInfo("EURUSD", MODE_BID);
double ask = MarketInfo("EURUSD", MODE_ASK);
double spread = ask - bid;

// Get symbol properties
double point = MarketInfo("EURUSD", MODE_POINT);
int digits = MarketInfo("EURUSD", MODE_DIGITS);
```

### MQL5: SymbolInfoTick() Function

```mql5
// MQL5 Structure for Tick Data
struct MqlTick
{
    datetime time;          // Time of last price update
    double   bid;           // Current bid price
    double   ask;           // Current ask price
    double   last;          // Last deal price
    ulong    volume;        // Volume for current day
    long     time_msc;      // Time in milliseconds
    uint     flags;         // Tick flags
    double   volume_real;   // Volume for current day (real)
};

// MQL5 Function to Get Current Prices
bool SymbolInfoTick(string symbol, MqlTick& tick);
```

**Example Usage:**

```mql5
MqlTick last_tick;
if(SymbolInfoTick("EURUSD", last_tick))
{
    double bid = last_tick.bid;
    double ask = last_tick.ask;
    datetime updateTime = last_tick.time;
}
```

### How MQL4/5 Gets Bid/Ask for Framework

**MQL4 Implementation (`Common.mq4`):**

```mql4
// Common.mq4 - c_getBidAsk() (lines 441-481)
void c_getBidAsk(string tradingSymbol, string accountCurrency, 
                 string baseConversionSymbol, string quoteConversionSymbol, 
                 double& bidAsk[])
{
    // Get current bid/ask for trading symbol
    bidAsk[IDX_BID] = MarketInfo(tradingSymbol, MODE_BID);
    bidAsk[IDX_ASK] = MarketInfo(tradingSymbol, MODE_ASK);
    
    // Get conversion symbol prices (if needed)
    if(getConversionRates)
    {
        bidAsk[IDX_BASE_CONVERSION_BID] = MarketInfo(baseConversionSymbol, MODE_BID);
        bidAsk[IDX_BASE_CONVERSION_ASK] = MarketInfo(baseConversionSymbol, MODE_ASK);
        bidAsk[IDX_QUOTE_CONVERSION_BID] = MarketInfo(quoteConversionSymbol, MODE_BID);
        bidAsk[IDX_QUOTE_CONVERSION_ASK] = MarketInfo(quoteConversionSymbol, MODE_ASK);
        
        // Fallback: Use open price if bid/ask unavailable
        if((bidAsk[IDX_BASE_CONVERSION_BID] == 0) || (bidAsk[IDX_BASE_CONVERSION_ASK] == 0))
        {
            bidAsk[IDX_BASE_CONVERSION_BID] = iOpen(baseConversionSymbol, 0, 0);
            bidAsk[IDX_BASE_CONVERSION_ASK] = iOpen(baseConversionSymbol, 0, 0);
        }
    }
}
```

**MQL5 Implementation (`Common.mq5`):**

```mql5
// Common.mq5 - c_getBidAsk() (lines 525-565)
void c_getBidAsk(string tradingSymbol, string accountCurrency, 
                 string baseConversionSymbol, string quoteConversionSymbol, 
                 double& bidAsk[])
{
    // Get current bid/ask for trading symbol
    bidAsk[IDX_BID] = MarketInfo(tradingSymbol, MODE_BID);
    bidAsk[IDX_ASK] = MarketInfo(tradingSymbol, MODE_ASK);
    
    // Get conversion symbol prices (if needed)
    if(getConversionRates)
    {
        bidAsk[IDX_BASE_CONVERSION_BID] = MarketInfo(baseConversionSymbol, MODE_BID);
        bidAsk[IDX_BASE_CONVERSION_ASK] = MarketInfo(baseConversionSymbol, MODE_ASK);
        bidAsk[IDX_QUOTE_CONVERSION_BID] = MarketInfo(quoteConversionSymbol, MODE_BID);
        bidAsk[IDX_QUOTE_CONVERSION_ASK] = MarketInfo(quoteConversionSymbol, MODE_ASK);
        
        // Fallback: Use open price if bid/ask unavailable
        if((bidAsk[IDX_BASE_CONVERSION_BID] == 0) || (bidAsk[IDX_BASE_CONVERSION_ASK] == 0))
        {
            bidAsk[IDX_BASE_CONVERSION_BID] = iOpen(baseConversionSymbol, 0, 0);
            bidAsk[IDX_BASE_CONVERSION_ASK] = iOpen(baseConversionSymbol, 0, 0);
        }
    }
}
```

**Key Points:**

1. **Real-time Access**: Prices are updated on every tick
2. **No Caching Needed**: MetaTrader maintains current prices in memory
3. **Multiple Symbols**: Can get prices for any symbol instantly
4. **Conversion Prices**: Also fetches base/quote conversion symbol prices
5. **Fallback Mechanism**: Uses bar open price if bid/ask unavailable

---

## Part 3: Data Flow in MQL4/5

### Complete Data Flow

```
MetaTrader Platform (Data Source)
│
├── Historical Data (Cached Locally)
│   ├── Downloaded from broker
│   ├── Stored in terminal data folder
│   └── Available via iBars(), iClose(), etc. (MQL4)
│       or CopyRates() (MQL5)
│
└── Current Data (Real-time Stream)
    ├── Streamed from broker server
    ├── Updated on every tick
    └── Available via MarketInfo() (MQL4)
        or SymbolInfoTick() (MQL5)
│
└── MQL4/5 EA (Expert Advisor)
    │
    ├── 1. Get Historical Candles
    │   ├── iBars() → Get total bars available
    │   ├── CopyRates() → Copy ALL bars to MqlRates array
    │   └── Pass to Framework as rates_0[], rates_1[], etc.
    │
    ├── 2. Get Current Bid/Ask
    │   ├── MarketInfo() → Get current bid/ask
    │   ├── Get conversion symbol prices
    │   └── Pass to Framework as bidAsk[] array
    │
    └── 3. Call Framework
        └── mql5_runStrategy(..., rates_0[], ..., bidAsk[], ...)
```

### Example: Strategy Execution

```mql4
// MQL4 Example: Kantu.mq4
int start()
{
    // Static arrays for strategy data
    static MqlRates rates_0[];
    static double bidAsk[BID_ASK_ARRAY_SIZE];
    
    // 1. Copy historical rates (ALL available bars)
    int totalBars = iBars("EURUSD", PERIOD_H1);
    CopyRates("EURUSD", PERIOD_H1, 0, totalBars, rates_0);
    
    // 2. Get current bid/ask
    bidAsk[IDX_BID] = MarketInfo("EURUSD", MODE_BID);
    bidAsk[IDX_ASK] = MarketInfo("EURUSD", MODE_ASK);
    
    // 3. Call Framework with data
    mql5_runStrategy(..., rates_0[], ..., bidAsk[], ...);
    
    return(0);
}
```

---

## Part 4: Comparison: MQL4/5 vs Python Solution

### Data Access

| Aspect | MQL4/5 | Python Solution |
|--------|--------|-----------------|
| **Historical Data** | Direct function calls (`iBars()`, `CopyRates()`) | API calls (`getCandles()`) |
| **Current Prices** | Direct function calls (`MarketInfo()`, `SymbolInfoTick()`) | API calls (`getBidAsk()`) |
| **Data Source** | MetaTrader platform (local cache + real-time stream) | Broker API (REST/Java SDK) |
| **Offline Access** | ✅ Yes (cached historical data) | ❌ No (requires API connection) |
| **Data Persistence** | ✅ Automatic (MetaTrader handles) | ⚠️ Manual (Python must cache) |
| **Update Frequency** | Real-time (every tick) | Polling (every 5 seconds for bid/ask) |
| **Number of Bars** | ALL available bars (thousands) | Limited (500 default, 10 if cached) |

### Performance

**MQL4/5:**
- **Historical Data**: Instant access (local cache)
- **Current Prices**: Instant access (in-memory)
- **No Network Latency**: Data is local
- **No API Rate Limits**: No external API calls

**Python Solution:**
- **Historical Data**: Network latency (API calls)
- **Current Prices**: Network latency (API calls)
- **API Rate Limits**: Must respect broker limits
- **Caching Required**: Must implement caching to reduce API calls

### Data Availability

**MQL4/5:**
- Historical data: **ALL available bars** (can be 10,000+ bars)
- Current prices: **Real-time** (updated on every tick)
- Multiple symbols: **Instant access** to any symbol
- Multiple timeframes: **Instant access** to any timeframe

**Python Solution:**
- Historical data: **Limited** (500 bars default, must supplement from CSV)
- Current prices: **Cached** (5 second expiration)
- Multiple symbols: **API calls** for each symbol
- Multiple timeframes: **API calls** for each timeframe

---

## Part 5: Key Advantages of MQL4/5 Approach

### 1. **No API Calls Required**

- Data is already available in MetaTrader
- No network latency
- No API rate limits
- Works offline (for backtesting)

### 2. **Complete Historical Data**

- Access to ALL available historical bars
- No need to supplement from external sources
- Automatic data synchronization
- Persistent local storage

### 3. **Real-time Updates**

- Prices updated on every tick
- No polling required
- No caching needed
- Instant access to current prices

### 4. **Multiple Symbols/Timeframes**

- Instant access to any symbol
- Instant access to any timeframe
- No additional API calls
- No rate limiting concerns

### 5. **Platform Integration**

- Data is part of the platform
- Automatic synchronization
- Automatic updates
- Seamless integration

---

## Summary

### MQL4/5 Price Data Handling

**Historical Candles:**
- ✅ Direct access via `iBars()`, `iClose()`, etc. (MQL4) or `CopyRates()` (MQL5)
- ✅ ALL available bars (thousands)
- ✅ Stored locally by MetaTrader
- ✅ Available offline for backtesting

**Current Bid/Ask:**
- ✅ Direct access via `MarketInfo()` (MQL4) or `SymbolInfoTick()` (MQL5)
- ✅ Real-time updates (every tick)
- ✅ No caching needed
- ✅ Instant access

**Key Characteristics:**
- **No API calls**: Data is local to MetaTrader platform
- **No network latency**: Direct memory access
- **No rate limits**: No external API restrictions
- **Complete data**: All historical bars available
- **Real-time**: Prices updated on every tick

**Comparison with Python:**
- MQL4/5: **Platform-integrated** data access (fast, complete, real-time)
- Python: **API-based** data access (slower, limited, polling-based)

---

**Document Status**: Complete
**Last Updated**: December 2024

