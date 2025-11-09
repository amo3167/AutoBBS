# Trading Strategies Infrastructure API Documentation

This document provides comprehensive documentation for the three core infrastructure components used in Phase 2 strategy migrations: **OrderBuilder**, **Indicators**, and **OrderManager**.

## Table of Contents

1. [OrderBuilder API](#orderbuilder-api)
2. [Indicators API](#indicators-api)
3. [OrderManager API](#ordermanager-api)
4. [Usage Examples](#usage-examples)

---

## OrderBuilder API

**Purpose**: Constructs and splits trading orders according to various strategic splitting patterns extracted from legacy C strategies.

**Header**: `OrderBuilder.hpp`

**Namespace**: `asirikuy`

### Constructor

```cpp
OrderBuilder(StrategyContext& context)
```

Creates an OrderBuilder instance tied to a specific strategy context.

**Parameters**:
- `context`: Reference to StrategyContext containing rates, parameters, and strategy state

**Example**:
```cpp
StrategyContext context(params);
OrderBuilder builder(context);
```

### Fluent Interface Methods

All setter methods return `OrderBuilder&` for method chaining.

#### setOrderType
```cpp
OrderBuilder& setOrderType(OrderType type)
```
Sets the order type (BUY or SELL).

#### setEntryPrice
```cpp
OrderBuilder& setEntryPrice(double price)
```
Sets the entry price for the order.

#### setStopLoss
```cpp
OrderBuilder& setStopLoss(double sl)
```
Sets the stop loss price (absolute price, not distance).

#### setTakeProfit
```cpp
OrderBuilder& setTakeProfit(double tp)
```
Sets the take profit price (absolute price, not distance).

#### setLotSize
```cpp
OrderBuilder& setLotSize(double lots)
```
Sets the total lot size to be split across orders.

#### setMagicNumber
```cpp
OrderBuilder& setMagicNumber(int magic)
```
Sets the magic number for order identification.

#### setSplitStrategy
```cpp
OrderBuilder& setSplitStrategy(SplitStrategy strategy)
```
Sets the splitting strategy to use.

**Available Strategies**:
- `SPLIT_NONE`: Single order, no splitting
- `SPLIT_SHORT_TERM`: 3 orders (30% @ 1x, 40% @ 1.5x, 30% @ 2x risk)
- `SPLIT_LONG_TERM`: Pullback-based split with trailing
- `SPLIT_KEYK`: 2 orders (50% @ 1x, 50% @ 2x risk)
- `SPLIT_ATR`: 2 orders (60% @ 1x ATR, 40% @ 2x ATR)
- `SPLIT_LIMIT`: Fibonacci retracement levels (38.2%, 50%)
- `SPLIT_WEEKLY_V1`: Weekly timeframe variant 1
- `SPLIT_WEEKLY_V2`: Weekly timeframe variant 2
- `SPLIT_WEEKLY_V3`: Weekly timeframe variant 3
- `SPLIT_4H_SWING_V1`: 4-hour swing variant 1
- `SPLIT_4H_SWING_V2`: 4-hour swing variant 2
- `SPLIT_4H_SWING_V3`: 4-hour swing variant 3
- `SPLIT_ICHIMOKU_V1`: Ichimoku-based split variant 1
- `SPLIT_ICHIMOKU_V2`: Ichimoku-based split variant 2

#### setATRPeriod / setATRMultiplier
```cpp
OrderBuilder& setATRPeriod(int period)
OrderBuilder& setATRMultiplier(double multiplier)
```
Sets ATR parameters for ATR-based splitting strategies.

### Build Method

```cpp
bool build(std::vector<OrderSpec>& orders)
```

Builds the order specification(s) based on configured parameters.

**Parameters**:
- `orders`: Output vector to receive OrderSpec structures

**Returns**: `true` on success, `false` on validation failure

**OrderSpec Structure**:
```cpp
struct OrderSpec {
    OrderType orderType;    // BUY or SELL
    double openPrice;       // Entry price
    double stopLoss;        // Stop loss price
    double takeProfit;      // Take profit price
    double lotSize;         // Lot size for this order
    int magicNumber;        // Magic number
};
```

### Reset Method

```cpp
void reset()
```

Resets all builder parameters to defaults for reuse.

---

## Indicators API

**Purpose**: Provides C++ interface for technical indicator calculations by wrapping existing EasyTrade C functions.

**Header**: `Indicators.hpp`

**Namespace**: `asirikuy`

### Constructor

```cpp
Indicators(StrategyContext& context)
```

Creates an Indicators instance tied to a specific strategy context.

**Parameters**:
- `context`: Reference to StrategyContext containing rate arrays

### Indicator Methods

All methods are `const` (read-only) and use the following common parameters:
- `timeframe`: Timeframe index (0 = primary, 1 = daily, etc.)
- `period`: Indicator period/length
- `shift`: Bar index (0 = current, 1 = previous, etc.)

#### ATR (Average True Range)
```cpp
double getATR(int timeframe, int period, int shift) const
```

Returns ATR value for volatility measurement.

**Example**:
```cpp
double atr = indicators.getATR(0, 14, 0); // 14-period ATR on current bar
```

#### RSI (Relative Strength Index)
```cpp
double getRSI(int timeframe, int period, int shift) const
```

Returns RSI value (0-100 range).

**Example**:
```cpp
double rsi = indicators.getRSI(0, 14, 1); // 14-period RSI on previous bar
```

#### Moving Average
```cpp
double getMA(int timeframe, int period, int shift, MAPrice priceType) const
```

Returns moving average value.

**MAPrice Types**:
- `MA_PRICE_OPEN`: Open prices
- `MA_PRICE_HIGH`: High prices
- `MA_PRICE_LOW`: Low prices
- `MA_PRICE_CLOSE`: Close prices

**Example**:
```cpp
double ma = indicators.getMA(0, 20, 0, MA_PRICE_CLOSE); // 20-period MA of close
```

#### MACD
```cpp
double getMACD(int timeframe, int fastPeriod, int slowPeriod, 
               int signalPeriod, MACDSignal signal, int shift) const
```

Returns MACD indicator component.

**MACDSignal Types**:
- `MACD_MAIN`: Main MACD line
- `MACD_SIGNAL`: Signal line
- `MACD_HISTOGRAM`: Histogram (main - signal)

**Example**:
```cpp
double macdMain = indicators.getMACD(0, 12, 26, 9, MACD_MAIN, 0);
double macdSignal = indicators.getMACD(0, 12, 26, 9, MACD_SIGNAL, 0);
```

#### MACD All-in-One
```cpp
void getMACDAll(int timeframe, int fastPeriod, int slowPeriod, 
                int signalPeriod, int shift, 
                double& main, double& signal, double& histogram) const
```

Returns all three MACD components in one call.

#### Bollinger Bands
```cpp
double getBollingerBands(int timeframe, int period, double deviation,
                         BBandsBuffer buffer, int shift) const
```

Returns Bollinger Bands value.

**BBandsBuffer Types**:
- `BBANDS_LOWER`: Lower band
- `BBANDS_MIDDLE`: Middle band (moving average)
- `BBANDS_UPPER`: Upper band

**Example**:
```cpp
double upperBand = indicators.getBollingerBands(0, 20, 2.0, BBANDS_UPPER, 0);
double lowerBand = indicators.getBollingerBands(0, 20, 2.0, BBANDS_LOWER, 0);
```

#### Bollinger Bands Stop
```cpp
double getBBandStop(int timeframe, int period, double deviation, int shift) const
```

Returns specialized Bollinger Bands stop level.

#### Stochastic
```cpp
double getStochastic(int timeframe, int kPeriod, int dPeriod, int slowing,
                     StochasticSignal signal, int shift) const
```

Returns Stochastic oscillator value (0-100 range).

**StochasticSignal Types**:
- `STOCHASTIC_MAIN`: %K line
- `STOCHASTIC_SIGNAL`: %D line

#### Standard Deviation
```cpp
double getStdDev(int timeframe, int period, int shift) const
```

Returns standard deviation for volatility measurement.

#### CCI (Commodity Channel Index)
```cpp
double getCCI(int timeframe, int period, int shift) const
```

Returns CCI value for trend strength measurement.

### Price Accessor Methods

```cpp
double getHigh(int timeframe, int shift) const
double getLow(int timeframe, int shift) const
double getOpen(int timeframe, int shift) const
double getClose(int timeframe, int shift) const
```

Return OHLC prices for specified bar.

**Example**:
```cpp
double currentHigh = indicators.getHigh(0, 0);
double previousClose = indicators.getClose(0, 1);
```

---

## OrderManager API

**Purpose**: Manages order lifecycle including sizing, margin checking, placement, modification, and closure.

**Header**: `OrderManager.hpp`

**Namespace**: `asirikuy`

### Constructor

```cpp
OrderManager(StrategyContext& context)
```

Creates an OrderManager instance tied to a specific strategy context.

### Order Counting Methods

#### getTotalOpenOrders
```cpp
int getTotalOpenOrders(OrderType orderType) const
```

Returns count of open orders of specified type.

**Example**:
```cpp
int buyOrders = manager.getTotalOpenOrders(BUY);
int sellOrders = manager.getTotalOpenOrders(SELL);
```

#### getTotalClosedOrders
```cpp
int getTotalClosedOrders(OrderType orderType) const
```

Returns count of closed orders of specified type.

### Order Sizing and Risk Methods

#### calculateOrderSize
```cpp
double calculateOrderSize(OrderType orderType, double entryPrice, double stopLoss) const
```

Calculates order size based on strategy's default risk parameters.

**Parameters**:
- `orderType`: BUY or SELL
- `entryPrice`: Intended entry price
- `stopLoss`: Stop loss price (absolute, not distance)

**Returns**: Order size in lots

**Example**:
```cpp
double size = manager.calculateOrderSize(BUY, 1.2000, 1.1950);
```

#### calculateOrderSizeWithRisk
```cpp
double calculateOrderSizeWithRisk(OrderType orderType, double entryPrice,
                                  double stopLoss, double risk) const
```

Calculates order size with specific risk amount.

**Parameters**:
- `risk`: Risk as decimal (e.g., 0.02 for 2%)

**Example**:
```cpp
double size = manager.calculateOrderSizeWithRisk(BUY, 1.2000, 1.1950, 0.01); // 1% risk
```

#### hasEnoughFreeMargin
```cpp
bool hasEnoughFreeMargin(OrderType orderType, double lotSize) const
```

Checks if account has sufficient margin for order.

#### getMaxLossPerLot
```cpp
double getMaxLossPerLot(OrderType orderType, double entryPrice, double stopLoss) const
```

Calculates maximum loss per lot in account currency.

### Order Lifecycle Methods

#### openOrUpdateLongTrade / openOrUpdateShortTrade
```cpp
bool openOrUpdateLongTrade(int ratesIndex, int resultsIndex, double stopLoss,
                           double takeProfit, double risk, bool useInternalSL,
                           bool useInternalTP)
                           
bool openOrUpdateShortTrade(int ratesIndex, int resultsIndex, double stopLoss,
                            double takeProfit, double risk, bool useInternalSL,
                            bool useInternalTP)
```

Opens new trade or updates existing one. Automatically closes opposite direction.

**Parameters**:
- `ratesIndex`: Rate array index (typically 0)
- `resultsIndex`: Results array index
- `stopLoss`: Stop loss distance in price units
- `takeProfit`: Take profit distance in price units
- `risk`: Risk amount (e.g., 0.02 for 2%)
- `useInternalSL`: Use internal (software) stop loss tracking
- `useInternalTP`: Use internal (software) take profit tracking

**Returns**: `true` on success, `false` on failure

**Example**:
```cpp
bool success = manager.openOrUpdateLongTrade(
    0,              // ratesIndex
    0,              // resultsIndex
    0.0050,         // 50 pip SL
    0.0100,         // 100 pip TP
    0.02,           // 2% risk
    false,          // broker SL
    false           // broker TP
);
```

#### updateLongTrade / updateShortTrade
```cpp
bool updateLongTrade(int ratesIndex, int resultsIndex, double stopLoss,
                     double takeProfit, bool useInternalSL, bool useInternalTP)
                     
bool updateShortTrade(int ratesIndex, int resultsIndex, double stopLoss,
                      double takeProfit, bool useInternalSL, bool useInternalTP)
```

Updates existing trade without creating new entry signal.

#### closeLongTrade / closeShortTrade
```cpp
bool closeLongTrade(int resultsIndex)
bool closeShortTrade(int resultsIndex)
```

Closes all trades in specified direction.

**Example**:
```cpp
manager.closeLongTrade(0);  // Close all long positions
```

### Order Validation Methods

#### areOrdersCorrect
```cpp
bool areOrdersCorrect(double stopLoss, double takeProfit) const
```

Validates stop loss and take profit values.

#### checkInternalStopLoss / checkInternalTakeProfit
```cpp
bool checkInternalStopLoss(int ratesIndex, int resultsIndex, double internalSL)
bool checkInternalTakeProfit(int ratesIndex, int resultsIndex, double internalTP)
```

Checks if internal stops/targets have been hit.

#### checkTimedExit
```cpp
bool checkTimedExit(int ratesIndex, int resultsIndex,
                    bool usingInternalSL, bool usingInternalTP)
```

Checks if trade has exceeded maximum time limit.

### Trailing Stop Methods

#### trailOpenTrades
```cpp
bool trailOpenTrades(int ratesIndex, double trailStart, double trailDistance,
                     bool useInternalSL, bool useInternalTP)
```

Applies trailing stop to open positions.

**Parameters**:
- `trailStart`: Distance price must move before trailing starts
- `trailDistance`: Distance between new SL and current price

**Example**:
```cpp
// Trail with 20 pip activation, 10 pip distance
manager.trailOpenTrades(0, 0.0020, 0.0010, false, false);
```

### Order Age Methods

#### getOrderAge
```cpp
int getOrderAge(int ratesIndex) const
```

Returns age of current order in bars.

#### getOrderAgeByPosition
```cpp
int getOrderAgeByPosition(int ratesIndex, int position) const
```

Returns age of specific order/position in bars.

### Advanced Stop Loss/Take Profit Methods

#### calculateEllipticalStopLoss
```cpp
double calculateEllipticalStopLoss(double takeProfit, int maxHoldingTime,
                                   double z, int orderBarsAge) const
```

Calculates time-decay elliptical stop loss.

**Parameters**:
- `takeProfit`: Target TP distance
- `maxHoldingTime`: Maximum bars to hold
- `z`: Confidence parameter (e.g., 2.0 for ~98%)
- `orderBarsAge`: Current age in bars

**Theory**: SL is tight initially, relaxes mid-trade for volatility, then tightens as time limit approaches.

#### calculateEllipticalTakeProfit
```cpp
double calculateEllipticalTakeProfit(double takeProfit, int maxHoldingTime,
                                     double z, int orderBarsAge) const
```

Calculates time-decay elliptical take profit (parabolic).

---

## Usage Examples

### Example 1: Simple Long Trade with Short-Term Split

```cpp
void onBar(StrategyContext& context) {
    // Check entry conditions
    Indicators indicators(context);
    double rsi = indicators.getRSI(0, 14, 0);
    
    if (rsi < 30) {  // Oversold
        // Build split orders
        OrderBuilder builder(context);
        std::vector<OrderSpec> orders;
        
        double entry = indicators.getClose(0, 0);
        double sl = entry - 0.0050;  // 50 pip SL
        double tp = entry + 0.0100;  // 100 pip TP
        
        bool success = builder
            .setOrderType(BUY)
            .setEntryPrice(entry)
            .setStopLoss(sl)
            .setTakeProfit(tp)
            .setLotSize(0.10)
            .setSplitStrategy(SPLIT_SHORT_TERM)
            .build(orders);
        
        if (success) {
            // Execute each order
            OrderManager manager(context);
            for (size_t i = 0; i < orders.size(); ++i) {
                manager.openOrUpdateLongTrade(
                    0, i, 
                    orders[i].stopLoss - orders[i].openPrice,
                    orders[i].takeProfit - orders[i].openPrice,
                    0.02, false, false
                );
            }
        }
    }
}
```

### Example 2: ATR-Based Stop Loss with Trailing

```cpp
void onBar(StrategyContext& context) {
    Indicators indicators(context);
    OrderManager manager(context);
    
    // Calculate dynamic stops based on ATR
    double atr = indicators.getATR(0, 14, 0);
    double entry = indicators.getClose(0, 0);
    double atrSL = 2.0 * atr;  // 2 ATR stop
    double atrTP = 4.0 * atr;  // 4 ATR target
    
    // Open trade
    bool opened = manager.openOrUpdateLongTrade(
        0, 0, atrSL, atrTP, 0.02, false, false
    );
    
    if (opened) {
        // Apply trailing stop: activate after 1 ATR profit, trail at 0.5 ATR
        manager.trailOpenTrades(0, atr, 0.5 * atr, false, false);
    }
}
```

### Example 3: Multiple Timeframe Analysis

```cpp
void onBar(StrategyContext& context) {
    Indicators indicators(context);
    
    // Weekly trend (timeframe 1)
    double weeklyMA = indicators.getMA(1, 50, 0, MA_PRICE_CLOSE);
    double weeklyClose = indicators.getClose(1, 0);
    bool weeklyUptrend = weeklyClose > weeklyMA;
    
    // 4H entry (timeframe 0)
    double rsi = indicators.getRSI(0, 14, 0);
    double macdMain, macdSignal, macdHist;
    indicators.getMACDAll(0, 12, 26, 9, 0, macdMain, macdSignal, macdHist);
    
    // Enter long only if weekly uptrend and 4H oversold with MACD crossover
    if (weeklyUptrend && rsi < 30 && macdMain > macdSignal) {
        OrderManager manager(context);
        manager.openOrUpdateLongTrade(0, 0, 0.0050, 0.0100, 0.02, false, false);
    }
}
```

### Example 4: Bollinger Bands Mean Reversion

```cpp
void onBar(StrategyContext& context) {
    Indicators indicators(context);
    OrderManager manager(context);
    
    // Get Bollinger Bands
    double upper = indicators.getBollingerBands(0, 20, 2.0, BBANDS_UPPER, 0);
    double middle = indicators.getBollingerBands(0, 20, 2.0, BBANDS_MIDDLE, 0);
    double lower = indicators.getBollingerBands(0, 20, 2.0, BBANDS_LOWER, 0);
    double close = indicators.getClose(0, 0);
    
    // Buy at lower band, target middle band
    if (close <= lower) {
        double sl = close - (middle - lower) * 0.5;  // 50% of band width
        double tp = middle;  // Target mean
        
        manager.openOrUpdateLongTrade(
            0, 0,
            close - sl,  // Distance
            tp - close,  // Distance
            0.01,        // 1% risk for mean reversion
            false, false
        );
    }
    
    // Sell at upper band, target middle band
    if (close >= upper) {
        double sl = close + (upper - middle) * 0.5;
        double tp = middle;
        
        manager.openOrUpdateShortTrade(
            0, 0,
            sl - close,
            close - tp,
            0.01,
            false, false
        );
    }
}
```

### Example 5: Elliptical Stop Management

```cpp
void onBar(StrategyContext& context) {
    Indicators indicators(context);
    OrderManager manager(context);
    
    // Get current position age
    int age = manager.getOrderAge(0);
    int maxAge = 20;  // 20 bar maximum hold
    
    if (age > 0 && age < maxAge) {
        // Calculate dynamic stops based on time decay
        double targetTP = 0.0100;  // 100 pip target
        double dynamicSL = manager.calculateEllipticalStopLoss(
            targetTP, maxAge, 2.0, age
        );
        double dynamicTP = manager.calculateEllipticalTakeProfit(
            targetTP, maxAge, 2.0, age
        );
        
        // Update trade with time-decay stops
        manager.updateLongTrade(0, 0, dynamicSL, dynamicTP, false, false);
    }
    
    // Force exit at max age
    if (age >= maxAge) {
        manager.closeLongTrade(0);
    }
}
```

### Example 6: Risk-Adjusted Position Sizing

```cpp
void onBar(StrategyContext& context) {
    Indicators indicators(context);
    OrderManager manager(context);
    
    double entry = indicators.getClose(0, 0);
    double atr = indicators.getATR(0, 14, 0);
    
    // Use ATR for dynamic stop
    double sl = entry - (2.0 * atr);
    
    // Calculate position size for 2% risk
    double lotSize = manager.calculateOrderSizeWithRisk(BUY, entry, sl, 0.02);
    
    // Verify margin available
    if (manager.hasEnoughFreeMargin(BUY, lotSize)) {
        // Calculate max loss to confirm
        double maxLoss = manager.getMaxLossPerLot(BUY, entry, sl);
        
        // Open position
        manager.openOrUpdateLongTrade(
            0, 0,
            entry - sl,      // SL distance
            (2.0 * atr) * 2, // TP = 2x SL distance
            0.02,
            false, false
        );
    }
}
```

---

## Best Practices

### 1. Always Check Return Values
```cpp
OrderBuilder builder(context);
std::vector<OrderSpec> orders;
if (builder.setOrderType(BUY)
          .setEntryPrice(1.2000)
          .setStopLoss(1.1950)
          .build(orders)) {
    // Success - use orders
} else {
    // Handle validation failure
}
```

### 2. Use Constants for Magic Numbers
```cpp
const int STRATEGY_MAGIC_BASE = 10000;
builder.setMagicNumber(STRATEGY_MAGIC_BASE + 1);
```

### 3. Validate Indicator Values
```cpp
double rsi = indicators.getRSI(0, 14, 0);
if (rsi >= 0.0 && rsi <= 100.0) {
    // Valid RSI value
}
```

### 4. Check Order Counts Before Acting
```cpp
int openLongs = manager.getTotalOpenOrders(BUY);
if (openLongs == 0) {
    // No positions, safe to open new
}
```

### 5. Reset Builder Between Uses
```cpp
OrderBuilder builder(context);
// ... use builder ...
builder.reset();  // Clear for next use
```

---

## Thread Safety

**Warning**: None of these classes are thread-safe. All methods must be called from the same thread. The underlying C functions maintain global state and are not reentrant.

## Memory Management

All classes use references to StrategyContext and do not own the context. Ensure StrategyContext outlives all infrastructure objects.

## Error Handling

- Methods return `bool` for success/failure (not exceptions - C++03 compatible)
- Check return values before proceeding
- Invalid parameters typically return 0.0 or false rather than throwing

---

## Migration Notes

When migrating strategies from C to C++:

1. **Order Building**: Replace C splitting functions with OrderBuilder
2. **Indicators**: Replace direct C calls with Indicators wrapper
3. **Order Management**: Replace C order functions with OrderManager
4. **Type Safety**: Use enums instead of magic numbers
5. **Modern Interface**: Use fluent builders instead of procedural calls

**Example C to C++ Migration**:

```c
// Old C code
double atr = iAtr(0, 14, 1);
int result = openOrUpdateLongTrade(pParams, 0, 0, 0.0050, 0.0100, 0.02, FALSE, FALSE);
```

```cpp
// New C++ code
Indicators indicators(context);
double atr = indicators.getATR(0, 14, 1);

OrderManager manager(context);
bool result = manager.openOrUpdateLongTrade(0, 0, 0.0050, 0.0100, 0.02, false, false);
```

---

## Support

For questions or issues with the infrastructure API:
- Check unit tests in `tests/` directory for additional examples
- Review original C implementations in `dev/OrderManager/` and `dev/AsirikuyEasyTrade/`
- See Phase 2 migration plan for strategy-specific guidance
