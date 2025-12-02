#!/bin/bash
# Docker wrapper script for premake4

# Get the directory of this script
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Build the Docker image if it doesn't exist
if ! docker image inspect autobbs-premake4:latest >/dev/null 2>&1; then
    echo "Building premake4 Docker image..."
    docker build -f "$SCRIPT_DIR/Dockerfile.premake4" -t autobbs-premake4:latest "$SCRIPT_DIR"
fi

# Run premake4 in the container, mounting the current directory
# Pass through BOOST_ROOT and other common environment variables
docker run --rm \
  -v "$SCRIPT_DIR:/workspace" \
  -w /workspace \
  -e BOOST_ROOT="${BOOST_ROOT:-}" \
  -e R_HOME="${R_HOME:-}" \
  autobbs-premake4:latest premake4 "$@"

