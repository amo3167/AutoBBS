#!/bin/bash
################################################################################
# Comprehensive build and release script for AsirikuyFramework
# Supports macOS and Linux platforms
################################################################################

set -euo pipefail

# ============================================================================
# CONFIGURATION
# ============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$SCRIPT_DIR"
RELEASE_DIR="${RELEASE_DIR:-releases}"
BUILD_DIR="${BUILD_DIR:-build}"
BIN_DIR="${BIN_DIR:-bin}"
TMP_DIR="${TMP_DIR:-tmp}"

# Color codes
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Default values
CONFIG="${1:-release64}"
PROJECT="${2:-all}"
CLEAN=false
RELEASE_MODE=false
VERBOSE=false
NO_VENDOR=false

# ============================================================================
# UTILITY FUNCTIONS
# ============================================================================

log_status() {
    echo -e "${BLUE}$(date '+%H:%M:%S') [INFO]${NC}  $1"
}

log_success() {
    echo -e "${GREEN}$(date '+%H:%M:%S') [OK]${NC}    $1"
}

log_warn() {
    echo -e "${YELLOW}$(date '+%H:%M:%S') [WARN]${NC}  $1"
}

log_error() {
    echo -e "${RED}$(date '+%H:%M:%S') [ERROR]${NC} $1"
}

show_help() {
    cat << EOF
Usage: $0 [OPTIONS] [CONFIG] [PROJECT]

ARGUMENTS:
  CONFIG    Build configuration: debug32, debug64, release32, release64 (default: release64)
  PROJECT   Project to build: AsirikuyFrameworkAPI, TradingStrategies, CTesterFrameworkAPI, 
            UnitTests, all (default: all)

OPTIONS:
  -c, --clean         Clean build directories before building
  -r, --release       Create release package with organized directory structure
  -v, --verbose       Verbose build output
  --no-vendor         Skip building vendor libraries
  -h, --help          Show this help message

EXAMPLES:
  $0                                           # Build release64 all projects
  $0 -c release32                              # Clean build debug32
  $0 -r release64                              # Build and create release package
  $0 -c -v debug64 AsirikuyFrameworkAPI        # Clean verbose build of single project

ENVIRONMENT VARIABLES:
  BOOST_ROOT          Path to Boost library (required)
  RELEASE_DIR         Output directory for release packages (default: releases)

EOF
}

# ============================================================================
# DEPENDENCY CHECKS
# ============================================================================

check_dependencies() {
    log_status "Checking dependencies..."
    
    local missing=false
    
    if ! command -v premake4 &> /dev/null; then
        log_warn "premake4 not found in PATH"
        if [ -x "$REPO_ROOT/premake4" ]; then
            log_status "Found local premake4"
        else
            log_error "premake4 not found. Please install it or set PATH."
            missing=true
        fi
    else
        log_success "premake4 found: $(command -v premake4)"
    fi
    
    if [ -z "${BOOST_ROOT:-}" ]; then
        log_warn "BOOST_ROOT not set"
        if [ "$(uname -s)" = "Darwin" ]; then
            if command -v brew &> /dev/null; then
                BOOST_ROOT=$(brew --prefix boost 2>/dev/null || echo "")
                if [ -n "$BOOST_ROOT" ]; then
                    log_status "Found Boost via Homebrew: $BOOST_ROOT"
                    export BOOST_ROOT
                else
                    log_warn "Boost not found via Homebrew"
                fi
            fi
        fi
    else
        log_success "BOOST_ROOT: $BOOST_ROOT"
    fi
    
    if [ "$missing" = true ]; then
        log_error "Some dependencies are missing. Please install them first."
        return 1
    fi
    
    return 0
}

# ============================================================================
# BUILD FUNCTIONS
# ============================================================================

initialize_premake() {
    log_status "Running premake4 to generate build files..."
    
    local premake_exe="${REPO_ROOT}/premake4"
    if ! [ -x "$premake_exe" ]; then
        premake_exe="premake4"
    fi
    
    if ! command -v "$premake_exe" &> /dev/null && [ ! -x "$premake_exe" ]; then
        log_error "premake4 executable not found"
        return 1
    fi
    
    cd "$REPO_ROOT"
    "$premake_exe" --file=premake4.lua gmake || {
        log_error "premake4 failed"
        return 1
    }
    log_success "premake4 completed"
    return 0
}

