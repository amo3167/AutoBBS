#!/bin/bash

# Regression Test Suite Runner
# Creates baseline test data and validates current results against it
# Usage: ./run_regression_tests.sh [--create-baseline] [--test]

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
cd "$PROJECT_ROOT"

TEST_DATA_DIR="test-data"
BASELINE_DIR="${TEST_DATA_DIR}/baseline"
CURRENT_DIR="${TEST_DATA_DIR}/current"
JAR_FILE="target/PortfolioResult-1.0.0-jar-with-dependencies.jar"
TOLERANCE=0.01  # 0.01% tolerance for floating point comparisons

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Parse arguments
CREATE_BASELINE=false
RUN_TESTS=false

if [[ "$1" == "--create-baseline" ]] || [[ "$1" == "--create" ]]; then
    CREATE_BASELINE=true
fi

if [[ "$1" == "--test" ]] || [[ "$2" == "--test" ]] || [[ "$1" == "" ]]; then
    RUN_TESTS=true
fi

echo "=========================================="
echo "Regression Test Suite"
echo "=========================================="
echo "Test Data Directory: $TEST_DATA_DIR"
echo "Baseline Directory: $BASELINE_DIR"
echo "Current Results: $CURRENT_DIR"
echo "Tolerance: ${TOLERANCE}%"
echo "=========================================="
echo ""

# Check if JAR exists
if [ ! -f "$JAR_FILE" ]; then
    echo -e "${RED}Error: JAR file not found. Building project...${NC}"
    mvn clean package -DskipTests
fi

# Create directories
mkdir -p "$BASELINE_DIR"
mkdir -p "$CURRENT_DIR"

