# MQL4 vs MQL5: Detailed Comparison & Migration Guide

## Overview

This document provides a comprehensive comparison between MQL4 and MQL5, identifies the major value propositions of MQL5, and estimates the effort required to migrate Expert Advisors (EAs) from MQL4 to MQL5.

---

## Part 1: Key Differences

### 1.1 Execution Model

| Aspect | MQL4 | MQL5 |
|--------|------|------|
| **Main Function** | `start()` | `OnTick()` |
| **Initialization** | `init()` | `OnInit()` |
| **Deinitialization** | `deinit()` | `OnDeinit(const int reason)` |
| **Execution Trigger** | Every tick OR new bar | Every tick (must check for new bar manually) |
| **Return Type** | `int` (returns 0) | `void` |

**Example:**

```mql4
// MQL4
int start()
{
  // Strategy logic
  return(0);
}

int init()
{
  // Initialization
  return(0);
}

int deinit()
{
  // Cleanup
  return(0);
}
```

```mql5
// MQL5
void OnTick()
{
  // Strategy logic
  return;
}

int OnInit()
{
  // Initialization
  return(0);
}

void OnDeinit(const int reason)
{
  // Cleanup (reason: why EA was removed)
  return;
}
```

### 1.2 Input Parameters

| Aspect | MQL4 | MQL5 |
|--------|------|------|
| **Keyword** | `extern` | `input` |
| **Compile-time** | Yes | Yes |
| **Runtime Modification** | No | No |

**Example:**

```mql4
// MQL4
extern string COMMENTS = "Click Modify to view comments";
extern int STRATEGY_INSTANCE_ID = -1;
extern double ACCOUNT_RISK_PERCENT = 0.0;
```

```mql5
// MQL5
input string COMMENTS = "Click Modify to view comments";
input int STRATEGY_INSTANCE_ID = -1;
input double ACCOUNT_RISK_PERCENT = 0.0;
```

### 1.3 Order Management (Major Difference)

**MQL4: Order-Based System**

```mql4
// MQL4 Order Functions
int ticket = OrderSend(
    Symbol(),           // symbol
    OP_BUY,            // operation type
    0.1,               // volume
    Ask,                // price
    3,                  // slippage
    StopLoss,           // stop loss
    TakeProfit,         // take profit
    "Comment",          // comment
    MagicNumber,        // magic number
    0,                  // expiration
    clrGreen            // arrow color
);

bool closed = OrderClose(
    ticket,             // ticket
    0.1,                // volume
    Bid,                // price
    3,                  // slippage
    clrRed              // arrow color
);

bool modified = OrderModify(
    ticket,             // ticket
    OrderOpenPrice(),   // price
    NewStopLoss,        // stop loss
    NewTakeProfit,      // take profit
    0,                  // expiration
    clrBlue             // arrow color
);

// Get orders
int total = OrdersTotal();
for(int i = 0; i < total; i++)
{
    OrderSelect(i, SELECT_BY_POS, MODE_TRADES);
    if(OrderMagicNumber() == MagicNumber)
    {
        // Process order
    }
}
```

**MQL5: Position-Based System**

```mql5
// MQL5 Position Functions
MqlTradeRequest request = {};
MqlTradeResult result = {};

request.action = TRADE_ACTION_DEAL;
request.symbol = Symbol();
request.volume = 0.1;
request.type = ORDER_TYPE_BUY;
request.price = SymbolInfoDouble(Symbol(), SYMBOL_ASK);
request.sl = StopLoss;
request.tp = TakeProfit;
request.comment = "Comment";
request.magic = MagicNumber;

bool opened = OrderSend(request, result);

// Close position
request.action = TRADE_ACTION_DEAL;
request.position = PositionTicket();
request.volume = PositionVolume();
request.type = (PositionType() == POSITION_TYPE_BUY) ? ORDER_TYPE_SELL : ORDER_TYPE_BUY;
request.price = (PositionType() == POSITION_TYPE_BUY) ? SymbolInfoDouble(Symbol(), SYMBOL_BID) : SymbolInfoDouble(Symbol(), SYMBOL_ASK);

bool closed = OrderSend(request, result);

// Modify position
request.action = TRADE_ACTION_SLTP;
request.position = PositionTicket();
request.sl = NewStopLoss;
request.tp = NewTakeProfit;

bool modified = OrderSend(request, result);

// Get positions
int total = PositionsTotal();
for(int i = 0; i < total; i++)
{
    ulong ticket = PositionGetTicket(i);
    if(PositionSelectByTicket(ticket))
    {
        if(PositionGetInteger(POSITION_MAGIC) == MagicNumber)
        {
            // Process position
        }
    }
}
```

