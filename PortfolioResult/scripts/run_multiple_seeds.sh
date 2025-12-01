#!/bin/bash

# Multi-Seed Optimization Runner
# Runs optimization with multiple seeds (1-10) and finds the best result
# Usage: ./run_multiple_seeds.sh [config_number] [start_date] [num_seeds]
# Example: ./run_multiple_seeds.sh 5 2018-01-01 10

set -e

CONFIG_NUM=${1:-5}
START_DATE=${2:-2018-01-01}
NUM_SEEDS=${3:-10}

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
cd "$PROJECT_ROOT"

CONFIG_DIR="portfolioRiskConfig"
CONFIG_FILE="${CONFIG_DIR}/portfolioOptimize${CONFIG_NUM}.config"
PRESET_FILE="${CONFIG_DIR}/portfolioOptimizePreset${CONFIG_NUM}.config"
FACTOR_FILE="${CONFIG_DIR}/portfolioOptimizeFactor${CONFIG_NUM}.config"
JAR_FILE="target/PortfolioResult-1.0.0-jar-with-dependencies.jar"

# Filenames to pass to Java app (without directory prefix, as App.java prepends PortfolioRisk_Location)
CONFIG_NAME="portfolioOptimize${CONFIG_NUM}.config"
PRESET_NAME="portfolioOptimizePreset${CONFIG_NUM}.config"
FACTOR_NAME="portfolioOptimizeFactor${CONFIG_NUM}.config"

# Validate files exist
if [ ! -f "$CONFIG_FILE" ]; then
    echo "Error: Config file not found: $CONFIG_FILE"
    exit 1
fi

if [ ! -f "$FACTOR_FILE" ]; then
    echo "Error: Factor file not found: $FACTOR_FILE"
    exit 1
fi

if [ ! -f "$JAR_FILE" ]; then
    echo "Error: JAR file not found. Building project..."
    mvn clean package -DskipTests
fi

# Create empty preset file if it doesn't exist
if [ ! -f "$PRESET_FILE" ]; then
    echo "Creating empty preset file: $PRESET_FILE"
    echo "StrategyID,Risk" > "$PRESET_FILE"
fi

echo "=========================================="
echo "Multi-Seed Optimization Runner"
echo "=========================================="
echo "Config:      $CONFIG_FILE"
echo "Preset:      $PRESET_FILE"
echo "Factors:     $FACTOR_FILE"
echo "Start Date:  $START_DATE"
echo "Seeds:       1 to $NUM_SEEDS"
echo "=========================================="
echo ""

# Create results directory
RESULTS_DIR="batch/output/multi_seed_$(date +%Y%m%d_%H%M%S)"
mkdir -p "$RESULTS_DIR"
SUMMARY_FILE="$RESULTS_DIR/summary.csv"
BEST_RESULTS_FILE="$RESULTS_DIR/best_results.csv"

# Initialize summary file
echo "Seed,OutputDir,TotalReturn,MartinRatio,CAGR,MaxDD,BestPortfolioFile" > "$SUMMARY_FILE"

echo "Running optimizations with seeds 1-$NUM_SEEDS..."
echo ""

