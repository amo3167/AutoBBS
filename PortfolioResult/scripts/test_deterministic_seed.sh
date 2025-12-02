#!/bin/bash

# Test script to verify deterministic behavior with same random seed
# This script runs optimization twice with the same seed and compares results

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
cd "$PROJECT_ROOT"

JAR_FILE="target/PortfolioResult-1.0.0-jar-with-dependencies.jar"
TEST_SEED=12345

# Config files
CONFIG_DIR="portfolioRiskConfig"
OPTIMIZE_CONFIG="${CONFIG_DIR}/portfolioOptimize5.config"
PRESET_CONFIG="${CONFIG_DIR}/portfolioOptimizePreset5.config"
FACTOR_CONFIG="${CONFIG_DIR}/portfolioOptimizeFactor5.config"
START_DATE="2018-01-01"

echo "=================================="
echo "Deterministic Seed Test"
echo "=================================="
echo ""
echo "Test Configuration:"
echo "  Seed: $TEST_SEED"
echo "  Config: $OPTIMIZE_CONFIG"
echo "  Start Date: $START_DATE"
echo ""

# Check if JAR exists
if [ ! -f "$JAR_FILE" ]; then
    echo "ERROR: JAR file not found: $JAR_FILE"
    echo "Please build the project first: mvn clean package"
    exit 1
fi

# Run first optimization with seed
echo "Running Test 1 with seed $TEST_SEED..."
OUTPUT_DIR1="batch/output/test_deterministic_run1_$(date +%Y%m%d_%H%M%S)"
mkdir -p "$OUTPUT_DIR1"

java -Xmx4g -cp "$JAR_FILE" \
    PortfolioResult.PortfolioResult.App \
    optimizer \
    "$OPTIMIZE_CONFIG" \
    "$PRESET_CONFIG" \
    "$FACTOR_CONFIG" \
    "$START_DATE" \
    "$TEST_SEED" \
    > "$OUTPUT_DIR1/optimizer.log" 2>&1

# Wait for output directory to be created and find it
sleep 2
ACTUAL_OUTPUT1=$(ls -td batch/output/optimizer_* 2>/dev/null | head -1)
if [ -z "$ACTUAL_OUTPUT1" ]; then
    echo "ERROR: Could not find output directory for run 1"
    echo "Checking log for errors..."
    tail -20 "$OUTPUT_DIR1/optimizer.log"
    exit 1
fi

echo "  Run 1 completed. Output: $ACTUAL_OUTPUT1"
echo ""

# Wait a moment to ensure different timestamps
sleep 3

# Run second optimization with same seed
echo "Running Test 2 with seed $TEST_SEED..."
OUTPUT_DIR2="batch/output/test_deterministic_run2_$(date +%Y%m%d_%H%M%S)"
mkdir -p "$OUTPUT_DIR2"

java -Xmx4g -cp "$JAR_FILE" \
    PortfolioResult.PortfolioResult.App \
    optimizer \
    "$OPTIMIZE_CONFIG" \
    "$PRESET_CONFIG" \
    "$FACTOR_CONFIG" \
    "$START_DATE" \
    "$TEST_SEED" \
    > "$OUTPUT_DIR2/optimizer.log" 2>&1

# Wait and find the actual output directory
sleep 2
ACTUAL_OUTPUT2=$(ls -td batch/output/optimizer_* 2>/dev/null | head -1)
if [ -z "$ACTUAL_OUTPUT2" ]; then
    echo "ERROR: Could not find output directory for run 2"
    exit 1
fi

echo "  Run 2 completed. Output: $ACTUAL_OUTPUT2"
echo ""

# Compare results
echo "=================================="
echo "Comparing Results"
echo "=================================="
echo ""

# Compare best_portfolio.csv
echo "1. Comparing best_portfolio.csv..."
if diff -q "$ACTUAL_OUTPUT1/best_portfolio.csv" "$ACTUAL_OUTPUT2/best_portfolio.csv" > /dev/null 2>&1; then
    echo "   ✓ IDENTICAL - Best portfolio allocations match"
else
    echo "   ✗ DIFFERENT - Best portfolio allocations differ!"
    echo "   Run 1:"
    cat "$ACTUAL_OUTPUT1/best_portfolio.csv"
    echo "   Run 2:"
    cat "$ACTUAL_OUTPUT2/best_portfolio.csv"
    exit 1
fi

# Compare portfolio statistics
echo "2. Comparing portfolioStatistics_adjusted.csv..."
STATS1=$(find "$ACTUAL_OUTPUT1" -name "portfolioStatistics_adjusted_*.csv" | head -1)
STATS2=$(find "$ACTUAL_OUTPUT2" -name "portfolioStatistics_adjusted_*.csv" | head -1)

if [ -z "$STATS1" ] || [ -z "$STATS2" ]; then
    echo "   ⚠️  Statistics files not found"
else
    if diff -q "$STATS1" "$STATS2" > /dev/null 2>&1; then
        echo "   ✓ IDENTICAL - Statistics match"
    else
        echo "   ✗ DIFFERENT - Statistics differ!"
        echo "   Run 1:"
        head -3 "$STATS1"
        echo "   Run 2:"
        head -3 "$STATS2"
        exit 1
    fi
fi

# Compare optimization results count
echo "3. Comparing optimization results count..."
OPT1=$(find "$ACTUAL_OUTPUT1" -name "portfolioStatistics_optimize_adjusted*.csv" | head -1)
OPT2=$(find "$ACTUAL_OUTPUT2" -name "portfolioStatistics_optimize_adjusted*.csv" | head -1)

if [ -z "$OPT1" ] || [ -z "$OPT2" ]; then
    echo "   ⚠️  Optimization results files not found"
else
    COUNT1=$(wc -l < "$OPT1")
    COUNT2=$(wc -l < "$OPT2")
    
    if [ "$COUNT1" -eq "$COUNT2" ]; then
        echo "   ✓ IDENTICAL - Same number of results ($COUNT1 combinations)"
    else
        echo "   ✗ DIFFERENT - Different number of results (Run 1: $COUNT1, Run 2: $COUNT2)"
        exit 1
    fi
    
    # Compare first few results
    if diff -q <(head -10 "$OPT1") <(head -10 "$OPT2") > /dev/null 2>&1; then
        echo "   ✓ IDENTICAL - Top results match"
    else
        echo "   ✗ DIFFERENT - Top results differ!"
        exit 1
    fi
fi

echo ""
echo "=================================="
echo "TEST RESULT: ✓ PASSED"
echo "=================================="
echo ""
echo "Both runs with seed $TEST_SEED produced IDENTICAL results."
echo "This confirms deterministic behavior is working correctly."
echo ""
echo "Output directories:"
echo "  Run 1: $ACTUAL_OUTPUT1"
echo "  Run 2: $ACTUAL_OUTPUT2"
echo ""