**Key Differences:**

| Aspect | MQL4 | MQL5 |
|--------|------|------|
| **Model** | Order-based | Position-based |
| **Order Types** | Pending orders + Market orders | Market orders + Pending orders (separate) |
| **Function Names** | `OrderSend()`, `OrderClose()`, `OrderModify()` | `OrderSend()` (unified) |
| **Data Structures** | Simple parameters | `MqlTradeRequest`, `MqlTradeResult` |
| **Error Handling** | `GetLastError()` | `result.retcode`, `result.comment` |
| **Ticket System** | Integer ticket | `ulong` ticket (64-bit) |

### 1.4 Historical Data Access

**MQL4:**

```mql4
// MQL4 Historical Data
int bars = iBars(Symbol(), PERIOD_H1);
double close = iClose(Symbol(), PERIOD_H1, 0);
double open = iOpen(Symbol(), PERIOD_H1, 0);
double high = iHigh(Symbol(), PERIOD_H1, 0);
double low = iLow(Symbol(), PERIOD_H1, 0);
long volume = iVolume(Symbol(), PERIOD_H1, 0);
datetime time = iTime(Symbol(), PERIOD_H1, 0);
```

**MQL5:**

```mql5
// MQL5 Historical Data
MqlRates rates[];
int bars = CopyRates(Symbol(), PERIOD_H1, 0, 100, rates);

// Access data
double close = rates[0].close;
double open = rates[0].open;
double high = rates[0].high;
double low = rates[0].low;
long volume = rates[0].tick_volume;
datetime time = rates[0].time;
```

**Key Differences:**

| Aspect | MQL4 | MQL5 |
|--------|------|------|
| **Data Structure** | Individual functions | `MqlRates` array |
| **Bulk Access** | One bar at a time | Array copy (faster) |
| **Volume Type** | `iVolume()` (trade volume) | `tick_volume` (tick count) |
| **Performance** | Slower (multiple calls) | Faster (bulk copy) |

### 1.5 Symbol Information

**MQL4:**

```mql4
// MQL4 Symbol Info
double bid = MarketInfo(Symbol(), MODE_BID);
double ask = MarketInfo(Symbol(), MODE_ASK);
double spread = MarketInfo(Symbol(), MODE_SPREAD);
double point = MarketInfo(Symbol(), MODE_POINT);
int digits = MarketInfo(Symbol(), MODE_DIGITS);
double lotSize = MarketInfo(Symbol(), MODE_LOTSIZE);
double minLot = MarketInfo(Symbol(), MODE_MINLOT);
double maxLot = MarketInfo(Symbol(), MODE_MAXLOT);
double lotStep = MarketInfo(Symbol(), MODE_LOTSTEP);
```

**MQL5:**

```mql5
// MQL5 Symbol Info
double bid = SymbolInfoDouble(Symbol(), SYMBOL_BID);
double ask = SymbolInfoDouble(Symbol(), SYMBOL_ASK);
double spread = SymbolInfoInteger(Symbol(), SYMBOL_SPREAD);
double point = SymbolInfoDouble(Symbol(), SYMBOL_POINT);
int digits = (int)SymbolInfoInteger(Symbol(), SYMBOL_DIGITS);
double lotSize = SymbolInfoDouble(Symbol(), SYMBOL_TRADE_CONTRACT_SIZE);
double minLot = SymbolInfoDouble(Symbol(), SYMBOL_VOLUME_MIN);
double maxLot = SymbolInfoDouble(Symbol(), SYMBOL_VOLUME_MAX);
double lotStep = SymbolInfoDouble(Symbol(), SYMBOL_VOLUME_STEP);
```

**Key Differences:**

| Aspect | MQL4 | MQL5 |
|--------|------|------|
| **Function** | `MarketInfo()` | `SymbolInfoDouble()`, `SymbolInfoInteger()` |
| **Return Type** | Always `double` | `double` or `long` (type-specific) |
| **Mode Constants** | `MODE_*` | `SYMBOL_*` |
| **Type Safety** | Less type-safe | More type-safe |

### 1.6 Account Information

**MQL4:**

```mql4
// MQL4 Account Info
double balance = AccountBalance();
double equity = AccountEquity();
double margin = AccountMargin();
double freeMargin = AccountFreeMargin();
string currency = AccountCurrency();
int leverage = AccountLeverage();
string company = AccountCompany();
string server = AccountServer();
```

