#!/bin/bash

# Cleanup script for batch directory
# Removes old timestamped CSV files and optionally old optimizer output directories
# Usage: ./cleanup_batch_files.sh [--keep-days N] [--clean-optimizer]

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
cd "$PROJECT_ROOT"

BATCH_DIR="batch"
KEEP_DAYS=7  # Keep files newer than 7 days by default
CLEAN_OPTIMIZER=false

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --keep-days)
            KEEP_DAYS="$2"
            shift 2
            ;;
        --clean-optimizer)
            CLEAN_OPTIMIZER=true
            shift
            ;;
        *)
            echo "Unknown option: $1"
            echo "Usage: $0 [--keep-days N] [--clean-optimizer]"
            exit 1
            ;;
    esac
done

echo "=========================================="
echo "Batch Directory Cleanup"
echo "=========================================="
echo "Batch directory: $BATCH_DIR"
echo "Keep files newer than: $KEEP_DAYS days"
echo "Clean optimizer directories: $CLEAN_OPTIMIZER"
echo "=========================================="
echo ""

# Clean up timestamped CSV files in batch root directory
echo "Cleaning up timestamped CSV files in batch directory..."
CLEANED=0

# Patterns for timestamped files (format: filename_YYYYMMDDHHMMSS.csv)
PATTERNS=(
    "portfolioStatistics_adjusted_*.csv"
    "portfolioResult_baseline_*.csv"
    "portfolioResult_adjusted_*.csv"
    "portfolioDailyReport_adjusted_*.csv"
    "portfolioWeeklyReport_adjusted_*.csv"
    "portfolioMonthlyReport_adjusted_*.csv"
)

for pattern in "${PATTERNS[@]}"; do
    # Find files matching pattern in batch root (not in output subdirectories)
    while IFS= read -r file; do
        if [ -f "$file" ]; then
            # Only remove files in batch root, not in output subdirectories
            if [[ "$file" == "${BATCH_DIR}/"* ]] && [[ "$file" != *"/output/"* ]]; then
                # Check file age if KEEP_DAYS is set
                if [ "$KEEP_DAYS" -gt 0 ]; then
                    # Get file modification time
                    if [ "$(uname)" = "Darwin" ]; then
                        # macOS
                        FILE_AGE=$(find "$file" -mtime +$KEEP_DAYS 2>/dev/null)
                    else
                        # Linux
                        FILE_AGE=$(find "$file" -mtime +$KEEP_DAYS 2>/dev/null)
                    fi
                    
                    if [ -n "$FILE_AGE" ]; then
                        rm -f "$file"
                        ((CLEANED++))
                        echo "  ✓ Removed: $(basename "$file")"
                    fi
                else
                    # Remove all matching files regardless of age
                    rm -f "$file"
                    ((CLEANED++))
                    echo "  ✓ Removed: $(basename "$file")"
                fi
            fi
        fi
    done < <(find "$BATCH_DIR" -maxdepth 1 -name "$pattern" -type f 2>/dev/null)
done

if [ $CLEANED -gt 0 ]; then
    echo ""
    echo "  Cleaned up $CLEANED timestamped CSV files"
else
    echo "  ✓ No timestamped CSV files to clean"
fi
echo ""

# Clean up old optimizer output directories
if [ "$CLEAN_OPTIMIZER" = true ]; then
    echo "Cleaning up old optimizer output directories..."
    OPTIMIZER_CLEANED=0
    
    # Find optimizer directories older than KEEP_DAYS
    while IFS= read -r dir; do
        if [ -d "$dir" ]; then
            if [ "$KEEP_DAYS" -gt 0 ]; then
                # Check directory age
                if [ "$(uname)" = "Darwin" ]; then
                    # macOS
                    DIR_AGE=$(find "$dir" -maxdepth 0 -mtime +$KEEP_DAYS 2>/dev/null)
                else
                    # Linux
                    DIR_AGE=$(find "$dir" -maxdepth 0 -mtime +$KEEP_DAYS 2>/dev/null)
                fi
                
                if [ -n "$DIR_AGE" ]; then
                    rm -rf "$dir"
                    ((OPTIMIZER_CLEANED++))
                    echo "  ✓ Removed: $(basename "$dir")"
                fi
            else
                # Remove all optimizer directories
                rm -rf "$dir"
                ((OPTIMIZER_CLEANED++))
                echo "  ✓ Removed: $(basename "$dir")"
            fi
        fi
    done < <(find "$BATCH_DIR/output" -maxdepth 1 -type d -name "optimizer_*" 2>/dev/null)
    
    if [ $OPTIMIZER_CLEANED -gt 0 ]; then
        echo ""
        echo "  Cleaned up $OPTIMIZER_CLEANED optimizer directories"
    else
        echo "  ✓ No old optimizer directories to clean"
    fi
    echo ""
fi

echo "=========================================="
echo "Cleanup completed"
echo "=========================================="

