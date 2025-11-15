# Python Solution: Process Management, Trade Matching & Price Data

## Part 1: Multiple Process Management

### Current State: No Process Manager

The Python solution currently runs as **independent processes** with no centralized manager:

```bash
# Current approach: Manual process management
python atrader.py -c config_eurusd.config &
python atrader.py -c config_gbpusd.config &
python atrader.py -c config_usdjpy.config &
```

**Limitations:**
- No centralized monitoring
- No automatic restart on failure
- No coordinated startup/shutdown
- No health checks
- No resource management
- Manual process management required

### Solution: Process Manager Architecture

#### Option 1: Custom Manager Process (Recommended)

**Architecture:**

```
┌─────────────────────────────────────────────────────────┐
│         Manager Process (atrader_manager.py)            │
│  - Configuration management                              │
│  - Process lifecycle management                         │
│  - Health monitoring                                     │
│  - Log aggregation                                       │
│  - Resource management                                   │
└───────────────────┬─────────────────────────────────────┘
                    │
        ┌───────────┼───────────┐
        │           │           │
┌───────▼───┐ ┌─────▼─────┐ ┌───▼──────┐
│ Worker 1  │ │ Worker 2  │ │ Worker 3│
│ (EURUSD)  │ │ (GBPUSD)   │ │ (USDJPY)│
│           │ │            │ │         │
│ - Strategy│ │ - Strategy │ │ - Strategy│
│ - Account │ │ - Account │ │ - Account│
│ - Broker  │ │ - Broker  │ │ - Broker │
└───────────┘ └───────────┘ └─────────┘
```

**Implementation:**

```python
# atrader_manager.py
import subprocess
import signal
import time
import json
import os
from multiprocessing import Process, Queue
import logging

class StrategyWorker:
    def __init__(self, config_file, instance_id, manager_queue):
        self.config_file = config_file
        self.instance_id = instance_id
        self.manager_queue = manager_queue
        self.process = None
        self.status = "stopped"
        
    def start(self):
        """Start worker process"""
        self.process = subprocess.Popen(
            ['python', 'atrader.py', '-c', self.config_file],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )
        self.status = "running"
        self.manager_queue.put({
            'type': 'worker_started',
            'instance_id': self.instance_id,
            'pid': self.process.pid
        })
        
    def stop(self):
        """Stop worker process gracefully"""
        if self.process:
            self.process.terminate()
            try:
                self.process.wait(timeout=10)
            except subprocess.TimeoutExpired:
                self.process.kill()
            self.status = "stopped"
            
    def restart(self):
        """Restart worker process"""
        self.stop()
        time.sleep(2)
        self.start()
        
    def is_alive(self):
        """Check if process is still running"""
        if self.process:
            return self.process.poll() is None
        return False

class ProcessManager:
    def __init__(self, config_file='manager_config.json'):
        self.config = self.load_config(config_file)
        self.workers = {}
        self.manager_queue = Queue()
        self.running = True
        
    def load_config(self, config_file):
        """Load manager configuration"""
        with open(config_file, 'r') as f:
            return json.load(f)
            
    def initialize_workers(self):
        """Initialize all worker processes from config"""
        for strategy_config in self.config['strategies']:
            instance_id = strategy_config['instance_id']
            config_file = strategy_config['config_file']
            
            worker = StrategyWorker(config_file, instance_id, self.manager_queue)
            self.workers[instance_id] = worker
            
    def start_all(self):
        """Start all worker processes"""
        for worker in self.workers.values():
            worker.start()
            
    def stop_all(self):
        """Stop all worker processes gracefully"""
        for worker in self.workers.values():
            worker.stop()
            
    def monitor_workers(self):
        """Monitor worker health and restart if needed"""
        while self.running:
            for instance_id, worker in self.workers.items():
                if not worker.is_alive() and worker.status == "running":
                    logging.warning(f"Worker {instance_id} died, restarting...")
                    worker.restart()
            time.sleep(5)  # Check every 5 seconds
            
    def handle_signals(self):
        """Handle shutdown signals"""
        def signal_handler(sig, frame):
            logging.info("Shutdown signal received, stopping all workers...")
            self.running = False
            self.stop_all()
            exit(0)
            
        signal.signal(signal.SIGINT, signal_handler)
        signal.signal(signal.SIGTERM, signal_handler)
        
    def run(self):
        """Main manager loop"""
        self.handle_signals()
        self.initialize_workers()
        self.start_all()
        
        # Start monitoring in background thread
        monitor_thread = threading.Thread(target=self.monitor_workers)
        monitor_thread.daemon = True
        monitor_thread.start()
        
        # Main loop: process messages from workers
        while self.running:
            try:
                if not self.manager_queue.empty():
                    message = self.manager_queue.get_nowait()
                    self.handle_message(message)
            except:
                pass
            time.sleep(1)
            
    def handle_message(self, message):
        """Handle messages from workers"""
        msg_type = message['type']
        if msg_type == 'worker_started':
            logging.info(f"Worker {message['instance_id']} started (PID: {message['pid']})")
        elif msg_type == 'worker_error':
            logging.error(f"Worker {message['instance_id']} error: {message['error']}")
        # ... handle other message types

if __name__ == '__main__':
    manager = ProcessManager('manager_config.json')
    manager.run()
```

