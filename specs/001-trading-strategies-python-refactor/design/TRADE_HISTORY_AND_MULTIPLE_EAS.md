# Trade History Feature & Multiple EAs Execution

## Part 1: Trade History in MQL4/5 vs Python Solution

### MQL4/5: Fetches Both Open and Closed Orders

MQL4/5 fetches **both open orders AND closed order history** and passes them to the Framework in the same `orderInfo` array.

#### How MQL4/5 Fetches Trade History

```mql4
// Common.mq4 - c_getOrderInfo() (lines 507-568)
void c_getOrderInfo(string symbol, int strategyIndex, 
                    double& strategySettings[][...], 
                    double& orderInfo[][TOTAL_ORDER_INFO_INDEXES])
{
  int orderCount = 0;
  int totalOpenOrders = nOrdersTotal();
  int totalClosedOrders = nOrdersHistoryTotal();  // ← Get history count
  
  // 1. First, get OPEN orders (filtered by magic number)
  for(i = totalOpenOrders - 1; (i >= 0) && (orderCount < maxSize); i--)
  {
    nOrderSelect(i, SELECT_BY_POS, MODE_TRADES);  // ← Open orders
    
    if((nOrderSymbol() == symbol) &&
       (nOrderMagicNumber() == strategyInstanceId) &&  // ← Filter by magic number
       ((nOrderType() == OP_BUY) || (nOrderType() == OP_SELL)))
    {
      orderInfo[orderCount][IDX_ORDER_TICKET] = nOrderTicket();
      orderInfo[orderCount][IDX_ORDER_IS_OPEN] = true;  // ← Mark as open
      // ... fill all order fields
      orderCount++;
    }
  }
  
  // 2. Then, get CLOSED orders from history (filtered by magic number)
  for(i = totalClosedOrders - 1; (i >= 0) && (orderCount < maxSize); i--)
  {
    nOrderSelect(i, SELECT_BY_POS, MODE_HISTORY);  // ← Closed orders
    
    if((nOrderSymbol() == symbol) &&
       (nOrderMagicNumber() == strategyInstanceId) &&  // ← Filter by magic number
       ((nOrderType() == OP_BUY) || (nOrderType() == OP_SELL)))
    {
      orderInfo[orderCount][IDX_ORDER_TICKET] = nOrderTicket();
      orderInfo[orderCount][IDX_ORDER_CLOSE_TIME] = nOrderCloseTime();
      orderInfo[orderCount][IDX_ORDER_CLOSE_PRICE] = nOrderClosePrice();
      orderInfo[orderCount][IDX_ORDER_PROFIT] = nOrderProfit();
      orderInfo[orderCount][IDX_ORDER_IS_OPEN] = false;  // ← Mark as closed
      // ... fill all order fields
      orderCount++;
    }
  }
}
```

#### Key Points

1. **Single Array for Both Open and Closed Orders**
   - `orderInfo[]` array contains both open and closed orders
   - Open orders first, then closed orders
   - `IDX_ORDER_IS_OPEN` flag distinguishes them

2. **Filtered by Magic Number (instanceID)**
   - Only orders with matching `magicNumber == strategyInstanceId` are included
   - This means Framework only sees orders for that specific strategy instance
   - Manual trades with matching magic number are included
   - Manual trades with different magic number are excluded

3. **Framework Uses History for Statistics**

```mql4
// Common.mq4 - c_calculateProfitAndDrawdown() (lines 737-789)
void c_calculateProfitAndDrawdown(string symbol, int strategyIndex, 
                                  int strategyInstanceId, 
                                  double &profitDrawdown[][...])
{
  int newOrdersHistoryTotal = nOrdersHistoryTotal();
  
  // Process NEW closed orders since last check
  for(int i = newOrdersHistoryTotal - 1; 
      i >= profitDrawdown[strategyIndex][IDX_ORDERS_HISTORY_TOTAL]; 
      i--)
  {
    nOrderSelect(i, SELECT_BY_POS, MODE_HISTORY);
    
    if(nOrderMagicNumber() != strategyInstanceId)  // ← Filter by magic number
      continue;
    
    // Calculate profit and drawdown statistics
    double orderProfit = nOrderProfit() + nOrderCommission();
    profitDrawdown[strategyIndex][IDX_CURRENT_PROFIT_PERCENT] += 
      100 * orderProfit / orderOpenBalance;
    
    // Track largest profit and drawdown
    // ...
  }
}
```

