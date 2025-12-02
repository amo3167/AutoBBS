#!/bin/bash
# Script to run XAUUSD MACD 860007 Strategy
# Cleans ctester folder and saves results to tmp folder with renamed files
#
# Usage: ./run_xauusd_macd_860007.sh [OPTIONS]
# Options:
#   --fromdate YYYY-MM-DD    Start date (default: from config file)
#   --todate YYYY-MM-DD      End date (default: from config file)
#   --logseverity LEVEL      Log level 0-7 (default: from config file)
#   -h, --help               Show this help message

set -e

# Configuration
CONFIG_FILE="config/Peso_MACD_XAUUSD-1H_860007.config"
SYMBOL="XAUUSD"
STRATEGY_ID="860007"
OUTPUT_DIR="tmp"
RESULTS_FOLDER="${OUTPUT_DIR}/${SYMBOL}_${STRATEGY_ID}"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TEMP_CONFIG=""

# Default values (will be read from config if not provided)
FROMDATE=""
TODATE=""
LOGSEVERITY=""

# Parse command line arguments
while [[ $# -gt 0 ]]; do
  case $1 in
    --fromdate)
      FROMDATE="$2"
      shift 2
      ;;
    --todate)
      TODATE="$2"
      shift 2
      ;;
    --logseverity)
      LOGSEVERITY="$2"
      shift 2
      ;;
    -h|--help)
      echo "Usage: $0 [OPTIONS]"
      echo ""
      echo "Options:"
      echo "  --fromdate YYYY-MM-DD    Start date (e.g., 2020-01-02)"
      echo "  --todate YYYY-MM-DD      End date (e.g., 2021-01-01)"
      echo "  --logseverity LEVEL      Log level 0-7 (0=Emergency, 4=Warning, 7=Debug)"
      echo "  -h, --help               Show this help message"
      echo ""
      echo "Examples:"
      echo "  $0 --fromdate 2020-01-02 --todate 2021-01-01 --logseverity 4"
      echo "  $0 --fromdate 2019-01-01 --todate 2020-12-31"
      exit 0
      ;;
    *)
      echo "Unknown option: $1"
      echo "Use -h or --help for usage information"
      exit 1
      ;;
  esac
done

# Change to script directory
cd "$SCRIPT_DIR/.."

# Extract STRATEGY_INSTANCE_ID from set file (single source of truth)
# First, get the set file path from config
SET_FILE=$(grep "^set = " "$CONFIG_FILE" | sed 's/^set = //' | sed 's/^\.\///')
if [ -z "$SET_FILE" ]; then
    # Try setFile instead of set
    SET_FILE=$(grep "^setFile = " "$CONFIG_FILE" | sed 's/^setFile = //' | sed 's/^\.\///')
fi