**Manager Configuration (`manager_config.json`):**

```json
{
  "strategies": [
    {
      "instance_id": 10021,
      "config_file": "config/eurusd_strategy.config",
      "restart_on_failure": true,
      "max_restarts": 5
    },
    {
      "instance_id": 10038,
      "config_file": "config/gbpusd_strategy.config",
      "restart_on_failure": true,
      "max_restarts": 5
    },
    {
      "instance_id": 20004,
      "config_file": "config/usdjpy_strategy.config",
      "restart_on_failure": true,
      "max_restarts": 5
    }
  ],
  "monitoring": {
    "health_check_interval": 5,
    "log_aggregation": true,
    "resource_monitoring": true
  }
}
```

**Usage:**

```bash
# Start manager (manages all workers)
python atrader_manager.py

# Manager will:
# 1. Start all worker processes
# 2. Monitor their health
# 3. Restart failed workers
# 4. Aggregate logs
# 5. Handle graceful shutdown
```

#### Option 2: System Service Manager (systemd/supervisor)

**systemd Service (`/etc/systemd/system/atrader-manager.service`):**

```ini
[Unit]
Description=Asirikuy Trader Process Manager
After=network.target

[Service]
Type=simple
User=trading
WorkingDirectory=/opt/asirikuy-trader
ExecStart=/usr/bin/python3 /opt/asirikuy-trader/atrader_manager.py
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
```

**Supervisor Configuration (`/etc/supervisor/conf.d/atrader.conf`):**

```ini
[program:atrader-manager]
command=/usr/bin/python3 /opt/asirikuy-trader/atrader_manager.py
directory=/opt/asirikuy-trader
autostart=true
autorestart=true
stderr_logfile=/var/log/atrader-manager.err.log
stdout_logfile=/var/log/atrader-manager.out.log
```

#### Option 3: Docker Compose (Container-based)

**`docker-compose.yml`:**

```yaml
version: '3.8'

services:
  manager:
    build: .
    command: python atrader_manager.py
    volumes:
      - ./config:/app/config
      - ./data:/app/data
      - ./log:/app/log
    restart: always
    
  worker-eurusd:
    build: .
    command: python atrader.py -c config/eurusd_strategy.config
    volumes:
      - ./config:/app/config
      - ./data:/app/data
      - ./log:/app/log
    depends_on:
      - manager
    restart: always
    
  worker-gbpusd:
    build: .
    command: python atrader.py -c config/gbpusd_strategy.config
    volumes:
      - ./config:/app/config
      - ./data:/app/data
      - ./log:/app/log
    depends_on:
      - manager
    restart: always
```

### Benefits of Process Manager

1. **Centralized Management**
   - Single point of control
   - Unified configuration
   - Coordinated startup/shutdown

2. **Automatic Recovery**
   - Restart failed processes
   - Health monitoring
   - Error detection

3. **Resource Management**
   - CPU/memory monitoring
   - Process limits
   - Resource allocation

4. **Logging & Monitoring**
   - Log aggregation
   - Centralized logging
   - Health status dashboard

5. **Scalability**
   - Easy to add/remove workers
   - Dynamic configuration
   - Load balancing (future)

---

## Part 2: Trade Matching (Magic Number / Trade ID)

### Current Implementation

The Python solution uses **different mechanisms** for trade matching depending on the broker:

#### OANDA REST API: **No Magic Number Support**

```python
# account.py - OandaAccount.getTrades() (lines 522-575)
def getTrades(self, maxOrderArraySize, instanceID):
    # Fetch all open positions
    url = "https://" + self.server + "/v1/accounts/%s/positions" % (self.accountID)
    self.openTrades = resp['positions']  # ← ALL positions (no filtering)
    
    # Fill orderInfo array
    for index, trade in enumerate(self.openTrades):
        self.openOrderInfo[index].ticket = float(ticket)  # ← Sequential ticket number
        self.openOrderInfo[index].instanceId = 0  # ← NO magic number support
        # ...
```

