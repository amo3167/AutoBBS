#!/bin/bash
#
# Portfolio Optimizer with Automatic Visualization
#
# This script runs the portfolio optimizer and automatically generates
# visualizations of the results.
#
# Usage:
#   ./run_optimizer_with_viz.sh <config_number> <start_date>
#
# Examples:
#   ./run_optimizer_with_viz.sh 4 2018-01-01
#   ./run_optimizer_with_viz.sh 4 2018-01-01 --skip-viz    # Skip visualization
#

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
MAGENTA='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
OUTPUT_BASE_DIR="${PROJECT_ROOT}/batch/output"
VISUALIZER_SCRIPT="${SCRIPT_DIR}/visualize_portfolio.py"

# Parse arguments
CONFIG_NUM="$1"
START_DATE="$2"
SKIP_VIZ=false

# Check for --skip-viz flag
if [[ "$3" == "--skip-viz" ]] || [[ "$2" == "--skip-viz" ]]; then
    SKIP_VIZ=true
fi

# Help message
if [[ -z "$CONFIG_NUM" ]] || [[ "$CONFIG_NUM" == "-h" ]] || [[ "$CONFIG_NUM" == "--help" ]]; then
    echo -e "${CYAN}Portfolio Optimizer with Auto-Visualization${NC}"
    echo ""
    echo "Usage: $0 <config_number> <start_date> [--skip-viz]"
    echo ""
    echo "Arguments:"
    echo "  config_number    Configuration file number (e.g., 4 for portfolioOptimize4.config)"
    echo "  start_date       Start date for optimization (format: YYYY-MM-DD)"
    echo "  --skip-viz       Skip automatic visualization (optional)"
    echo ""
    echo "Examples:"
    echo "  $0 4 2018-01-01"
    echo "  $0 4 2018-01-01 --skip-viz"
    echo ""
    exit 0
fi

# Validate arguments
if [[ -z "$START_DATE" ]] && [[ "$SKIP_VIZ" == false ]]; then
    echo -e "${RED}Error: Missing start_date argument${NC}"
    echo "Usage: $0 <config_number> <start_date> [--skip-viz]"
    exit 1
fi

# Print header
echo ""
echo -e "${MAGENTA}═══════════════════════════════════════════════════════════════════════${NC}"
echo -e "${MAGENTA}        Portfolio Optimizer with Auto-Visualization${NC}"
echo -e "${MAGENTA}═══════════════════════════════════════════════════════════════════════${NC}"
echo ""
echo -e "${CYAN}Configuration:${NC}"
echo -e "  Config Number: ${GREEN}${CONFIG_NUM}${NC}"
echo -e "  Start Date:    ${GREEN}${START_DATE}${NC}"
echo -e "  Skip Viz:      ${GREEN}${SKIP_VIZ}${NC}"
echo ""

# Step 1: Run the optimizer
echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo -e "${BLUE}Step 1: Running Portfolio Optimizer${NC}"
echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo ""

# Check if run_optimizer.sh exists
if [[ ! -f "${SCRIPT_DIR}/run_optimizer.sh" ]]; then
    echo -e "${RED}Error: run_optimizer.sh not found in ${SCRIPT_DIR}${NC}"
    exit 1
fi

# Run optimizer and capture output
OPTIMIZER_START=$(date +%s)
echo -e "${YELLOW}Running: ${SCRIPT_DIR}/run_optimizer.sh ${CONFIG_NUM} ${START_DATE}${NC}"
echo ""

cd "${PROJECT_ROOT}"
if "${SCRIPT_DIR}/run_optimizer.sh" "${CONFIG_NUM}" "${START_DATE}"; then
    OPTIMIZER_END=$(date +%s)
    OPTIMIZER_DURATION=$((OPTIMIZER_END - OPTIMIZER_START))
    echo ""
    echo -e "${GREEN}✓ Optimizer completed successfully in ${OPTIMIZER_DURATION} seconds${NC}"
else
    echo -e "${RED}✗ Optimizer failed${NC}"
    exit 1
fi

# Step 2: Find the latest output directory
echo ""
echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo -e "${BLUE}Step 2: Locating Output Directory${NC}"
echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo ""

# Find the most recent optimizer output directory
LATEST_OUTPUT=$(ls -td "${OUTPUT_BASE_DIR}"/optimizer_* 2>/dev/null | head -1)

if [[ -z "$LATEST_OUTPUT" ]]; then
    echo -e "${RED}✗ No optimizer output directory found in ${OUTPUT_BASE_DIR}${NC}"
    exit 1
fi

echo -e "${GREEN}✓ Found output directory:${NC}"
echo -e "  ${CYAN}${LATEST_OUTPUT}${NC}"
echo ""

# List files in output directory
echo -e "${YELLOW}Output files:${NC}"
ls -lh "${LATEST_OUTPUT}" | tail -n +2 | awk '{printf "  %-40s %8s\n", $9, $5}'
echo ""

