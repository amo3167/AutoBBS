#!/bin/bash
# Asirikuy Monitor - Startup Script (Unix/macOS/Linux)
# Starts the monitor with proper error handling and logging

set -e  # Exit on error

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
CONFIG_FILE="${1:-config/checker.config}"

# Change to project directory
cd "$PROJECT_DIR"

# Check if virtual environment exists
if [ ! -d "venv" ]; then
    echo "Error: Virtual environment not found. Please run scripts/install.sh first."
    exit 1
fi

# Activate virtual environment
source venv/bin/activate

# Check if Python is available
if ! command -v python3 &> /dev/null; then
    echo "Error: python3 not found in virtual environment."
    exit 1
fi

# Check if checker.py exists
if [ ! -f "checker.py" ]; then
    echo "Error: checker.py not found in project directory."
    exit 1
fi

# Create log directory if it doesn't exist
mkdir -p log

# Log startup
STARTUP_LOG="log/startup.log"
echo "$(date '+%Y-%m-%d %H:%M:%S') - Starting Asirikuy Monitor" >> "$STARTUP_LOG"
echo "$(date '+%Y-%m-%d %H:%M:%S') - Config file: $CONFIG_FILE" >> "$STARTUP_LOG"
echo "$(date '+%Y-%m-%d %H:%M:%S') - Python: $(python3 --version)" >> "$STARTUP_LOG"
echo "$(date '+%Y-%m-%d %H:%M:%S') - Platform: $(python3 -c 'from include.platform_utils import get_platform_name; print(get_platform_name())')" >> "$STARTUP_LOG"

# Function to handle errors
handle_error() {
    ERROR_CODE=$?
    echo "$(date '+%Y-%m-%d %H:%M:%S') - Error: Monitor exited with code $ERROR_CODE" >> "$STARTUP_LOG"
    echo "Error: Monitor exited with code $ERROR_CODE"
    exit $ERROR_CODE
}

# Set up error handling
trap 'handle_error' ERR

# Start the monitor
if [ -f "$CONFIG_FILE" ]; then
    echo "Starting Asirikuy Monitor with config: $CONFIG_FILE"
    python3 checker.py -c "$CONFIG_FILE" 2>&1 | tee -a "$STARTUP_LOG"
else
    echo "Warning: Config file not found: $CONFIG_FILE"
    echo "Starting Asirikuy Monitor with default config"
    python3 checker.py 2>&1 | tee -a "$STARTUP_LOG"
fi

