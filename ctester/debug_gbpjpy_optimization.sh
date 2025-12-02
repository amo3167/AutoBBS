#!/bin/bash
# Debugger script for GBPJPY optimization hang
# Usage: ./debug_gbpjpy_optimization.sh

cd "$(dirname "$0")"

echo "=== Debugging GBPJPY 860006 Optimization ==="
echo ""

# Clean up
pkill -9 -f "asirikuy_strategy_tester.*860006" 2>/dev/null || true
rm -f debug_optimization.txt

# Get absolute paths
SCRIPT_DIR="$(pwd)"
PROJECT_ROOT="$(cd .. && pwd)"
SRC_FILE="$PROJECT_ROOT/core/CTesterFrameworkAPI/src/optimizer.c"

echo "Project root: $PROJECT_ROOT"
echo "Source file: $SRC_FILE"
echo ""

# Create lldb init file with breakpoints
cat > /tmp/lldb_init.txt << EOF
# Set breakpoints at critical locations
# Function entry point
breakpoint set --name runOptimizationMultipleSymbols

# Source file breakpoints (lldb will find the file from debug symbols)
breakpoint set --file "$SRC_FILE" --line 335
breakpoint set --file "$SRC_FILE" --line 400
breakpoint set --file "$SRC_FILE" --line 440
breakpoint set --file "$SRC_FILE" --line 750

# Function that might be called
breakpoint set --name runPortfolioTest

# Show breakpoints
breakpoint list

echo ""
echo "=== Starting execution ==="
echo "Breakpoints are set. The program will stop at each breakpoint."
echo "Use 'continue' to proceed, 'bt' for backtrace, 'print <var>' to inspect."
echo ""

run
EOF

echo "Starting lldb session..."
echo "Breakpoints will be set at:"
echo "  1. runOptimizationMultipleSymbols (function entry)"
echo "  2. optimizer.c:335 (first log statement)"
echo "  3. optimizer.c:400 (brute force parameter generation)"
echo "  4. optimizer.c:440 (iteration loop start)"
echo "  5. optimizer.c:750 (runPortfolioTest call)"
echo "  6. runPortfolioTest (function entry)"
echo ""
echo "When breakpoint hits:"
echo "  - Backtrace will show where execution stopped"
echo "  - Use 'continue' to proceed"
echo "  - Use 'print <variable>' to inspect values"
echo "  - Use 'frame select <N>' to examine different stack frames"
echo ""

# Get actual Python executable
PYTHON_EXE=$(python3 -c "import sys; print(sys.executable)")

echo "Using Python: $PYTHON_EXE"
echo ""

# Run lldb with init file
lldb --source /tmp/lldb_init.txt -- "$PYTHON_EXE" asirikuy_strategy_tester.py -c config/Peso_MACD_GBPJPY-1H_860006_optimize.config -ot results -oo optimization_results
