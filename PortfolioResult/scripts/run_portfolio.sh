#!/bin/bash

# Portfolio Simulation Runner with Visualization
# Runs portfolio simulation and generates visual charts
# Usage: ./run_portfolio.sh <config_number> [start_date] [check_orders]
# Example: ./run_portfolio.sh 5 2018-01-01 false

set -e

CONFIG_NUM=${1:-5}
START_DATE=${2:-2018-01-01}
CHECK_ORDERS=${3:-false}

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
cd "$PROJECT_ROOT"

CONFIG_DIR="portfolioRiskConfig"
RISK_CONFIG="${CONFIG_DIR}/portfoliorisk${CONFIG_NUM}.config"
# Filename to pass to Java app (without directory prefix, as App.java prepends PortfolioRisk_Location)
RISK_CONFIG_NAME="portfoliorisk${CONFIG_NUM}.config"
JAR_FILE="target/PortfolioResult-1.0.0-jar-with-dependencies.jar"
BATCH_DIR="batch"
OUTPUT_BASE="batch/output"

# Validate config file exists
if [ ! -f "$RISK_CONFIG" ]; then
    echo "Error: Risk config file not found: $RISK_CONFIG"
    echo "Available config files:"
    ls -1 ${CONFIG_DIR}/portfoliorisk*.config 2>/dev/null | sed 's/.*\//  /' || echo "  (none found)"
    exit 1
fi

# Check if JAR exists
if [ ! -f "$JAR_FILE" ]; then
    echo "Error: JAR file not found. Building project..."
    mvn clean package -DskipTests
fi

# Create timestamped output directory
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
OUTPUT_DIR="${OUTPUT_BASE}/portfolio_${TIMESTAMP}"
mkdir -p "$OUTPUT_DIR"

echo "=========================================="
echo "Portfolio Simulation Runner"
echo "=========================================="
echo "Config Number: $CONFIG_NUM"
echo "Risk Config:   $RISK_CONFIG"
echo "Start Date:    $START_DATE"
echo "Check Orders:  $CHECK_ORDERS"
echo "Output Dir:    $OUTPUT_DIR"
echo "=========================================="
echo ""

# Run portfolio simulation
echo "Starting portfolio simulation at $(date)..."
echo ""

java -Xmx4g -cp "$JAR_FILE" \
    PortfolioResult.PortfolioResult.App \
    run \
    "$RISK_CONFIG_NAME" \
    "$CHECK_ORDERS" \
    "$START_DATE" \
    > "$OUTPUT_DIR/simulation.log" 2>&1

EXIT_CODE=${PIPESTATUS[0]}

if [ $EXIT_CODE -ne 0 ]; then
    echo ""
    echo "=========================================="
    echo "Portfolio simulation FAILED"
    echo "Exit code: $EXIT_CODE"
    echo "Check $OUTPUT_DIR/simulation.log for details"
    echo "=========================================="
    exit $EXIT_CODE
fi

echo "Portfolio simulation completed successfully"
echo ""

# Move results to output directory
echo "Organizing results..."
echo ""

# Find and move all generated files (with timestamps)
MOVED_COUNT=0

# Function to move files matching pattern (remove timestamp, keep clean names only)
move_files() {
    local pattern="$1"
    local target_name="$2"
    
    # Find most recent file matching pattern
    local latest_file=$(ls -t ${BATCH_DIR}/${pattern} 2>/dev/null | head -1)
    
    if [ -n "$latest_file" ] && [ -f "$latest_file" ]; then
        BASENAME=$(basename "$latest_file")
        
        # Use provided target name, or remove timestamp to create clean name
        if [ -n "$target_name" ]; then
            FINAL_NAME="$target_name"
        else
            FINAL_NAME=$(echo "$BASENAME" | sed 's/_[0-9]\{14\}\.csv$/.csv/')
        fi
        
        # Move file (not copy) to output directory with clean name
        mv "$latest_file" "$OUTPUT_DIR/$FINAL_NAME"
        ((MOVED_COUNT++))
        echo "  ✓ Moved: $BASENAME → $FINAL_NAME"
    fi
}

