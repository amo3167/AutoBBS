#!/bin/bash
# Asirikuy Monitor - Installation Script
# Installs the monitor and sets up the environment

set -e  # Exit on error

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

echo "============================================================"
echo "Asirikuy Monitor - Installation"
echo "============================================================"
echo ""

# Check Python version
echo "Checking Python version..."
if ! command -v python3 &> /dev/null; then
    echo "Error: python3 not found. Please install Python 3.8 or higher."
    exit 1
fi

PYTHON_VERSION=$(python3 --version | cut -d' ' -f2 | cut -d'.' -f1,2)
REQUIRED_VERSION="3.8"

if [ "$(printf '%s\n' "$REQUIRED_VERSION" "$PYTHON_VERSION" | sort -V | head -n1)" != "$REQUIRED_VERSION" ]; then
    echo "Error: Python 3.8 or higher required. Found: $PYTHON_VERSION"
    exit 1
fi

echo "✓ Python $PYTHON_VERSION found"
echo ""

# Create virtual environment
echo "Creating virtual environment..."
cd "$PROJECT_DIR"
if [ ! -d "venv" ]; then
    python3 -m venv venv
    echo "✓ Virtual environment created"
else
    echo "✓ Virtual environment already exists"
fi
echo ""

# Activate virtual environment
echo "Activating virtual environment..."
source venv/bin/activate
echo "✓ Virtual environment activated"
echo ""

# Upgrade pip
echo "Upgrading pip..."
pip install --upgrade pip --quiet
echo "✓ pip upgraded"
echo ""

# Install dependencies
echo "Installing dependencies..."
pip install -r requirements.txt --quiet
echo "✓ Dependencies installed"
echo ""

# Verify installation
echo "Verifying installation..."
python3 checker.py -v > /dev/null 2>&1
if [ $? -eq 0 ]; then
    echo "✓ Installation verified"
else
    echo "✗ Installation verification failed"
    exit 1
fi
echo ""

# Create log directory
echo "Creating log directory..."
mkdir -p "$PROJECT_DIR/log"
echo "✓ Log directory created"
echo ""

# Set up config file if it doesn't exist
if [ ! -f "$PROJECT_DIR/config/checker.config" ]; then
    echo "Creating config file from example..."
    if [ -f "$PROJECT_DIR/config/checker.config.example" ]; then
        cp "$PROJECT_DIR/config/checker.config.example" "$PROJECT_DIR/config/checker.config"
        echo "✓ Config file created (please edit config/checker.config with your settings)"
    else
        echo "⚠ Config example not found, skipping"
    fi
    echo ""
fi

echo "============================================================"
echo "Installation Complete!"
echo "============================================================"
echo ""
echo "Next steps:"
echo "1. Edit config/checker.config with your settings"
echo "2. Set up .env file for email credentials (optional but recommended)"
echo "3. Run: source venv/bin/activate && python3 checker.py"
echo ""
echo "For more information, see README.md and DEPLOYMENT.md"
echo ""