build_gmake() {
    local config="$1"
    local project="$2"
    local make_jobs="${MAKE_JOBS:-$(nproc 2>/dev/null || echo 4)}"
    
    log_status "Building with gmake: config=$config, project=$project"
    
    local build_subdir="${BUILD_DIR}/gmake"
    if [ ! -d "$build_subdir" ]; then
        log_error "Build directory not found: $build_subdir"
        return 1
    fi
    
    cd "$build_subdir"
    
    local make_config="config=$config"
    local make_target=""
    
    if [ "$project" != "all" ]; then
        make_target="$project"
    fi
    
    if [ "$VERBOSE" = true ]; then
        make -j "$make_jobs" "$make_config" $make_target verbose=1 || return 1
    else
        make -j "$make_jobs" "$make_config" $make_target || return 1
    fi
    
    cd "$REPO_ROOT"
    log_success "Build completed"
    return 0
}

clean_build() {
    log_status "Cleaning build directories..."
    
    for dir in "$BUILD_DIR" "$BIN_DIR" "$TMP_DIR"; do
        if [ -d "$dir" ]; then
            log_status "Removing: $dir"
            rm -rf "$dir"
        fi
    done
    
    log_success "Clean completed"
}

verify_outputs() {
    log_status "Verifying build outputs..."
    
    local count=0
    
    if [ -d "$BIN_DIR" ]; then
        # Find all libraries and executables
        while IFS= read -r file; do
            local size=$(stat -f%z "$file" 2>/dev/null || stat -c%s "$file" 2>/dev/null)
            local size_kb=$((size / 1024))
            echo "  📦 $(basename "$file") ($size_kb KB)" 
            ((count++))
        done < <(find "$BIN_DIR" -type f \( -name "*.a" -o -name "*.so" -o -name "*.dylib" -o -name "*.exe" \) 2>/dev/null)
    fi
    
    log_status "Found $count artifacts"
    return 0
}

# ============================================================================
# RELEASE PACKAGE FUNCTIONS
# ============================================================================

