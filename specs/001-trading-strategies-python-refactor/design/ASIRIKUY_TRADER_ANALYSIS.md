# Asirikuy Trader Codebase Analysis

## Overview

The `asirikuy-trader` directory contains the **Live Trading Platform** - a Python 2 application that connects to broker APIs (OANDA, JForex/Dukascopy) and executes trading strategies using the Asirikuy Framework C library.

**Status**: Python 2 codebase, requires migration to Python 3.8+

**Date**: December 2024

---

## Architecture

### High-Level Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    atrader.py (Main Entry)                   │
│  - Config parsing                                             │
│  - Logger initialization                                      │
│  - Account initialization                                     │
│  - Strategy initialization                                    │
│  - Main execution loop                                        │
└───────────────────────┬───────────────────────────────────────┘
                        │
        ┌───────────────┼───────────────┐
        │               │               │
┌───────▼──────┐ ┌──────▼──────┐ ┌─────▼──────┐
│  Account     │ │  Strategy    │ │  Order     │
│  (account.py)│ │(strategy.py) │ │  Wrapper   │
│              │ │              │ │(orderwrapper│
│  - Broker    │ │  - Loads C   │ │   .py)     │
│    API       │ │    library   │ │            │
│    access    │ │  - Executes  │ │  - Virtual │
│  - Balance   │ │    strategy  │ │    trades  │
│  - Orders    │ │  - Gets      │ │  - History │
└───────┬──────┘ │    signals   │ └────────────┘
        │        └──────┬───────┘
        │               │
        └───────┬───────┘
                │
    ┌───────────▼───────────┐
    │  AsirikuyFrameworkAPI  │
    │  (C Library via ctypes)│
    │  - libAsirikuyFramework│
    │    API.dylib/.so/.dll  │
    └────────────────────────┘
                │
    ┌───────────▼───────────┐
    │  Java Bridge (JPype)   │
    │  - OANDA Java SDK      │
    │  - JForex SDK          │
    └────────────────────────┘