# Step 3: Generate visualizations
if [[ "$SKIP_VIZ" == true ]]; then
    echo -e "${YELLOW}⊘ Skipping visualization (--skip-viz flag set)${NC}"
    echo ""
else
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${BLUE}Step 3: Generating Visualizations${NC}"
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo ""
    
    # Check if Python is available
    if ! command -v python3 &> /dev/null && ! command -v python &> /dev/null; then
        echo -e "${RED}✗ Python not found. Please install Python 3.${NC}"
        exit 1
    fi
    
    # Use python3 if available, otherwise python
    PYTHON_CMD="python3"
    if ! command -v python3 &> /dev/null; then
        PYTHON_CMD="python"
    fi
    
    # Check if visualizer script exists
    if [[ ! -f "$VISUALIZER_SCRIPT" ]]; then
        echo -e "${RED}✗ Visualizer script not found: ${VISUALIZER_SCRIPT}${NC}"
        echo -e "${YELLOW}Please ensure visualize_portfolio.py is in the scripts directory${NC}"
        exit 1
    fi
    
    # Check Python dependencies
    echo -e "${YELLOW}Checking Python dependencies...${NC}"
    if ! $PYTHON_CMD -c "import pandas, matplotlib, seaborn, numpy" 2>/dev/null; then
        echo -e "${YELLOW}⚠ Missing Python dependencies. Installing...${NC}"
        echo ""
        if [[ -f "${SCRIPT_DIR}/requirements.txt" ]]; then
            $PYTHON_CMD -m pip install -q -r "${SCRIPT_DIR}/requirements.txt"
            echo -e "${GREEN}✓ Dependencies installed${NC}"
        else
            echo -e "${RED}✗ requirements.txt not found${NC}"
            echo -e "${YELLOW}Please run: pip install pandas matplotlib seaborn numpy${NC}"
            exit 1
        fi
    else
        echo -e "${GREEN}✓ All dependencies available${NC}"
    fi
    echo ""
    
    # Run visualizer
    VIZ_START=$(date +%s)
    echo -e "${YELLOW}Running: ${PYTHON_CMD} ${VISUALIZER_SCRIPT} ${LATEST_OUTPUT}${NC}"
    echo ""
    
    if $PYTHON_CMD "${VISUALIZER_SCRIPT}" "${LATEST_OUTPUT}"; then
        VIZ_END=$(date +%s)
        VIZ_DURATION=$((VIZ_END - VIZ_START))
        echo ""
        echo -e "${GREEN}✓ Visualizations generated successfully in ${VIZ_DURATION} seconds${NC}"
        
        # Show generated files
        VIZ_DIR="${LATEST_OUTPUT}/visualizations"
        if [[ -d "$VIZ_DIR" ]]; then
            echo ""
            echo -e "${GREEN}Generated visualizations:${NC}"
            ls -lh "${VIZ_DIR}" | tail -n +2 | awk '{printf "  %-50s %8s\n", $9, $5}'
        fi
    else
        echo -e "${RED}✗ Visualization failed${NC}"
        exit 1
    fi
fi

# Step 4: Summary
echo ""
echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo -e "${BLUE}Summary${NC}"
echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo ""

# Display summary report if available
SUMMARY_FILE="${LATEST_OUTPUT}/visualizations/00_summary_report.txt"
if [[ -f "$SUMMARY_FILE" ]]; then
    echo -e "${CYAN}Portfolio Performance Summary:${NC}"
    echo -e "${YELLOW}────────────────────────────────────────────────────────────────────${NC}"
    cat "$SUMMARY_FILE"
    echo -e "${YELLOW}────────────────────────────────────────────────────────────────────${NC}"
else
    echo -e "${YELLOW}Summary report not available${NC}"
fi

echo ""
echo -e "${GREEN}═══════════════════════════════════════════════════════════════════════${NC}"
echo -e "${GREEN}All operations completed successfully!${NC}"
echo -e "${GREEN}═══════════════════════════════════════════════════════════════════════${NC}"
echo ""
echo -e "${CYAN}Output Location:${NC}"
echo -e "  ${LATEST_OUTPUT}"
echo ""

if [[ "$SKIP_VIZ" == false ]]; then
    echo -e "${CYAN}View Visualizations:${NC}"
    echo -e "  cd ${LATEST_OUTPUT}/visualizations"
    echo -e "  open *.png  # macOS"
    echo ""
fi

echo -e "${CYAN}Quick Access:${NC}"
echo -e "  Latest output:      ${LATEST_OUTPUT}"
if [[ "$SKIP_VIZ" == false ]]; then
    echo -e "  Visualizations:     ${LATEST_OUTPUT}/visualizations"
    echo -e "  Summary report:     ${LATEST_OUTPUT}/visualizations/00_summary_report.txt"
fi
echo -e "  Best portfolio:     ${LATEST_OUTPUT}/best_portfolio.csv"
echo ""