# If set file path is relative, make it absolute
if [ -n "$SET_FILE" ] && [[ ! "$SET_FILE" = /* ]]; then
    SET_FILE="$(pwd)/$SET_FILE"
fi

# Extract STRATEGY_INSTANCE_ID from set file
if [ -f "$SET_FILE" ]; then
    EXTRACTED_INSTANCE_ID=$(grep "^STRATEGY_INSTANCE_ID=" "$SET_FILE" | sed 's/^STRATEGY_INSTANCE_ID=//' | tr -d '\r\n' | head -1)
    if [ -n "$EXTRACTED_INSTANCE_ID" ]; then
        STRATEGY_ID="$EXTRACTED_INSTANCE_ID"
        echo "✓ Extracted STRATEGY_INSTANCE_ID=${STRATEGY_ID} from set file: $SET_FILE"
    else
        echo "⚠️  WARNING: Could not find STRATEGY_INSTANCE_ID in set file: $SET_FILE"
        echo "  Using hardcoded STRATEGY_ID=${STRATEGY_ID} as fallback"
    fi
else
    echo "⚠️  WARNING: Set file not found: $SET_FILE"
    echo "  Using hardcoded STRATEGY_ID=${STRATEGY_ID} as fallback"
fi

# Update RESULTS_FOLDER with the extracted/fallback ID
# Check if this is an optimization script (has "_optimize" in script name)
if [[ "$(basename "$0")" == *"_optimize"* ]]; then
    RESULTS_FOLDER="${OUTPUT_DIR}/${SYMBOL}_${STRATEGY_ID}_optimize"
else
    RESULTS_FOLDER="${OUTPUT_DIR}/${SYMBOL}_${STRATEGY_ID}"
fi

echo "=== Running ${SYMBOL} MACD Strategy ${STRATEGY_ID} ==="
echo ""

# Step 1: Clean and create results folder (needed for temp config)
echo "Step 1: Cleaning and creating results folder..."
if [ -d "$RESULTS_FOLDER" ]; then
    rm -rf "$RESULTS_FOLDER"
    echo "✓ Cleaned existing folder: ${RESULTS_FOLDER}/"
fi
mkdir -p "$RESULTS_FOLDER"
echo "✓ Created folder: ${RESULTS_FOLDER}/"
echo ""

# Step 1.5: Clean up any .state files in tmp folder (from previous runs)
echo "Step 1.5: Cleaning up state files in tmp folder..."
rm -f "${OUTPUT_DIR}"/*.state 2>/dev/null
if [ $? -eq 0 ]; then
    echo "✓ Cleaned up state files"
fi
echo ""

# Step 2: Create temporary config file if parameters are provided
if [ -n "$FROMDATE" ] || [ -n "$TODATE" ] || [ -n "$LOGSEVERITY" ]; then
    echo "Step 2: Creating temporary config file with custom parameters..."
    TEMP_CONFIG="${RESULTS_FOLDER}/temp_config_${STRATEGY_ID}_$$.config"
    cp "$CONFIG_FILE" "$TEMP_CONFIG"
    
    # Update fromdate if provided
    if [ -n "$FROMDATE" ]; then
        if [[ "$OSTYPE" == "darwin"* ]]; then
            # macOS sed
            sed -i '' "s/^fromdate = .*/fromdate = ${FROMDATE}/" "$TEMP_CONFIG"
        else
            # Linux sed
            sed -i "s/^fromdate = .*/fromdate = ${FROMDATE}/" "$TEMP_CONFIG"
        fi
        echo "  ✓ Set fromdate = ${FROMDATE}"
    fi
    
    # Update todate if provided
    if [ -n "$TODATE" ]; then
        if [[ "$OSTYPE" == "darwin"* ]]; then
            sed -i '' "s/^todate = .*/todate = ${TODATE}/" "$TEMP_CONFIG"
        else
            sed -i "s/^todate = .*/todate = ${TODATE}/" "$TEMP_CONFIG"
        fi
        echo "  ✓ Set todate = ${TODATE}"
    fi
    
    # Update logseverity if provided
    if [ -n "$LOGSEVERITY" ]; then
        if [[ "$OSTYPE" == "darwin"* ]]; then
            sed -i '' "s/^logseverity = .*/logseverity = ${LOGSEVERITY}/" "$TEMP_CONFIG"
        else
            sed -i "s/^logseverity = .*/logseverity = ${LOGSEVERITY}/" "$TEMP_CONFIG"
        fi
        echo "  ✓ Set logseverity = ${LOGSEVERITY}"
    fi
    
    CONFIG_FILE="$TEMP_CONFIG"
    echo "✓ Created temporary config: $TEMP_CONFIG"
    echo ""
fi

# Step 3: Clean ctester folder (remove old output files)
echo "Step 3: Cleaning ctester folder..."
rm -f results.* test_results.* allStatistics.csv
echo "✓ Cleaned old output files"
echo ""

# Step 4: Run the backtest
echo "Step 4: Running backtest..."
python3 asirikuy_strategy_tester.py -c "$CONFIG_FILE" -ot results 2>&1 | tee "${RESULTS_FOLDER}/backtest_${STRATEGY_ID}.log"

# Step 5: Rename and move files to results folder
echo ""
echo "Step 5: Renaming and moving results to ${RESULTS_FOLDER}/..."

if [ -f "results.txt" ]; then
    mv "results.txt" "${RESULTS_FOLDER}/results_${STRATEGY_ID}.txt"
    echo "✓ Moved results.txt -> ${RESULTS_FOLDER}/results_${STRATEGY_ID}.txt"
fi

if [ -f "results.png" ]; then
    mv "results.png" "${RESULTS_FOLDER}/results_${STRATEGY_ID}.png"
    echo "✓ Moved results.png -> ${RESULTS_FOLDER}/results_${STRATEGY_ID}.png"
fi

if [ -f "results.open" ]; then
    mv "results.open" "${RESULTS_FOLDER}/results_${STRATEGY_ID}.open"
    echo "✓ Moved results.open -> ${RESULTS_FOLDER}/results_${STRATEGY_ID}.open"
fi

if [ -f "results.set" ]; then
    mv "results.set" "${RESULTS_FOLDER}/results_${STRATEGY_ID}.set"
    echo "✓ Moved results.set -> ${RESULTS_FOLDER}/results_${STRATEGY_ID}.set"
fi

if [ -f "allStatistics.csv" ]; then
    mv "allStatistics.csv" "${RESULTS_FOLDER}/allStatistics_${STRATEGY_ID}.csv"
    echo "✓ Moved allStatistics.csv -> ${RESULTS_FOLDER}/allStatistics_${STRATEGY_ID}.csv"
fi

if [ -f "results.xml" ]; then
    mv "results.xml" "${RESULTS_FOLDER}/results_${STRATEGY_ID}.xml"
    echo "✓ Moved results.xml -> ${RESULTS_FOLDER}/results_${STRATEGY_ID}.xml"
fi

# Step 6: Keep temporary config file for reference (if created)
if [ -n "$TEMP_CONFIG" ] && [ -f "$TEMP_CONFIG" ]; then
    # Rename temp config to a more descriptive name
    FINAL_CONFIG_NAME="${RESULTS_FOLDER}/config_${STRATEGY_ID}.config"
    mv "$TEMP_CONFIG" "$FINAL_CONFIG_NAME"
    echo "✓ Saved config file: $FINAL_CONFIG_NAME"
fi

echo ""
echo "=== Backtest Complete ==="
echo "Results saved to: ${RESULTS_FOLDER}/"
ls -lh "${RESULTS_FOLDER}/" 2>/dev/null || true