```

### Key Components

1. **atrader.py** - Main entry point
   - Initializes logging, configuration, account, and strategies
   - Main execution loop that runs strategies periodically
   - Handles broker-specific initialization (OANDA vs JForex)

2. **include/account.py** - Account management
   - Broker API integration (OANDA REST/Java, JForex)
   - Balance and equity tracking
   - Order execution
   - Virtual trade management (via OrderWrapper)

3. **include/strategy.py** - Strategy execution
   - Loads AsirikuyFrameworkAPI C library
   - Initializes strategy instances
   - Executes strategies and processes signals
   - Manages rate data and symbol requirements

4. **include/asirikuy.py** - C library interface
   - Defines ctypes structures (Rate, OrderInfo, SettingsType, etc.)
   - Loads C library dynamically (Windows/Linux/macOS)
   - Defines error codes and constants
   - Helper functions for rate loading, config parsing

5. **include/orderwrapper.py** - Virtual order management
   - Manages virtual trades (for NFA-compliant brokers)
   - Uses TinyDB for persistence
   - Tracks open orders, history, and statistics

6. **include/web_server.py** - Web interface
   - Simple HTTP server for viewing plots and statistics
   - Serves HTML/CSS/JS/PNG files from `plots/` directory

7. **include/plotter.py** - Visualization
   - Generates charts using matplotlib
   - Creates HTML reports using Mako templates
   - Updates plots periodically

8. **Java Wrappers** (`src/OandaWrapper/`, `src/JForex-SDK/`)
   - Java classes that interface with broker SDKs
   - Called via JPype from Python
   - Handles streaming data and order execution

---

## File Structure

```
asirikuy-trader/
├── atrader.py                    # Main entry point
├── config/                       # Configuration files
│   ├── AsirikuyConfig.xml        # Framework config
│   ├── atrader_oanda.config      # OANDA account config
│   ├── atrader_jforex.config     # JForex account config
│   └── broker-tz.csv             # Broker timezone data
├── include/                      # Python modules
│   ├── __init__.py
│   ├── asirikuy.py               # C library interface (601 lines)
│   ├── account.py                # Account management (1828 lines)
│   ├── strategy.py               # Strategy execution (424 lines)
│   ├── orderwrapper.py           # Virtual order wrapper (309 lines)
│   ├── mt.py                     # MT4 set file parser
│   ├── misc.py                   # Utilities
│   ├── graphics.py               # Graphics utilities
│   ├── plotter.py                # Plotting and HTML generation
│   ├── web_server.py             # HTTP server
│   ├── rfc3339.py                # RFC3339 date parsing
│   ├── asirikuy.jar              # JForex wrapper JAR
│   └── oanda_helper.jar          # OANDA wrapper JAR
├── src/                          # Java source
│   ├── OandaWrapper/             # OANDA Java wrapper
│   │   ├── asirikuy/
│   │   │   ├── OandaWrapper.java # Interface
│   │   │   └── Ticker.java       # Streaming tick handler
│   └── JForex-SDK/               # JForex Java wrapper
│       └── src/asirikuy/
│           ├── Main.java
│           ├── DukascopyWrapper.java
│           └── ...
├── vendor/                       # Java dependencies (JARs)
│   ├── oanda_fxtrade.jar
│   ├── JForex-API-*.jar
│   └── [50+ JAR files]
├── sets/                         # Strategy parameter files (.set)
├── history/                      # Historical data (CSV)
├── plots/                        # Generated plots and HTML
├── log/                          # Log files
├── data/                         # Order data (JSON/TinyDB)
└── tmp/                          # Temporary files
```

**Statistics**:
- **15 Python files** (~4,409 lines total)
- **10 Java files** (wrapper classes)
- **50+ JAR dependencies** (vendor libraries)
- **72 strategy set files**

---

## Python 2 → Python 3 Migration Requirements

### 1. Syntax Changes

#### Print Statements → Print Functions
```python
# Python 2
print "Stop file present, will quit in 5 seconds..."
print "AsirikuyFrameworkAPI v"+ASKFrameworkVersion

# Python 3
print("Stop file present, will quit in 5 seconds...")
print("AsirikuyFrameworkAPI v" + ASKFrameworkVersion)
```
**Files affected**: All 15 Python files (131+ print statements)

#### Dictionary Iteration
```python
# Python 2
for k, v in dict(self.content.items('main')).iteritems():

# Python 3
for k, v in dict(self.content.items('main')).items():
```
**Files affected**: `include/mt.py` (lines 36, 56)

#### String Formatting
```python
# Python 2
print "AsirikuyTrader v{}".format(PROGRAM_VERSION)  # Mixed style

# Python 3
print("AsirikuyTrader v{}".format(PROGRAM_VERSION))  # Consistent
```

### 2. Module Changes

#### ConfigParser → configparser
```python
# Python 2
import ConfigParser
config = ConfigParser.ConfigParser()

# Python 3
import configparser
config = configparser.ConfigParser()
```
**Files affected**: `include/asirikuy.py`, `include/mt.py`

#### urllib2 → urllib
```python
# Python 2
import urllib2
from urllib2 import HTTPError

# Python 3
import urllib.request, urllib.error
from urllib.error import HTTPError
```
**Files affected**: `include/strategy.py`, `include/account.py`

#### BaseHTTPServer → http.server
```python
# Python 2
from BaseHTTPServer import BaseHTTPRequestHandler, HTTPServer

# Python 3
from http.server import BaseHTTPRequestHandler, HTTPServer
```
**Files affected**: `include/web_server.py`

### 3. Division Behavior

#### Integer Division
```python
# Python 2 (already using __future__)
from __future__ import division  # Already present in account.py, orderwrapper.py

