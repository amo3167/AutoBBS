# Portfolio Simulation Guide

## Overview

The `run_portfolio.sh` script runs portfolio simulations with a given risk configuration, saves results in an organized subfolder, and automatically generates visual charts.

**Features**:
- ✅ Runs portfolio simulation with specified risk configuration
- ✅ Saves results in timestamped subfolder (`batch/output/portfolio_YYYYMMDD_HHMMSS/`)
- ✅ Automatically generates visual charts
- ✅ Displays performance summary
- ✅ Organizes all output files

---

## Usage

### Basic Usage

```bash
./run_portfolio.sh portfoliorisk5.config
```

### With Custom Start Date

```bash
./run_portfolio.sh portfoliorisk5.config 2018-01-01
```

### With Order Checking

```bash
./run_portfolio.sh portfoliorisk5.config 2018-01-01 true
```

### Parameters

1. **Risk Config** (required)
   - Path to portfolio risk configuration file
   - Example: `portfoliorisk5.config`

2. **Start Date** (optional, default: 2018-01-01)
   - Format: YYYY-MM-DD
   - Example: `2020-01-01`

3. **Check Orders** (optional, default: false)
   - `true` or `false`
   - Validates orders against MT4 platform

---

## Output Structure

Results are saved to: `batch/output/portfolio_YYYYMMDD_HHMMSS/`

### Files Generated

**Statistics & Reports**:
- `portfolioStatistics_adjusted.csv` - Performance statistics
- `portfolioStatistics_adjusted_YYYYMMDDHHMMSS.csv` - Timestamped version
- `portfolioMonthlyReport_adjusted.csv` - Monthly performance
- `portfolioWeeklyReport_adjusted.csv` - Weekly performance
- `portfolioDailyReport_adjusted.csv` - Daily performance
- `portfolioResult_baseline.csv` - Baseline portfolio results
- `portfolioResult_adjusted.csv` - Adjusted portfolio results

**Configuration**:
- `portfolio_risk_config.csv` - Copy of input risk configuration
- `best_portfolio.csv` - Portfolio allocation (for visualization)

**Logs**:
- `simulation.log` - Execution log
- `visualization.log` - Visualization generation log
- `simulation_summary.txt` - Summary of files and results

**Visualizations** (in `visualizations/` subfolder):
- `00_summary_report.txt` - Text summary
- `01_portfolio_allocation.png` - Strategy allocation charts
- `02_monthly_performance.png` - Monthly equity curve and returns
- `03_weekly_performance.png` - Weekly performance analysis
- `04_daily_performance.png` - Daily performance analysis
- `05_portfolio_statistics.png` - Performance metrics (if optimization data available)
- `06_risk_return_analysis.png` - Risk-return scatter plots (if optimization data available)

---

## Example Output

```
==========================================
Portfolio Simulation Runner
==========================================
Risk Config:  portfoliorisk5.config
Start Date:   2018-01-01
Check Orders: false
Output Dir:   batch/output/portfolio_20251201_180023
==========================================

Starting portfolio simulation...

Portfolio simulation completed successfully

Organizing results...
  ✓ Copied: portfolioStatistics_adjusted_20251201180023.csv
  ✓ Copied: portfolioMonthlyReport_adjusted_20251201180023.csv
  ✓ Copied: portfolioWeeklyReport_adjusted_20251201180023.csv
  ✓ Copied: portfolioDailyReport_adjusted_20251201180023.csv

==========================================
Portfolio Performance Summary
==========================================
  Total Return:     72.9478x
  CAGR:             73.16%
  Martin Ratio:     22.3169
  Max Drawdown:     12.96%
  Sharpe Ratio:     2.0978
  Profit Factor:    1.4620
  Total Trades:     4,539
  Winning %:        49.79%

  Performance:      EXCELLENT
  Profit:           +7194.78%

==========================================
Generating Visualizations
==========================================
  ✓ Visualizations generated successfully
  ✓ Generated 4 chart files

==========================================
Simulation Complete!
==========================================
Results saved to: batch/output/portfolio_20251201_180023
```

---

## Quick Access Commands

### View Statistics

```bash
cat batch/output/portfolio_*/portfolioStatistics_adjusted.csv
```

### View Latest Results

```bash
# Find latest run
LATEST=$(ls -td batch/output/portfolio_* | head -1)

# View statistics
cat $LATEST/portfolioStatistics_adjusted.csv

# View summary
cat $LATEST/simulation_summary.txt

# Open visualizations (macOS)
open $LATEST/visualizations/*.png
```

### Compare Multiple Runs