**Problem:**
- OANDA REST API does **NOT support magic numbers** (instanceID)
- All positions are fetched, regardless of instanceID
- Cannot filter trades by strategy instance
- Manual trades cannot be distinguished from automated trades

**When Opening Trades:**

```python
# account.py - OandaAccount.openTrade() (lines 797-818)
def openTrade(self, lots, direction, pair, stopLoss, takeProfit, instanceID):
    # OANDA REST API does NOT support order comments/tags
    # Cannot attach instanceID to order
    url = "https://" + self.server + "/v1/accounts/" + str(self.accountID) + "/orders"
    # ... no instanceID in request
```

#### JForex: **Uses Order Comment (Magic Number)**

```python
# account.py - JForexAccount.getTrades() (lines 976-1119)
def getTrades(self, maxOrderArraySize, instanceID):
    # Fetch all open orders
    self.openTrades = self.dcw.dumbStrategy.getOrders()
    
    # Filter by instanceID using order comment
    for index, trade in enumerate(self.openTrades):
        if trade.getComment() == str(instanceID) or self.useOrderWrapper:  # ← Filter by comment
            self.openOrderInfo[index].ticket = float(trade.getId())  # ← Real trade ID
            self.openOrderInfo[index].instanceId = float(trade.getComment())  # ← Magic number from comment
            # ...
```

**How It Works:**
- JForex supports **order comments** (similar to MQL4/5 magic numbers)
- When opening a trade, `instanceID` is set as the order comment
- When fetching trades, orders are filtered by `trade.getComment() == str(instanceID)`
- Manual trades with matching comment are included

**When Opening Trades:**

```python
# account.py - JForexAccount.openTrade() (lines 1174-1208)
def openTrade(self, lots, direction, pair, stopLoss, takeProfit, instanceID):
    label = 'ID%d' % time.time()
    # Pass instanceID as comment
    self.dcw.dumbStrategy.openOrder(label, lots, directionString, 
                                    DukascopySymbol[pair], stopLoss, takeProfit, 
                                    -1.0, str(instanceID));  # ← instanceID as comment
```

**When Closing/Modifying Trades:**

```python
# account.py - JForexAccount.closeTrade() (lines 1210-1246)
def closeTrade(self, ticket, instanceID, orderType):
    if ticket != -1:
        # Close by ticket ID
        self.dcw.dumbStrategy.closeOrder(str(ticket))
    else:
        # Close by instanceID (filter by comment)
        for trade in self.openTrades:
            if trade.getComment() == str(instanceID):  # ← Filter by comment
                self.dcw.dumbStrategy.closeOrder(trade.getId())
```

#### OANDA Java SDK: **Uses Order Tags (if supported)**

```python
# account.py - OandaJavaAccount.getTrades() (lines 1504-1631)
def getTrades(self, maxOrderArraySize, instanceID):
    # OANDA Java SDK may support order tags
    # Implementation depends on SDK version
    # Similar to JForex: filter by tag/comment if available
```

### Comparison: Trade Matching Mechanisms

| Broker | Magic Number Support | Trade ID | Filtering Method | Manual Trade Detection |
|--------|---------------------|----------|------------------|------------------------|
| **OANDA REST** | ❌ No | ✅ Yes (sequential) | ❌ None (all trades) | ❌ Cannot distinguish |
| **JForex** | ✅ Yes (comment) | ✅ Yes (real ID) | ✅ By comment | ✅ Can filter by comment |
| **OANDA Java** | ⚠️ Maybe (tags) | ✅ Yes | ⚠️ By tag (if supported) | ⚠️ Depends on SDK |
| **MQL4/5** | ✅ Yes (magic number) | ✅ Yes | ✅ By magic number | ✅ Can filter by magic number |

### Problem: OANDA REST API Limitation

**Current Issue:**
- OANDA REST API does **NOT support** order comments, tags, or magic numbers
- Cannot filter trades by instanceID
- All positions are included, regardless of strategy
- Manual trades cannot be distinguished from automated trades

**Workaround: OrderWrapper**
- For NFA-compliant brokers (like OANDA), use `OrderWrapper`
- OrderWrapper maintains **virtual trades** in TinyDB
- Virtual trades are filtered by instanceID
- Real broker positions are balanced with virtual positions