**MQL5:**

```mql5
// MQL5 Account Info
double balance = AccountInfoDouble(ACCOUNT_BALANCE);
double equity = AccountInfoDouble(ACCOUNT_EQUITY);
double margin = AccountInfoDouble(ACCOUNT_MARGIN);
double freeMargin = AccountInfoDouble(ACCOUNT_FREEMARGIN);
string currency = AccountInfoString(ACCOUNT_CURRENCY);
int leverage = (int)AccountInfoInteger(ACCOUNT_LEVERAGE);
string company = AccountInfoString(ACCOUNT_COMPANY);
string server = AccountInfoString(ACCOUNT_SERVER);
```

**Key Differences:**

| Aspect | MQL4 | MQL5 |
|--------|------|------|
| **Function** | Individual functions | `AccountInfoDouble()`, `AccountInfoInteger()`, `AccountInfoString()` |
| **Type Safety** | Less type-safe | More type-safe |
| **Consistency** | Different function names | Unified API |

### 1.7 Timeframe Constants

**MQL4:**

```mql4
// MQL4 Timeframes
PERIOD_M1 = 1
PERIOD_M5 = 5
PERIOD_M15 = 15
PERIOD_M30 = 30
PERIOD_H1 = 60
PERIOD_H4 = 240
PERIOD_D1 = 1440
PERIOD_W1 = 10080
PERIOD_MN1 = 43200
```

**MQL5:**

```mql5
// MQL5 Timeframes
PERIOD_M1 = 1
PERIOD_M2 = 2
PERIOD_M3 = 3
PERIOD_M4 = 4
PERIOD_M5 = 5
PERIOD_M6 = 6
PERIOD_M10 = 10
PERIOD_M12 = 12
PERIOD_M15 = 15
PERIOD_M20 = 20
PERIOD_M30 = 30
PERIOD_H1 = 60
PERIOD_H2 = 120
PERIOD_H3 = 180
PERIOD_H4 = 240
PERIOD_H6 = 360
PERIOD_H8 = 480
PERIOD_H12 = 720
PERIOD_D1 = 1440
PERIOD_W1 = 10080
PERIOD_MN1 = 43200
```

**Key Differences:**

| Aspect | MQL4 | MQL5 |
|--------|------|------|
| **Timeframes** | 9 timeframes | 21 timeframes |
| **Additional** | Standard set | More granular options (M2, M3, M4, M6, M10, M12, M20, H2, H3, H6, H8, H12) |

### 1.8 Libraries vs Includes

**MQL4:**

```mql4
// MQL4: Include files (.mqh)
#include "../include/Common.mqh"
#include "../include/Defines.mqh"
#include "../include/AsirikuyFramework.mqh"

// Functions are included directly
```

**MQL5:**

```mql5
// MQL5: Libraries (.mq5) and Includes (.mqh)
#include "../include/Common.mqh"
#include "../include/Defines.mqh"
#include "../include/AsirikuyFramework.mqh"

// Libraries are compiled separately and linked
#property library
```

**Key Differences:**

| Aspect | MQL4 | MQL5 |
|--------|------|------|
| **Code Organization** | Include files only | Libraries + Includes |
| **Compilation** | All code compiled together | Libraries compiled separately |
| **Reusability** | Less modular | More modular |
| **Performance** | Same | Same (but better code organization) |

### 1.9 Object-Oriented Programming

**MQL4:**

```mql4
// MQL4: Procedural only
// No classes, no OOP
// Functions only
```

**MQL5:**

```mql5
// MQL5: Object-Oriented Programming
class CTrade
{
private:
    ulong m_magic;
    
public:
    CTrade() { m_magic = 0; }
    bool Buy(double volume, string symbol = NULL);
    bool Sell(double volume, string symbol = NULL);
    bool PositionClose(ulong ticket);
};

// Usage
CTrade trade;
trade.Buy(0.1);
```

**Key Differences:**

| Aspect | MQL4 | MQL5 |
|--------|------|------|
| **OOP Support** | ❌ No | ✅ Yes |
| **Classes** | ❌ No | ✅ Yes |
| **Inheritance** | ❌ No | ✅ Yes |
| **Polymorphism** | ❌ No | ✅ Yes |
| **Code Reusability** | ⚠️ Limited | ✅ High |

### 1.10 Backtesting