if [ "$CREATE_BASELINE" = true ]; then
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${BLUE}Creating Baseline Test Data${NC}"
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo ""
    
    BASELINE_OUTPUT="${BASELINE_DIR}/output"
    mkdir -p "$BASELINE_OUTPUT"
    
    # Test Case 1: Basic Portfolio (5 strategies)
    echo -e "${YELLOW}Test Case 1: Basic Portfolio (portfoliorisk5.config)${NC}"
    echo "Running portfolio simulation..."
    
    java -Xmx4g -cp "$JAR_FILE" \
        PortfolioResult.PortfolioResult.App \
        run \
        "portfolioRiskConfig/portfoliorisk5.config" \
        false \
        "2018-01-01" > "$BASELINE_OUTPUT/portfolio_test1.log" 2>&1
    
    # Wait for files to be written
    sleep 2
    
    # Copy results (check for both timestamped and non-timestamped versions)
    if [ -f "batch/portfolioStatistics_adjusted.csv" ]; then
        cp "batch/portfolioStatistics_adjusted.csv" "$BASELINE_DIR/baseline_portfolio_stats.csv"
        echo -e "${GREEN}  ✓ Saved: baseline_portfolio_stats.csv${NC}"
    else
        # Try to find timestamped version
        STATS_FILE=$(ls -t batch/portfolioStatistics_adjusted_*.csv 2>/dev/null | head -1)
        if [ -n "$STATS_FILE" ]; then
            cp "$STATS_FILE" "$BASELINE_DIR/baseline_portfolio_stats.csv"
            echo -e "${GREEN}  ✓ Saved: baseline_portfolio_stats.csv${NC}"
        else
            echo -e "${RED}  ✗ Error: portfolioStatistics_adjusted.csv not found${NC}"
            echo "  Check log: $BASELINE_OUTPUT/portfolio_test1.log"
        fi
    fi
    
    # Test Case 2: Optimization (5 strategies)
    echo ""
    echo -e "${YELLOW}Test Case 2: Optimization (portfolioOptimize5.config)${NC}"
    echo "Running optimization..."
    
    java -Xmx4g -cp "$JAR_FILE" \
        PortfolioResult.PortfolioResult.App \
        optimizer \
        "portfolioRiskConfig/portfolioOptimize5.config" \
        "portfolioRiskConfig/portfolioOptimizePreset5.config" \
        "portfolioRiskConfig/portfolioOptimizeFactor5.config" \
        "2018-01-01" \
        > "$BASELINE_OUTPUT/optimization_test1.log" 2>&1
    
    # Find and copy optimization results
    sleep 1
    OPT_OUTPUT=$(ls -td batch/output/optimizer_* 2>/dev/null | head -1)
    if [ -n "$OPT_OUTPUT" ]; then
        if [ -f "$OPT_OUTPUT/portfolioStatistics_adjusted.csv" ]; then
            cp "$OPT_OUTPUT/portfolioStatistics_adjusted.csv" "$BASELINE_DIR/baseline_optimization_results.csv"
            echo -e "${GREEN}  ✓ Saved: baseline_optimization_results.csv${NC}"
        else
            # Try to find timestamped version
            STATS_FILE=$(find "$OPT_OUTPUT" -name "portfolioStatistics_adjusted_*.csv" | head -1)
            if [ -n "$STATS_FILE" ]; then
                cp "$STATS_FILE" "$BASELINE_DIR/baseline_optimization_results.csv"
                echo -e "${GREEN}  ✓ Saved: baseline_optimization_results.csv${NC}"
            else
                echo -e "${RED}  ✗ Error: Optimization statistics file not found${NC}"
            fi
        fi
    else
        echo -e "${RED}  ✗ Error: Optimization output directory not found${NC}"
    fi
    
    echo ""
    echo -e "${GREEN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${GREEN}Baseline Created Successfully!${NC}"
    echo -e "${GREEN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo ""
    echo "Baseline files:"
    ls -lh "$BASELINE_DIR"/*.csv 2>/dev/null | awk '{print "  " $9, "(" $5 ")"}'
    echo ""
    
elif [ "$RUN_TESTS" = true ]; then
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${BLUE}Running Regression Tests${NC}"
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo ""
    
    # Check if baseline exists
    if [ ! -f "$BASELINE_DIR/baseline_portfolio_stats.csv" ] || [ ! -f "$BASELINE_DIR/baseline_optimization_results.csv" ]; then
        echo -e "${RED}Error: Baseline files not found!${NC}"
        echo "Run with --create-baseline first:"
        echo "  ./scripts/run_regression_tests.sh --create-baseline"
        exit 1
    fi
    
    CURRENT_OUTPUT="${CURRENT_DIR}/output"
    mkdir -p "$CURRENT_OUTPUT"
    
    # Test Case 1: Basic Portfolio
    echo -e "${YELLOW}Test Case 1: Basic Portfolio${NC}"
    echo "Running portfolio simulation..."
    
    java -Xmx4g -cp "$JAR_FILE" \
        PortfolioResult.PortfolioResult.App \
        run \
        "portfolioRiskConfig/portfoliorisk5.config" \
        false \
        "2018-01-01" > "$CURRENT_OUTPUT/portfolio_test1.log" 2>&1
    
    # Wait for files to be written
    sleep 2
    
    # Copy results (check for both timestamped and non-timestamped versions)
    if [ -f "batch/portfolioStatistics_adjusted.csv" ]; then
        cp "batch/portfolioStatistics_adjusted.csv" "$CURRENT_DIR/current_portfolio_stats.csv"
        echo -e "${GREEN}  ✓ Generated: current_portfolio_stats.csv${NC}"
    else
        # Try to find timestamped version
        STATS_FILE=$(ls -t batch/portfolioStatistics_adjusted_*.csv 2>/dev/null | head -1)
        if [ -n "$STATS_FILE" ]; then
            cp "$STATS_FILE" "$CURRENT_DIR/current_portfolio_stats.csv"
            echo -e "${GREEN}  ✓ Generated: current_portfolio_stats.csv${NC}"
        else
            echo -e "${RED}  ✗ Error: portfolioStatistics_adjusted.csv not found${NC}"
            echo "  Check log: $CURRENT_OUTPUT/portfolio_test1.log"
            exit 1
        fi
    fi
    
    # Test Case 2: Optimization
    echo ""
    echo -e "${YELLOW}Test Case 2: Optimization${NC}"
    echo "Running optimization..."
    
    java -Xmx4g -cp "$JAR_FILE" \
        PortfolioResult.PortfolioResult.App \
        optimizer \
        "portfolioRiskConfig/portfolioOptimize5.config" \
        "portfolioRiskConfig/portfolioOptimizePreset5.config" \
        "portfolioRiskConfig/portfolioOptimizeFactor5.config" \
        "2018-01-01" \
        > "$CURRENT_OUTPUT/optimization_test1.log" 2>&1
    
    sleep 1
    OPT_OUTPUT=$(ls -td batch/output/optimizer_* 2>/dev/null | head -1)
    if [ -n "$OPT_OUTPUT" ]; then
        STATS_FILE=$(find "$OPT_OUTPUT" -name "portfolioStatistics_adjusted*.csv" | head -1)
        if [ -n "$STATS_FILE" ]; then
            cp "$STATS_FILE" "$CURRENT_DIR/current_optimization_results.csv"
            echo -e "${GREEN}  ✓ Generated: current_optimization_results.csv${NC}"
        else
            echo -e "${RED}  ✗ Error: Optimization statistics file not found${NC}"
            exit 1
        fi
    else
        echo -e "${RED}  ✗ Error: Optimization output directory not found${NC}"
        exit 1
    fi
    
    echo ""
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${BLUE}Comparing Results${NC}"
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo ""
    
    # Run comparison script
    if [ -f "${SCRIPT_DIR}/compare_results.py" ]; then
        python3 "${SCRIPT_DIR}/compare_results.py" \
            "$BASELINE_DIR/baseline_portfolio_stats.csv" \
            "$CURRENT_DIR/current_portfolio_stats.csv" \
            "$BASELINE_DIR/baseline_optimization_results.csv" \
            "$CURRENT_DIR/current_optimization_results.csv" \
            "$TOLERANCE"
        
        EXIT_CODE=$?
        
        if [ $EXIT_CODE -eq 0 ]; then
            echo ""
            echo -e "${GREEN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
            echo -e "${GREEN}✓ All Regression Tests PASSED${NC}"
            echo -e "${GREEN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
            exit 0
        else
            echo ""
            echo -e "${RED}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
            echo -e "${RED}✗ Regression Tests FAILED${NC}"
            echo -e "${RED}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
            exit 1
        fi
    else
        echo -e "${YELLOW}Warning: compare_results.py not found. Skipping automated comparison.${NC}"
        echo "Baseline files:"
        ls -lh "$BASELINE_DIR"/*.csv 2>/dev/null
        echo ""
        echo "Current files:"
        ls -lh "$CURRENT_DIR"/*.csv 2>/dev/null
        echo ""
        echo "Manual comparison required."
    fi
else
    echo "Usage: $0 [--create-baseline|--test]"
    echo ""
    echo "Options:"
    echo "  --create-baseline  Create baseline test data (golden dataset)"
    echo "  --test             Run regression tests against baseline (default)"
    echo ""
    echo "Examples:"
    echo "  $0 --create-baseline    # Create baseline"
    echo "  $0 --test               # Run tests"
    echo "  $0                      # Run tests (default)"
fi