4. **Framework Uses History for Instance Balance**

```mql4
// Common.mq4 - c_getInstanceBalance() (lines 905-919)
double c_getInstanceBalance(int strategyIndex, ...)
{
  // Process closed orders to calculate instance balance
  for(; nextIndex < nOrdersHistoryTotal(); nextIndex++)
  {
    if(nOrderSelect(nextIndex, SELECT_BY_POS, MODE_HISTORY))
    {
      if(nOrderMagicNumber() == strategyInstanceId)  // ← Filter by magic number
      {
        instanceBalance += nOrderProfit() + nOrderCommission();
      }
    }
  }
  return instanceBalance;
}
```

### Python Solution: Currently Missing Trade History

The Python solution **currently only fetches OPEN positions**, not closed trade history from the broker API.

#### Current Implementation

```python
# account.py - getTrades() (OANDA REST example, lines 522-659)
def getTrades(self, maxOrderArraySize, instanceID):
    # 1. Fetch ONLY open positions from broker API
    url = "https://" + self.server + "/v1/accounts/%s/positions" % (self.accountID)
    req = requests.get(url, headers = self.headers)
    resp = req.json()
    self.openTrades = resp['positions']  # ← ONLY open positions
    
    # 2. Fill openOrderInfo array with open positions
    for trade in self.openTrades:
        self.openOrderInfo[index].ticket = ...
        self.openOrderInfo[index].isOpen = 1  # ← All marked as open
        # ...
    
    # 3. For OrderWrapper: Load virtual order history from TinyDB
    #    (This is only for virtual trades, not real broker history)
    db = TinyDB('./data/' + str(self.accountID) + "_orderHistory.json")
    all_trades = db.all()  # ← Only virtual trades, not broker history
    # ...
```

#### What's Missing

1. **No Broker Trade History Fetching**
   - Python solution does NOT fetch closed trades from broker API
   - Only fetches open positions
   - Framework receives only open orders, not closed order history

2. **Impact on Framework**
   - Framework cannot calculate profit/drawdown statistics properly
   - Framework cannot track instance balance correctly
   - Framework cannot work well with manual trades (no history context)

3. **OrderWrapper History is Different**
   - OrderWrapper stores virtual trade history in TinyDB
   - This is for virtual trades only (NFA-compliant brokers)
   - Not the same as real broker trade history

### Required Feature: Fetch Trade History in Python Solution

#### What Needs to Be Added

**1. Fetch Closed Trades from Broker API**

```python
# account.py - New method needed
def getTradeHistory(self, maxOrderArraySize, instanceID, symbol=None):
    """
    Fetch closed trade history from broker API.
    Similar to getTrades() but for closed orders.
    """
    # OANDA REST API
    url = "https://" + self.server + "/v1/accounts/%s/trades" % (self.accountID)
    params = {
        'state': 'closed',  # Only closed trades
        'count': maxOrderArraySize  # Limit results
    }
    if symbol:
        params['instrument'] = OandaSymbol[symbol]  # Filter by symbol
    
    req = requests.get(url, headers=self.headers, params=params)
    resp = req.json()
    closedTrades = resp['trades']  # ← Closed trades
    
    # Filter by instanceID (if broker supports order comments/tags)
    # Note: OANDA doesn't support magic numbers, so filtering may be limited
    
    # Fill orderInfo array with closed trades
    for trade in closedTrades:
        orderInfo[index].ticket = trade['id']
        orderInfo[index].isOpen = False  # ← Mark as closed
        orderInfo[index].closeTime = parse_time(trade['closeTime'])
        orderInfo[index].closePrice = trade['closePrice']
        orderInfo[index].profit = trade['realizedPL']
        # ...
```

**2. Combine Open and Closed Orders**

