#!/bin/bash

# Script to copy strategy results from ctester/tmp to PortfolioResult/batch
# Usage: ./copy_strategy_results.sh [strategy_folder]
#   e.g., ./copy_strategy_results.sh AUDUSD_200003
#   or    ./copy_strategy_results.sh (to copy all strategies)

set -e  # Exit on error

# Determine script directory and project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

CTESTER_TMP="$PROJECT_ROOT/ctester/tmp"
PORTFOLIO_BATCH="$PROJECT_ROOT/PortfolioResult/batch"

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Function to extract strategy ID from folder name
# Examples: AUDUSD_200003 -> 200003, GBPJPY_860006_optimize -> 860006
extract_strategy_id() {
    local folder_name="$1"
    # Remove _optimize suffix if present
    folder_name="${folder_name%_optimize}"
    # Extract the numeric part (strategy ID)
    echo "$folder_name" | grep -oE '[0-9]{6}$'
}

# Function to copy results for a single strategy
copy_strategy() {
    local strategy_folder="$1"
    local source_dir="$CTESTER_TMP/$strategy_folder"
    
    # Skip if not a directory
    if [ ! -d "$source_dir" ]; then
        echo -e "${YELLOW}Skipping $strategy_folder (not a directory)${NC}"
        return
    fi
    
    # Extract strategy ID
    local strategy_id=$(extract_strategy_id "$strategy_folder")
    
    if [ -z "$strategy_id" ]; then
        echo -e "${YELLOW}Skipping $strategy_folder (no strategy ID found)${NC}"
        return
    fi
    
    echo -e "${GREEN}Processing: $strategy_folder -> Strategy ID: $strategy_id${NC}"
    
    # Create batch directory if it doesn't exist
    mkdir -p "$PORTFOLIO_BATCH"
    
    local files_copied=0
    
    # Copy allStatistics_*.csv
    if [ -f "$source_dir/allStatistics_${strategy_id}.csv" ]; then
        cp "$source_dir/allStatistics_${strategy_id}.csv" "$PORTFOLIO_BATCH/"
        echo "  ✓ Copied allStatistics_${strategy_id}.csv"
        ((files_copied++))
    fi
    
    # Copy results.txt -> results_*.txt
    if [ -f "$source_dir/results.txt" ]; then
        cp "$source_dir/results.txt" "$PORTFOLIO_BATCH/results_${strategy_id}.txt"
        echo "  ✓ Copied results.txt -> results_${strategy_id}.txt"
        ((files_copied++))
    fi
    
    # Copy results.png -> results_*.png
    if [ -f "$source_dir/results.png" ]; then
        cp "$source_dir/results.png" "$PORTFOLIO_BATCH/results_${strategy_id}.png"
        echo "  ✓ Copied results.png -> results_${strategy_id}.png"
        ((files_copied++))
    fi
    
    if [ $files_copied -eq 0 ]; then
        echo -e "  ${YELLOW}⚠ No files found to copy${NC}"
    else
        echo -e "  ${GREEN}✓ Copied $files_copied file(s)${NC}"
    fi
    echo ""
}

# Main execution
echo "=================================="
echo "Strategy Results Copy Script"
echo "=================================="
echo ""

# Check if ctester/tmp exists
if [ ! -d "$CTESTER_TMP" ]; then
    echo -e "${RED}Error: ctester/tmp directory not found at $CTESTER_TMP${NC}"
    exit 1
fi

# Check if PortfolioResult directory exists
if [ ! -d "$PROJECT_ROOT/PortfolioResult" ]; then
    echo -e "${RED}Error: PortfolioResult directory not found${NC}"
    exit 1
fi

# If a specific strategy folder is provided as argument
if [ $# -eq 1 ]; then
    strategy_folder="$1"
    if [ ! -d "$CTESTER_TMP/$strategy_folder" ]; then
        echo -e "${RED}Error: Strategy folder not found: $strategy_folder${NC}"
        exit 1
    fi
    copy_strategy "$strategy_folder"
else
    # Process all strategy folders
    echo "Scanning for strategy folders in $CTESTER_TMP"
    echo ""
    
    total_processed=0
    
    # Loop through all directories in ctester/tmp
    for strategy_folder in "$CTESTER_TMP"/*; do
        if [ -d "$strategy_folder" ]; then
            folder_name=$(basename "$strategy_folder")
            # Skip folders without strategy IDs (e.g., analysis files)
            if [[ "$folder_name" =~ [0-9]{6} ]]; then
                copy_strategy "$folder_name"
                ((total_processed++))
            fi
        fi
    done
    
    echo "=================================="
    echo -e "${GREEN}✓ Processed $total_processed strategy folders${NC}"
    echo "Results copied to: $PORTFOLIO_BATCH"
fi

echo ""
echo "Done!"
