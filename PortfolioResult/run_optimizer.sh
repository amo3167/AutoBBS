#!/bin/bash

# Portfolio Optimizer Runner Script
# Usage: ./run_optimizer.sh <config_number> [start_date]
# Example: ./run_optimizer.sh 4 2018-01-01

set -e

CONFIG_NUM=${1:-4}
START_DATE=${2:-2018-01-01}

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

CONFIG_FILE="portfolioOptimize${CONFIG_NUM}.config"
PRESET_FILE="portfolioOptimizePreset${CONFIG_NUM}.config"
FACTOR_FILE="portfolioOptimizeFactor${CONFIG_NUM}.config"
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
    echo "Results written to:"
    ls -lh portfolioStatistics_optimize_adjusted.csv 2>/dev/null || echo "  (No output file generated)"
else
    echo ""
    echo "=========================================="
    echo "Optimization FAILED with exit code: $EXIT_CODE"
    echo "Check $LOG_FILE for details"
    echo "=========================================="
    exit $EXIT_CODE
fi