```python
# account.py - Modified getTrades()
def getTrades(self, maxOrderArraySize, instanceID):
    # 1. Get open positions
    self.getOpenTrades(maxOrderArraySize, instanceID)
    
    # 2. Get closed trade history
    closedTrades = self.getTradeHistory(maxOrderArraySize, instanceID)
    
    # 3. Combine into single orderInfo array
    #    (Open orders first, then closed orders)
    orderCount = len(self.openOrderInfo)
    for closedTrade in closedTrades:
        if orderCount < maxOrderArraySize:
            self.openOrderInfo[orderCount] = closedTrade
            orderCount += 1
```

**3. Filter by instanceID/Magic Number**

**Challenge**: OANDA and JForex APIs may not support magic numbers/instanceID in the same way as MetaTrader.

**Solutions**:
- **OANDA**: Use order tags/comments (if supported)
- **JForex**: Use order comments (already used: `trade.getComment() == str(instanceID)`)
- **Fallback**: If no magic number support, include all trades (less ideal)

#### Broker API Support

**OANDA REST API:**
```python
# OANDA supports filtering by state (open/closed)
GET /v1/accounts/{accountID}/trades?state=closed

# But may not support filtering by magic number/instanceID
# May need to filter client-side by order comment/tag
```

**OANDA Java SDK:**
```python
# OANDA Java SDK may support order tags
positions = account.getPositions()
for position in positions:
    # Check position tags/comments for instanceID
    # (Implementation depends on SDK version)
```

**JForex:**
```python
# JForex already supports order comments
orders = engine.getOrders()
for order in orders:
    if order.getComment() == str(instanceID):  # ← Already filtering
        # Include this order
```

### Benefits of Adding Trade History

1. **Framework Statistics Work Correctly**
   - Profit/drawdown calculations accurate
   - Instance balance tracking correct
   - Worst case scenario detection works

2. **Manual Trade Handling**
   - Framework sees manual trades in history
   - Framework can calculate statistics including manual trades
   - Framework can work with manual trade modifications (matched by instanceID)

3. **Consistency with MQL4/5**
   - Same behavior as MQL4/5 implementation
   - Framework receives same data structure
   - No feature gap

---

## Part 2: Multiple EAs in MetaTrader

### Execution Model: Parallel EAs, Sequential Strategies

**Multiple EAs in MetaTrader run in parallel**, but **strategies within each EA run sequentially**.

### How Multiple EAs Work

```
MetaTrader Platform
│
├── Chart 1: EURUSD H1
│   └── EA 1 (Kantu.mq4, instanceID=10021)
│       └── Strategy execution (sequential)
│           ├── Strategy 1 (if multiple in EA)
│           └── Strategy 2 (if multiple in EA)
│
├── Chart 2: GBPUSD H1
│   └── EA 2 (Kantu.mq4, instanceID=10038)
│       └── Strategy execution (sequential)
│
├── Chart 3: USDJPY H1
│   └── EA 3 (Kantu.mq4, instanceID=20004)
│       └── Strategy execution (sequential)
│
└── Chart 4: EURUSD M15
    └── EA 4 (Teyacanani.mq5, instanceID=30001)
        └── Strategy execution (sequential)
```

### Key Characteristics

1. **Each EA Runs on Its Own Chart**
   - Each EA is attached to a specific chart
   - Each EA has its own execution context
   - Each EA can have its own instanceID (magic number)

2. **MetaTrader Handles Parallel Execution**
   - MetaTrader platform manages EA execution
   - Each EA runs in its own execution context
   - EAs can execute simultaneously (parallel)
   - MetaTrader handles threading/scheduling

3. **Within Each EA: Sequential Execution**
   - Strategies within one EA execute sequentially
   - Same as Python solution
   - Same as single EA execution

4. **Order Filtering by Magic Number**
   - Each EA uses its own magic number (instanceID)
   - Orders are filtered by magic number
   - EAs don't interfere with each other's orders
   - Manual trades with matching magic number are included

### Execution Timeline Example

