# BTCUSD MACD 860013 Optimization Guide

## Overview

This guide explains how to set up and run parameter optimization for the BTCUSD MACD strategy (Strategy ID 860013).

## Files Involved

- **Config File**: `ctester/config/Peso_MACD_BTCUSD-1H_860013.config`
- **Set File**: `ctester/sets/Daily_MACD_BTCUSD_1H.set`
- **Script**: `ctester/scripts/run_btcusd_macd_860013.sh`

## Step 1: Enable Optimization in Config File

Edit `ctester/config/Peso_MACD_BTCUSD-1H_860013.config` and modify the `[optimization]` section:

```ini
[optimization]
optimize = 1                    # Change from 0 to 1 to enable optimization
optimizationtype = 1            # 0 = Brute Force, 1 = Genetic Algorithm (recommended)
numcores = 4                    # Number of CPU cores to use (adjust based on your system)
optimizationgoal = 0            # 0=Profit, 1=MaxDD, 2=MaxDDLength, 3=PF, 4=R2, 5=UlcerIndex, 6=Sharpe, 7=CAGR/MaxDD, 8=CAGR
population = 50                 # GA population size (30-100 recommended)
maxgenerations = 1000           # Max generations (0 = unlimited, 1000 is a good start)
stopIfConverged = 1             # Stop when population converges (1 = yes, 0 = no)
discardAssymetricSets = 1       # Discard asymmetric long/short results (1 = yes)
minTradesAYear = 20             # Minimum trades per year filter
crossoverprobability = 0.90     # Crossover rate (0.0-1.0)
mutationprobability = 0.20      # Mutation rate (0.0-1.0)
migrationprobability = 0.00    # Migration rate for MPI (usually 0.00)
evolutionarymode = 0            # 0=Darwin, 1-3=Lamarck, 4-12=Baldwin
elitismmode = 1                 # 1=Parents survive, 2=One parent, 3=Parents die, 4=Rescore
mutationmode = 0                # 0=Single point drift, 1=Single point randomize, 2=Multipoint, 3=All point
crossovermode = 0               # 0=Single points, 1=Double points, 2=Mean, 3=Mixing, 4=Allele mixing
```

### Optimization Goal Options

- **0 - Profit**: Maximize total profit
- **1 - MaxDD**: Minimize maximum drawdown
- **2 - MaxDDLength**: Minimize drawdown duration
- **3 - PF**: Maximize profit factor
- **4 - R²**: Maximize R² (linearity of equity curve)
- **5 - UlcerIndex**: Minimize ulcer index
- **6 - Sharpe**: Maximize Sharpe ratio
- **7 - CAGR/MaxDD**: Maximize CAGR to MaxDD ratio (risk-adjusted return)
- **8 - CAGR**: Maximize Compound Annual Growth Rate

**Recommended**: Start with **0 (Profit)** or **7 (CAGR/MaxDD)** for balanced optimization.

## Step 2: Define Optimization Parameters in Set File

Edit `ctester/sets/Daily_MACD_BTCUSD_1H.set` and add optimization ranges for parameters you want to optimize.

### Format for Optimization Parameters

For each parameter you want to optimize, add these lines after the parameter:

```ini
PARAMETER_NAME = value
PARAMETER_NAME,F = 1          # Enable optimization (1 = yes, 0 = no)
PARAMETER_NAME,1 = start_value # Starting value
PARAMETER_NAME,2 = step_value  # Step size
PARAMETER_NAME,3 = stop_value  # Ending value
```

### Example: Optimizing AUTOBBS_RISK_CAP

Current value in set file:
```ini
AUTOBBS_RISK_CAP=0
```

Add optimization lines:
```ini
AUTOBBS_RISK_CAP=0
AUTOBBS_RISK_CAP,F = 1
AUTOBBS_RISK_CAP,1 = 1.0
AUTOBBS_RISK_CAP,2 = 1.0
AUTOBBS_RISK_CAP,3 = 10.0
```

This will test values: 1.0, 2.0, 3.0, ..., 10.0

### Recommended Parameters to Optimize for MACD Strategy

Based on the current set file, here are suggested optimization ranges:

#### Example 1: Risk Management Parameters