```python
# account.py - OandaAccount.getTrades() (lines 576-659)
# If OrderWrapper is enabled, load virtual trades
if self.useOrderWrapper:
    db = TinyDB('./data/' + str(self.accountID) + "_openOrders.json")
    all_trades = db.all()
    
    # Filter virtual trades by instanceID
    for trade in all_trades:
        if trade['InstanceID'] == instanceID:  # ← Filter by instanceID
            self.virtualOpenOrderInfo[i].instanceId = trade['InstanceID']
            # ...
```

### Solution: Enhanced Trade Matching

**For OANDA REST API (if tags become available):**

```python
# Enhanced openTrade() with tag support
def openTrade(self, lots, direction, pair, stopLoss, takeProfit, instanceID):
    url = "https://" + self.server + "/v1/accounts/" + str(self.accountID) + "/orders"
    
    data = {
        "instrument": OandaSymbol[pair],
        "units": int(lots * 100000),
        "side": directionString,
        "type": "market",
        "stopLoss": stopLoss,
        "takeProfit": takeProfit,
        "tag": f"instanceID_{instanceID}"  # ← Add tag if API supports it
    }
    
    req = requests.post(url, headers=self.headers, json=data)
```

**For Trade History (to match MQL4/5 behavior):**

```python
# Enhanced getTradeHistory() with filtering
def getTradeHistory(self, maxOrderArraySize, instanceID, symbol=None):
    url = "https://" + self.server + "/v1/accounts/%s/trades" % (self.accountID)
    params = {
        'state': 'closed',
        'count': maxOrderArraySize
    }
    
    # If API supports tag filtering:
    if symbol:
        params['instrument'] = OandaSymbol[symbol]
    # Note: OANDA REST may not support tag filtering in history
    
    req = requests.get(url, headers=self.headers, params=params)
    closedTrades = resp['trades']
    
    # Filter by tag/comment client-side (if available)
    filteredTrades = []
    for trade in closedTrades:
        if trade.get('tag') == f"instanceID_{instanceID}":  # ← Filter by tag
            filteredTrades.append(trade)
    
    return filteredTrades
```

---

## Part 3: Price Data Fetching

### Current Implementation: **Both Historical and Latest Prices**

The Python solution fetches **BOTH historical candle data AND latest bid/ask prices**:

#### 1. Historical Candle Data (`getCandles()`)

```python
# account.py - OandaAccount.getCandles() (lines 392-508)
def getCandles(self, numCandles, pair, timeframe, allSymbolsLoaded, allRatesLoaded, isPlotter=False):
    # Fetch historical candles from broker API
    count = 500  # Default: fetch 500 candles
    if pair+str(timeframe) in allSymbolsLoaded:
        count = 10  # If cached, only fetch 10 new candles
    
    url = "https://" + self.server + "/v1/candles?instrument=" + OandaSymbol[pair] + \
          "&count=" + str(count) + "&granularity=" + OandaTF[timeframe]
    
    req = requests.get(url, headers=self.headers)
    resp = req.json()
    candles = resp['candles']  # ← Historical OHLCV data
    
    # Convert to Rate array for Framework
    RatesType = Rate * numCandles
    rates = RatesType()
    
    for i, candle in enumerate(reversed(candles)):
        rates[numCandles-1-i].time = int(rfc3339.FromTimestamp(candle['time']))
        rates[numCandles-1-i].open = float(candle['openBid'])
        rates[numCandles-1-i].high = float(candle['highBid'])
        rates[numCandles-1-i].low = float(candle['lowBid'])
        rates[numCandles-1-i].close = float(candle['closeBid'])
        rates[numCandles-1-i].volume = 4
    
    # If not enough candles, supplement from local CSV file
    if neededSupplementaryCandles > 0:
        result = loadRates('./history/' + pair + '_' + str(timeframe) + '.csv')
        # Merge with broker data
        # ...
    
    return rates  # ← Full historical candle array
```

**Key Points:**
- Fetches **historical OHLCV candles** (Open, High, Low, Close, Volume)
- Default: 500 candles from broker API
- If cached: Only 10 new candles
- If insufficient: Supplements from local CSV file (`./history/`)
- Returns **full historical array** for Framework

#### 2. Latest Bid/Ask Prices (`getBidAsk()`)

