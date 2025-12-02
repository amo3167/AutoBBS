#!/bin/bash
# Asirikuy Monitor - Uninstall Script
# Removes the monitor installation

set -e  # Exit on error

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

echo "============================================================"
echo "Asirikuy Monitor - Uninstall"
echo "============================================================"
echo ""

read -p "Are you sure you want to uninstall? This will remove the virtual environment. (y/N): " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "Uninstall cancelled."
    exit 0
fi

cd "$PROJECT_DIR"

# Remove virtual environment
if [ -d "venv" ]; then
    echo "Removing virtual environment..."
    rm -rf venv
    echo "✓ Virtual environment removed"
else
    echo "⚠ Virtual environment not found"
fi
echo ""

# Optionally remove log files
read -p "Remove log files? (y/N): " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    if [ -d "log" ]; then
        echo "Removing log files..."
        rm -rf log
        echo "✓ Log files removed"
    fi
    echo ""
fi

# Optionally remove config files
read -p "Remove config files? (y/N): " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    if [ -f "config/checker.config" ]; then
        echo "Removing config file..."
        rm -f config/checker.config
        echo "✓ Config file removed"
    fi
    if [ -f ".env" ]; then
        echo "Removing .env file..."
        rm -f .env
        echo "✓ .env file removed"
    fi
    echo ""
fi

echo "============================================================"
echo "Uninstall Complete!"
echo "============================================================"
echo ""
echo "Note: Source code and other files were not removed."
echo "To completely remove, delete the entire project directory."
echo ""