| Aspect | MQL4 | MQL5 |
|--------|------|------|
| **Strategy Tester** | Basic | Advanced |
| **Multi-Currency** | ❌ No | ✅ Yes |
| **Multi-Timeframe** | ⚠️ Limited | ✅ Yes |
| **Optimization** | Single-threaded | Multi-threaded |
| **Speed** | Slower | Faster (multi-core) |
| **Visualization** | Basic | Advanced |

---

## Part 2: Major Value Propositions of MQL5

### 2.1 Performance Improvements

**1. Multi-Threaded Backtesting**
- MQL4: Single-threaded (uses one CPU core)
- MQL5: Multi-threaded (uses all available CPU cores)
- **Result**: 4-8x faster backtesting on modern CPUs

**2. Optimized Execution Engine**
- MQL5 has a more efficient execution engine
- Better memory management
- Faster data access (bulk array operations)

**3. Better Data Structures**
- `MqlRates` arrays for bulk historical data access
- More efficient than individual function calls in MQL4

### 2.2 Advanced Features

**1. Object-Oriented Programming**
- Classes, inheritance, polymorphism
- Better code organization and reusability
- Easier to maintain and extend

**2. More Timeframes**
- 21 timeframes vs 9 in MQL4
- More granular analysis options

**3. Better Order Management**
- Position-based system (more intuitive)
- Unified `OrderSend()` function
- Better error handling with `MqlTradeResult`

**4. Enhanced Backtesting**
- Multi-currency testing
- Multi-timeframe testing
- Better optimization algorithms

### 2.3 Future-Proofing

**1. Active Development**
- MQL5 is actively developed by MetaQuotes
- MQL4 is in maintenance mode (bug fixes only)
- New features only in MQL5

**2. Platform Support**
- MT5 is the future platform
- MT4 is legacy (still supported, but not actively developed)

**3. Broker Support**
- More brokers supporting MT5
- Better execution quality on MT5
- More instruments available on MT5

### 2.4 Code Quality

**1. Type Safety**
- Better type checking in MQL5
- `SymbolInfoDouble()` vs `SymbolInfoInteger()` (type-specific)
- Reduces runtime errors

**2. Error Handling**
- `MqlTradeResult` provides detailed error information
- Better debugging capabilities

**3. Code Organization**
- Libraries for code reusability
- Better separation of concerns

---

## Part 3: Migration Effort Assessment

### 3.1 Effort Breakdown by Component

#### **Low Effort (1-2 days per EA)**

**1. Function Name Changes**
- `start()` → `OnTick()`
- `init()` → `OnInit()`
- `deinit()` → `OnDeinit()`
- `extern` → `input`
- **Effort**: Find & replace (30 minutes)

**2. Account Information**
- `AccountBalance()` → `AccountInfoDouble(ACCOUNT_BALANCE)`
- `AccountEquity()` → `AccountInfoDouble(ACCOUNT_EQUITY)`
- **Effort**: Find & replace (1 hour)

**3. Symbol Information**
- `MarketInfo()` → `SymbolInfoDouble()` / `SymbolInfoInteger()`
- **Effort**: Find & replace with type checking (2 hours)

**4. Historical Data (Simple Cases)**
- `iClose()` → `CopyRates()` + array access
- **Effort**: Refactor data access (2-4 hours)

#### **Medium Effort (3-5 days per EA)**

**1. Order Management**
- Complete rewrite of order functions
- `OrderSend()` / `OrderClose()` / `OrderModify()` → `OrderSend()` with `MqlTradeRequest`
- Error handling changes
- **Effort**: 2-3 days (most complex part)

**2. Historical Data (Complex Cases)**
- Multiple timeframes
- Bulk data access optimization
- **Effort**: 1-2 days

**3. New Bar Detection**
- MQL4: Automatic (runs on new bar if `RUN_EVERY_TICK = false`)
- MQL5: Manual check required
- **Effort**: 1 day

#### **High Effort (1-2 weeks per EA)**

**1. Complete Refactoring**
- Object-oriented redesign
- Library extraction
- Code organization improvements
- **Effort**: 1-2 weeks

**2. Advanced Features**
- Multi-currency support
- Multi-timeframe optimization
- **Effort**: 1-2 weeks

### 3.2 Total Migration Effort Estimate

**For Simple EA (Basic Strategy):**
- **Effort**: 3-5 days
- **Components**: Function names, account info, symbol info, basic order management, historical data

**For Medium EA (Standard Strategy):**
- **Effort**: 1-2 weeks
- **Components**: All above + complex order management, multiple timeframes, error handling

**For Complex EA (Advanced Strategy):**
- **Effort**: 2-4 weeks
- **Components**: All above + OOP refactoring, library extraction, advanced features