# Run optimization for each seed
for seed in $(seq 1 $NUM_SEEDS); do
    echo "----------------------------------------"
    echo "Running optimization with seed $seed/$NUM_SEEDS..."
    echo "----------------------------------------"
    
    LOG_FILE="$RESULTS_DIR/seed_${seed}.log"
    
    # Run optimizer (pass filenames only, App.java prepends PortfolioRisk_Location)
    java -Xmx4g -cp "$JAR_FILE" \
        PortfolioResult.PortfolioResult.App \
        optimizer \
        "$CONFIG_NAME" \
        "$PRESET_NAME" \
        "$FACTOR_NAME" \
        "$START_DATE" \
        "$seed" \
        > "$LOG_FILE" 2>&1
    
    EXIT_CODE=${PIPESTATUS[0]}
    
    if [ $EXIT_CODE -ne 0 ]; then
        echo "  ✗ FAILED with exit code: $EXIT_CODE"
        echo "  Check $LOG_FILE for details"
        continue
    fi
    
    # Find the output directory (most recent optimizer_* directory)
    sleep 1
    OUTPUT_DIR=$(ls -td batch/output/optimizer_* 2>/dev/null | head -1)
    
    if [ -z "$OUTPUT_DIR" ]; then
        echo "  ✗ Could not find output directory"
        continue
    fi
    
    # Copy output directory to results
    SEED_OUTPUT_DIR="$RESULTS_DIR/seed_${seed}"
    cp -r "$OUTPUT_DIR" "$SEED_OUTPUT_DIR"
    
    # Track optimizer directory for cleanup later
    echo "$OUTPUT_DIR" >> "$RESULTS_DIR/.optimizer_dirs_to_cleanup.txt"
    
    # Extract statistics
    STATS_FILE=$(find "$SEED_OUTPUT_DIR" -name "portfolioStatistics_adjusted_*.csv" | head -1)
    BEST_PORTFOLIO="$SEED_OUTPUT_DIR/best_portfolio.csv"
    
    if [ -z "$STATS_FILE" ] || [ ! -f "$BEST_PORTFOLIO" ]; then
        echo "  ✗ Could not find statistics or best portfolio file"
        continue
    fi
    
    # Parse statistics using Python
    STATS=$(python3 << EOF
import csv
import sys

try:
    with open('$STATS_FILE', 'r') as f:
        reader = csv.DictReader(f)
        row = next(reader)
        total_return = float(row['TotalReturn'])
        martin = float(row['Martin'])
        cagr = float(row['Cagr'])
        max_dd = float(row['Max_DD'])
        print(f"{total_return:.6f},{martin:.6f},{cagr:.6f},{max_dd:.6f}")
except Exception as e:
    print("ERROR,ERROR,ERROR,ERROR", file=sys.stderr)
    sys.exit(1)
EOF
)
    
    if [ $? -ne 0 ] || [ "$STATS" = "ERROR,ERROR,ERROR,ERROR" ]; then
        echo "  ✗ Could not parse statistics"
        continue
    fi
    
    TOTAL_RETURN=$(echo "$STATS" | cut -d',' -f1)
    MARTIN=$(echo "$STATS" | cut -d',' -f2)
    CAGR=$(echo "$STATS" | cut -d',' -f3)
    MAX_DD=$(echo "$STATS" | cut -d',' -f4)
    
    # Add to summary
    echo "$seed,$SEED_OUTPUT_DIR,$TOTAL_RETURN,$MARTIN,$CAGR,$MAX_DD,$BEST_PORTFOLIO" >> "$SUMMARY_FILE"
    
    echo "  ✓ Completed"
    echo "    Total Return: ${TOTAL_RETURN}x"
    echo "    Martin Ratio: ${MARTIN}"
    echo "    CAGR: ${CAGR}%"
    echo "    Max DD: ${MAX_DD}%"
    echo ""
done

echo "=========================================="
echo "Analysis Complete"
echo "=========================================="
echo ""

# Find best result using Python
python3 << EOF
import csv
import sys

summary_file = '$SUMMARY_FILE'
best_results_file = '$BEST_RESULTS_FILE'

try:
    with open(summary_file, 'r') as f:
        reader = csv.DictReader(f)
        results = []
        for row in reader:
            try:
                results.append({
                    'seed': int(row['Seed']),
                    'output_dir': row['OutputDir'],
                    'total_return': float(row['TotalReturn']),
                    'martin': float(row['MartinRatio']),
                    'cagr': float(row['CAGR']),
                    'max_dd': float(row['MaxDD']),
                    'best_portfolio': row['BestPortfolioFile']
                })
            except (ValueError, KeyError) as e:
                continue
        
        if not results:
            print("ERROR: No valid results found")
            sys.exit(1)
        
        # Sort by Martin Ratio (primary) and Total Return (secondary)
        results.sort(key=lambda x: (x['martin'], x['total_return']), reverse=True)
        
        # Write best results
        with open(best_results_file, 'w') as out:
            writer = csv.DictWriter(out, fieldnames=['Rank', 'Seed', 'MartinRatio', 'TotalReturn', 'CAGR', 'MaxDD', 'OutputDir'])
            writer.writeheader()
            for i, r in enumerate(results[:10], 1):  # Top 10
                writer.writerow({
                    'Rank': i,
                    'Seed': r['seed'],
                    'MartinRatio': f"{r['martin']:.6f}",
                    'TotalReturn': f"{r['total_return']:.6f}",
                    'CAGR': f"{r['cagr']:.6f}",
                    'MaxDD': f"{r['max_dd']:.6f}",
                    'OutputDir': r['output_dir']
                })
        
        # Print summary
        print("TOP 5 RESULTS (sorted by Martin Ratio):")
        print("-" * 100)
        print(f"{'Rank':<6} {'Seed':<6} {'Martin Ratio':<15} {'Total Return':<15} {'CAGR':<12} {'Max DD':<12}")
        print("-" * 100)
        
        for i, r in enumerate(results[:5], 1):
            print(f"{i:<6} {r['seed']:<6} {r['martin']:<15.6f} {r['total_return']:<15.6f} {r['cagr']:<12.2f} {r['max_dd']:<12.2f}")
        
        print()
        print("=" * 100)
        print("BEST RESULT:")
        print("=" * 100)
        best = results[0]
        print(f"  Seed: {best['seed']}")
        print(f"  Martin Ratio: {best['martin']:.6f}")
        print(f"  Total Return: {best['total_return']:.6f}x")
        print(f"  CAGR: {best['cagr']:.2f}%")
        print(f"  Max Drawdown: {best['max_dd']:.2f}%")
        print(f"  Output Directory: {best['output_dir']}")
        print(f"  Best Portfolio: {best['best_portfolio']}")
        print()
        
        # Check for identical results (indicates convergence to optimal solution)
        unique_results = {}
        for r in results:
            key = (r['martin'], r['total_return'], r['cagr'], r['max_dd'])
            if key not in unique_results:
                unique_results[key] = []
            unique_results[key].append(r['seed'])
        
        if len(unique_results) < len(results):
            print("=" * 100)
            print("NOTE: Some seeds produced identical results (convergence to optimal solution):")
            print("=" * 100)
            for key, seeds in unique_results.items():
                if len(seeds) > 1:
                    print(f"  Seeds {', '.join(map(str, sorted(seeds)))}: Martin={key[0]:.6f}, Return={key[1]:.6f}x")
                    print(f"    → This indicates the genetic algorithm found the same optimal solution")
                    print(f"    → Different seeds explore different paths but may converge to the same result")
            print()
        
        # Show improvement over worst
        if len(results) > 1:
            worst = results[-1]
            martin_improvement = ((best['martin'] - worst['martin']) / worst['martin']) * 100
            return_improvement = ((best['total_return'] - worst['total_return']) / worst['total_return']) * 100
            print("=" * 100)
            print("IMPROVEMENT OVER WORST RESULT:")
            print("=" * 100)
            print(f"  Martin Ratio: +{martin_improvement:.2f}% ({worst['martin']:.6f} → {best['martin']:.6f})")
            print(f"  Total Return: +{return_improvement:.2f}% ({worst['total_return']:.6f}x → {best['total_return']:.6f}x)")
            print()
        
        print("=" * 100)
        print("FILES CREATED:")
        print("=" * 100)
        print(f"  Summary: $SUMMARY_FILE")
        print(f"  Best Results: $BEST_RESULTS_FILE")
        print(f"  Results Directory: $RESULTS_DIR")
        print()
        print("To view the best portfolio:")
        print(f"  cat {best['best_portfolio']}")
        print()
        
