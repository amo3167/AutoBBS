# Portfolio Optimization Visualization Tool

This tool creates comprehensive visualizations for portfolio optimization results from the AutoBBS optimizer.

## Features

The script generates the following visualizations:

1. **Portfolio Allocation** - Bar chart and pie chart showing risk distribution across strategies
2. **Monthly Performance** - Equity curve, monthly returns ($), and monthly returns (%)
3. **Portfolio Statistics** - Comparison of performance metrics across top 5 portfolios
4. **Risk-Return Analysis** - Scatter plots showing risk vs return relationships
5. **Summary Report** - Text-based summary with key metrics

## Installation

### 1. Install Required Python Packages

```bash
cd /Users/andym/projects/AutoBBS/PortfolioResult
pip install -r requirements.txt
```

Or install individually:
```bash
pip install pandas matplotlib seaborn numpy
```

### 2. Make Script Executable (Optional)

```bash
chmod +x visualize_portfolio.py
```

## Usage

### Basic Usage

Visualize the latest optimizer output:

```bash
python visualize_portfolio.py batch/output/optimizer_20251130_233337
```

This will create a `visualizations` folder inside the input directory with all charts.

### Custom Output Directory

Specify a custom output location:

```bash
python visualize_portfolio.py batch/output/optimizer_20251130_233337 -o my_custom_charts
```

### View All Options

```bash
python visualize_portfolio.py --help
```

## Output Files

The script generates the following files:

- `00_summary_report.txt` - Text summary of key metrics
- `01_portfolio_allocation.png` - Strategy allocation charts
- `02_monthly_performance.png` - Time series analysis
- `03_portfolio_statistics.png` - Performance metrics comparison
- `04_risk_return_analysis.png` - Risk-return scatter plots

## Input Data Files

The script automatically loads these files from the optimizer output directory:

- `best_portfolio.csv` - Optimal strategy allocation
- `portfolioMonthlyReport_adjusted_*.csv` - Monthly performance data
- `portfolioWeeklyReport_adjusted_*.csv` - Weekly performance data (optional)
- `portfolioStatistics_adjusted_*.csv` - Portfolio statistics
- `portfolioStatistics_optimize_adjusted.csv_*.csv` - All evaluated portfolios
- `portfolioResult_baseline.csv_*.csv` - Baseline comparison (optional)

## Examples

### Example 1: Visualize Latest Run

```bash
# Find latest optimizer output
ls -lt batch/output/

# Visualize it
python visualize_portfolio.py batch/output/optimizer_20251130_233337
```

### Example 2: Compare Multiple Runs

```bash
# Visualize run 1
python visualize_portfolio.py batch/output/optimizer_20251130_233337 -o comparison/run1

# Visualize run 2
python visualize_portfolio.py batch/output/optimizer_20251201_120000 -o comparison/run2

# Now you can compare the charts side by side
```

### Example 3: Quick Check

```bash
# Generate visualizations and view summary immediately
python visualize_portfolio.py batch/output/optimizer_20251130_233337
cat batch/output/optimizer_20251130_233337/visualizations/00_summary_report.txt
```

## Customization

To customize the visualizations, edit `visualize_portfolio.py`:

- **Colors**: Modify the color schemes in plot functions
- **Figure Size**: Adjust `plt.rcParams['figure.figsize']` at the top
- **Metrics**: Add/remove metrics in `plot_portfolio_statistics()`
- **Date Format**: Change date formatting in `plot_monthly_performance()`

## Troubleshooting

### Missing Data Error

If you see "Not found" messages:
- Check that the optimizer completed successfully
- Verify CSV files exist in the input directory
- Some files (like weekly report) are optional

### Import Errors

If you get import errors:
```bash
# Ensure you're using the correct Python environment
which python

# Reinstall dependencies
pip install --upgrade -r requirements.txt
```

### Memory Issues

For large datasets:
- Reduce the number of portfolios displayed (edit `n_portfolios` in code)
- Process one visualization at a time (comment out others in `generate_all_visualizations()`)

## Integration with Optimizer

After running the optimizer:

```bash
# Run optimizer
./run_optimizer.sh 4 2018-01-01

# Find the output directory (printed at end of optimizer run)
OUTPUT_DIR=batch/output/optimizer_YYYYMMDD_HHMMSS

# Generate visualizations
python visualize_portfolio.py $OUTPUT_DIR
```

You can add this to your workflow script for automatic visualization after each optimization run.

## Technical Details

### Dependencies
- **pandas**: Data manipulation and CSV reading
- **matplotlib**: Core plotting functionality
- **seaborn**: Statistical visualizations and styling
- **numpy**: Numerical operations

### Performance
- Typical runtime: < 10 seconds for standard datasets
- Memory usage: Proportional to number of portfolios evaluated
- Output size: ~2-5 MB for all PNG files (high resolution)

## License

This tool is part of the AutoBBS project.
