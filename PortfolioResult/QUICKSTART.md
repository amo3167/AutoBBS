# Portfolio Optimizer Automation - Quick Start Guide

## 🚀 Quick Start

### Run Optimizer with Auto-Visualization

```bash
cd /Users/andym/projects/AutoBBS/PortfolioResult
./run_optimizer_with_viz.sh 4 2018-01-01
```

This single command will:
1. ✅ Run the portfolio optimizer
2. ✅ Automatically find the output directory
3. ✅ Generate all visualizations
4. ✅ Display a summary report

### Run Optimizer Only (Skip Visualization)

```bash
./run_optimizer_with_viz.sh 4 2018-01-01 --skip-viz
```

### Visualize Existing Results

```bash
python3 visualize_portfolio.py path/to/optimizer_output_directory
```

## 📁 File Structure

```
PortfolioResult/
├── run_optimizer.sh                    # Original optimizer script
├── run_optimizer_with_viz.sh          # NEW: Optimizer + auto-viz
├── visualize_portfolio.py             # NEW: Visualization script
├── requirements.txt                   # NEW: Python dependencies
├── README_VISUALIZATION.md            # NEW: Full documentation
└── batch/output/
    └── optimizer_YYYYMMDD_HHMMSS/     # Output directory
        ├── best_portfolio.csv
        ├── portfolioStatistics_*.csv
        └── visualizations/             # AUTO-GENERATED
            ├── 00_summary_report.txt
            ├── 01_portfolio_allocation.png
            ├── 02_monthly_performance.png
            ├── 03_portfolio_statistics.png
            └── 04_risk_return_analysis.png
```

## 🎯 Common Workflows

### Workflow 1: New Optimization Run
```bash
# Single command - does everything!
./run_optimizer_with_viz.sh 4 2018-01-01
```

### Workflow 2: Re-visualize Old Results
```bash
# Find old output
ls -lt batch/output/

# Visualize specific run
python3 visualize_portfolio.py batch/output/optimizer_20251130_233337
```

### Workflow 3: Compare Multiple Configurations
```bash
# Run config 4
./run_optimizer_with_viz.sh 4 2018-01-01

# Run config 5
./run_optimizer_with_viz.sh 5 2018-01-01

# Compare the visualizations/00_summary_report.txt from each
```

### Workflow 4: Batch Processing
```bash
# Create a batch script
cat > batch_optimize.sh << 'EOF'
#!/bin/bash
for config in 4 5 6; do
    echo "Running configuration $config..."
    ./run_optimizer_with_viz.sh $config 2018-01-01
done
EOF

chmod +x batch_optimize.sh
./batch_optimize.sh
```

## 📊 Generated Visualizations Explained

### 1. Portfolio Allocation (01_portfolio_allocation.png)
- **Bar Chart**: Shows risk multiplier for each strategy
- **Pie Chart**: Shows percentage distribution of risk
- **Use**: Understand which strategies dominate your portfolio

### 2. Monthly Performance (02_monthly_performance.png)
- **Top Panel**: Equity curve over time
- **Middle Panel**: Monthly P&L in dollars
- **Bottom Panel**: Monthly returns as percentage
- **Use**: Track performance trends and identify problem periods

### 3. Portfolio Statistics (03_portfolio_statistics.png)
- Compares top 5 portfolio configurations
- Shows: CAGR, Sharpe Ratio, Max DD, Martin Ratio, Ulcer Index, etc.
- **Use**: Compare alternative portfolio configurations

### 4. Risk-Return Analysis (04_risk_return_analysis.png)
- **Left**: Max Drawdown vs CAGR (colored by Martin Ratio)
- **Right**: Ulcer Index vs Sharpe Ratio (colored by Profit Factor)
- **Use**: Find the efficient frontier and identify optimal risk/return tradeoff

### 5. Summary Report (00_summary_report.txt)
- Text file with all key metrics
- **Use**: Quick reference, documentation, or pasting into reports

## 🛠️ Setup (One-Time)

### Install Python Dependencies
```bash
cd /Users/andym/projects/AutoBBS/PortfolioResult
pip3 install -r requirements.txt
```

Or manually:
```bash
pip3 install pandas matplotlib seaborn numpy
```

