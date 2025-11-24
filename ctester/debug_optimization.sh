#!/bin/bash
# Debugger script for GBPJPY optimization hang
# Usage: ./debug_optimization.sh

set -e

cd "$(dirname "$0")"

echo "=== Setting up debugger session for GBPJPY optimization ==="
echo ""

# Clean up any existing processes
pkill -9 -f "asirikuy_strategy_tester.*860006" 2>/dev/null || true

# Create a Python script that will be debugged
cat > /tmp/debug_opt.py << 'PYEOF'
import sys
import os
sys.path.insert(0, '/Users/andym/projects/AutoBBS/ctester')
os.chdir('/Users/andym/projects/AutoBBS/ctester')

from include.asirikuy import *
import configparser

config = configparser.ConfigParser()
config.read('config/Peso_MACD_GBPJPY-1H_860006_optimize.config')

# Initialize
asirikuyCtesterLogPath = './log/AsirikuyCTester.log'
log_severity_str = config.get("misc", "logSeverity").split(';')[0].strip()
astdll.initCTesterFramework(asirikuyCtesterLogPath.encode('utf-8'), int(log_severity_str))

# Load minimal setup to get to optimization call
# (This is a simplified version - you may need to adjust)

print("Python script ready for debugging")
print("Press Ctrl+C to exit")
import time
time.sleep(3600)  # Sleep to allow debugger attachment
PYEOF

echo "Created debug script at /tmp/debug_opt.py"
echo ""
echo "To debug:"
echo "1. Run: lldb python3 /tmp/debug_opt.py"
echo "2. Set breakpoint: (lldb) breakpoint set --name runOptimizationMultipleSymbols"
echo "3. Run: (lldb) run"
echo ""
echo "Or attach to running process:"
echo "1. Start optimization in another terminal"
echo "2. Find PID: ps aux | grep asirikuy_strategy_tester | grep 860006"
echo "3. Attach: lldb -p <PID>"
echo ""

# Alternative: Create a simpler approach - run with lldb directly
echo "=== Running with lldb (simpler approach) ==="
echo "Starting Python with lldb..."
echo ""

lldb -- python3 asirikuy_strategy_tester.py -c config/Peso_MACD_GBPJPY-1H_860006_optimize.config -ot results -oo optimization_results << 'LLDBEOF'
# Set breakpoint at function entry
breakpoint set --name runOptimizationMultipleSymbols
breakpoint set --file optimizer.c --line 335
breakpoint set --file optimizer.c --line 400
breakpoint set --file optimizer.c --line 440

# Set breakpoint at potential hang points
breakpoint set --file optimizer.c --line 402
breakpoint set --file optimizer.c --line 415

# Run with arguments
run

# When it stops, print backtrace
bt

# Continue execution step by step
continue
LLDBEOF

