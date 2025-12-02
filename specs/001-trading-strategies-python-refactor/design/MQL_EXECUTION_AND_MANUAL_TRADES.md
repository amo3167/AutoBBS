# MQL4/5 Execution Model & Manual Trade Handling

## Part 1: MQL4/5 Multiple Strategies Execution

### Execution Model: Sequential (Same as Python)

MQL4/5 uses the **same sequential execution model** as the Python solution. Multiple strategies within a single Expert Advisor (EA) are executed one after another, not in parallel.

### MQL4/5 Strategy Execution Flow

```mql4
// MQL4 Example: Kantu.mq4
int start()
{
  // Static arrays for multiple strategies
  static charArray strategyStrings [TOTAL_INTERNAL_STRATEGIES][STRATEGY_STRINGS_ARRAY_SIZE];
  static double systemSettings  [TOTAL_INTERNAL_STRATEGIES][SYSTEM_SETTINGS_ARRAY_SIZE];
  static double strategySettings[TOTAL_INTERNAL_STRATEGIES][STRATEGY_SETTINGS_ARRAY_SIZE];
  static double profitDrawdown  [TOTAL_INTERNAL_STRATEGIES][PROFIT_DRAWDOWN_ARRAY_SIZE];
  static charArray ratesSymbols    [TOTAL_INTERNAL_STRATEGIES][TOTAL_RATES_ARRAYS];
  static double ratesInformation[TOTAL_INTERNAL_STRATEGIES][TOTAL_RATES_ARRAYS][RATES_INFO_ARRAY_SIZE];

  // Initialize once
  if(!g_initializedSettings)
  {
    g_initializedSettings = initialize(...);
  }
  
  // Execute ALL strategies SEQUENTIALLY
  for(int i = 0; i < TOTAL_INTERNAL_STRATEGIES; i++)
  {
    if((strategySettings[i][IDX_OPERATIONAL_MODE] != DISABLE) && 
       c_validateSystemSettings(i, ...))
    {
      c_runStrategy(i, ...);  // Sequential execution
    }
  }

  return(0);
}
```

### Key Characteristics

1. **Single EA, Multiple Strategies**
   - One Expert Advisor can contain multiple internal strategies
   - Defined by `TOTAL_INTERNAL_STRATEGIES` (usually 1, but can be more)
   - All strategies share the same EA execution context

2. **Sequential Execution**
   - Strategies execute in array order (0, 1, 2, ...)
   - Each strategy blocks until completion
   - No parallel execution

3. **Shared Data Access**
   - All strategies access the same MetaTrader account
   - All strategies see the same orders (filtered by instanceID/magic number)
   - All strategies see the same price data

4. **Multiple EAs on Same Account**
   - You can attach multiple EAs to different charts
   - Each EA runs independently (MetaTrader handles this)
   - Each EA can have its own instanceID (magic number)
   - Orders are filtered by magic number to prevent conflicts

### MQL5 Execution (OnTick Event)

```mql5
// MQL5 Example: Teyacanani.mq5
void OnTick()
{
  // Same structure as MQL4
  static charArray strategyStrings [TOTAL_INTERNAL_STRATEGIES][...];
  static double systemSettings  [TOTAL_INTERNAL_STRATEGIES][...];
  // ...
  
  // Execute sequentially
  for(int i = 0; i < TOTAL_INTERNAL_STRATEGIES; i++)
  {
    if((strategySettings[i][IDX_OPERATIONAL_MODE] != DISABLE) && 
       c_validateSystemSettings(i, ...))
    {
      c_runStrategy(i, ...);  // Sequential execution
    }
  }
}
```

**Difference from MQL4:**
- MQL4: `start()` function called on every tick/bar
- MQL5: `OnTick()` event handler called on every tick
- Both execute strategies sequentially

### Comparison: MQL4/5 vs Python

| Aspect | MQL4/5 | Python Solution |
|--------|--------|-----------------|
| **Execution Model** | Sequential (same) | Sequential (same) |
| **Multiple Strategies** | Array of strategies in one EA | List of Strategy objects |
| **Execution Order** | Array index order (0, 1, 2...) | Config file order |
| **Data Access** | Direct MetaTrader functions | Broker API calls |
| **Threading** | None (MetaTrader handles) | Minimal (auxiliary only) |
| **Multiple Instances** | Multiple EAs on different charts | Single process, multiple strategies |
| **Order Filtering** | Magic number (instanceID) | instanceID in order comment |

---

## Part 2: Manual Trade Handling in Python Solution

### How Manual Trades Are Detected

The Python solution **detects manual trades by fetching all positions from the broker API** on every strategy execution cycle.