except Exception as e:
    print(f"ERROR analyzing results: {e}", file=sys.stderr)
    import traceback
    traceback.print_exc()
    sys.exit(1)
EOF

EXIT_CODE=$?

if [ $EXIT_CODE -eq 0 ]; then
    echo "=========================================="
    echo "Multi-seed optimization completed!"
    echo "=========================================="
    echo ""
    echo "All results saved to: $RESULTS_DIR"
    echo ""
else
    echo "ERROR: Failed to analyze results"
    exit 1
fi

# Cleanup temporary files (disable exit on error for cleanup)
set +e
echo ""
echo "=========================================="
echo "Cleaning up temporary files..."
echo "=========================================="

# Clean up timestamped CSV files in batch root directory
BATCH_DIR="batch"
CLEANED=0

PATTERNS=(
    "portfolioStatistics_adjusted_*.csv"
    "portfolioResult_baseline_*.csv"
    "portfolioResult_adjusted_*.csv"
    "portfolioDailyReport_adjusted_*.csv"
    "portfolioWeeklyReport_adjusted_*.csv"
    "portfolioMonthlyReport_adjusted_*.csv"
)

for pattern in "${PATTERNS[@]}"; do
    while IFS= read -r file; do
        if [ -f "$file" ]; then
            # Only remove files in batch root, not in output subdirectories
            if [[ "$file" == "${BATCH_DIR}/"* ]] && [[ "$file" != *"/output/"* ]]; then
                rm -f "$file" 2>/dev/null
                if [ $? -eq 0 ]; then
                    ((CLEANED++))
                fi
            fi
        fi
    done < <(find "$BATCH_DIR" -maxdepth 1 -name "$pattern" -type f 2>/dev/null)
done

if [ $CLEANED -gt 0 ]; then
    echo "  ✓ Cleaned up $CLEANED timestamped CSV files from batch directory"
else
    echo "  ✓ No timestamped CSV files to clean"
fi

# Clean up optimizer directories that were copied to seed_* directories
OPTIMIZER_CLEANED=0
if [ -f "$RESULTS_DIR/.optimizer_dirs_to_cleanup.txt" ]; then
    while IFS= read -r optimizer_dir; do
        if [ -n "$optimizer_dir" ] && [ -d "$optimizer_dir" ]; then
            rm -rf "$optimizer_dir" 2>/dev/null
            if [ $? -eq 0 ]; then
                ((OPTIMIZER_CLEANED++))
                echo "  ✓ Removed optimizer directory: $(basename "$optimizer_dir")"
            fi
        fi
    done < "$RESULTS_DIR/.optimizer_dirs_to_cleanup.txt"
    rm -f "$RESULTS_DIR/.optimizer_dirs_to_cleanup.txt" 2>/dev/null
fi

if [ $OPTIMIZER_CLEANED -gt 0 ]; then
    echo "  ✓ Cleaned up $OPTIMIZER_CLEANED optimizer directories"
else
    echo "  ✓ No optimizer directories to clean"
fi

echo ""
echo "Cleanup completed!"
echo ""

# Re-enable exit on error
set -e

