# MQL4/5 vs Python Solution Architecture Comparison

## Overview

This document compares how the Asirikuy Framework integrates with **MQL4/5** (MetaTrader) versus the **Python solution** (asirikuy-trader), focusing on data flow, state management, and storage requirements.

---

## MQL4/5 Architecture: Framework as Stateless Library

### Architecture Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                    MetaTrader Platform                      │
│  ┌──────────────────────────────────────────────────────┐  │
│  │  MQL4/5 Expert Advisor (EA)                          │  │
│  │  - Runs on every tick/bar                            │  │
│  │  - Has direct access to MT4/MT5 data                 │  │
│  │  - Calls Framework library functions                 │  │
│  └───────────────────┬──────────────────────────────────┘  │
│                      │                                        │
│  ┌───────────────────▼──────────────────────────────────┐  │
│  │  MetaTrader Data Layer (Built-in)                     │  │
│  │  - Account Info: AccountBalance(), AccountEquity()   │  │
│  │  - Orders: OrderSelect(), OrderTicket(), etc.       │  │
│  │  - Prices: Bid, Ask, iClose(), iOpen(), etc.        │  │
│  │  - Historical Data: iBars(), iTime(), iHigh(), etc.│  │
│  │  - All data is REAL-TIME and PERSISTENT              │  │
│  └───────────────────┬──────────────────────────────────┘  │
└───────────────────────┼──────────────────────────────────────┘
                        │
                        │ Data passed as function parameters
                        │
┌───────────────────────▼──────────────────────────────────────┐
│         AsirikuyFrameworkAPI.dll (C Library)                   │
│  ┌──────────────────────────────────────────────────────┐    │
│  │  Framework Functions                                  │    │
│  │  - initInstanceMQL5()                                 │    │
│  │  - mql5_runStrategy()                                 │    │
│  │  - deinitInstance()                                   │    │
│  └──────────────────────────────────────────────────────┘    │
│                                                                │
│  ┌──────────────────────────────────────────────────────┐    │
│  │  Framework State Storage (Minimal)                    │    │
│  │  - Instance state files: <instanceId>.state          │    │
│  │    * lastRunTime                                      │    │
│  │    * lastOrderUpdateTime                              │    │
│  │    * parameterSpace (strategy parameters)            │    │
│  │    * isParameterSpaceLoaded                           │    │
│  │  - Location: <config_folder>/<instanceId>.state       │    │
│  │  - Purpose: Recover state after EA restart            │    │
│  └──────────────────────────────────────────────────────┘    │
└────────────────────────────────────────────────────────────────┘
```

### Key Characteristics

1. **MQL4/5 Provides ALL Data**
   - Account data: `AccountBalance()`, `AccountEquity()`, `AccountMargin()`, `AccountFreeMargin()`
   - Order data: `OrderSelect()`, `OrderTicket()`, `OrderOpenPrice()`, `OrderLots()`, `OrderProfit()`
   - Price data: `Bid`, `Ask`, `iClose()`, `iOpen()`, `iHigh()`, `iLow()`, `iVolume()`
   - Historical data: `iBars()`, `iTime()`, `iClose()`, etc. for any symbol/timeframe
   - All data is **real-time** and **persistent** (stored by MetaTrader)

2. **Framework is Stateless**
   - Framework does **NOT** store account data
   - Framework does **NOT** store order data
   - Framework does **NOT** store price data
   - Framework only stores **instance state** (last run time, parameters) for recovery after restart
   - Framework is a **pure library** - it processes data and returns signals

3. **Data Flow in MQL4/5**

```mql4
// MQL4 Example: Kantu.mq4
int start()
{
  // 1. MQL4 collects ALL data from MetaTrader
  double accountInfo[10];
  accountInfo[IDX_BALANCE] = AccountBalance();
  accountInfo[IDX_EQUITY] = AccountEquity();
  accountInfo[IDX_MARGIN] = AccountFreeMargin();
  
  // 2. MQL4 gets all open orders
  int ordersCount = 0;
  double orderInfo[][TOTAL_ORDER_INFO_INDEXES];
  for(int i = 0; i < OrdersTotal(); i++)
  {
    OrderSelect(i, SELECT_BY_POS);
    if(OrderMagicNumber() == STRATEGY_INSTANCE_ID)
    {
      orderInfo[ordersCount][IDX_TICKET] = OrderTicket();
      orderInfo[ordersCount][IDX_OPEN_PRICE] = OrderOpenPrice();
      // ... etc
      ordersCount++;
    }
  }
  
  // 3. MQL4 gets historical rates
  MqlRates rates_0[];
  ArrayCopyRates(rates_0, Symbol(), PERIOD_H1);
  
  // 4. MQL4 gets bid/ask
  double bidAsk[6];
  bidAsk[IDX_BID] = Bid;
  bidAsk[IDX_ASK] = Ask;
  
  // 5. MQL4 calls Framework with ALL data
  int returnCode = mql5_runStrategy(
    settings, symbol, currency, brokerName, refBrokerName,
    brokerTime, ordersCount, orderInfo, accountInfo, bidAsk,
    ratesInfo, rates_0, rates_1, ..., results);
  
  // 6. Framework returns trading signals in results[]
  // 7. MQL4 executes orders based on signals
  // 8. Framework saves instance state to <instanceId>.state
}
```

4. **What Framework Stores (MQL4/5)**
   - **Instance State Files**: `<instanceId>.state` (binary files)
     - `instanceId`: Strategy instance ID
     - `lastRunTime`: Last time strategy ran (to prevent duplicate runs on same bar)
     - `lastOrderUpdateTime`: Last time orders were updated
     - `parameterSpace`: Strategy parameter space (for optimization)
     - `isParameterSpaceLoaded`: Flag indicating if parameters are loaded
   - **Location**: `<AsirikuyConfig.xml folder>/<instanceId>.state`
   - **Purpose**: Recover state after EA restart (live/demo trading only, not backtesting)

5. **What MQL4/5 Stores**
   - **Account Data**: Stored by MetaTrader platform (persistent)
   - **Order Data**: Stored by MetaTrader platform (persistent)
   - **Price Data**: Stored by MetaTrader platform (historical + real-time)
   - **Historical Data**: Stored by MetaTrader platform (database)

---

## Python Solution Architecture: Framework + Data Layer

### Architecture Diagram

```
┌─────────────────────────────────────────────────────────────┐
│              Python Application (atrader.py)                │
│  ┌──────────────────────────────────────────────────────┐  │
│  │  Account Classes (account.py)                        │  │
│  │  - OandaAccount / OandaJavaAccount                  │  │
│  │  - JForexAccount                                     │  │
│  │  - Fetches data from broker APIs                     │  │
│  └───────────────────┬──────────────────────────────────┘  │
│                      │                                        │
│  ┌───────────────────▼──────────────────────────────────┐  │
│  │  Broker API Integration                               │  │
│  │  - OANDA REST API / Java SDK                          │  │
│  │  - JForex Java SDK                                    │  │
│  │  - Real-time data fetching                            │  │
│  └───────────────────┬──────────────────────────────────┘  │
│                      │                                        │
│  ┌───────────────────▼──────────────────────────────────┐  │
│  │  Data Caching & Management                            │  │
│  │  - allSymbolsLoaded: Cached symbol data              │  │
│  │  - allRatesLoaded: Cached historical rates          │  │
│  │  - allBidAskSymbols: Cached bid/ask data             │  │
│  │  - OrderWrapper: Virtual order management (TinyDB)   │  │
│  └───────────────────┬──────────────────────────────────┘  │
└───────────────────────┼──────────────────────────────────────┘
                        │
                        │ Data prepared and passed to Framework
                        │