### Detection Mechanism

```python
# account.py - getTrades() (called before each strategy run)
def getTrades(self, maxOrderArraySize, instanceID):
    # 1. Fetch ALL positions from broker API
    # OANDA REST:
    url = "https://" + self.server + "/v1/accounts/%s/positions" % (self.accountID)
    req = requests.get(url, headers = self.headers)
    resp = req.json()
    self.openTrades = resp['positions']  # ALL positions, not just strategy-managed
    
    # JForex:
    self.openTrades = self.dcw.dumbStrategy.engine.getOrders()  # ALL orders
    
    # OANDA Java:
    self.openTrades = self.account.getPositions()  # ALL positions
    
    # 2. Filter by instanceID (if not using OrderWrapper)
    for trade in self.openTrades:
        if trade matches instanceID:
            # Add to openOrderInfo array
            self.openOrderInfo[index].ticket = ...
            self.openOrderInfo[index].instanceId = instanceID
            # ...
```

### Key Points

1. **Fetches ALL Positions**
   - Not just strategy-managed positions
   - Includes manual trades from broker platform
   - Includes trades from other strategies
   - Includes trades from other systems

2. **Detection Timing**
   - Manual trades are detected on the **next strategy execution cycle**
   - If `executionTime = 30` seconds, manual trades detected within 30 seconds
   - No real-time detection (polling-based)

3. **InstanceID Filtering**
   - If `useOrderWrapper = False`: Only trades with matching `instanceID` are considered
   - If `useOrderWrapper = True`: ALL trades are considered (for position balancing)

### Handling Scenarios

#### Scenario 1: Manual Trade Created (New Position)

**What Happens:**

```python
# Before manual trade:
# Strategy runs → getTrades() → 2 positions found

# User manually opens EURUSD BUY 0.1 lots from broker platform

# Next strategy run (within executionTime seconds):
# Strategy runs → getTrades() → 3 positions found (includes manual trade)
# Framework sees 3 positions (including manual trade)
# Framework processes all 3 positions
```

**Impact:**
- Framework sees the manual trade
- Framework includes it in position calculations
- Framework may adjust strategy behavior based on total positions
- If using OrderWrapper: Position balancing may occur

#### Scenario 2: Manual Trade Closed

**What Happens:**

```python
# Before manual close:
# Strategy has 1 open position (ticket 12345)
# User manually closes position 12345 from broker platform

# Next strategy run:
# Strategy runs → getTrades() → 0 positions found
# Framework sees 0 positions
# Framework no longer sees the closed position
```

**Impact:**
- Framework no longer sees the closed position
- Framework may open a new position if strategy logic requires it
- If using OrderWrapper: Position balancing may occur

#### Scenario 3: Manual Trade Modified (SL/TP Changed)

**What Happens:**

```python
# Before manual modification:
# Position has SL = 1.1000, TP = 1.1100
# User manually changes SL to 1.0950 from broker platform

# Next strategy run:
# Strategy runs → getTrades() → Position fetched with new SL = 1.0950
# Framework sees the modified SL
# Framework may modify it again if strategy logic requires different SL
```

**Impact:**
- Framework sees the modified SL/TP
- Framework may modify it again based on strategy logic
- Last modification wins (Framework or manual)

### OrderWrapper Handling

When `useOrderWrapper = True`, the Python solution has **special handling** for manual trades:

#### Position Balancing

```python
# orderwrapper.py - balancePositioning()
def balancePositioning(self, pair):
    # 1. Calculate ideal net positioning (from virtual orders)
    idealNetPositioning = 0
    for trade in virtual_trades:
        if trade['Type'] == BUY and trade['Symbol'] == pair:
            idealNetPositioning += trade['Volume']
        if trade['Type'] == SELL and trade['Symbol'] == pair:
            idealNetPositioning -= trade['Volume']
    
    # 2. Calculate current real positioning (from broker)
    currentRealPositioning = 0
    for trade in real_trades:
        if trade.type == BUY and trade.symbol == pair:
            currentRealPositioning += trade.lots
        if trade.type == SELL and trade.symbol == pair:
            currentRealPositioning -= trade.lots
    
    # 3. Calculate difference
    volumeCorrection = idealNetPositioning - currentRealPositioning
    
    # 4. Open real trade to balance
    if volumeCorrection > 0:
        # Open BUY to match virtual position
        self.account.openTrade(abs(volumeCorrection), BUY, pair, ...)
    if volumeCorrection < 0:
        # Open SELL to match virtual position
        self.account.openTrade(abs(volumeCorrection), SELL, pair, ...)
```

