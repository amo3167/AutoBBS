# Python Solution: Multiple Strategies & Symbols Execution Model

## Overview

The Python solution (`asirikuy-trader`) uses a **single-threaded, sequential execution model** for strategy execution. Multiple strategies and symbols are handled through a **main loop** that executes strategies one after another, not in parallel.

---

## Execution Model: Single-Threaded Sequential

### Main Execution Loop

```python
# atrader.py - Main loop (lines 395-475)
while True:
    try:
        # 1. Check weekend/time restrictions
        if (weekend_time):
            sleep(60)
            continue
        
        # 2. Execute ALL strategies sequentially
        account.runStrategies(allSymbolsLoaded, allRatesLoaded, 
                              allBidAskSymbols, allBidAskUpdateTimes, 
                              allBidAskLoaded)
        
        # 3. Display UI for all strategies
        for strategy in account.strategies:
            account.displaySystemUI(strategy.instanceID)
        
        # 4. Sleep for executionTime seconds (default: 30 seconds)
        sleep(executionTime)
        
    except KeyboardInterrupt:
        exit()
    except Exception:
        # Handle errors
```

### Strategy Execution Flow

```python
# account.py - runStrategies() (lines 69-76)
def runStrategies(self, allSymbolsLoaded, allRatesLoaded, 
                  allBidAskSymbols, allBidAskUpdateTimes, 
                  allBidAskLoaded, symbol=None):
    # 1. Get account info once (shared by all strategies)
    self.getAccountInfo(allBidAskSymbols, allBidAskUpdateTimes, allBidAskLoaded)
    
    # 2. Execute each strategy SEQUENTIALLY
    for strategy in self.strategies:
        # Check if strategy is stuck (not run in 5 minutes)
        if (strategy.lastRunningTime - datetime.datetime.now()).seconds > 300:
            strategy.isRunning = False
        
        # Execute strategy if not already running and symbol matches
        if (symbol == None or symbol == DukascopySymbol[strategy.pair]) and not strategy.isRunning:
            strategy.isRunning = True
            strategy.run(allSymbolsLoaded, allRatesLoaded, 
                        allBidAskSymbols, allBidAskUpdateTimes, 
                        allBidAskLoaded)
```

### Strategy.run() Execution

```python
# strategy.py - run() (lines 138-424)
def run(self, allSymbolsLoaded, allRatesLoaded, 
        allBidAskSymbols, allBidAskUpdateTimes, 
        allBidAskLoaded):
    # 1. Get account info (may be cached)
    self.account.getAccountInfo(...)
    
    # 2. Get historical rates for this strategy's symbols
    for i in range(0, 9):
        if self.symbolRequirements[i] != "N":
            if self.symbolRequirements[i] != "D":
                # Get rates for specific symbol
                rates = self.account.getCandles(maxNumCandles, 
                                                self.symbolRequirements[i], 
                                                self.passedTimeFrame, ...)
            else:
                # Get rates for default symbol (strategy.pair)
                rates = self.account.getCandles(maxNumCandles, 
                                                self.pair, 
                                                self.passedTimeFrame, ...)
    
    # 3. Get open trades for this instance
    self.account.getTrades(self.settings[ORDERINFO_ARRAY_SIZE], 
                          self.instanceID)
    
    # 4. Get bid/ask for this strategy's symbol
    self.account.getBidAsk(self.pair, self.basePair, self.quotePair, ...)
    
    # 5. Call Framework C library
    f4output = self.asfdll.c_runStrategy(...)
    
    # 6. Process results and execute orders
    # ...
```

---

## Threading Usage

### Threads Used (Limited)

The Python solution uses **very limited threading** - only for auxiliary services:

#### 1. WebServer Thread (Optional)

```python
# web_server.py
class WebServer(threading.Thread):
    def run(self):
        server = HTTPServer(('', self.port), myHandler)
        server.serve_forever()  # Runs in separate thread

# atrader.py (lines 375-380)
if enableWebServer:
    web_server = WebServer(webServerPort)
    web_server.daemon = True  # Dies when main thread dies
    web_server.start()  # Starts in background thread
```

**Purpose**: Serves HTML/PNG files for web interface  
**Thread Type**: Daemon thread (dies when main thread exits)  
**Impact**: Does NOT affect strategy execution

#### 2. Email Logging Thread (Optional)