```ini
# Optimize risk cap (index 3)
AUTOBBS_RISK_CAP=0
AUTOBBS_RISK_CAP,F = 1
AUTOBBS_RISK_CAP,1 = 1.0
AUTOBBS_RISK_CAP,2 = 1.0
AUTOBBS_RISK_CAP,3 = 10.0

# Optimize max account risk (index 5)
AUTOBBS_MAX_ACCOUNT_RISK=20
AUTOBBS_MAX_ACCOUNT_RISK,F = 1
AUTOBBS_MAX_ACCOUNT_RISK,1 = 10.0
AUTOBBS_MAX_ACCOUNT_RISK,2 = 5.0
AUTOBBS_MAX_ACCOUNT_RISK,3 = 30.0

# Optimize max strategy risk (index 8)
AUTOBBS_MAX_STRATEGY_RISK=5
AUTOBBS_MAX_STRATEGY_RISK,F = 1
AUTOBBS_MAX_STRATEGY_RISK,1 = 1.0
AUTOBBS_MAX_STRATEGY_RISK,2 = 1.0
AUTOBBS_MAX_STRATEGY_RISK,3 = 10.0
```

#### Example 2: Trading Parameters

```ini
# Optimize adjust points (index 0)
AUTOBBS_ADJUSTPOINTS=10
AUTOBBS_ADJUSTPOINTS,F = 1
AUTOBBS_ADJUSTPOINTS,1 = 5.0
AUTOBBS_ADJUSTPOINTS,2 = 5.0
AUTOBBS_ADJUSTPOINTS,3 = 20.0

# Optimize range (index 7)
AUTOBBS_RANGE=10
AUTOBBS_RANGE,F = 1
AUTOBBS_RANGE,1 = 5.0
AUTOBBS_RANGE,2 = 5.0
AUTOBBS_RANGE,3 = 20.0

# Optimize execution rates (index 10)
AUTOBBS_EXECUTION_RATES=60
AUTOBBS_EXECUTION_RATES,F = 1
AUTOBBS_EXECUTION_RATES,1 = 30.0
AUTOBBS_EXECUTION_RATES,2 = 15.0
AUTOBBS_EXECUTION_RATES,3 = 120.0
```

### Complete Example Set File with Optimization

Here's a complete example adding optimization to 3 key parameters:

```ini
COMMENTS=Click Modify expert to view comments inside the source code.
SECTION_1=################# General Settings #################
FRAMEWORK_CONFIG=./experts/config/AsirikuyConfig.xml
USE_ORDER_WRAPPER=0

ENABLE_SCREENSHOTS=0

HISTORIC_DATA_ID=0
MAX_SLIPPAGE_PIPS=0.2
UI_FONT_SIZE=12
UI_CUSTOM_INFO_COLOR=16776960
SECTION_2=############## Common Strategy Settings ##############
OPERATIONAL_MODE=1
RUN_EVERY_TICK=0
STRATEGY_INSTANCE_ID=223456
STRATEGY_TIMEFRAME=60
ACCOUNT_RISK_PERCENT=1
MAX_DRAWDOWN_PERCENT=100
MAX_SPREAD_PIPS=100
DISABLE_COMPOUNDING=0
USE_INSTANCE_BALANCE=0
INIT_INSTANCE_BALANCE=0.0
MAX_OPEN_ORDERS=199
TIMED_EXIT_BARS=0
SL_ATR_MULTIPLIER=1
TP_ATR_MULTIPLIER=1
ATR_AVERAGING_PERIOD=20
SECTION_3=############## Additional Strategy Settings ##############
AUTOBBS_ADJUSTPOINTS=10
AUTOBBS_TP_MODE = 1
AUTOBBS_TREND_MODE=23
AUTOBBS_RISK_CAP=0
AUTOBBS_RISK_CAP,F = 1
AUTOBBS_RISK_CAP,1 = 1.0
AUTOBBS_RISK_CAP,2 = 1.0
AUTOBBS_RISK_CAP,3 = 10.0
AUTOBBS_LONG_SHORT_MODE=1
AUTOBBS_MAX_ACCOUNT_RISK=20
AUTOBBS_MAX_ACCOUNT_RISK,F = 1
AUTOBBS_MAX_ACCOUNT_RISK,1 = 10.0
AUTOBBS_MAX_ACCOUNT_RISK,2 = 5.0
AUTOBBS_MAX_ACCOUNT_RISK,3 = 30.0
AUTOBBS_KEYK=0
AUTOBBS_RANGE=10 
AUTOBBS_RANGE,F = 1
AUTOBBS_RANGE,1 = 5.0
AUTOBBS_RANGE,2 = 5.0
AUTOBBS_RANGE,3 = 20.0
AUTOBBS_MAX_STRATEGY_RISK=5	
AUTOBBS_MAX_STRATEGY_RISK,F = 1
AUTOBBS_MAX_STRATEGY_RISK,1 = 1.0
AUTOBBS_MAX_STRATEGY_RISK,2 = 1.0
AUTOBBS_MAX_STRATEGY_RISK,3 = 10.0
AUTOBBS_MACRO_TREND=0
AUTOBBS_EXECUTION_RATES=60
AUTOBBS_ONE_SIDE=0
AUTOBBS_IS_ATREURO_RANGE=0	
```