┌───────────────────────▼──────────────────────────────────────┐
│      libAsirikuyFrameworkAPI.dylib/.so (C Library)          │
│  ┌──────────────────────────────────────────────────────┐    │
│  │  Framework Functions                                  │    │
│  │  - initInstanceC()                                   │    │
│  │  - c_runStrategy()                                   │    │
│  │  - deinitInstance()                                  │    │
│  └──────────────────────────────────────────────────────┘    │
│                                                                │
│  ┌──────────────────────────────────────────────────────┐    │
│  │  Framework State Storage (Same as MQL4/5)            │    │
│  │  - Instance state files: <instanceId>.state          │    │
│  │  - Location: <config_folder>/<instanceId>.state      │    │
│  └──────────────────────────────────────────────────────┘    │
└────────────────────────────────────────────────────────────────┘
```

### Key Characteristics

1. **Python Must Fetch ALL Data**
   - Account data: Fetched from broker API (`getAccountInfo()`)
   - Order data: Fetched from broker API (`getTrades()`)
   - Price data: Fetched from broker API (`getBidAsk()`)
   - Historical data: Fetched from broker API (`getCandles()`)
   - **No built-in data layer** - Python must implement data fetching

2. **Python Manages Data Caching**
   - `allSymbolsLoaded`: List of symbols that have been loaded
   - `allRatesLoaded`: Dictionary of cached historical rates (to avoid repeated API calls)
   - `allBidAskSymbols`: List of symbols with cached bid/ask data
   - `allBidAskUpdateTimes`: Timestamps for bid/ask cache expiration
   - `allBidAskLoaded`: Cached bid/ask values

3. **Python Must Store Additional Data**

   **a) OrderWrapper Data (for NFA-compliant brokers)**
   - **Location**: `./data/<accountID>_openOrders.json` (TinyDB)
   - **Purpose**: Virtual order management (NFA-compliant brokers don't allow hedging)
   - **Data Stored**:
     ```json
     {
       "Ticket": 123456,
       "InstanceID": 860013,
       "Type": 0,  // BUY
       "OpenTime": 1234567890,
       "OpenPrice": 1.12345,
       "StopLoss": 1.12000,
       "TakeProfit": 1.13000,
       "Volume": 0.1,
       "Symbol": "EURUSD",
       "Profit": 0.0
     }
     ```
   - **Why**: Some brokers (NFA-compliant) don't allow multiple positions in same direction. OrderWrapper manages virtual positions.

   **b) Historical Data Cache**
   - **Location**: In-memory (`allRatesLoaded` dictionary)
   - **Purpose**: Avoid repeated API calls for same historical data
   - **Data Stored**: Rate arrays (OHLCV) keyed by symbol+timeframe
   - **Expiration**: Cached until application restart

   **c) Bid/Ask Cache**
   - **Location**: In-memory (`allBidAskLoaded` dictionary)
   - **Purpose**: Avoid repeated API calls for bid/ask (expires after 5 seconds)
   - **Data Stored**: Bid/Ask values with timestamps
   - **Expiration**: 5 seconds (`BID_ASK_EXPIRATION_IN_SECONDS = 5.0`)

4. **Data Flow in Python Solution**

```python
# Python Example: strategy.py
def run(self, allSymbolsLoaded, allRatesLoaded, allBidAskSymbols, 
        allBidAskUpdateTimes, allBidAskLoaded):
    # 1. Python fetches account info from broker API
    self.account.getAccountInfo(allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded)
    # Returns: self.account.accountInfo[IDX_BALANCE], [IDX_EQUITY], etc.
    
    # 2. Python fetches historical rates from broker API (with caching)
    rates = self.account.getCandles(maxNumCandles, self.pair, self.passedTimeFrame, 
                                     allSymbolsLoaded, allRatesLoaded)
    # Returns: Rate array (OHLCV data)
    
    # 3. Python fetches open orders from broker API
    self.account.getTrades(self.settings[ORDERINFO_ARRAY_SIZE], self.instanceID)
    # Returns: self.account.openOrderInfo (OrderInfo array)
    
    # 4. Python fetches bid/ask from broker API (with caching)
    self.account.getBidAsk(self.pair, self.basePair, self.quotePair, 
                            allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded)
    # Returns: self.account.BidAsk (BidAskType)
    
    # 5. Python prepares data structures for Framework
    passedOpenOrderInfo = self.account.openOrderInfo  # or virtualOpenOrderInfo
    passedOpenTradesCount = self.account.openTradesCount
    
    # 6. Python calls Framework with ALL data
    f4output = self.asfdll.c_runStrategy(
        self.settings,
        self.pair,
        "USD",
        self.account.brokerName,
        self.account.refBrokerName,
        c_int(int(self.account.currentBrokerTime)),
        c_int(int(passedOpenTradesCount)),
        ctypes.pointer(passedOpenOrderInfo),
        ctypes.pointer(self.account.accountInfo),
        ctypes.pointer(self.account.BidAsk),
        ctypes.pointer(ratesInfo),
        ctypes.pointer(rates_0),
        # ... rates_1 through rates_9 ...
        ctypes.pointer(strategyResults)
    )
    
    # 7. Framework returns trading signals in strategyResults
    # 8. Python executes orders via broker API (or OrderWrapper)
    # 9. Framework saves instance state to <instanceId>.state
