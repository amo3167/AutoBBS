# CTester Usage Guide

**Date**: December 2024  
**Status**: Complete  
**Task**: T035 - Create CTester integration documentation  
**Version**: Python 3.8+

## Table of Contents

1. [Introduction](#introduction)
2. [Installation](#installation)
3. [Quick Start](#quick-start)
4. [Configuration](#configuration)
5. [Running Tests](#running-tests)
6. [Optimization](#optimization)
7. [Output Files](#output-files)
8. [Command-Line Options](#command-line-options)
9. [GUI Usage](#gui-usage)
10. [Examples](#examples)

## Introduction

CTester is a Python-based backtesting framework for trading strategies. It allows you to:

- Test trading strategies on historical data
- Run portfolio tests with multiple strategies and symbols
- Optimize strategy parameters using genetic algorithms or brute force
- Generate detailed reports and visualizations
- Export results in XML format

### System Requirements

- **Python**: 3.8 or higher
- **Operating System**: macOS, Linux, or Windows
- **Dependencies**: Automatically installed via `auto_installer.py`
  - `numpy`
  - `matplotlib`
  - `requests`
  - `colorama`
  - `mpi4py` (optional, for parallel optimization)

### Architecture

CTester uses the `CTesterFrameworkAPI` C library to execute trading strategies. The Python layer handles:
- Configuration management
- Historical data loading
- Result processing and visualization
- Optimization orchestration

## Installation

### 1. Prerequisites

Ensure Python 3.8+ is installed:

```bash
python3 --version
```

### 2. Build C Library

The `CTesterFrameworkAPI` shared library must be built first:

```bash
cd core/CTesterFrameworkAPI
premake4 gmake
cd build/gmake
make
```

The library will be in:
- **macOS**: `bin/gmake/x64/Debug/libCTesterFrameworkAPI.dylib`
- **Linux**: `bin/gmake/x64/Debug/libCTesterFrameworkAPI.so`
- **Windows**: `bin/gmake/x64/Debug/CTesterFrameworkAPI.dll`

### 3. Verify Installation

Run the diagnostic test suite:

```bash
cd ctester
python3 ast_diagnostics.py
```

This runs 8 diagnostic tests to verify the installation.

## Quick Start

### Basic Test

1. **Create a config file** (`config/my_test.config`):

```ini
[misc]
logSeverity = 6

[account]
currency = USD
brokerName = Alpari (UK) Historic Data
refBrokerName = Alpari (UK) Historic Data
balance = 100000
leverage = 200
contractSize = 100000
digits = 3
stopOutPercent = 1
minimumStop = 0.05
spread = 0.03
minLotSize = 0.00001
generate_plot = 1

[strategy]
setFile = 30002_USDJPY.set
pair = USDJPY1987
strategyID = 22
passedTimeFrame = 60
fromDate = 01/01/86
toDate = 26/04/20
rateRequirements = 10000
symbolRequirements = D
timeframeRequirements = 0

[optimization]
optimize = 0
```

2. **Ensure historical data exists**:

Place CSV files in `ctester/history/` directory:
- Format: `SYMBOL_TIMEFRAME.csv` (e.g., `USDJPY1987_60.csv`)
- Columns: `Time,Open,High,Low,Close,Volume`

3. **Run the test**:

```bash
cd ctester
python3 asirikuy_strategy_tester.py -c config/my_test.config -ot my_results
```

4. **View results**:

- `my_results.xml` - Detailed trade history
- `my_results.html` - HTML report with charts
- `my_results.png` - Equity curve plot

## Configuration

### Config File Structure

CTester uses INI-style configuration files with three main sections:

#### `[misc]` Section

```ini
[misc]
logSeverity = 6  ; 0=Emergency, 1=Alert, 2=Critical, 3=Error, 4=Warning, 5=Notice, 6=Info, 7=Debug
```

**Parameters**:
- `logSeverity`: Logging level (0-7). Lower numbers = more verbose.

#### `[account]` Section

```ini
[account]
currency = USD
brokerName = Alpari (UK) Historic Data
refBrokerName = Alpari (UK) Historic Data
balance = 100000
leverage = 200
contractSize = 100000
digits = 3,3,3
stopOutPercent = 1
minimumStop = 0.05,0.05,0.05
spread = 0.03,0.03,0.05
minLotSize = 0.00001
generate_plot = 1
```

**Parameters**:
- `currency`: Account base currency (USD, EUR, etc.)
- `brokerName`: Broker name for historical data
- `refBrokerName`: Reference broker name
- `balance`: Starting account balance
- `leverage`: Account leverage (e.g., 200 = 1:200)
- `contractSize`: Contract size (typically 100000 for forex)
- `digits`: Decimal places for each symbol (comma-separated for portfolios)
- `stopOutPercent`: Stop-out level percentage
- `minimumStop`: Minimum stop loss distance (comma-separated for portfolios)
- `spread`: Spread in pips (comma-separated for portfolios)
- `minLotSize`: Minimum lot size
- `generate_plot`: 1 = generate plots, 0 = skip

#### `[strategy]` Section

```ini
[strategy]
setFile = 30002_USDJPY.set,30003_USDJPY.set,30004_GBPJPY.set
pair = USDJPY1987,USDJPY1987,GBPJPY1987
strategyID = 22,22,22
passedTimeFrame = 60
fromDate = 01/01/86
toDate = 26/04/20
rateRequirements = 10000|10000|10000
symbolRequirements = D|D|D
timeframeRequirements = 0|0|0
```

**Parameters**:
- `setFile`: Strategy parameter files (comma-separated for portfolios)
  - Can use `set` or `setFile` key (backward compatibility)
  - Paths can be relative to `sets/` directory or absolute
- `pair`: Trading symbols (comma-separated for portfolios)
  - Format: `SYMBOLYEAR` (e.g., `USDJPY1987`)
  - Must match CSV file names in `history/` directory
- `strategyID`: Strategy IDs (comma-separated for portfolios)
  - See [Strategy IDs](#strategy-ids) below
- `passedTimeFrame`: Timeframe in minutes (e.g., 60 = 1 hour)
- `fromDate`: Start date (format: `dd/mm/yy`)
- `toDate`: End date (format: `dd/mm/yy`)
- `rateRequirements`: Number of bars required (pipe-separated for portfolios)
- `symbolRequirements`: Symbol requirements (pipe-separated for portfolios)
  - `D` = default symbol, `N` = not needed
- `timeframeRequirements`: Timeframe requirements (pipe-separated for portfolios)
  - `0` = use strategy timeframe, otherwise specify minutes

#### `[optimization]` Section

```ini
[optimization]
optimize = 0
optimizationType = 1
numCores = 2
optimizationGoal = 0
population = 25
maxGenerations = 10000
stopIfConverged = 1
discardAssymetricSets = 1
minTradesAYear = 20
crossoverProbability = 0.9
mutationProbability = 0.2
migrationProbability = 0.0
evolutionaryMode = 0
elitismMode = 1
mutationMode = 0
crossoverMode = 0
```

**Parameters**:
- `optimize`: 0 = test only, 1 = optimize
- `optimizationType`: 0 = Brute Force, 1 = Genetic Algorithm
- `numCores`: Number of CPU cores (for OpenMP parallel optimization)
- `optimizationGoal`: 0=Profit, 1=MaxDD, 2=MaxDDLength, 3=PF, 4=R2, 5=UlcerIndex
- `population`: Population size (genetic algorithm)
- `maxGenerations`: Maximum generations (0 = unlimited)
- `stopIfConverged`: 1 = stop when converged, 0 = continue
- `discardAssymetricSets`: 1 = discard asymmetric parameter sets
- `minTradesAYear`: Minimum trades per year required
- `crossoverProbability`: Crossover probability (0.0-1.0)
- `mutationProbability`: Mutation probability (0.0-1.0)
- `migrationProbability`: Migration probability (0.0-1.0)
- `evolutionaryMode`: 0=Darwin, 1=Lamarck Parents, 2=Lamarck Children, 3=Lamarck All, 4=Baldwin Parents, 8=Baldwin Children, 12=Baldwin All
- `elitismMode`: 0=Unknown, 1=Parents survive, 2=One parent survives, 3=Parents die, 4=Rescore Parents
- `mutationMode`: 0=Single point drift, 1=Single point randomize, 2=Multipoint, 3=All point
- `crossoverMode`: 0=Single points, 1=Double points, 3=Mean, 4=Mixing, 5=Allele mixing

### Strategy IDs

Available strategy IDs (defined in `include/asirikuy.py`):

| ID | Strategy Name |
|----|---------------|
| 0 | WATUKUSHAY_FE_BB |
| 1 | WATUKUSHAY_FE_CCI |
| 2 | ATIPAQ |
| 3 | AYOTL |
| 4 | COATL |
| 5 | COMITL_BB |
| 6 | COMITL_KC |
| 7 | COMITL_PA |
| 8 | GODS_GIFT_ATR |
| 9 | QALLARYI |
| 10 | QUIMICHI |
| 11 | SAPAQ |
| 12 | ASIRIKUY_BRAIN |
| 13 | TEYACANANI |
| 14 | WATUKUSHAY_FE_RSI |
| 15 | RUPHAY |
| 16 | TEST_EA |
| 17 | EURCHF_GRID |
| 18 | KANTU |
| 19 | RECORD_BARS |
| 20 | MUNAY |
| 21 | RENKO_TEST |
| 22 | PKANTU_ML |
| 23 | KANTU_RL |
| 24 | KELPIE |
| 25 | BBS |
| 26 | TakeOver |
| 27 | SCREENING |
| 28 | KEYK |
| 29 | AUTOBBS |
| 30 | AUTOBBSWEEKLY |

### Set Files

Set files (`.set`) contain strategy-specific parameters. They are INI-style files with sections:

```ini
SECTION_1=################# General Settings #################
FRAMEWORK_CONFIG=./experts/config/AsirikuyConfig.xml
USE_ORDER_WRAPPER=0
ENABLE_SCREENSHOTS=0
HISTORIC_DATA_ID=1
MAX_SLIPPAGE_PIPS=2.00000000

SECTION_2=############## Common Strategy Settings ##############
OPERATIONAL_MODE=1
STRATEGY_INSTANCE_ID=25
STRATEGY_TIMEFRAME=60
ACCOUNT_RISK_PERCENT=1.00000000
MAX_DRAWDOWN_PERCENT=100.00000000
MAX_SPREAD_PIPS=100.00000000
ENABLE_WFO=0
WFO_WINDOW_SIZE=0
PARAMETER_SET_POOL=0.00000000
DISABLE_COMPOUNDING=1
USE_INSTANCE_BALANCE=0
INIT_INSTANCE_BALANCE=0.00000000
TIMED_EXIT_BARS=31
ATR_AVERAGING_PERIOD=3
MAX_OPEN_ORDERS=1

SECTION_3=############## Additional Strategy Settings ##############
OPEN_ATR_MULTIPLIER=0.41000000
CLOSE_ATR_MULTIPLIER=0.28000000
TRADE_TARGET=0.01
TRADE_CONFIDENCE=2
```

## Running Tests

### Single Strategy Test

```bash
python3 asirikuy_strategy_tester.py -c config/my_test.config -ot results
```

### Portfolio Test

Configure multiple strategies in the config file:

```ini
[strategy]
setFile = strategy1.set,strategy2.set,strategy3.set
pair = EURUSD1987,GBPUSD1987,USDJPY1987
strategyID = 22,22,22
```

### Using MPI for Parallel Testing

For large-scale testing, use MPI:

```bash
mpirun -np 4 python3 asirikuy_strategy_tester.py -c config/my_test.config -ot results
```

This distributes optimization across 4 processes.

## Optimization

### Genetic Algorithm Optimization

Set `optimize = 1` and `optimizationType = 1`:

```ini
[optimization]
optimize = 1
optimizationType = 1
population = 50
maxGenerations = 100
optimizationGoal = 0
```

Run with optimization output file:

```bash
python3 asirikuy_strategy_tester.py -c config/optimize.config -oo optimization_results
```

### Brute Force Optimization

Set `optimizationType = 0`:

```ini
[optimization]
optimize = 1
optimizationType = 0
numCores = 4
```

### Optimization Output

The optimization file (`.txt`) contains:

```
Iteration, Symbol, NumTrades, Profit, maxDD, maxDDLength, PF, R2, ulcerIndex, Sharpe, CAGR, CAGR to Max DD, numShorts, numLongs, Set Parameters
```

## Output Files

### Test Results

- **`<output>.xml`**: Detailed trade history in XML format
  - Contains all trades, equity curve, statistics
  - Compatible with MT4 backtest XML format
- **`<output>.html`**: HTML report with charts
  - Equity curve
  - Drawdown chart
  - Trade statistics
- **`<output>.png`**: Equity curve plot (if `generate_plot = 1`)

### Optimization Results

- **`<output>.txt`**: CSV file with optimization iterations
- **`<output>.png`**: Optimization progress plot

### Log Files

- **`log/AsirikuyCTester.log`**: Main log file
- **`log/AsirikuyCTester_<rank>.log`**: Per-process logs (MPI)

## Command-Line Options

```bash
python3 asirikuy_strategy_tester.py [OPTIONS]
```

**Options**:
- `-c, --config-file <path>`: Path to config file (default: `config/ast.config`)
- `-ot, --output-test-file <name>`: Output file name for test results (default: `results`)
- `-oo, --output-optimization-file <name>`: Output file name for optimization results (default: `optimization`)
- `-v, --version`: Show version information

**Examples**:

```bash
# Use custom config file
python3 asirikuy_strategy_tester.py -c config/my_strategy.config

# Specify output file
python3 asirikuy_strategy_tester.py -ot my_test_results

# Optimization with custom output
python3 asirikuy_strategy_tester.py -c config/optimize.config -oo my_optimization

# Show version
python3 asirikuy_strategy_tester.py -v
```

## GUI Usage

CTester includes a PyQt4 GUI:

```bash
python3 vast.py
```

**Features**:
- Visual configuration editor
- Strategy and symbol selection
- Set file management
- Test execution with progress display
- Result visualization

**Note**: PyQt4 must be installed separately. The GUI is optional; command-line usage is fully functional.

## Automated Backtesting Script

### Overview

The `run_btcusd_macd_860013.sh` script automates backtesting with parameter support and organized file management.

### Features

- **Command-line parameters**: Customize date range and log level
- **Automatic cleanup**: Removes old results before each run
- **Organized output**: Results saved to `tmp/SYMBOL_STRATEGYID/` folder
- **Config preservation**: Saves used config file if parameters provided

### Usage

#### Basic Usage (Default Config)

```bash
cd ctester
./run_btcusd_macd_860013.sh
```

Uses default config values from `config/Peso_MACD_BTCUSD-1H_860013.config`.

#### With Custom Parameters

```bash
./run_btcusd_macd_860013.sh --fromdate 2020-01-02 --todate 2021-01-01 --logseverity 6
```

**Parameters**:
- `--fromdate YYYY-MM-DD`: Start date for backtest
- `--todate YYYY-MM-DD`: End date for backtest
- `--logseverity LEVEL`: Log level (0-7)
  - `0` = Emergency
  - `4` = Warning
  - `6` = Info
  - `7` = Debug

#### Help

```bash
./run_btcusd_macd_860013.sh --help
```

### Output Structure

Results are saved to `tmp/BTCUSD_860013/`:

```
tmp/BTCUSD_860013/
├── allStatistics_860013.csv      # Statistics summary
├── backtest_860013.log            # Backtest execution log
├── config_860013.config           # Config file used (if parameters provided)
├── results_860013.png              # Equity curve chart
├── results_860013.set              # Parameter set file
└── results_860013.txt              # Detailed trade results
```

### Examples

#### Example 1: Test Specific Date Range

```bash
./run_btcusd_macd_860013.sh --fromdate 2020-01-02 --todate 2021-01-01
```

#### Example 2: Test with Debug Logging

```bash
./run_btcusd_macd_860013.sh --fromdate 2020-01-02 --todate 2021-01-01 --logseverity 7
```

#### Example 3: Test with Warning Level Only

```bash
./run_btcusd_macd_860013.sh --fromdate 2020-01-02 --todate 2021-01-01 --logseverity 4
```

### Notes

- The script automatically cleans the results folder before each run
- Old `.state` files in `tmp/` are automatically cleaned up
- The script creates a temporary config file if parameters are provided
- The temporary config file is saved as `config_860013.config` for reference

## Examples

### Example 1: Simple Single Strategy Test

**Config** (`config/simple_test.config`):
```ini
[misc]
logSeverity = 6

[account]
currency = USD
brokerName = Alpari (UK) Historic Data
refBrokerName = Alpari (UK) Historic Data
balance = 100000
leverage = 200
contractSize = 100000
digits = 3
stopOutPercent = 1
minimumStop = 0.05
spread = 0.03
minLotSize = 0.00001
generate_plot = 1

[strategy]
setFile = 30002_USDJPY.set
pair = USDJPY1987
strategyID = 22
passedTimeFrame = 60
fromDate = 01/01/86
toDate = 26/04/20
rateRequirements = 10000
symbolRequirements = D
timeframeRequirements = 0

[optimization]
optimize = 0
```

**Run**:
```bash
python3 asirikuy_strategy_tester.py -c config/simple_test.config -ot simple_test
```

### Example 2: Portfolio Test

**Config** (`config/portfolio_test.config`):
```ini
[misc]
logSeverity = 6

[account]
currency = USD
brokerName = Alpari (UK) Historic Data
refBrokerName = Alpari (UK) Historic Data
balance = 100000
leverage = 200
contractSize = 100000
digits = 3,3,3
stopOutPercent = 1
minimumStop = 0.05,0.05,0.05
spread = 0.03,0.03,0.05
minLotSize = 0.00001
generate_plot = 1

[strategy]
setFile = 30002_USDJPY.set,30003_USDJPY.set,30004_GBPJPY.set
pair = USDJPY1987,USDJPY1987,GBPJPY1987
strategyID = 22,22,22
passedTimeFrame = 60
fromDate = 01/01/86
toDate = 26/04/20
rateRequirements = 10000|10000|10000
symbolRequirements = D|D|D
timeframeRequirements = 0|0|0

[optimization]
optimize = 0
```

**Run**:
```bash
python3 asirikuy_strategy_tester.py -c config/portfolio_test.config -ot portfolio_test
```

### Example 3: Genetic Algorithm Optimization

**Config** (`config/optimize_ga.config`):
```ini
[misc]
logSeverity = 6

[account]
currency = USD
brokerName = Alpari (UK) Historic Data
refBrokerName = Alpari (UK) Historic Data
balance = 100000
leverage = 200
contractSize = 100000
digits = 3
stopOutPercent = 1
minimumStop = 0.05
spread = 0.03
minLotSize = 0.00001
generate_plot = 1

[strategy]
setFile = 30002_USDJPY.set
pair = USDJPY1987
strategyID = 22
passedTimeFrame = 60
fromDate = 01/01/86
toDate = 26/04/20
rateRequirements = 10000
symbolRequirements = D
timeframeRequirements = 0

[optimization]
optimize = 1
optimizationType = 1
numCores = 2
optimizationGoal = 0
population = 25
maxGenerations = 100
stopIfConverged = 1
discardAssymetricSets = 1
minTradesAYear = 20
crossoverProbability = 0.9
mutationProbability = 0.2
migrationProbability = 0.0
evolutionaryMode = 0
elitismMode = 1
mutationMode = 0
crossoverMode = 0
```

**Run**:
```bash
python3 asirikuy_strategy_tester.py -c config/optimize_ga.config -oo ga_optimization
```

## Next Steps

- See [CTester Integration Guide](CTESTER_INTEGRATION_GUIDE.md) for developer documentation
- See [CTester Troubleshooting Guide](CTESTER_TROUBLESHOOTING.md) for common issues
- See [CTester Interface Documentation](CTESTER_INTERFACE_DOCUMENTATION.md) for API details