# Python 3
# No change needed - division is already float by default
```

### 4. String/Unicode Handling

Python 3 uses Unicode strings by default. Need to check:
- File I/O (binary vs text mode)
- C library string passing (ctypes)
- JSON encoding/decoding

**Files to review**: `include/asirikuy.py` (C library interface), `include/account.py` (JSON handling)

### 5. Exception Handling

```python
# Python 2
except Exception: 
    e = Exception
    print sys.exc_info()[1]

# Python 3
except Exception as e:
    print(e)
```
**Files affected**: Multiple files (many instances)

### 6. Import Changes

```python
# Python 2
import sys
print sys.exc_info()[1]

# Python 3
import sys
print(sys.exc_info()[1])
```

---

## Dependencies

### Python Dependencies (Python 2)

```python
# Core dependencies
colorama          # Terminal colors
jpype             # Java-Python bridge
pytz              # Timezone handling
python-dateutil   # Date parsing
requests          # HTTP requests
tinydb            # JSON database
matplotlib        # Plotting
mako               # HTML templating
numpy              # Numerical operations
```

**Migration Notes**:
- Most dependencies have Python 3 versions
- `jpype` → `jpype1` (Python 3 compatible)
- Check compatibility of all dependencies

### Java Dependencies

- **OANDA SDK**: `oanda_fxtrade.jar`, `oanda_fxtrade_implementation.jar`
- **JForex SDK**: `JForex-API-2.12.33.jar`, `DDS2-jClient-JForex-2.45.37.jar`
- **Logging**: `log4j-1.2.16.jar`, `slf4j-*.jar`
- **Other**: 50+ vendor JARs in `vendor/` directory

**Note**: Java dependencies should work with Python 3 via JPype, but need to verify JPype compatibility.

---

## C Library Integration

### Library Loading

```python
def loadLibrary(library):
    if os.name == 'nt':
        return windll.LoadLibrary(library)
    elif os.name == 'posix':
        return cdll.LoadLibrary(library)
    else:
        return None
```

**Platform-specific library names**:
- Windows: `AsirikuyFrameworkAPI.dll`
- Linux: `libAsirikuyFrameworkAPI.so`
- macOS: `libAsirikuyFrameworkAPI.dylib`

### Key C Library Functions Used

From `include/asirikuy.py` and `include/strategy.py`:

1. **`initInstanceC`** - Initialize strategy instance
   ```python
   init = self.asfdll.initInstanceC(self.instanceID, 0, asirikuyFrameworkPath, str(traderInstance))
   ```

2. **`mql5_getConversionSymbols`** - Get currency conversion symbols
   ```python
   self.asfdll.mql5_getConversionSymbols(self.pair, "USD", byref(basePair), byref(quotePair))
   ```
   **Note**: This uses MQL5-specific function name, but should use `C_getConversionSymbols` for cross-platform (similar to CTester fix).

3. **`getASKFrameworkVersion`** - Get framework version
   ```python
   version = getASKFrameworkVersion(asfdll)
   ```

4. **Strategy execution functions** (called from `strategy.py`)

### ctypes Structures

Defined in `include/asirikuy.py`:
- `Rate` - OHLCV data
- `OrderInfo` - Trade information
- `SettingsType` - Strategy settings (64 doubles)
- `accountInfoType` - Account information
- `RateInfo` - Rate buffer information
- `TestStatistics` - Backtest statistics

**Migration Notes**: ctypes structures should work identically in Python 3, but need to verify string handling.

---

## Broker Integration

### OANDA

**Two modes**:
1. **Java Interface** (`isJava = yes` in config)
   - Uses OANDA Java SDK via JPype
   - Streaming tick support
   - Classes: `OandaWrapper`, `Ticker`
   - JARs: `oanda_helper.jar`, `oanda_fxtrade*.jar`

2. **REST Interface** (`isJava = no`)
   - Uses OANDA REST API via `requests` library
   - Implemented in `account.py` (OandaAccount class)

**Symbol Format**: `EUR_USD` (underscore separator)

### JForex/Dukascopy

- Uses JForex Java SDK via JPype
- Classes: `DukascopyWrapper`, `Main`
- JARs: `asirikuy.jar`, `JForex-API-*.jar`, `DDS2-*.jar`

**Symbol Format**: `EUR/USD` (slash separator)

**Note**: Dukascopy does not support pending orders (see `account.py` line 1179).

---

## Configuration

### Main Config File (`atrader.config`)

```ini
[misc]
logSeverity = 2                    # 0=None, 1=Debug, 2=Info, 3=Warning, 4=Error, 5=Critical
screenLogSeverity = 4
emailLogSeverity = 0
smtp = smtp.gmail.com
port = 25
executionTime = 1                   # Seconds between strategy runs