```

5. **What Framework Stores (Python Solution)**
   - **Same as MQL4/5**: Instance state files (`<instanceId>.state`)
   - **Location**: `<AsirikuyConfig.xml folder>/<instanceId>.state`
   - **Purpose**: Recover state after application restart

6. **What Python Solution Stores**

   **a) Framework State (via Framework)**
   - Instance state files: `<instanceId>.state` (same as MQL4/5)

   **b) OrderWrapper Data (Python-specific)**
   - Virtual orders: `./data/<accountID>_openOrders.json` (TinyDB)
   - Order history: `./data/<accountID>_orderHistory.json` (TinyDB)
   - **Why**: NFA-compliant brokers don't allow hedging. OrderWrapper manages virtual positions.

   **c) Data Cache (Python-specific, in-memory)**
   - Historical rates cache: `allRatesLoaded` dictionary
   - Bid/ask cache: `allBidAskLoaded` dictionary
   - **Why**: Reduce API calls, improve performance

   **d) Logs (Python-specific)**
   - Application logs: `./log/<instanceID>_atrader.log`
   - Framework logs: `./log/AsirikuyFramework.log` (via Framework)

   **e) Plots/Reports (Python-specific)**
   - HTML reports: `./plots/<accountID>.html`
   - PNG charts: `./plots/<accountID>_<symbol>.png`
   - **Why**: Visualization and monitoring

---

## Key Differences Summary

| Aspect | MQL4/5 | Python Solution |
|--------|--------|-----------------|
| **Data Source** | MetaTrader platform (built-in) | Broker APIs (OANDA, JForex) |
| **Data Access** | Direct function calls (`AccountBalance()`, `OrderSelect()`, etc.) | API calls (`requests.get()`, Java SDK methods) |
| **Data Persistence** | MetaTrader platform handles all persistence | Python must implement caching and storage |
| **Account Data** | `AccountBalance()`, `AccountEquity()` | `getAccountInfo()` → API call |
| **Order Data** | `OrderSelect()`, `OrderTicket()`, etc. | `getTrades()` → API call |
| **Price Data** | `Bid`, `Ask`, `iClose()`, etc. | `getBidAsk()`, `getCandles()` → API calls |
| **Historical Data** | `iBars()`, `iTime()`, `iClose()`, etc. | `getCandles()` → API call |
| **Data Caching** | Not needed (MetaTrader handles it) | Required (in-memory caches) |
| **Virtual Orders** | Not needed (MetaTrader handles orders) | Required for NFA-compliant brokers (OrderWrapper + TinyDB) |
| **Framework Storage** | Instance state files only | Instance state files only (same) |
| **Additional Storage** | None | OrderWrapper JSON files, logs, plots |

---

## Implications for Python Solution

### 1. **Performance Considerations**

- **API Rate Limits**: Broker APIs have rate limits. Python solution must cache data to avoid excessive API calls.
- **Network Latency**: Every data fetch requires network round-trip. Caching is critical.
- **Data Freshness**: Bid/ask data expires after 5 seconds. Historical data cached until restart.

### 2. **Reliability Considerations**

- **API Failures**: Python must handle API failures gracefully (retry logic, error handling).
- **Connection Loss**: Python must detect connection loss and reconnect.
- **Data Consistency**: OrderWrapper must maintain consistency between virtual and real orders.

### 3. **State Management**

- **Framework State**: Handled by Framework (same as MQL4/5).
- **OrderWrapper State**: Handled by Python (TinyDB JSON files).
- **Cache State**: In-memory only (lost on restart, but can be rebuilt).

### 4. **Storage Requirements**

**MQL4/5:**
- Framework state files: `<instanceId>.state` (small binary files, ~few KB each)

**Python Solution:**
- Framework state files: `<instanceId>.state` (same as MQL4/5)
- OrderWrapper JSON files: `./data/<accountID>_openOrders.json`, `_orderHistory.json` (can grow large)
- Log files: `./log/*.log` (can grow large)
- Plot files: `./plots/*.html`, `*.png` (can grow large)

### 5. **Complexity**

**MQL4/5:**
- Simple: MQL4/5 provides all data, Framework processes it.
- No data fetching logic needed.
- No caching logic needed.
- No virtual order management needed (unless using OrderWrapper in MQL4/5).

**Python Solution:**
- Complex: Python must implement:
  - Broker API integration (OANDA REST/Java, JForex Java)
  - Data fetching and caching
  - Error handling and retry logic
  - Virtual order management (OrderWrapper)
  - Connection management
  - Data synchronization

---

## Conclusion

### MQL4/5: Framework as Pure Library
- **MQL4/5 provides all data** (account, orders, prices, historical)
- **Framework is stateless** (only stores instance state for recovery)
- **Simple integration** (just pass data to Framework functions)
- **No additional storage** (MetaTrader handles everything)

### Python Solution: Framework + Data Layer
- **Python must fetch all data** from broker APIs
- **Framework is still stateless** (only stores instance state)
- **Python must implement data layer** (fetching, caching, storage)
- **Additional storage required** (OrderWrapper JSON files, logs, plots)
- **More complex** (API integration, error handling, caching, virtual orders)

### Key Takeaway

The **Framework itself is the same** in both cases - it's a stateless library that processes data and returns trading signals. The difference is:

- **MQL4/5**: MetaTrader provides the data layer
- **Python**: Python must implement the data layer

This is why the Python solution is more complex - it must replicate the data layer that MetaTrader provides natively.

---

**Document Status**: Complete
**Last Updated**: December 2024