```bash
# Run with different configs
./run_portfolio.sh portfoliorisk5.config 2018-01-01
./run_portfolio.sh portfoliorisk2.config 2018-01-01

# Compare results
diff batch/output/portfolio_*/portfolioStatistics_adjusted.csv
```

---

## Visualization Details

### Charts Generated

1. **Portfolio Allocation** (`01_portfolio_allocation.png`)
   - Bar chart showing risk allocation per strategy
   - Pie chart showing allocation percentages

2. **Monthly Performance** (`02_monthly_performance.png`)
   - Equity curve over time
   - Monthly returns ($)
   - Monthly returns (%)

3. **Weekly Performance** (`03_weekly_performance.png`)
   - Weekly equity progression
   - Weekly returns analysis

4. **Daily Performance** (`04_daily_performance.png`)
   - Daily equity curve
   - Daily returns distribution

5. **Portfolio Statistics** (`05_portfolio_statistics.png`)
   - Only generated if optimization data is available
   - Compares multiple portfolio configurations

6. **Risk-Return Analysis** (`06_risk_return_analysis.png`)
   - Only generated if optimization data is available
   - Scatter plots of risk vs return

---

## Requirements

### Java
- Java 8+ required
- JAR file: `target/PortfolioResult-1.0.0-jar-with-dependencies.jar`

### Python (for visualizations)
- Python 3.6+
- Required packages: `pandas`, `matplotlib`, `seaborn`, `numpy`

**Install Python dependencies**:
```bash
pip install pandas matplotlib seaborn numpy
```

Or use requirements.txt:
```bash
pip install -r requirements.txt
```

---

## Troubleshooting

### Issue: Visualization Script Not Found

**Solution**: Ensure `visualize_portfolio.py` is in the PortfolioResult directory

### Issue: Python Dependencies Missing

**Error**: `ModuleNotFoundError: No module named 'pandas'`

**Solution**:
```bash
pip install pandas matplotlib seaborn numpy
```

### Issue: No Results Files Found

**Check**:
- Portfolio simulation completed successfully
- Check `simulation.log` for errors
- Verify risk config file exists and is valid

### Issue: Visualizations Not Generated

**Possible Causes**:
- Missing Python dependencies
- Missing data files (some visualizations require specific files)
- Check `visualization.log` for details

---

## Comparison with Optimizer

| Feature | Portfolio Simulation | Optimizer |
|---------|---------------------|-----------|
| **Purpose** | Run simulation with fixed allocation | Find optimal allocation |
| **Input** | Risk config file | Risk + Preset + Factor configs |
| **Output** | Single portfolio results | Multiple portfolios tested |
| **Time** | ~5-10 seconds | ~30 seconds - 5 minutes |
| **Visualizations** | Basic charts | Full optimization analysis |

**Use Portfolio Simulation when**:
- You have a known good allocation
- You want to test a specific configuration
- You need quick results

**Use Optimizer when**:
- You want to find the best allocation
- You need to explore the solution space
- You want to compare multiple configurations

---

## Integration with Other Scripts

### Run Simulation After Optimization

```bash
# 1. Run optimizer to find best allocation
./run_optimizer.sh 5 2018-01-01

# 2. Extract best portfolio
BEST=$(ls -td batch/output/optimizer_* | head -1)
cp $BEST/best_portfolio.csv portfoliorisk_best.config

# 3. Run simulation with best allocation
./run_portfolio.sh portfoliorisk_best.config 2018-01-01
```

### Batch Processing

```bash
# Run multiple configurations
for config in portfoliorisk{1..7}.config; do
    ./run_portfolio.sh "$config" 2018-01-01
done
```

---

## Best Practices

1. **Use Descriptive Config Names**: Name your risk config files clearly
   - `portfoliorisk5.config` - Good
   - `my_config.csv` - Less clear

2. **Save Important Results**: Copy results to a permanent location
   ```bash
   cp -r batch/output/portfolio_* important_results/
   ```

3. **Review Visualizations**: Always check the generated charts
   - Look for unusual patterns
   - Verify equity curve is smooth
   - Check for excessive drawdowns

4. **Compare with Previous Runs**: Keep track of performance over time
   ```bash
   # Save results with descriptive name
   cp -r batch/output/portfolio_* results_$(date +%Y%m%d)_v2
   ```

---

## Related Scripts

- `run_optimizer.sh` - Portfolio optimization
- `run_multiple_seeds.sh` - Multi-seed optimization
- `test_deterministic_seed.sh` - Test deterministic behavior
- `visualize_portfolio.py` - Standalone visualization tool

---

**Last Updated**: December 2024  
**Related Documents**: 
- `MULTI_SEED_OPTIMIZATION_GUIDE.md`
- `README_VISUALIZATION.md`