**What This Means:**
- If you manually open a trade, OrderWrapper detects the imbalance
- OrderWrapper opens/closes real trades to match virtual positions
- Manual trades are "absorbed" into the virtual position system

#### Close Trades from Non-Running Systems

```python
# orderwrapper.py - closeTradesFromNonRunning()
def closeTradesFromNonRunning(self, all_instanceIDs):
    # Closes virtual trades that don't belong to running strategies
    # This prevents orphaned virtual trades
```

**What This Means:**
- On startup, closes virtual trades from strategies that aren't running
- Prevents conflicts between old and new strategy instances

### Code Flow: Manual Trade Detection

```
Main Loop (atrader.py)
│
├── account.runStrategies()
│   │
│   ├── account.getAccountInfo()  # Gets account balance, equity
│   │
│   └── for strategy in account.strategies:
│       │
│       └── strategy.run()
│           │
│           ├── account.getTrades(instanceID)  # ← FETCHES ALL POSITIONS
│           │   │
│           │   ├── OANDA REST: GET /v1/accounts/{id}/positions
│           │   ├── JForex: engine.getOrders()
│           │   └── OANDA Java: account.getPositions()
│           │
│           │   └── Returns: ALL positions (including manual trades)
│           │
│           ├── account.getBidAsk(...)
│           │
│           ├── Framework.c_runStrategy(...)  # Framework sees all positions
│           │
│           └── Process results (open/close/modify orders)
│
└── Sleep for executionTime seconds
```

### Limitations

1. **No Real-Time Detection**
   - Manual trades detected on next strategy execution cycle
   - Delay = `executionTime` seconds (default: 30 seconds)
   - No event-driven detection

2. **No Prevention**
   - Cannot prevent manual trades
   - Cannot lock positions
   - Manual trades are always accepted by broker

3. **Potential Conflicts**
   - Framework may modify positions that were manually modified
   - Last modification wins (Framework or manual)
   - No coordination between manual and automated trading

4. **OrderWrapper Complexity**
   - Position balancing may create unexpected trades
   - Manual trades trigger balancing logic
   - Can lead to "ping-pong" effect (balancing creates imbalance)

### Best Practices

1. **Avoid Manual Trading When Automated System is Running**
   - Manual trades can interfere with strategy logic
   - OrderWrapper may try to "correct" manual trades
   - Can lead to unexpected behavior

2. **Use Separate Accounts**
   - Use one account for automated trading
   - Use another account for manual trading
   - Prevents conflicts

3. **Monitor Position Changes**
   - Check logs for position balancing actions
   - Monitor for unexpected trades
   - Review OrderWrapper behavior

4. **Disable OrderWrapper if Manual Trading**
   - Set `useOrderWrapper = no` in config
   - Prevents position balancing
   - Manual and automated trades coexist independently

### Comparison: MQL4/5 vs Python

| Aspect | MQL4/5 | Python Solution |
|--------|--------|-----------------|
| **Manual Trade Detection** | Immediate (MetaTrader provides real-time data) | Polling-based (next execution cycle) |
| **Detection Delay** | Real-time (on next tick) | Up to `executionTime` seconds |
| **Position Access** | Direct (`OrderSelect()`, `OrderTicket()`) | API call (`getTrades()`) |
| **Manual Trade Handling** | Framework sees all orders (filtered by magic number) | Framework sees all positions (filtered by instanceID) |
| **OrderWrapper** | Available in MQL4/5 (optional) | Available in Python (optional) |
| **Position Balancing** | Same logic (if OrderWrapper enabled) | Same logic (if OrderWrapper enabled) |
| **Prevention** | Cannot prevent manual trades | Cannot prevent manual trades |

---

## Summary

### MQL4/5 Execution

- **Sequential execution** (same as Python)
- Multiple strategies in one EA execute one after another
- Multiple EAs can run on different charts (MetaTrader handles)
- Orders filtered by magic number (instanceID)

### Python Manual Trade Handling

- **Polling-based detection**: Manual trades detected on next strategy execution cycle
- **Fetches ALL positions**: Not just strategy-managed positions
- **OrderWrapper balancing**: Automatically balances virtual vs real positions
- **No real-time detection**: Delay up to `executionTime` seconds
- **No prevention**: Cannot prevent or lock manual trades

### Recommendations

1. **For MQL4/5**: Use separate magic numbers for different strategies/instances
2. **For Python**: Avoid manual trading when automated system is running, or use separate accounts
3. **For Both**: Monitor logs for unexpected position changes
4. **For OrderWrapper**: Understand position balancing logic before enabling

---

**Document Status**: Complete
**Last Updated**: December 2024