```python
# misc.py - ThreadedTlsSMTPHandler
class ThreadedTlsSMTPHandler(logging.handlers.SMTPHandler):
    def emit(self, record):
        thread = Thread(target=smtp_at_your_own_leasure, ...)
        thread.start()  # Sends email in background thread
```

**Purpose**: Sends email logs asynchronously  
**Thread Type**: Background thread (one per email)  
**Impact**: Does NOT affect strategy execution

#### 3. GraphPlotter (NOT Actually Threaded)

```python
# plotter.py
class GraphPlotter(threading.Thread):
    def __init__(self, ...):
        # threading.Thread.__init__(self)  # COMMENTED OUT!
        # So it's NOT actually a thread!

# atrader.py (lines 453-461)
if useOrderWrapper and graphicPaintingPeriod > 0:
    if graphExecutionCounter == 0:
        graph_plotter.run(...)  # Called SYNCHRONOUSLY in main thread
        graphExecutionCounter = graphExecutionCounter + 1
```

**Purpose**: Generates plots and HTML reports  
**Thread Type**: NOT a thread (despite inheriting from Thread)  
**Impact**: Runs synchronously in main thread (blocks strategy execution)

#### 4. OANDA Java Client Internal Threads

```python
# account.py - OandaJavaAccount
self.fxclient.setWithRateThread(True)      # Internal Java thread for rates
self.fxclient.setWithKeepAliveThread(True) # Internal Java thread for keep-alive
```

**Purpose**: OANDA Java SDK internal threads for rate updates and connection keep-alive  
**Thread Type**: Java threads (via JPype)  
**Impact**: Background threads, do NOT block strategy execution

---

## Multiple Strategies Handling

### Configuration

Multiple strategies are configured in the config file:

```ini
[account]
strategies = kantu_10021,kantu_10038,kantu_10039,kantu_20004,kantu_20005

[kantu_10021]
name = kantu_10021
pair = EURUSD
strategyID = 18
instanceID = 10021

[kantu_10038]
name = kantu_10038
pair = EURUSD
strategyID = 18
instanceID = 10038

[kantu_20004]
name = kantu_20004
pair = USDJPY
strategyID = 18
instanceID = 20004
```

### Initialization

```python
# atrader.py (lines 328-369)
strategySections = map(str.strip, config.get('account', 'strategies').split(','))
account.strategies = []
all_instanceIDs = []

for strategySection in strategySections:
    # Parse config for each strategy
    pair = config.get(strategySection, 'pair')
    strategyID = config.getint(strategySection, 'strategyId')
    instanceID = int(settings.mainParams["STRATEGY_INSTANCE_ID"]['value'])
    
    # Check for duplicate instance IDs
    if instanceID in all_instanceIDs:
        logger.critical("Duplicate instance ID found {}. Aborting run".format(instanceID))
        quit()
    
    # Create Strategy object
    strategy = Strategy(setfilePath, pair, strategyID, ...)
    account.strategies.append(strategy)
```

### Execution Order

**Strategies are executed sequentially in the order they appear in the config file:**

```python
# account.py - runStrategies()
for strategy in self.strategies:  # Sequential loop
    if not strategy.isRunning:
        strategy.isRunning = True
        strategy.run(...)  # Blocks until strategy completes
```

**Example Execution Timeline:**

```
Time 0s:  Start main loop
Time 0s:  Execute Strategy 1 (EURUSD, instanceID=10021)
Time 5s:  Strategy 1 completes
Time 5s:  Execute Strategy 2 (EURUSD, instanceID=10038)
Time 10s: Strategy 2 completes
Time 10s: Execute Strategy 3 (USDJPY, instanceID=20004)
Time 15s: Strategy 3 completes
Time 15s: Display UI for all strategies
Time 16s: Sleep for executionTime (30 seconds)
Time 46s: Repeat loop
```

---

## Multiple Symbols Handling

### Per-Strategy Symbols

Each strategy can have **multiple symbols** for rate requirements:

```python
# strategy.py - run() (lines 152-181)
for i in range(0, 9):  # Up to 10 rate arrays
    if self.symbolRequirements[i] != "N":
        if self.symbolRequirements[i] != "D":
            # Get rates for specific symbol
            rates = self.account.getCandles(maxNumCandles, 
                                            self.symbolRequirements[i],  # e.g., "GBPUSD"
                                            self.passedTimeFrame, ...)
        else:
            # Get rates for default symbol (strategy.pair)
            rates = self.account.getCandles(maxNumCandles, 
                                            self.pair,  # e.g., "EURUSD"
                                            self.passedTimeFrame, ...)
```

**Example Configuration:**

```ini
[kantu_10021]
pair = EURUSD                    # Default symbol
rateRequirements = 600,30
symbolRequirements = D,D          # D = default (EURUSD), N = no symbol
timeframeRequirements = 0,1440    # 0 = strategy timeframe, 1440 = daily
```

This means:
- Rate array 0: EURUSD at strategy timeframe (e.g., 60 minutes)
- Rate array 1: EURUSD at daily timeframe (1440 minutes)

### Symbol Caching

**All symbols share the same cache:**

```python
# atrader.py (lines 384-389)
allSymbolsLoaded     = []  # List of symbols that have been loaded
allRatesLoaded       = {}  # Dictionary: key = symbol+timeframe, value = rates
allBidAskSymbols     = []  # List of symbols with cached bid/ask
allBidAskLoaded      = {}  # Dictionary: key = symbol, value = bid/ask
allBidAskUpdateTimes = {}  # Dictionary: key = symbol, value = timestamp
```

**Benefits:**
- If Strategy 1 loads EURUSD rates, Strategy 2 can reuse them (if same timeframe)
- Reduces API calls
- Shared across all strategies

**Example:**

```
Strategy 1 (EURUSD): Loads EURUSD H1 rates → Cached in allRatesLoaded
Strategy 2 (EURUSD): Reuses cached EURUSD H1 rates → No API call
Strategy 3 (GBPUSD): Loads GBPUSD H1 rates → Cached in allRatesLoaded
```

---

## Concurrency Model Summary

### Strategy Execution: **Sequential (Single-Threaded)**

```
Main Thread
├── Strategy 1 (EURUSD, instanceID=10021)
│   ├── Get account info
│   ├── Get EURUSD rates
│   ├── Get open trades
│   ├── Get bid/ask
│   ├── Call Framework C library
│   └── Process results
│
├── Strategy 2 (EURUSD, instanceID=10038)
│   ├── Get account info (cached)
│   ├── Get EURUSD rates (cached)
│   ├── Get open trades
│   ├── Get bid/ask (cached)
│   ├── Call Framework C library
│   └── Process results
│
├── Strategy 3 (USDJPY, instanceID=20004)
│   ├── Get account info (cached)
│   ├── Get USDJPY rates
│   ├── Get open trades
│   ├── Get bid/ask
│   ├── Call Framework C library
│   └── Process results
│
└── Display UI
    └── Sleep for executionTime seconds
```

### Background Threads: **Auxiliary Services Only**

```
Main Thread (Strategy Execution)
│
├── WebServer Thread (daemon)
│   └── Serves HTTP requests (non-blocking)
│
├── Email Threads (as needed)
│   └── Sends email logs (non-blocking)
│
└── OANDA Java Threads (internal)
    ├── Rate update thread
    └── Keep-alive thread
```

---

## Performance Characteristics

### Execution Time

**Total execution time = Sum of all strategy execution times**

```
Total Time = Σ(Strategy Execution Time) + UI Display Time + Sleep Time
```

**Example:**
- Strategy 1: 5 seconds
- Strategy 2: 4 seconds
- Strategy 3: 6 seconds
- UI Display: 1 second
- **Total: 16 seconds per loop iteration**
- Sleep: 30 seconds
- **Loop period: 46 seconds**

### Blocking Behavior

**All strategies block the main thread:**

- If Strategy 1 takes 10 seconds, Strategy 2 waits 10 seconds
- If Strategy 1 fails/hangs, all subsequent strategies are delayed
- Network API calls block (no async I/O)

### Rate Limiting

**API rate limits are handled by:**
1. **Caching**: Reuse cached data when possible
2. **Sequential execution**: Natural rate limiting (one strategy at a time)
3. **Sleep period**: `executionTime` seconds between loop iterations

---

## Limitations

### 1. **No Parallel Strategy Execution**

- Strategies cannot run in parallel
- If Strategy 1 is slow, Strategy 2 waits
- Cannot take advantage of multiple CPU cores

