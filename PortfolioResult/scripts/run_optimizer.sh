#!/bin/bash

# Portfolio Optimizer Runner Script
# Usage: ./run_optimizer.sh <config_number> [start_date]
# Example: ./run_optimizer.sh 4 2018-01-01

set -e

CONFIG_NUM=${1:-4}
START_DATE=${2:-2018-01-01}

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
cd "$PROJECT_ROOT"

CONFIG_DIR="portfolioRiskConfig"
CONFIG_FILE="${CONFIG_DIR}/portfolioOptimize${CONFIG_NUM}.config"
PRESET_FILE="${CONFIG_DIR}/portfolioOptimizePreset${CONFIG_NUM}.config"
FACTOR_FILE="${CONFIG_DIR}/portfolioOptimizeFactor${CONFIG_NUM}.config"
LOG_FILE="optimizer${CONFIG_NUM}.log"
JAR_FILE="target/PortfolioResult-1.0.0-jar-with-dependencies.jar"

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
echo "Portfolio Optimizer"
echo "=========================================="
echo "Config:      $CONFIG_FILE"
echo "Preset:      $PRESET_FILE"
echo "Factors:     $FACTOR_FILE"
echo "Start Date:  $START_DATE"
echo "Log File:    $LOG_FILE"
echo "=========================================="
echo ""

# Run optimizer
echo "Starting optimizer at $(date)..."
java -Xmx4g -cp "$JAR_FILE" \
    PortfolioResult.PortfolioResult.App \
    optimizer \
    "$CONFIG_FILE" \
    "$PRESET_FILE" \
    "$FACTOR_FILE" \
    "$START_DATE" \
    2>&1 | tee "$LOG_FILE"

EXIT_CODE=${PIPESTATUS[0]}

if [ $EXIT_CODE -eq 0 ]; then
    echo ""
    echo "=========================================="
    echo "Optimization completed successfully!"
    echo "Finished at: $(date)"
    echo "=========================================="
    echo ""
    
    # Find the latest output directory
    sleep 1
    LATEST_OUTPUT=$(ls -td batch/output/optimizer_* 2>/dev/null | head -1)
    
    if [ -n "$LATEST_OUTPUT" ]; then
        echo "Results written to: $LATEST_OUTPUT"
        echo ""
        
        # Generate visualizations
        echo "=========================================="
        echo "Generating Visualizations"
        echo "=========================================="
        echo ""
        
        VISUALIZER_SCRIPT="${SCRIPT_DIR}/visualize_portfolio.py"
        
        # Check if Python is available
        if ! command -v python3 &> /dev/null && ! command -v python &> /dev/null; then
            echo "  Warning: Python not found. Skipping visualization generation."
            echo "  Install Python 3 to enable automatic visualizations."
        elif [ ! -f "$VISUALIZER_SCRIPT" ]; then
            echo "  Warning: visualize_portfolio.py not found in scripts directory."
            echo "  Skipping visualization generation."
        else
            # Use python3 if available, otherwise python
            PYTHON_CMD="python3"
            if ! command -v python3 &> /dev/null; then
                PYTHON_CMD="python"
            fi
            
            # Check Python dependencies
            if ! $PYTHON_CMD -c "import pandas, matplotlib, seaborn, numpy" 2>/dev/null; then
                echo "  Warning: Python visualization dependencies not installed"
                echo "  Install with: pip install pandas matplotlib seaborn numpy"
                echo "  Skipping visualization generation"
            else
                echo "  Running visualization script..."
                VIZ_OUTPUT=$($PYTHON_CMD "$VISUALIZER_SCRIPT" "$LATEST_OUTPUT" 2>&1)
                VIZ_EXIT_CODE=$?
                
                # Filter out expected warnings but show important output
                echo "$VIZ_OUTPUT" | grep -v "Not found\|not available" | head -20
                
                if [ $VIZ_EXIT_CODE -eq 0 ]; then
                    echo ""
                    echo "  ✓ Visualizations generated successfully"
                    
                    # Show generated files
                    VIZ_DIR="${LATEST_OUTPUT}/visualizations"
                    if [ -d "$VIZ_DIR" ]; then
                        echo ""
                        echo "  Generated visualizations:"
                        ls -lh "$VIZ_DIR"/*.png 2>/dev/null | awk '{printf "    %-50s %8s\n", $9, $5}' || echo "    (No PNG files found)"
                        if [ -f "$VIZ_DIR/00_summary_report.txt" ]; then
                            echo ""
                            echo "  Summary report: $VIZ_DIR/00_summary_report.txt"
                        fi
                    fi
                else
                    echo "  Warning: Visualization generation had issues (check output above)"
                fi
            fi
        fi
        echo ""
    else
        echo "Results written to:"
        ls -lh portfolioStatistics_optimize_adjusted.csv 2>/dev/null || echo "  (No output file generated)"
    fi
    
    echo "=========================================="
    echo "Complete!"
    echo "=========================================="
    if [ -n "$LATEST_OUTPUT" ]; then
        echo ""
        echo "Output directory: $LATEST_OUTPUT"
        if [ -d "${LATEST_OUTPUT}/visualizations" ]; then
            echo "Visualizations: ${LATEST_OUTPUT}/visualizations"
        fi
    fi
else
    echo ""
    echo "=========================================="
    echo "Optimization FAILED with exit code: $EXIT_CODE"
    echo "Check $LOG_FILE for details"
    echo "=========================================="
    exit $EXIT_CODE
fi