```
Time 0ms:  MetaTrader tick event
Time 0ms:  ├── EA 1 (Chart 1) starts execution
Time 0ms:  ├── EA 2 (Chart 2) starts execution  ← Parallel
Time 0ms:  ├── EA 3 (Chart 3) starts execution  ← Parallel
Time 0ms:  └── EA 4 (Chart 4) starts execution  ← Parallel

Time 5ms:  EA 1 completes
Time 8ms:  EA 2 completes
Time 12ms: EA 3 completes
Time 15ms: EA 4 completes

Time 100ms: Next tick event
Time 100ms: All EAs start again (parallel)
```

### Comparison: Multiple EAs vs Python Solution

| Aspect | Multiple EAs (MQL4/5) | Python Solution |
|--------|----------------------|-----------------|
| **Execution Model** | Parallel EAs, Sequential strategies | Sequential strategies only |
| **Isolation** | Each EA on separate chart | All strategies in one process |
| **Order Filtering** | Magic number per EA | instanceID per strategy |
| **Resource Usage** | Multiple EA processes | Single Python process |
| **Coordination** | No coordination needed | Shared account object |
| **Manual Trades** | Filtered by magic number | Filtered by instanceID (if implemented) |

### Python Solution Equivalent

**To achieve similar isolation in Python:**

1. **Run Multiple Python Processes**
   ```bash
   # Process 1: EURUSD strategy
   python atrader.py -c config_eurusd.config
   
   # Process 2: GBPUSD strategy
   python atrader.py -c config_gbpusd.config
   
   # Process 3: USDJPY strategy
   python atrader.py -c config_usdjpy.config
   ```
   - Each process is isolated
   - Each process has its own account object
   - Can run in parallel (OS handles scheduling)

2. **Single Process with Multiple Strategies** (Current)
   - All strategies in one process
   - Sequential execution
   - Shared account object
   - Less isolation

### Why Multiple EAs Can Run in Parallel

**MetaTrader Platform Architecture:**

```
MetaTrader Terminal
│
├── Execution Engine (Multi-threaded)
│   ├── Thread 1: EA 1 (Chart 1)
│   ├── Thread 2: EA 2 (Chart 2)
│   ├── Thread 3: EA 3 (Chart 3)
│   └── Thread 4: EA 4 (Chart 4)
│
└── Data Layer (Shared)
    ├── Account data (thread-safe)
    ├── Order data (thread-safe)
    └── Price data (thread-safe)
```

**Key Points:**
- MetaTrader uses multi-threading internally
- Each EA can run in its own thread
- Data layer is thread-safe
- EAs don't block each other

**Within Each EA:**
- Strategies execute sequentially (same thread)
- No parallel execution within EA
- Same as Python solution

---

## Summary

### Trade History Feature

**MQL4/5:**
- ✅ Fetches both open and closed orders
- ✅ Filters by magic number (instanceID)
- ✅ Framework receives complete trade history
- ✅ Framework can calculate statistics correctly
- ✅ Framework works with manual trades (matched by magic number)

**Python Solution:**
- ❌ Currently only fetches open positions
- ❌ Missing closed trade history from broker API
- ❌ Framework cannot calculate statistics correctly
- ❌ Framework cannot work well with manual trades
- ✅ **Feature needs to be added**

**Required Implementation:**
1. Add `getTradeHistory()` method to fetch closed trades from broker API
2. Combine open and closed orders into single `orderInfo` array
3. Filter by instanceID (if broker API supports it)
4. Pass complete order history to Framework (same as MQL4/5)

### Multiple EAs Execution

**MQL4/5:**
- ✅ Multiple EAs run in parallel (MetaTrader handles)
- ✅ Each EA on separate chart (isolated)
- ✅ Strategies within EA run sequentially
- ✅ Orders filtered by magic number

**Python Solution:**
- ❌ Single process, sequential execution
- ❌ All strategies share same account object
- ✅ Can run multiple processes for isolation (manual setup)
- ✅ Strategies filtered by instanceID

**Key Difference:**
- **MQL4/5**: MetaTrader platform provides parallel execution automatically
- **Python**: Must manually run multiple processes for parallel execution

---

**Document Status**: Complete
**Last Updated**: December 2024

