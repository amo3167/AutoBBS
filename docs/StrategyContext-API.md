# StrategyContext API Documentation

## Overview

`StrategyContext` is a C++ wrapper class that provides type-safe, object-oriented access to the legacy C `StrategyParams` structure. It serves as the primary interface for strategy implementations to access market data, account information, indicators, and order management functionality.

**Namespace:** `trading`  
**Header:** `include/StrategyContext.hpp`  
**Implementation:** `src/StrategyContext.cpp`

---

## Design Philosophy

### Core Principles

1. **Non-Owning Wrapper**: StrategyContext does NOT own the underlying `StrategyParams*` pointer. The caller maintains ownership and must ensure the params remain valid for the lifetime of the StrategyContext.

2. **Lazy Loading**: Heavy resources (Indicators, OrderManager) are loaded on first access and cached. This improves performance for strategies that don't need these features.

3. **Type Safety**: Provides strongly-typed accessors over the raw C double arrays and void pointers.

4. **Exception-Based Error Handling**: Uses C++ exceptions for boundary checking and validation, making error handling explicit.

5. **Const Correctness**: Methods are marked const where appropriate, enabling better compiler optimization and clearer intent.

6. **Non-Copyable**: Copy operations are disabled to prevent accidental duplication of wrapper state.

---

## Construction and Lifetime

### Constructor

```cpp
explicit StrategyContext(StrategyParams* params);
```

**Parameters:**
- `params` - Pointer to C StrategyParams structure (must not be NULL)

**Throws:**
- `std::invalid_argument` if params is NULL

**Example:**
```cpp
StrategyParams* params = ...; // From C API
trading::StrategyContext ctx(params);
```

### Destructor

```cpp
~StrategyContext();
```

Automatically cleans up lazy-loaded resources (Indicators, OrderManager). Does NOT delete the underlying StrategyParams pointer.

### Move Semantics (C++03 Limited)

```cpp
#if _MSC_VER >= 1600 // VS2010 and later
StrategyContext(StrategyContext&& other);
StrategyContext& operator=(StrategyContext&& other);
#endif
```

Move operations transfer ownership of lazy-loaded resources but not the underlying params pointer.

---

## API Reference

### Strategy Identification

#### getStrategyId()

```cpp
StrategyId getStrategyId() const;
```

Returns the strategy identifier enum value.

**Returns:** `StrategyId` - One of: RECORD_BARS, TAKEOVER, SCREENING, AUTOBBS, AUTOBBSWEEKLY, TRENDLIMIT

**Example:**
```cpp
if (ctx.getStrategyId() == AUTOBBS) {
    // AutoBBS-specific logic
}
```

#### getSymbol()

```cpp
const char* getSymbol() const;
```

Returns the trading symbol (e.g., "EURUSD", "GBPJPY").

**Returns:** `const char*` - Null-terminated string, never NULL

**Example:**
```cpp
const char* symbol = ctx.getSymbol();
printf("Trading %s\n", symbol);
```

#### getCurrentTime()

```cpp
time_t getCurrentTime() const;
```

Returns the current broker time.

**Returns:** `time_t` - Unix timestamp

**Example:**
```cpp
time_t now = ctx.getCurrentTime();
struct tm* timeinfo = gmtime(&now);
```

---

### Settings Access

#### getSetting()

```cpp
double getSetting(SettingsIndex index) const;
```

Retrieves a setting value by index.

**Parameters:**
- `index` - One of the SettingsIndex enum values (e.g., MAX_OPEN_ORDERS, IS_BACKTESTING)

**Returns:** `double` - Setting value

**Example:**
```cpp
double riskPercent = ctx.getSetting(ACCOUNT_RISK_PERCENT);
int timeframe = static_cast<int>(ctx.getSetting(TIMEFRAME));
```

#### isBacktesting()

```cpp
bool isBacktesting() const;
```

Checks if running in backtest mode vs live/demo.

**Returns:** `bool` - true if backtesting, false otherwise

**Example:**
```cpp
if (ctx.isBacktesting()) {
    // Skip real-time validation
}
```

#### getMaxOpenOrders()

```cpp
int getMaxOpenOrders() const;
```

Returns the maximum number of orders allowed.

**Returns:** `int` - Maximum open orders (from MAX_OPEN_ORDERS setting)

---

### Market Data Access

#### getBidAsk()

```cpp
const BidAsk& getBidAsk() const;
```

Returns the complete bid/ask structure.

**Returns:** `const BidAsk&` - Reference to bid/ask data

**Example:**
```cpp
const BidAsk& bidAsk = ctx.getBidAsk();
for (int i = 0; i < bidAsk.arraySize; i++) {
    printf("Bid[%d]: %.5f\n", i, bidAsk.bid[i]);
}
```

#### getBid()

```cpp
double getBid(int index = 0) const;
```

Returns bid price at specific array index.

**Parameters:**
- `index` - Array index (0 = current, 1 = previous, etc.), default 0