# Move files (remove timestamps, keep clean names only)
move_files "portfolioStatistics_adjusted_*.csv" "portfolioStatistics_adjusted.csv"
move_files "portfolioResult_baseline.csv_*.csv" "portfolioResult_baseline.csv"
move_files "portfolioResult_adjusted.csv_*.csv" "portfolioResult_adjusted.csv"
move_files "portfolioDailyReport_adjusted_*.csv" "portfolioDailyReport_adjusted.csv"
move_files "portfolioWeeklyReport_adjusted_*.csv" "portfolioWeeklyReport_adjusted.csv"
move_files "portfolioMonthlyReport_adjusted_*.csv" "portfolioMonthlyReport_adjusted.csv"

# Copy risk config to output directory for reference
cp "$RISK_CONFIG" "$OUTPUT_DIR/portfolio_risk_config.csv"
echo "  ✓ Copied: portfolio_risk_config.csv"

# Create a best_portfolio.csv file from the risk config (for visualization compatibility)
if [ ! -f "$OUTPUT_DIR/best_portfolio.csv" ]; then
    echo "StrategyID,Risk" > "$OUTPUT_DIR/best_portfolio.csv"
    tail -n +2 "$RISK_CONFIG" >> "$OUTPUT_DIR/best_portfolio.csv"
    echo "  ✓ Created: best_portfolio.csv"
fi

echo ""
echo "  Moved $MOVED_COUNT result files to $OUTPUT_DIR"
echo ""

# Clean up any remaining temp files in batch directory
echo "Cleaning up temporary files in batch directory..."
CLEANED=0
for pattern in "portfolioStatistics_adjusted_*.csv" "portfolioResult_baseline_*.csv" "portfolioResult_adjusted_*.csv" "portfolioDailyReport_adjusted_*.csv" "portfolioWeeklyReport_adjusted_*.csv" "portfolioMonthlyReport_adjusted_*.csv"; do
    for file in ${BATCH_DIR}/${pattern}; do
        if [ -f "$file" ]; then
            # Only remove files in batch root, not in output subdirectories
            if [[ "$file" == "${BATCH_DIR}/"* ]] && [[ "$file" != *"/output/"* ]]; then
                rm -f "$file"
                ((CLEANED++))
            fi
        fi
    done
done

if [ $CLEANED -gt 0 ]; then
    echo "  ✓ Cleaned up $CLEANED temporary files"
else
    echo "  ✓ No temporary files to clean"
fi
echo ""

# Extract and display key statistics
echo "=========================================="
echo "Portfolio Performance Summary"
echo "=========================================="

STATS_FILE=$(find "$OUTPUT_DIR" -name "portfolioStatistics_adjusted*.csv" | head -1)

if [ -n "$STATS_FILE" ] && [ -f "$STATS_FILE" ]; then
    python3 << EOF
import csv
import sys

try:
    with open('$STATS_FILE', 'r') as f:
        reader = csv.DictReader(f)
        row = next(reader)
        
        total_return = float(row['TotalReturn'])
        cagr = float(row['Cagr'])
        martin = float(row['Martin'])
        max_dd = float(row['Max_DD'])
        sharpe = float(row['Sharp_ratio'])
        profit_factor = float(row['Profit_factor'])
        num_trades = int(row['Num_trades'])
        winning = float(row['Winning'])
        
        print(f"  Total Return:     {total_return:.4f}x")
        print(f"  CAGR:             {cagr:.2f}%")
        print(f"  Martin Ratio:     {martin:.4f}")
        print(f"  Max Drawdown:     {max_dd:.2f}%")
        print(f"  Sharpe Ratio:     {sharpe:.4f}")
        print(f"  Profit Factor:    {profit_factor:.4f}")
        print(f"  Total Trades:     {num_trades:,}")
        print(f"  Winning %:        {winning:.2f}%")
        print()
        
        # Performance rating
        if total_return > 50:
            rating = "EXCELLENT"
        elif total_return > 20:
            rating = "VERY GOOD"
        elif total_return > 10:
            rating = "GOOD"
        elif total_return > 5:
            rating = "MODERATE"
        else:
            rating = "POOR"
        
        print(f"  Performance:      {rating}")
        if total_return > 1:
            profit_pct = (total_return - 1) * 100
            print(f"  Profit:           +{profit_pct:.2f}%")
        