## Step 3: Calculate Parameter Combinations

Before running, calculate the number of combinations to ensure it's reasonable:

### For Brute Force (optimizationtype = 0)

**Formula**: `(stop - start) / step + 1` for each parameter, then multiply all together.

**Example with 3 parameters**:
- AUTOBBS_RISK_CAP: (10-1)/1 + 1 = 10 values
- AUTOBBS_MAX_ACCOUNT_RISK: (30-10)/5 + 1 = 5 values  
- AUTOBBS_RANGE: (20-5)/5 + 1 = 4 values
- **Total**: 10 × 5 × 4 = **200 combinations**

**Limit**: Maximum 10 million combinations. If you exceed this, use Genetic Algorithm instead.

### For Genetic Algorithm (optimizationtype = 1)

No limit on parameter space size. The GA will efficiently search through large spaces.

## Step 4: Run the Optimization

### Option A: Using the Script (Recommended)

The script will automatically handle optimization if enabled in the config:

```bash
cd ctester
./scripts/run_btcusd_macd_860013.sh
```

### Option B: Direct Python Command

```bash
cd ctester
python3 asirikuy_strategy_tester.py -c config/Peso_MACD_BTCUSD-1H_860013.config -ot results -oo optimization_results
```

**Parameters**:
- `-c`: Config file path
- `-ot`: Output test results prefix
- `-oo`: Output optimization results file (optional, but recommended)

### Option C: With Custom Date Range

```bash
cd ctester
./scripts/run_btcusd_macd_860013.sh --fromdate 2020-01-01 --todate 2023-01-01
```

## Step 5: Monitor Progress

### Output Files

During optimization, you'll see:
- **Console output**: Real-time iteration progress
- **Optimization results file**: `optimization_results.txt` (if using `-oo` flag)
- **Log file**: `tmp/BTCUSD_860013/backtest_860013.log`

### Optimization Results Format

The results file contains CSV data:
```
Iteration, Symbol, NumTrades, Profit, maxDD, maxDDLength, PF, R2, ulcerIndex, Sharpe, CAGR, CAGR to Max DD, numShorts, numLongs, Set Parameters
```

### Stopping Optimization

To stop optimization early:
- Press `Ctrl+C` in the terminal
- Or call `stopOptimization()` if using the API

## Step 6: Analyze Results

### Finding Best Results

1. **Sort by optimization goal**: If optimizing for profit, sort by Profit column
2. **Check filters**: Ensure results meet your criteria (min trades/year, etc.)
3. **Review parameters**: Note the parameter values of top performers
4. **Validate**: Run a backtest with the best parameters to verify

### Example Analysis

```bash
# View optimization results
cat optimization_results.txt | sort -t, -k4 -rn | head -20
```

This sorts by profit (column 4) in descending order and shows top 20.

## Best Practices

### 1. Start Small

- Begin with 1-2 parameters
- Use small ranges initially
- Test with shorter date ranges (e.g., 1-2 years)

### 2. Use Genetic Algorithm for Large Spaces

- If > 10,000 combinations, use GA (optimizationtype = 1)
- GA is more efficient for 3+ parameters

### 3. Choose Appropriate Goal

- **Profit (0)**: Maximize returns (may have high drawdown)
- **CAGR/MaxDD (7)**: Balanced risk-adjusted return (recommended)
- **Sharpe (6)**: Risk-adjusted return with volatility consideration

### 4. Set Reasonable Filters