[plots]
graphicPaintingPeriod = 5           # Executions between plot updates
enableWebServer = yes
webServerPort = 25000

[account]
accountName = oanda1
brokerName = OANDA Corporation
refBrokerName = Alpari (UK) Historic Data
strategies = kantu_10021,kantu_10038,...  # Comma-separated
login = amo3167
password = MO665162
useOrderWrapper = yes               # For NFA-compliant brokers
hasStaticIP = no
accountID = 1409724
isDemo = no
isJava = yes                        # OANDA only: Java vs REST
useStreaming = no                   # OANDA only: tick-by-tick streaming
initialBalance = 1038.98
depositSymbol = USD

[strategy_name]
name = kantu_10021
setFile = kantu_10021_EURUSD.set
pair = EURUSD
strategyID = 18
passedTimeFrame = 60
rateRequirements = 600,30
symbolRequirements = D,D
timeframeRequirements = 0,1440
```

### Framework Config (`AsirikuyConfig.xml`)

Same as CTester - defines logging, timezone, and framework settings.

---

## Key Differences from CTester

| Aspect | CTester | Asirikuy Trader |
|--------|---------|-----------------|
| **Purpose** | Backtesting | Live Trading |
| **Data Source** | Historical CSV files | Broker API (real-time) |
| **Execution** | Simulated | Real orders (or virtual via OrderWrapper) |
| **Broker Support** | None (uses historical data) | OANDA, JForex/Dukascopy |
| **Java Integration** | None | Yes (JPype for broker APIs) |
| **Order Management** | Simulated | Real broker orders + virtual wrapper |
| **Web Interface** | None | Yes (simple HTTP server) |
| **Plotting** | Static (after backtest) | Dynamic (periodic updates) |
| **Strategy Execution** | Once per backtest | Continuous loop |

---

## Migration Strategy

### Phase 1: Code Analysis (Current)
- ✅ Analyze codebase structure
- ✅ Identify Python 2 → 3 migration requirements
- ✅ Document dependencies
- ✅ Map C library integration points

### Phase 2: Syntax Migration
1. Convert all `print` statements to `print()` functions
2. Update imports (ConfigParser → configparser, urllib2 → urllib, etc.)
3. Fix dictionary iteration (`.iteritems()` → `.items()`)
4. Update exception handling syntax
5. Fix string/bytes handling for C library calls

### Phase 3: Dependency Updates
1. Update `requirements.txt` with Python 3 compatible versions
2. Test JPype compatibility (`jpype` → `jpype1`)
3. Verify all Python dependencies work with Python 3.8+
4. Test Java bridge functionality

### Phase 4: C Library Integration
1. Verify ctypes structures work correctly
2. Test string passing to C functions
3. Update `mql5_getConversionSymbols` → `C_getConversionSymbols` (if needed)
4. Test library loading on macOS/Linux/Windows

### Phase 5: Testing
1. Unit tests for each module
2. Integration tests with mock broker APIs
3. End-to-end tests with demo accounts
4. Verify OrderWrapper functionality
5. Test web server and plotting

### Phase 6: Documentation
1. Update README with Python 3 requirements
2. Document migration changes
3. Update installation instructions
4. Create migration guide

---

## Risks and Considerations

### High Risk
1. **JPype Compatibility**: Java bridge may have issues with Python 3
2. **String Handling**: C library string passing may break with Python 3 Unicode
3. **Broker API Changes**: OANDA/JForex APIs may have changed
4. **Dependency Conflicts**: Some Python 2 dependencies may not have Python 3 equivalents

### Medium Risk
1. **ConfigParser Changes**: Behavior differences between Python 2/3 versions
2. **File I/O**: Binary vs text mode handling
3. **Exception Handling**: Different exception types/chaining

### Low Risk
1. **ctypes Structures**: Should work identically
2. **Matplotlib**: Well-maintained, Python 3 compatible
3. **TinyDB**: Python 3 compatible

---

## Next Steps

1. **Create migration branch**: `git checkout -b python3-migration-asirikuy-trader`
2. **Set up Python 3 environment**: Create virtual environment with Python 3.8+
3. **Install dependencies**: Test compatibility of all dependencies
4. **Start with low-risk files**: Begin with `include/misc.py`, `include/rfc3339.py`
5. **Progress to core modules**: `include/asirikuy.py`, `include/strategy.py`, `include/account.py`
6. **Test incrementally**: After each file/module migration, run tests
7. **Document changes**: Keep track of all modifications in migration log

---

## References

- [Python 2 to 3 Migration Guide](https://docs.python.org/3/howto/pyporting.html)
- [JPype Documentation](https://jpype.readthedocs.io/)
- [ctypes Documentation](https://docs.python.org/3/library/ctypes.html)
- CTester Python 3 migration (completed) - reference implementation
- AsirikuyFrameworkAPI C library documentation
- **MQL_VS_PYTHON_ARCHITECTURE.md** - Detailed comparison of MQL4/5 vs Python solution architecture
- **PYTHON_MULTI_STRATEGY_EXECUTION.md** - How Python solution handles multiple strategies and symbols (single-threaded sequential execution)
- **MQL_EXECUTION_AND_MANUAL_TRADES.md** - MQL4/5 execution model and how Python solution handles manual trades from broker
- **TRADE_HISTORY_AND_MULTIPLE_EAS.md** - Trade history feature (missing in Python) and multiple EAs execution model
- **PYTHON_PROCESS_MANAGEMENT_AND_TRADE_MATCHING.md** - Process management, trade matching (magic numbers), and price data fetching
- **MQL4_5_PRICE_DATA.md** - How MQL4/5 handles price data (direct platform access vs Python's API-based approach)
- **MQL4_5_MULTIPLE_TIMEFRAMES.md** - How MQL4/5 supports multiple timeframes (up to 10 per strategy) and implementation in EA strategies
- **BROKER_API_COMPATIBILITY.md** - Current status and compatibility of OANDA and JForex APIs
- **FUTURE_SOLUTION_RECOMMENDATIONS.md** - Recommendations for future solution architecture (MQL4 vs MQL5 vs REST API), specifically for Pepperstone
- **MQL4_VS_MQL5_COMPARISON.md** - Detailed comparison between MQL4 and MQL5, major value propositions, and migration effort assessment
- **ORDER_MANAGEMENT_MQL4_VS_MQL5.md** - Detailed order management differences between MQL4 and MQL5 in the AutoBBS codebase
- **PYTHON_SOLUTION_DESIGN_PLAN.md** - Comprehensive design plan for Python solution to achieve MQL4/5 parity, value proposition, and implementation roadmap

---

## Questions to Resolve

1. **JPype Version**: Which version of JPype supports Python 3? (`jpype1`?)
2. **C Library Function Names**: Should `mql5_getConversionSymbols` be changed to `C_getConversionSymbols` for cross-platform compatibility?
3. **Broker API Status**: Are OANDA and JForex APIs still active and compatible?
4. **OrderWrapper Persistence**: TinyDB format compatibility between Python 2/3?
5. **Testing Strategy**: How to test without live broker accounts? (Use demo accounts?)

---

**Document Status**: Initial Analysis Complete
**Last Updated**: December 2024
**Next Review**: After migration planning session