except Exception as e:
    print(f"  Error reading statistics: {e}", file=sys.stderr)
    sys.exit(1)
EOF
else
    echo "  Warning: Statistics file not found"
fi

echo ""

# Generate visualizations
echo "=========================================="
echo "Generating Visualizations"
echo "=========================================="
echo ""

# Check if visualization script exists
VISUALIZER_SCRIPT="${SCRIPT_DIR}/visualize_portfolio.py"
if [ ! -f "$VISUALIZER_SCRIPT" ]; then
    echo "  Warning: visualize_portfolio.py not found in scripts directory"
    echo "  Skipping visualization generation"
    echo ""
else
    # Check Python dependencies
    if ! python3 -c "import pandas, matplotlib, seaborn" 2>/dev/null; then
        echo "  Warning: Python visualization dependencies not installed"
        echo "  Install with: pip install pandas matplotlib seaborn numpy"
        echo "  Skipping visualization generation"
        echo ""
    else
        echo "  Running visualization script..."
        # Suppress warnings about missing optimization files (expected for portfolio simulation)
        python3 "$VISUALIZER_SCRIPT" "$OUTPUT_DIR" -o "$OUTPUT_DIR/visualizations" 2>&1 | grep -v "Not found\|not available" | tee "$OUTPUT_DIR/visualization.log"
        
        VIZ_EXIT_CODE=${PIPESTATUS[0]}
        
        if [ $VIZ_EXIT_CODE -eq 0 ]; then
            echo ""
            echo "  ✓ Visualizations generated successfully"
            VIZ_COUNT=$(find "$OUTPUT_DIR/visualizations" -name "*.png" 2>/dev/null | wc -l)
            echo "  ✓ Generated $VIZ_COUNT chart files"
        else
            echo ""
            echo "  ⚠️  Visualization generation had issues (check visualization.log)"
        fi
        echo ""
    fi
fi

# Create summary file
SUMMARY_FILE="$OUTPUT_DIR/simulation_summary.txt"
cat > "$SUMMARY_FILE" << EOF
Portfolio Simulation Summary
===========================
Date: $(date)
Risk Config: $RISK_CONFIG
Start Date: $START_DATE
Check Orders: $CHECK_ORDERS

Output Directory: $OUTPUT_DIR

Files Generated:
EOF

for file in "$OUTPUT_DIR"/*.csv "$OUTPUT_DIR"/*.log; do
    if [ -f "$file" ]; then
        echo "  - $(basename "$file")" >> "$SUMMARY_FILE"
    fi
done

if [ -d "$OUTPUT_DIR/visualizations" ]; then
    echo "" >> "$SUMMARY_FILE"
    echo "Visualizations:" >> "$SUMMARY_FILE"
    for file in "$OUTPUT_DIR/visualizations"/*.png; do
        if [ -f "$file" ]; then
            echo "  - visualizations/$(basename "$file")" >> "$SUMMARY_FILE"
        fi
    done
fi

echo ""
echo "=========================================="
echo "Simulation Complete!"
echo "=========================================="
echo ""
echo "Results saved to: $OUTPUT_DIR"
echo ""
echo "Key Files:"
echo "  - portfolioStatistics_adjusted.csv (performance statistics)"
echo "  - portfolioMonthlyReport_adjusted.csv (monthly performance)"
echo "  - portfolio_risk_config.csv (portfolio configuration)"
echo "  - simulation.log (execution log)"
echo ""

if [ -d "$OUTPUT_DIR/visualizations" ]; then
    echo "Visualizations:"
    echo "  - visualizations/01_portfolio_allocation.png"
    echo "  - visualizations/02_monthly_performance.png"
    echo "  - visualizations/03_weekly_performance.png"
    echo "  - visualizations/04_daily_performance.png"
    echo "  - visualizations/05_portfolio_statistics.png"
    echo "  - visualizations/06_risk_return_analysis.png"
    echo ""
fi

echo "To view statistics:"
echo "  cat $OUTPUT_DIR/portfolioStatistics_adjusted.csv"
echo ""
echo "To view summary:"
echo "  cat $SUMMARY_FILE"
echo ""