- `minTradesAYear = 20`: Ensures sufficient trading activity
- `discardAssymetricSets = 1`: Prevents biased long/short results

### 5. Parallel Execution

- Set `numcores` to number of CPU cores (e.g., 4, 8)
- Significantly speeds up optimization

### 6. Convergence Detection

- `stopIfConverged = 1`: Stops when population converges
- Saves time if solution is found early

## Troubleshooting

### Optimization Not Running

- **Check**: `optimize = 1` in config file
- **Check**: At least one parameter has `,F = 1` in set file
- **Check**: Parameter ranges are valid (start < stop, step > 0)

### Too Many Combinations

- **Error**: "Number of parameter combinations is too large"
- **Solution**: Reduce parameter ranges or use Genetic Algorithm (optimizationtype = 1)

### No Results Generated

- **Check**: Date range has sufficient data
- **Check**: Parameter ranges don't filter out all results
- **Check**: `minTradesAYear` isn't too high

### Poor Performance

- **Increase**: `numcores` for parallel execution
- **Reduce**: `maxgenerations` if GA is taking too long
- **Narrow**: Parameter ranges based on initial results

## Example: Complete Optimization Setup

### Config File Changes

```ini
[optimization]
optimize = 1
optimizationtype = 1
numcores = 4
optimizationgoal = 7
population = 50
maxgenerations = 500
stopIfConverged = 1
discardAssymetricSets = 1
minTradesAYear = 20
crossoverprobability = 0.90
mutationprobability = 0.20
migrationprobability = 0.00
evolutionarymode = 0
elitismmode = 1
mutationmode = 0
crossovermode = 0
```

### Set File Changes (3 parameters)

Add to `Daily_MACD_BTCUSD_1H.set`:

```ini
AUTOBBS_RISK_CAP=0
AUTOBBS_RISK_CAP,F = 1
AUTOBBS_RISK_CAP,1 = 1.0
AUTOBBS_RISK_CAP,2 = 1.0
AUTOBBS_RISK_CAP,3 = 10.0

AUTOBBS_MAX_ACCOUNT_RISK=20
AUTOBBS_MAX_ACCOUNT_RISK,F = 1
AUTOBBS_MAX_ACCOUNT_RISK,1 = 10.0
AUTOBBS_MAX_ACCOUNT_RISK,2 = 5.0
AUTOBBS_MAX_ACCOUNT_RISK,3 = 30.0

AUTOBBS_RANGE=10
AUTOBBS_RANGE,F = 1
AUTOBBS_RANGE,1 = 5.0
AUTOBBS_RANGE,2 = 5.0
AUTOBBS_RANGE,3 = 20.0
```

### Run Command

```bash
cd ctester
./scripts/run_btcusd_macd_860013.sh --fromdate 2020-01-01 --todate 2023-01-01
```

## Parameter Index Reference

Common AUTOBBS parameters and their indexes:

| Parameter | Index | Description |
|-----------|-------|-------------|
| AUTOBBS_ADJUSTPOINTS | 0 | Adjustment points |
| AUTOBBS_TP_MODE | 1 | Take profit mode |
| AUTOBBS_TREND_MODE | 2 | Trend detection mode |
| AUTOBBS_RISK_CAP | 3 | Risk cap limit |
| AUTOBBS_LONG_SHORT_MODE | 4 | Long/short mode |
| AUTOBBS_MAX_ACCOUNT_RISK | 5 | Maximum account risk |
| AUTOBBS_KEYK | 6 | Key K parameter |
| AUTOBBS_RANGE | 7 | Range parameter |
| AUTOBBS_MAX_STRATEGY_RISK | 8 | Maximum strategy risk |
| AUTOBBS_MACRO_TREND | 9 | Macro trend setting |
| AUTOBBS_EXECUTION_RATES | 10 | Execution rates |
| AUTOBBS_ONE_SIDE | 11 | One side trading |
| AUTOBBS_IS_AUTO_MODE | 12 | Auto mode flag |
| AUTOBBS_IS_ATREURO_RANGE | 13 | ATR Euro range |

## Additional Resources

- **Optimizer Documentation**: See `docs/OPTIMIZER_USAGE.md`
- **CTester Guide**: See `ctester/docs/CTESTER_USAGE_GUIDE.md`
- **Config Examples**: See `ctester/config/ast.config` for optimization examples