**Returns:** `double` - Bid price

**Throws:** `std::out_of_range` if index is negative or >= arraySize

**Example:**
```cpp
double currentBid = ctx.getBid(0);
double previousBid = ctx.getBid(1);
```

#### getAsk()

```cpp
double getAsk(int index = 0) const;
```

Returns ask price at specific array index.

**Parameters:**
- `index` - Array index (0 = current, 1 = previous, etc.), default 0

**Returns:** `double` - Ask price

**Throws:** `std::out_of_range` if index is negative or >= arraySize

**Example:**
```cpp
double currentAsk = ctx.getAsk(0);
```

#### getSpread()

```cpp
double getSpread() const;
```

Calculates current spread (ask - bid).

**Returns:** `double` - Spread in pips

**Example:**
```cpp
double spread = ctx.getSpread();
if (spread > ctx.getSetting(MAX_SPREAD)) {
    return; // Spread too wide
}
```

---

### Account Information

#### getAccountInfo()

```cpp
const AccountInfo& getAccountInfo() const;
```

Returns the complete account information structure.

**Returns:** `const AccountInfo&` - Reference to account data

#### getBalance()

```cpp
double getBalance() const;
```

Returns account balance in account currency.

**Returns:** `double` - Account balance

**Example:**
```cpp
double balance = ctx.getBalance();
```

#### getEquity()

```cpp
double getEquity() const;
```

Returns account equity (balance + floating P/L).

**Returns:** `double` - Account equity

**Example:**
```cpp
double equity = ctx.getEquity();
double drawdown = (ctx.getBalance() - equity) / ctx.getBalance();
```

#### getMargin()

```cpp
double getMargin() const;
```

Returns margin currently used for open positions.

**Returns:** `double` - Used margin

**Example:**
```cpp
double usedMargin = ctx.getMargin();
double freeMargin = ctx.getEquity() - usedMargin;
```

---

### Price Data (Rates/Bars)

#### getRatesBuffers()

```cpp
RatesBuffers* getRatesBuffers() const;
```

Returns pointer to rates buffers (OHLC data for all timeframes).

**Returns:** `RatesBuffers*` - Pointer to rates data

**Example:**
```cpp
RatesBuffers* rates = ctx.getRatesBuffers();
```

#### getRates()

```cpp
const Rates& getRates(BaseRatesIndexes index) const;
```

Returns rates for a specific timeframe.

**Parameters:**
- `index` - One of: B_PRIMARY_RATES, B_SECONDARY_RATES, B_HOURLY_RATES, B_FOURHOURLY_RATES, B_DAILY_RATES, B_WEEKLY_RATES, B_MONTHLY_RATES

**Returns:** `const Rates&` - Reference to rates for that timeframe

**Throws:** `std::out_of_range` if index is invalid

**Example:**
```cpp
const Rates& daily = ctx.getRates(B_DAILY_RATES);
double dailyClose = daily.close[0]; // Current day's close
double previousClose = daily.close[1];

const Rates& weekly = ctx.getRates(B_WEEKLY_RATES);
double weeklyHigh = weekly.high[0];
```

---

### Order Information

#### getOrderInfo()

```cpp
OrderInfo* getOrderInfo() const;
```

Returns pointer to order information array.

**Returns:** `OrderInfo*` - Pointer to orders (may be NULL)

**Note:** Array size is determined by `settings[ORDERINFO_ARRAY_SIZE]`. Iterate and check `isOpen` field to find active orders.

**Example:**
```cpp
OrderInfo* orders = ctx.getOrderInfo();
int maxOrders = static_cast<int>(ctx.getSetting(ORDERINFO_ARRAY_SIZE));

int openCount = 0;
for (int i = 0; i < maxOrders; i++) {
    if (orders[i].isOpen) {
        openCount++;
        printf("Order %d: %.5f lots at %.5f\n", 
               orders[i].ticket, orders[i].lots, orders[i].openPrice);
    }
}
```

---

### Results

#### getResults()

```cpp
StrategyResults* getResults() const;
```

Returns pointer to strategy results structure.

**Returns:** `StrategyResults*` - Pointer to results array

**Note:** Write signals to this structure for MQL layer to read. Results are passed back through the C API.

**Example:**
```cpp
StrategyResults* results = ctx.getResults();
results[0].tradingSignals = BUY_SIGNAL;
results[0].lots = 0.1;
results[0].entryPrice = ctx.getAsk(0);
results[0].brokerSL = ctx.getAsk(0) - 50 * point;
results[0].brokerTP = ctx.getAsk(0) + 100 * point;
```

---

### Lazy-Loaded Resources

#### getIndicators()

```cpp
Indicators& getIndicators();
const Indicators& getIndicators() const;
```

Returns reference to indicators wrapper (lazy-loaded).

**Returns:** `Indicators&` or `const Indicators&` - Reference to cached indicators