```python
# account.py - OandaAccount.getBidAsk() (lines 661-740)
def getBidAsk(self, pair, basePair, quotePair, allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded):
    pairIndex = -1
    bidAskExpired = False
    
    # Check cache (5 second expiration)
    if pair in allBidAskSymbols:
        pairIndex = allBidAskSymbols.index(pair)
        if (datetime.datetime.now() - allBidAskUpdateTimes[pairIndex]).total_seconds() > 5.0:
            bidAskExpired = True  # ← Cache expired
    
    # Fetch latest prices if not cached or expired
    if pairIndex == -1 or bidAskExpired == True:
        url = "https://" + self.server + "/v1/prices?instruments=" + OandaSymbol[pair]
        req = requests.get(url, headers=self.headers)
        resp = req.json()
        prices = resp['prices']  # ← Latest bid/ask prices
    else:
        prices = allBidAskLoaded[pairIndex]  # ← Use cached prices
    
    # Extract bid/ask
    for price in prices:
        self.currentBrokerTime = int(rfc3339.FromTimestamp(price['time']))
        self.BidAsk[0] = price['bid']  # ← Latest bid
        self.BidAsk[1] = price['ask']  # ← Latest ask
        # ...
    
    # Update cache
    if pairIndex == -1:
        allBidAskSymbols.append(pair)
        allBidAskLoaded.append(prices)
        allBidAskUpdateTimes.append(datetime.datetime.now())
    elif bidAskExpired:
        allBidAskLoaded[pairIndex] = prices
        allBidAskUpdateTimes[pairIndex] = datetime.datetime.now()
```

**Key Points:**
- Fetches **latest bid/ask prices** (current market prices)
- Cached for 5 seconds (`BID_ASK_EXPIRATION_IN_SECONDS = 5.0`)
- Used for order execution (entry/exit prices)
- Also fetches conversion pair prices (base/quote currency conversion)

### Data Flow Summary

```
Strategy Execution Cycle:
│
├── 1. getCandles() → Historical OHLCV data
│   ├── Fetch 500 candles from broker API (or 10 if cached)
│   ├── Supplement from local CSV if needed
│   └── Return full historical array (numCandles bars)
│
├── 2. getBidAsk() → Latest bid/ask prices
│   ├── Fetch latest prices from broker API (if cache expired)
│   ├── Cache for 5 seconds
│   └── Return current bid/ask + conversion prices
│
└── 3. getTrades() → Open positions
    ├── Fetch all open positions from broker API
    └── Filter by instanceID (if broker supports it)
```

### Comparison: Price Data Fetching

| Data Type | Source | Frequency | Caching | Purpose |
|-----------|--------|-----------|---------|---------|
| **Historical Candles** | Broker API + Local CSV | Once per symbol/timeframe | Yes (in-memory) | Framework strategy calculations |
| **Latest Bid/Ask** | Broker API | Every 5 seconds (cached) | Yes (5 sec cache) | Order execution, current prices |
| **Conversion Prices** | Broker API | Every 5 seconds (cached) | Yes (5 sec cache) | Currency conversion |

### Optimization: Caching Strategy

**Current Caching:**
- **Historical candles**: Cached in `allRatesLoaded[]` (in-memory)
- **Bid/ask prices**: Cached for 5 seconds in `allBidAskLoaded[]`
- **Symbols loaded**: Tracked in `allSymbolsLoaded[]`

**Benefits:**
- Reduces API calls
- Improves performance
- Reduces rate limiting issues

**Limitations:**
- In-memory cache (lost on restart)
- No persistent cache
- Cache expiration is fixed (5 seconds for bid/ask)

---

## Summary

### Process Management

**Current State:**
- ❌ No process manager
- ❌ Manual process management
- ❌ No automatic recovery

**Recommended Solution:**
- ✅ Custom manager process (`atrader_manager.py`)
- ✅ Automatic restart on failure
- ✅ Health monitoring
- ✅ Log aggregation

### Trade Matching

**OANDA REST:**
- ❌ No magic number support
- ✅ Trade ID (sequential)
- ❌ Cannot filter by instanceID
- ⚠️ Workaround: OrderWrapper (virtual trades)

**JForex:**
- ✅ Magic number via order comment
- ✅ Trade ID (real broker ID)
- ✅ Can filter by instanceID
- ✅ Manual trades can be matched

**Required Enhancement:**
- Add trade history fetching (closed orders)
- Filter closed orders by instanceID (if broker supports it)
- Match MQL4/5 behavior

### Price Data

**Historical Candles:**
- ✅ Fetches full historical OHLCV data
- ✅ 500 candles from broker (or 10 if cached)
- ✅ Supplements from local CSV if needed
- ✅ Returns full array for Framework

**Latest Prices:**
- ✅ Fetches latest bid/ask prices
- ✅ Cached for 5 seconds
- ✅ Used for order execution

**Answer:** Python solution fetches **BOTH** historical candle data AND latest bid/ask prices, not just the latest price.

---

**Document Status**: Complete
**Last Updated**: December 2024