### Verify Installation
```bash
python3 -c "import pandas, matplotlib, seaborn, numpy; print('✓ All dependencies installed')"
```

## 💡 Pro Tips

### Tip 1: View Results Immediately
```bash
# Run optimizer
./run_optimizer_with_viz.sh 4 2018-01-01

# Find output directory (printed at end)
OUTPUT_DIR=$(ls -td batch/output/optimizer_* | head -1)

# View summary
cat $OUTPUT_DIR/visualizations/00_summary_report.txt

# Open all charts (macOS)
open $OUTPUT_DIR/visualizations/*.png
```

### Tip 2: Create Aliases
Add to your `~/.zshrc`:
```bash
alias opt='cd /Users/andym/projects/AutoBBS/PortfolioResult && ./run_optimizer_with_viz.sh'
alias optviz='python3 /Users/andym/projects/AutoBBS/PortfolioResult/visualize_portfolio.py'
```

Then you can run:
```bash
opt 4 2018-01-01
```

### Tip 3: Compare Multiple Runs
```bash
# Run multiple times with different parameters
./run_optimizer_with_viz.sh 4 2018-01-01
./run_optimizer_with_viz.sh 4 2019-01-01
./run_optimizer_with_viz.sh 4 2020-01-01

# Compare summary reports
diff batch/output/optimizer_20251130_*/visualizations/00_summary_report.txt
```

### Tip 4: Archive Good Results
```bash
# Find the best run
OUTPUT_DIR=$(ls -td batch/output/optimizer_* | head -1)

# Archive it with a meaningful name
cp -r $OUTPUT_DIR ~/portfolio_results/best_run_$(date +%Y%m%d)
```

## 🔧 Troubleshooting

### Problem: "Python not found"
```bash
# Install Python 3
brew install python3  # macOS
```

### Problem: "Module not found"
```bash
# Reinstall dependencies
pip3 install --upgrade -r requirements.txt
```

### Problem: "Permission denied"
```bash
# Make scripts executable
chmod +x run_optimizer_with_viz.sh
chmod +x visualize_portfolio.py
```

### Problem: "Visualization failed"
```bash
# Run with debug output
python3 -u visualize_portfolio.py batch/output/optimizer_YYYYMMDD_HHMMSS
```

### Problem: "Output directory not found"
Check that the optimizer completed successfully:
```bash
ls -ltr batch/output/
```

## 📖 More Information

- **Full Documentation**: See `README_VISUALIZATION.md`
- **Script Help**: `./run_optimizer_with_viz.sh --help`
- **Visualization Help**: `python3 visualize_portfolio.py --help`

## 🎓 Learning Resources

### Understanding the Metrics

- **CAGR**: Compound Annual Growth Rate - your average yearly return
- **Max Drawdown**: Largest peak-to-trough decline (lower is better)
- **Sharpe Ratio**: Risk-adjusted return (higher is better, >1.0 is good)
- **Martin Ratio**: CAGR divided by Ulcer Index (higher is better)
- **Ulcer Index**: Measures depth and duration of drawdowns (lower is better)
- **Profit Factor**: Gross profit / gross loss (>1.5 is good)
- **Win Rate**: Percentage of winning trades

### Reading the Charts

1. **Equity Curve**: Should trend upward over time
2. **Monthly Returns**: Should have more green (positive) than red (negative)
3. **Risk-Return Scatter**: Look for points in upper-left (high return, low risk)
4. **Portfolio Allocation**: Check for over-concentration in single strategies

## ⚡ Performance Notes

- **Optimizer Runtime**: Varies by configuration (minutes to hours)
- **Visualization Runtime**: Typically < 10 seconds
- **Output Size**: ~2-5 MB for high-resolution PNGs
- **Disk Space**: Each run needs ~10-20 MB

## 🔄 Integration with CI/CD

For automated testing:
```bash
#!/bin/bash
# test_optimization.sh
./run_optimizer_with_viz.sh 4 2018-01-01

# Check if optimization succeeded
if [ $? -eq 0 ]; then
    echo "✓ Optimization passed"
    # Archive results, send notifications, etc.
else
    echo "✗ Optimization failed"
    exit 1
fi
```

---

**Last Updated**: November 30, 2025
**Version**: 1.0