**Note:** Indicators are created on first access and cached for subsequent calls. Use this to access technical indicators like MA, MACD, RSI, etc.

**Example:**
```cpp
Indicators& ind = ctx.getIndicators();
// Use indicators (implementation pending Week 2)
```

#### getOrderManager()

```cpp
OrderManager& getOrderManager();
const OrderManager& getOrderManager() const;
```

Returns reference to order manager wrapper (lazy-loaded).

**Returns:** `OrderManager&` or `const OrderManager&` - Reference to cached order manager

**Note:** OrderManager is created on first access. Use this to place, modify, or close orders.

**Example:**
```cpp
OrderManager& orderMgr = ctx.getOrderManager();
// Place orders (implementation pending Week 2)
```

---

### Raw Access (Compatibility)

#### getRawParams()

```cpp
StrategyParams* getRawParams() const;
```

Returns the raw underlying StrategyParams pointer.

**Returns:** `StrategyParams*` - Raw C structure pointer

**Warning:** Use only when interfacing with legacy C code. Prefer type-safe accessors.

**Example:**
```cpp
StrategyParams* rawParams = ctx.getRawParams();
// Pass to legacy C function
legacyFunction(rawParams);
```

---

## Usage Patterns

### Basic Strategy Execution

```cpp
AsirikuyReturnCode MyStrategy::execute(const StrategyContext& context) {
    // 1. Get market data
    double bid = context.getBid(0);
    double ask = context.getAsk(0);
    double spread = context.getSpread();
    
    // 2. Check conditions
    if (spread > context.getSetting(MAX_SPREAD)) {
        return SUCCESS; // No signal due to spread
    }
    
    // 3. Get rates
    const Rates& daily = context.getRates(B_DAILY_RATES);
    double dailyClose = daily.close[0];
    
    // 4. Generate signal
    StrategyResults* results = context.getResults();
    if (dailyClose > someThreshold) {
        results[0].tradingSignals = BUY_SIGNAL;
        results[0].entryPrice = ask;
        results[0].lots = calculateLots(context);
    }
    
    return SUCCESS;
}
```

### Error Handling

```cpp
try {
    double bid = context.getBid(10); // May throw if index invalid
} catch (const std::out_of_range& e) {
    pantheios_log(PANTHEIOS_SEV_ERROR, e.what());
    return STRATEGY_INVALID_PARAMETERS;
}
```

### Accessing Historical Data

```cpp
const Rates& hourly = context.getRates(B_HOURLY_RATES);
const Rates& daily = context.getRates(B_DAILY_RATES);

// Compare current hour vs previous
double currentHourHigh = hourly.high[0];
double previousHourHigh = hourly.high[1];

// Daily trend analysis
bool uptrend = true;
for (int i = 0; i < 5; i++) {
    if (daily.close[i] < daily.close[i+1]) {
        uptrend = false;
        break;
    }
}
```

---

## Thread Safety

**⚠️ Not Thread-Safe**: StrategyContext is NOT thread-safe. Each strategy execution should have its own StrategyContext instance. Do not share StrategyContext across threads.

---

## Performance Considerations

1. **Lazy Loading**: Indicators and OrderManager are only created when first accessed. Strategies that don't need these features pay no performance penalty.

2. **Reference Returns**: Most getters return const references, avoiding unnecessary copies.

3. **Inline Candidates**: Simple getters like `getBid()`, `getAsk()` are good candidates for compiler inlining.

4. **Cache Locality**: StrategyContext is a thin wrapper with minimal state (3 pointers). The underlying data structures remain in their original C layout for cache efficiency.

---

## Migration Notes

### Differences from Legacy C API

| Legacy C Code | New C++ Code |
|---------------|--------------|
| `params->settings[IS_BACKTESTING]` | `context.isBacktesting()` |
| `params->bidAsk.bid[0]` | `context.getBid(0)` |
| `params->accountInfo.balance` | `context.getBalance()` |
| `params->ratesBuffers->rates[B_DAILY_RATES]` | `context.getRates(B_DAILY_RATES)` |
| Manual NULL checks | Exceptions |
| Array indexing errors | Bounds checking with exceptions |

### Compatibility

- **C++03 Compatible**: Uses only C++03 features (no C++11 required)
- **MSVC 2010+**: Requires VS2010 or later for rvalue reference support
- **Boost 1.49**: Compatible with project's Boost version

---

## Related Documentation

- **IStrategy Interface**: `docs/IStrategy-API.md`
- **BaseStrategy Template**: `docs/BaseStrategy-API.md`
- **StrategyTypes Header**: `include/StrategyTypes.h`

---

## Version History

| Version | Date | Changes |
|---------|------|---------|
| 1.0.0 | 2025-11-09 | Initial implementation with 32 unit tests |

---

## See Also

- Unit tests: `tests/TradingStrategiesTests.hpp`
- Header file: `include/StrategyContext.hpp`
- Implementation: `src/StrategyContext.cpp`
- Shared types: `include/StrategyTypes.h`
