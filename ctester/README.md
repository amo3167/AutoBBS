# CTester - Strategy Backtesting Framework

CTester is a comprehensive backtesting framework for trading strategies, built on the Asirikuy testing infrastructure. It provides automated backtesting capabilities with detailed performance analysis and reporting.

## Table of Contents

- [CTester - Strategy Backtesting Framework](#ctester---strategy-backtesting-framework)
  - [Table of Contents](#table-of-contents)
  - [Overview](#overview)
  - [Directory Structure](#directory-structure)
  - [Quick Start](#quick-start)
    - [Running a Backtest](#running-a-backtest)
    - [Direct Python Execution](#direct-python-execution)
  - [Configuration Files](#configuration-files)
    - [Key Configuration Parameters](#key-configuration-parameters)
  - [Running Backtests](#running-backtests)
    - [Available Scripts](#available-scripts)
    - [Script Options](#script-options)
    - [Example Usage](#example-usage)
  - [Strategy IDs](#strategy-ids)
  - [Output Files](#output-files)
    - [Key Metrics](#key-metrics)
  - [Troubleshooting](#troubleshooting)
    - [Common Issues](#common-issues)
    - [Verifying Strategy Configuration](#verifying-strategy-configuration)
  - [Creating New Backtest Scripts](#creating-new-backtest-scripts)
  - [Additional Documentation](#additional-documentation)
  - [Notes](#notes)
  - [Support](#support)

## Overview

CTester allows you to:
- Run automated backtests on trading strategies
- Generate performance reports and charts
- Test multiple timeframes and currency pairs
- Analyze strategy performance metrics (Profit Factor, Max Drawdown, R², etc.)
- Organize results by strategy ID and symbol

## Directory Structure

```
ctester/
├── config/              # Strategy configuration files
│   ├── Inkling_*.config # Inkling strategy configs
│   ├── Peso_MACD_*.config # MACD strategy configs
│   └── Limit_*.config   # Limit strategy configs
├── scripts/             # Automated backtest scripts
│   └── run_*.sh         # Shell scripts for running backtests
├── sets/                # Strategy parameter sets (.set files)
├── history/             # Historical price data (CSV files)
├── tmp/                 # Backtest results output directory
│   └── {SYMBOL}_{ID}/   # Results organized by symbol and strategy ID
├── docs/                # Comprehensive documentation
├── include/             # Python modules and libraries
└── asirikuy_strategy_tester.py  # Main backtesting engine
```

## Quick Start

### Running a Backtest

1. **Using a pre-configured script** (recommended):
```bash
cd ctester/scripts
./run_xauusd_inkling_5m_860001.sh
```

2. **With custom date range**:
```bash
./run_xauusd_inkling_5m_860001.sh --fromdate 2018-01-01 --todate 2021-01-01
```

3. **With custom log level**:
```bash
./run_xauusd_inkling_5m_860001.sh --logseverity 4
```

### Direct Python Execution

```bash
cd ctester
python3 asirikuy_strategy_tester.py -c config/Inkling_XAUUSD-5M_860001.config -ot results
```

## Configuration Files

Configuration files define strategy parameters, account settings, and test periods. They follow this structure:

```ini
[misc]
logseverity = 6

[account]
currency = USD
balance = 100000
leverage = 200
contractsize = 100000
digits = 5
stopoutpercent = 1
minimumstop = 0.0002
spread = 0.0002
minlotsize = 0.01
generate_plot = 1

[strategy]
pair = GBPUSD
passedtimeframe = 5
strategyid = 29
fromdate = 2018-01-01
todate = 2031-01-01
portfoliostrategies = 29
set = ./sets/multipledays_gbpusd_5M.set
raterequirements = 300,300,240,120,30,10
symbolrequirements = D,D,D,D,D,D,D
timeframerequirements = 0,15,60,240,1440,10080

[optimization]
optimize = 0
# ... optimization parameters
```

### Key Configuration Parameters

- **pair**: Currency pair (e.g., GBPUSD, XAUUSD, GBPJPY)
- **passedtimeframe**: Timeframe in minutes (5 = 5-minute, 60 = 1-hour)
- **strategyid**: Internal strategy ID (usually 29 for Inkling strategies)
- **fromdate/todate**: Test period (YYYY-MM-DD format)
- **set**: Path to strategy parameter set file
- **digits**: Price precision (3 for JPY pairs, 5 for most others)
- **minimumstop/spread**: Minimum stop loss and spread in price units

## Running Backtests

### Available Scripts

All backtest scripts are located in `ctester/scripts/` and follow the naming pattern:
```
run_{symbol}_{strategy}_{timeframe}_{id}.sh
```

**Example scripts:**
- `run_xauusd_inkling_5m_860001.sh` - XAUUSD Inkling 5M Strategy
- `run_gbpjpy_inkling_5m_860002.sh` - GBPJPY Inkling 5M Strategy
- `run_gbpusd_inkling_5m_860003.sh` - GBPUSD Inkling 5M Strategy ⚠️ **ID needs verification**
- `run_btcusd_macd_860013.sh` - BTCUSD MACD Strategy
- `run_gbpjpy_macd_860006.sh` - GBPJPY MACD 1H Strategy

### Script Options

All scripts support the following command-line options:

```bash
--fromdate YYYY-MM-DD    # Override start date from config
--todate YYYY-MM-DD      # Override end date from config
--logseverity LEVEL      # Log level 0-7 (0=Emergency, 4=Warning, 7=Debug)
-h, --help               # Show help message
```

### Example Usage

```bash
# Run with default settings
./run_gbpjpy_inkling_5m_860002.sh

# Run with custom date range
./run_gbpjpy_inkling_5m_860002.sh --fromdate 2018-01-01 --todate 2021-01-01

# Run with custom date and log level
./run_gbpjpy_inkling_5m_860002.sh --fromdate 2018-01-01 --todate 2021-01-01 --logseverity 4
```

## Strategy IDs

Strategy IDs are unique identifiers for each strategy configuration. The naming convention follows:

- **860001**: XAUUSD Inkling 5M
- **860002**: GBPJPY Inkling 5M
- **860003**: GBPUSD Inkling 5M ⚠️ **VERIFY - May not be correct**
- **860004**: BTCUSD Inkling 5M
- **860006**: GBPJPY MACD 1H
- **860007**: XAUUSD MACD 1H
- **860008**: GBPAUD MACD 1H
- **860011**: GBPUSD MACD 1H
- **860013**: BTCUSD MACD 1H
- **861001**: XAGUSD Inkling 5M

**⚠️ Important**: Always verify the correct strategy ID before creating new configs or scripts. Check existing configs and scripts to ensure consistency.

## Output Files

After running a backtest, results are saved to `tmp/{SYMBOL}_{ID}/`:

- **results_{ID}.txt** - Detailed backtest results with trade-by-trade analysis
- **results_{ID}.png** - Performance chart showing equity curve
- **results_{ID}.set** - Strategy settings used for the backtest
- **allStatistics_{ID}.csv** - Statistical summary in CSV format
- **backtest_{ID}.log** - Complete execution log
- **config_{ID}.config** - Config file used (if custom parameters were provided)

### Key Metrics

The results include:
- **Total trades**: Number of trades executed
- **Longs/Shorts**: Breakdown of trade directions
- **Final balance**: Ending account balance
- **Max DD**: Maximum drawdown
- **PF**: Profit Factor (ratio of gross profit to gross loss)
- **R²**: R-squared (goodness of fit for equity curve)
- **Ulcer Index**: Risk-adjusted performance metric

## Troubleshooting

### Common Issues

1. **"Invalid trading time" warnings**: These occur for weekend/holiday data and are usually harmless.

2. **No trades generated**: 
   - Check that the date range has sufficient data
   - Verify the strategy set file exists and is correct
   - Review strategy parameters in the .set file

3. **Config file not found**:
   - Ensure you're running from the `ctester` directory
   - Check that config file path is correct (relative to ctester root)

4. **Permission denied on script**:
   ```bash
   chmod +x ctester/scripts/run_*.sh
   ```

### Verifying Strategy Configuration

Before running a backtest, verify:
1. ✅ Config file exists in `config/` directory
2. ✅ Set file path in config is correct (use relative paths: `./sets/...`)
3. ✅ Strategy ID matches the naming convention
4. ✅ Historical data exists in `history/` for the symbol and timeframe
5. ✅ Date range is valid and has data coverage

## Creating New Backtest Scripts

To create a new backtest script:

1. **Find or create the matching config file**:
   - Check `config/` for existing configs
   - Verify the correct strategy ID
   - Ensure set file path uses relative format: `./sets/...`

2. **Create the script** based on existing templates:
   ```bash
   cp ctester/scripts/run_gbpjpy_inkling_5m_860002.sh ctester/scripts/run_{symbol}_{strategy}_{timeframe}_{id}.sh
   ```

3. **Update script variables**:
   - `CONFIG_FILE`: Path to your config file
   - `SYMBOL`: Currency pair symbol
   - `STRATEGY_ID`: Unique strategy ID

4. **Make executable**:
   ```bash
   chmod +x ctester/scripts/run_{symbol}_{strategy}_{timeframe}_{id}.sh
   ```

## Additional Documentation

For more detailed information, see the `docs/` directory:

- **[CTESTER_USAGE_GUIDE.md](docs/CTESTER_USAGE_GUIDE.md)** - Comprehensive usage guide
- **[CTESTER_TROUBLESHOOTING.md](docs/CTESTER_TROUBLESHOOTING.md)** - Troubleshooting guide
- **[CTESTER_INTEGRATION_GUIDE.md](docs/CTESTER_INTEGRATION_GUIDE.md)** - Developer integration guide
- **[CTESTER_API_REFERENCE.md](docs/CTESTER_API_REFERENCE.md)** - API reference
- **[README.md](docs/README.md)** - Documentation index

## Notes

- ⚠️ **GBPUSD Inkling 5M Strategy ID 860003**: This ID may not be correct. Please verify against existing strategy configurations before using.
- All scripts automatically clean previous results and organize output by symbol and strategy ID
- Results are saved to `tmp/` directory with descriptive filenames
- Custom date ranges can be specified via command-line arguments without modifying config files

## Support

For issues or questions:
1. Check [CTESTER_TROUBLESHOOTING.md](docs/CTESTER_TROUBLESHOOTING.md)
2. Review existing configs and scripts for examples
3. Check the execution log in `tmp/{SYMBOL}_{ID}/backtest_{ID}.log`