### 2. **Blocking I/O**

- All API calls are synchronous (blocking)
- Network latency affects all strategies
- No async/await pattern

### 3. **Single Point of Failure**

- If one strategy crashes, it may affect others
- No isolation between strategies
- Shared account object (potential race conditions if threading added)

### 4. **No Strategy Prioritization**

- Strategies execute in config file order
- No way to prioritize urgent strategies
- No way to skip slow strategies

### 5. **Limited Scalability**

- Adding more strategies increases total execution time
- Cannot scale horizontally (single process)
- Memory usage grows with number of strategies

---

## Comparison with MQL4/5

### MQL4/5 Execution Model

**MQL4/5 also uses sequential execution:**

```mql4
// MQL4 Example: Kantu.mq4
int start()
{
    for(int i = 0; i < TOTAL_INTERNAL_STRATEGIES; i++)
    {
        if(strategySettings[i][IDX_OPERATIONAL_MODE] != DISABLE)
        {
            c_runStrategy(i, ...);  // Sequential execution
        }
    }
    return(0);
}
```

**Key Differences:**

| Aspect | MQL4/5 | Python Solution |
|--------|--------|-----------------|
| **Execution Model** | Sequential (same) | Sequential (same) |
| **Data Access** | Direct (fast) | API calls (slower) |
| **Blocking** | Minimal (local data) | Network I/O (blocking) |
| **Threading** | None (MetaTrader handles it) | Minimal (auxiliary only) |
| **Scalability** | Limited by EA execution | Limited by API rate limits |

---

## Potential Improvements

### 1. **Async I/O (Python 3.8+)**

```python
# Potential improvement (not implemented)
import asyncio

async def runStrategyAsync(strategy):
    # Non-blocking API calls
    account_info = await account.getAccountInfoAsync()
    rates = await account.getCandlesAsync(...)
    # ...

async def runAllStrategies():
    tasks = [runStrategyAsync(s) for s in account.strategies]
    await asyncio.gather(*tasks)  # Parallel execution
```

**Benefits:**
- Non-blocking I/O
- Can fetch data for multiple strategies concurrently
- Better resource utilization

**Challenges:**
- Requires Python 3.8+ (migration needed)
- Framework C library calls are still blocking
- More complex error handling

### 2. **Thread Pool for Strategy Execution**

```python
# Potential improvement (not implemented)
from concurrent.futures import ThreadPoolExecutor

with ThreadPoolExecutor(max_workers=5) as executor:
    futures = [executor.submit(strategy.run, ...) 
               for strategy in account.strategies]
    for future in futures:
        future.result()
```

**Benefits:**
- Parallel strategy execution
- Better CPU utilization
- Faster overall execution

**Challenges:**
- Framework C library may not be thread-safe
- Shared account object requires synchronization
- More complex error handling
- Potential race conditions

### 3. **Process Pool (Isolation)**

```python
# Potential improvement (not implemented)
from multiprocessing import Process, Queue

def runStrategyProcess(strategy, queue):
    result = strategy.run(...)
    queue.put(result)

processes = [Process(target=runStrategyProcess, args=(s, queue)) 
             for s in account.strategies]
for p in processes:
    p.start()
```

**Benefits:**
- Complete isolation between strategies
- Crash in one strategy doesn't affect others
- Can use multiple CPU cores

**Challenges:**
- Cannot share in-memory cache
- More complex data sharing (queues, pipes)
- Higher memory usage
- Framework state files may conflict

---

## Conclusion

### Current Implementation

- **Single-threaded, sequential execution**
- **No parallel strategy execution**
- **Limited threading** (only for auxiliary services)
- **Shared data cache** (benefits all strategies)
- **Blocking I/O** (network API calls)

### Why This Design?

1. **Simplicity**: Easier to debug and maintain
2. **Framework Compatibility**: Framework C library may not be thread-safe
3. **API Rate Limits**: Sequential execution naturally limits API calls
4. **Shared State**: Account object and caches are easier to manage
5. **Reliability**: Less chance of race conditions or deadlocks

### When to Consider Parallelization?

- **Many strategies** (>10) with long execution times
- **Fast API responses** (low latency)
- **High CPU usage** (CPU-bound operations)
- **Framework thread-safety** verified

---

**Document Status**: Complete
**Last Updated**: December 2024