create_release_package() {
    log_status "========================================================================="
    log_status "Creating Release Package"
    log_status "========================================================================="
    
    local timestamp=$(date +%Y%m%d-%H%M%S)
    local release_name="AsirikuyFramework-$timestamp"
    local release_path="${RELEASE_DIR}/${release_name}"
    
    log_status "Release directory: $release_path"
    
    # Create directory structure
    mkdir -p "$release_path"/{bin,lib,include,docs}
    
    # Copy binaries
    log_status "Copying binaries..."
    if [ -d "$BIN_DIR" ]; then
        find "$BIN_DIR" -type f \( -name "*.so" -o -name "*.dylib" -o -name "*.exe" \) -exec cp {} "$release_path/bin/" \; 2>/dev/null || true
    fi
    
    # Copy libraries
    log_status "Copying libraries..."
    if [ -d "$BIN_DIR" ]; then
        find "$BIN_DIR" -type f -name "*.a" -exec cp {} "$release_path/lib/" \; 2>/dev/null || true
    fi
    
    # Copy headers
    log_status "Copying headers..."
    if [ -d "core" ]; then
        find core -type d -name "include" | while read -r inc_dir; do
            local parent=$(basename "$(dirname "$inc_dir")")
            mkdir -p "$release_path/include/$parent"
            cp -r "$inc_dir"/* "$release_path/include/$parent/" 2>/dev/null || true
        done
    fi
    
    # Copy documentation
    log_status "Copying documentation..."
    find docs -maxdepth 1 -name "*.md" -exec cp {} "$release_path/docs/" \; 2>/dev/null || true
    
    # Create manifest
    cat > "$release_path/MANIFEST.txt" << EOF
Release: $release_name
Generated: $(date)
Platform: $(uname -s) $(uname -m)
Build Configuration: $CONFIG

Contents:
- bin/     Shared libraries and executables
- lib/     Static libraries
- include/ Header files
- docs/    Documentation
EOF
    
    log_success "Release package created: $release_path"
    
    # Create tarball
    log_status "Creating tarball..."
    cd "$RELEASE_DIR"
    tar -czf "${release_name}.tar.gz" "$release_name" 2>/dev/null || true
    cd "$REPO_ROOT"
    log_success "Tarball created: $RELEASE_DIR/${release_name}.tar.gz"
}

# ============================================================================
# REPORT GENERATION
# ============================================================================

create_build_report() {
    local report_file="${REPO_ROOT}/build-report.md"
    
    log_status "Creating build report: $report_file"
    
    cat > "$report_file" << EOF
# Build Report

**Generated:** $(date '+%Y-%m-%d %H:%M:%S')  
**Platform:** $(uname -s) $(uname -m)  
**Configuration:** $CONFIG  
**Project:** $PROJECT  

## Summary

- Build Type: $CONFIG
- Date: $(date)
- Status: $([ -d "$BIN_DIR" ] && echo "✅ Completed" || echo "❌ Failed")

## Build Artifacts

EOF
    
    if [ -d "$BIN_DIR" ]; then
        echo "### Libraries & Executables" >> "$report_file"
        echo "" >> "$report_file"
        find "$BIN_DIR" -type f \( -name "*.a" -o -name "*.so" -o -name "*.dylib" -o -name "*.exe" \) | while read -r file; do
            local size=$(stat -f%z "$file" 2>/dev/null || stat -c%s "$file" 2>/dev/null)
            local size_kb=$((size / 1024))
            echo "- \`$(basename "$file")\` ($size_kb KB)" >> "$report_file"
        done
    fi
    
    log_success "Build report saved: $report_file"
}

# ============================================================================
# ARGUMENT PARSING
# ============================================================================

while [[ $# -gt 0 ]]; do
    case $1 in
        -c|--clean)
            CLEAN=true
            shift
            ;;
        -r|--release)
            RELEASE_MODE=true
            shift
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        --no-vendor)
            NO_VENDOR=true
            shift
            ;;
        -h|--help)
            show_help
            exit 0
            ;;
        *)
            if [ -z "$CONFIG" ] || [ "$CONFIG" = "release64" ]; then
                CONFIG=$1
            elif [ -z "$PROJECT" ] || [ "$PROJECT" = "all" ]; then
                PROJECT=$1
            fi
            shift
            ;;
    esac
done

# ============================================================================
# MAIN EXECUTION
# ============================================================================

log_status "========================================================================="
log_status "AsirikuyFramework Build & Release Script"
log_status "========================================================================="
log_status "Repository root: $REPO_ROOT"
log_status "Configuration: $CONFIG"
log_status "Project: $PROJECT"
log_status "Clean: $CLEAN"
log_status "Release mode: $RELEASE_MODE"

cd "$REPO_ROOT"

# Check dependencies
if ! check_dependencies; then
    exit 1
fi

# Clean if requested
if [ "$CLEAN" = true ]; then
    clean_build
fi

# Initialize premake
if ! initialize_premake; then
    exit 1
fi

# Perform build
log_status ""
log_status "========================================================================="
log_status "Building Configuration: $CONFIG"
log_status "========================================================================="

if ! build_gmake "$CONFIG" "$PROJECT"; then
    log_error "Build failed"
    exit 1
fi

# Verify outputs
log_status ""
log_status "========================================================================="
log_status "Verifying Build Outputs"
log_status "========================================================================="

verify_outputs

# Create release package if requested
if [ "$RELEASE_MODE" = true ]; then
    mkdir -p "$RELEASE_DIR"
    create_release_package
fi

# Create report
log_status ""
create_build_report

# Summary
log_status ""
log_status "========================================================================="
log_status "Build Complete"
log_status "========================================================================="
log_success "Build artifacts location: $BIN_DIR"
if [ "$RELEASE_MODE" = true ]; then
    log_success "Release package location: $RELEASE_DIR"
fi
log_success "Build report: build-report.md"

exit 0