### 3.3 Framework Compatibility

**Good News:**
- ✅ Your Framework already supports MQL5
- ✅ Same `mql5_runStrategy()` function signature
- ✅ Same data structures (`MqlRates`, `charArray`, etc.)
- ✅ Same initialization (`initInstanceMQL5()`)

**Migration Impact:**
- **Framework calls**: No changes needed
- **Data preparation**: Minor changes (function names)
- **Order execution**: Major changes (order management)

### 3.4 Migration Checklist

**Phase 1: Basic Conversion (3-5 days)**
- [ ] Change `start()` → `OnTick()`
- [ ] Change `init()` → `OnInit()`
- [ ] Change `deinit()` → `OnDeinit()`
- [ ] Change `extern` → `input`
- [ ] Update account info functions
- [ ] Update symbol info functions
- [ ] Update historical data access
- [ ] Add new bar detection (if needed)

**Phase 2: Order Management (2-3 days)**
- [ ] Rewrite `OrderSend()` calls
- [ ] Rewrite `OrderClose()` calls
- [ ] Rewrite `OrderModify()` calls
- [ ] Update order selection logic
- [ ] Update error handling
- [ ] Test order execution

**Phase 3: Testing & Optimization (2-3 days)**
- [ ] Test on demo account
- [ ] Verify Framework integration
- [ ] Test backtesting
- [ ] Performance optimization
- [ ] Bug fixes

**Total: 7-11 days per EA (medium complexity)**

---

## Part 4: Migration Strategy

### 4.1 Recommended Approach

**1. Start with One EA**
- Choose a simple EA first
- Learn the migration process
- Document common patterns

**2. Create Migration Templates**
- Template for function name changes
- Template for order management
- Template for historical data access

**3. Batch Migration**
- Migrate similar EAs together
- Reuse migration patterns
- Share common code

### 4.2 Risk Mitigation

**1. Parallel Development**
- Keep MQL4 version running
- Test MQL5 version on demo
- Compare results

**2. Gradual Rollout**
- Migrate one EA at a time
- Test thoroughly before next
- Don't rush

**3. Documentation**
- Document all changes
- Keep migration notes
- Share learnings

---

## Part 5: Cost-Benefit Analysis

### 5.1 Benefits

**Performance:**
- ✅ 4-8x faster backtesting
- ✅ Better execution speed
- ✅ Multi-threaded optimization

**Features:**
- ✅ Object-oriented programming
- ✅ More timeframes
- ✅ Better order management
- ✅ Enhanced backtesting

**Future:**
- ✅ Active development
- ✅ Future-proof platform
- ✅ Better broker support

### 5.2 Costs

**Development:**
- ⚠️ 1-2 weeks per EA (migration)
- ⚠️ Testing and validation
- ⚠️ Learning curve

**Risk:**
- ⚠️ Potential bugs during migration
- ⚠️ Need to maintain both versions initially

### 5.3 ROI Calculation

**For 10 EAs:**
- **Migration Cost**: 10-20 weeks
- **Performance Gain**: 4-8x faster backtesting
- **Time Saved**: Significant (faster optimization cycles)
- **Future Value**: High (active platform, new features)

**Recommendation:**
- ✅ **Worth it** for long-term use
- ✅ **Worth it** if you have multiple EAs
- ✅ **Worth it** if you need faster backtesting
- ⚠️ **Consider** if you have only 1-2 simple EAs

---

## Summary

### Key Differences

1. **Execution Model**: `start()` → `OnTick()`, `init()` → `OnInit()`
2. **Order Management**: Order-based → Position-based (major change)
3. **Data Access**: Individual functions → Bulk array operations
4. **OOP Support**: No → Yes
5. **Backtesting**: Single-threaded → Multi-threaded

### Major Value of MQL5

1. **Performance**: 4-8x faster backtesting
2. **Features**: OOP, more timeframes, better order management
3. **Future-Proof**: Active development, better broker support
4. **Code Quality**: Better type safety, error handling, organization

### Migration Effort

- **Simple EA**: 3-5 days
- **Medium EA**: 1-2 weeks
- **Complex EA**: 2-4 weeks

### Recommendation

**✅ Migrate to MQL5 if:**
- You have multiple EAs
- You need faster backtesting
- You want future-proof solution
- You can invest 1-2 weeks per EA

**⚠️ Stay on MQL4 if:**
- You have only 1-2 simple EAs
- Migration effort > benefit
- MQL4 meets all your needs

---

**Document Status**: Complete
**Last Updated**: December 2024

