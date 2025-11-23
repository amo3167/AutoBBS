#!/bin/bash
# Build script for AsirikuyFrameworkAPI and UnitTests
# Automates the complete build process on macOS

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Default values
CONFIG="debug64"
PROJECT="AsirikuyFrameworkAPI"
CLEAN=false
VERBOSE=false

# Parse command line arguments
while [[ $# -gt 0 ]]; do
  case $1 in
    -c|--clean)
      CLEAN=true
      shift
      ;;
    -v|--verbose)
      VERBOSE=true
      shift
      ;;
    -p|--project)
      PROJECT="$2"
      shift 2
      ;;
    -config)
      CONFIG="$2"
      shift 2
      ;;
    -h|--help)
      echo "Usage: $0 [OPTIONS]"
      echo ""
      echo "Options:"
      echo "  -c, --clean       Clean build directories before building"
      echo "  -v, --verbose     Verbose output"
      echo "  -p, --project     Project to build (default: AsirikuyFrameworkAPI)"
      echo "                    Options: AsirikuyFrameworkAPI, UnitTests, TradingStrategies, CTesterFrameworkAPI, all"
      echo "  -config          Build configuration (default: debug64)"
      echo "                    Options: debug32, debug64, release32, release64"
      echo "  -h, --help        Show this help message"
      echo ""
      echo "Examples:"
      echo "  $0                                    # Build AsirikuyFrameworkAPI (debug64)"
      echo "  $0 -c                                 # Clean build"
      echo "  $0 -p UnitTests                       # Build UnitTests"
      echo "  $0 -p all -config release64            # Build all projects (release64)"
      exit 0
      ;;
    *)
      echo -e "${RED}Unknown option: $1${NC}"
      echo "Use -h or --help for usage information"
      exit 1
      ;;
  esac
done

# Check if we're on macOS
if [ "$(uname -s)" != "Darwin" ]; then
  echo -e "${YELLOW}Warning: This script is optimized for macOS. Building on other platforms may require adjustments.${NC}"
fi

# Set BOOST_ROOT if not already set
if [ -z "$BOOST_ROOT" ]; then
  if [ -d "/Users/andym/homebrew/opt/boost" ]; then
    export BOOST_ROOT=/Users/andym/homebrew/opt/boost
    echo -e "${GREEN}Set BOOST_ROOT=$BOOST_ROOT${NC}"
  elif command -v brew &> /dev/null; then
    export BOOST_ROOT=$(brew --prefix boost)
    echo -e "${GREEN}Set BOOST_ROOT=$BOOST_ROOT${NC}"
  else
    echo -e "${RED}Error: BOOST_ROOT not set and brew not found.${NC}"
    echo "Please set BOOST_ROOT environment variable:"
    echo "  export BOOST_ROOT=/path/to/boost"
    exit 1
  fi
fi

# Check if premake4 Docker image exists
if ! docker images | grep -q "autobbs-premake4"; then
  echo -e "${YELLOW}Warning: Docker image 'autobbs-premake4' not found.${NC}"
  echo "You may need to build it first or use a different premake4 setup."
fi

echo -e "${GREEN}=== Starting Build Process ===${NC}"
echo "Project: $PROJECT"
echo "Configuration: $CONFIG"
echo "BOOST_ROOT: $BOOST_ROOT"
echo ""

# Step 1: Clean if requested
if [ "$CLEAN" = true ]; then
  echo -e "${YELLOW}Cleaning build directories...${NC}"
  rm -rf build/gmake bin/gmake tmp/gmake
  echo -e "${GREEN}✓ Clean complete${NC}"
  echo ""
fi

# Step 2: Generate Makefiles
echo -e "${YELLOW}Step 1: Generating Makefiles...${NC}"
if [ "$VERBOSE" = true ]; then
  docker run --rm -v "$(pwd):/workspace" -w /workspace -e BOOST_ROOT="$BOOST_ROOT" autobbs-premake4:latest premake4 gmake
else
  docker run --rm -v "$(pwd):/workspace" -w /workspace -e BOOST_ROOT="$BOOST_ROOT" autobbs-premake4:latest premake4 gmake 2>&1 | tail -3
fi
echo -e "${GREEN}✓ Makefiles generated${NC}"
echo ""

# Step 3: Fix Makefiles (remove -std=c++11 from CFLAGS, remove -lrt on macOS, rename .so to .dylib)
echo -e "${YELLOW}Step 2: Fixing Makefiles...${NC}"
./fix_makefiles.sh
echo -e "${GREEN}✓ Makefiles fixed${NC}"
echo ""

# Step 4: Build
echo -e "${YELLOW}Step 3: Building...${NC}"
cd build/gmake
export BOOST_ROOT="$BOOST_ROOT"

if [ "$PROJECT" = "all" ]; then
  # Build all projects
  if [ "$VERBOSE" = true ]; then
    make -j4 config="$CONFIG"
  else
    make -j4 config="$CONFIG" 2>&1 | grep -E "Building|Linking|Error|error" | tail -20
  fi
else
  # Build specific project
  if [ "$VERBOSE" = true ]; then
    make "$PROJECT" config="$CONFIG" -j4
  else
    make "$PROJECT" config="$CONFIG" -j4 2>&1 | tail -15
  fi
fi

cd ../..

# Step 4.4: Move TradingStrategies library to correct location if needed
# Check multiple possible locations where the library might be created
TRADING_STRATEGIES_FOUND=false

# Check in core/TradingStrategies/bin/gmake/ (if targetdir was set)
if [ -f "core/TradingStrategies/bin/gmake/x64/Debug/lib/libtrading_strategies.so" ] || [ -f "core/TradingStrategies/bin/gmake/x64/Debug/lib/libtrading_strategies.dylib" ]; then
  echo -e "${YELLOW}Moving TradingStrategies library to correct location...${NC}"
  mkdir -p bin/gmake/x64/Debug/lib
  mv core/TradingStrategies/bin/gmake/x64/Debug/lib/libtrading_strategies.* bin/gmake/x64/Debug/lib/ 2>/dev/null || true
  TRADING_STRATEGIES_FOUND=true
  # Also check for other configurations
  for config_dir in core/TradingStrategies/bin/gmake/*/Debug/lib core/TradingStrategies/bin/gmake/*/Release/lib; do
    if [ -d "$config_dir" ]; then
      config_name=$(basename $(dirname $(dirname "$config_dir")))
      build_type=$(basename $(dirname "$config_dir"))
      mkdir -p "bin/gmake/$config_name/$build_type/lib"
      mv "$config_dir/libtrading_strategies."* "bin/gmake/$config_name/$build_type/lib/" 2>/dev/null || true
    fi
  done
fi

# Check in core/TradingStrategies/ (default location when no targetdir is set)
if [ -f "core/TradingStrategies/libtrading_strategies.so" ] || [ -f "core/TradingStrategies/libtrading_strategies.dylib" ]; then
  if [ "$TRADING_STRATEGIES_FOUND" = false ]; then
    echo -e "${YELLOW}Moving TradingStrategies library to correct location...${NC}"
  fi
  mkdir -p bin/gmake/x64/Debug/lib
  mv core/TradingStrategies/libtrading_strategies.* bin/gmake/x64/Debug/lib/ 2>/dev/null || true
  TRADING_STRATEGIES_FOUND=true
fi

if [ "$TRADING_STRATEGIES_FOUND" = true ]; then
  echo -e "${GREEN}✓ Library moved${NC}"
  echo ""
fi

# Step 4.5: Rename .so to .dylib on macOS (in case build created new .so files)
if [ "$(uname -s)" = "Darwin" ]; then
  echo -e "${YELLOW}Renaming .so files to .dylib on macOS...${NC}"
  find bin/gmake -name "*.so" -type f 2>/dev/null | while read sofile; do
    dylibfile="${sofile%.so}.dylib"
    if [ -f "$sofile" ]; then
      if [ -f "$dylibfile" ]; then
        # If .dylib already exists, remove the .so file
        rm -f "$sofile" 2>/dev/null || true
      else
        # Rename .so to .dylib
        mv "$sofile" "$dylibfile" 2>/dev/null || true
      fi
    fi
  done
  echo -e "${GREEN}✓ Rename complete${NC}"
  echo ""
  
  # Step 4.6: Fix library dependencies (install names and rpaths)
  echo -e "${YELLOW}Step 4.6: Fixing library dependencies...${NC}"
  PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"
  
  # Find all libraries first
  TRADING_STRATEGIES_LIB="$PROJECT_ROOT/bin/gmake/x64/Debug/lib/libtrading_strategies.dylib"
  ASIRIKUY_FRAMEWORK_LIB="$PROJECT_ROOT/bin/gmake/x64/Debug/libAsirikuyFrameworkAPI.dylib"
  CTESTER_FRAMEWORK_LIB=$(find "$PROJECT_ROOT/bin/gmake" -name "*CTesterFrameworkAPI.dylib" 2>/dev/null | head -1)
  MXML_LIB="$PROJECT_ROOT/vendor/MiniXML/libmxml.dylib"
  
  # Fix each library
  find bin/gmake -name "*.dylib" -type f 2>/dev/null | while read -r lib; do
    if [ ! -f "$lib" ]; then
      continue
    fi
    
    lib_name=$(basename "$lib")
    lib_dir=$(dirname "$lib")
    
    # Fix self-reference (install name) - change .so to .dylib
    self_ref=$(otool -D "$lib" 2>/dev/null | tail -1)
    if [[ "$self_ref" == *".so"* ]]; then
      new_self_ref="${self_ref%.so}.dylib"
      install_name_tool -id "$new_self_ref" "$lib" 2>/dev/null || true
    fi
    
    # Fix cross-library references - collect dependencies first to avoid subshell issues
    deps_to_fix=()
    while IFS= read -r dep; do
      # Skip empty lines and system libraries
      [ -z "$dep" ] && continue
      if [[ "$dep" == "/usr/lib/"* ]] || [[ "$dep" == "/System/"* ]] || [[ "$dep" == "/Users/andym/homebrew/"* ]]; then
        continue
      fi
      
      # Collect .so references, absolute paths to our libraries, relative paths (../), and vendor library references
      if [[ "$dep" == *".so"* ]] || [[ "$dep" == "$PROJECT_ROOT"* ]] || [[ "$dep" == *"libmxml"* ]] || [[ "$dep" == "/usr/local/lib/"* ]] || [[ "$dep" == "../"* ]] || [[ "$dep" == "./"* ]]; then
        deps_to_fix+=("$dep")
      fi
    done < <(otool -L "$lib" 2>/dev/null | grep -E "\.so|\.dylib" | grep -v ":" | awk '{print $1}')
    
    # Now fix each dependency
    for dep in "${deps_to_fix[@]}"; do
      # Determine which library this dependency refers to
      rpath_name=""
      
      if [[ "$dep" == *"libAsirikuyFrameworkAPI"* ]] && [ -f "$ASIRIKUY_FRAMEWORK_LIB" ]; then
        rpath_name="@rpath/libAsirikuyFrameworkAPI.dylib"
      elif [[ "$dep" == *"libCTesterFrameworkAPI"* ]] && [ -f "$CTESTER_FRAMEWORK_LIB" ]; then
        rpath_name="@rpath/$(basename "$CTESTER_FRAMEWORK_LIB")"
      elif [[ "$dep" == *"libtrading_strategies"* ]] && [ -f "$TRADING_STRATEGIES_LIB" ]; then
        rpath_name="@rpath/libtrading_strategies.dylib"
      elif [[ "$dep" == *"libmxml"* ]] && [ -f "$MXML_LIB" ]; then
        rpath_name="@rpath/libmxml.dylib"
      elif [[ "$dep" == *".so"* ]]; then
        # Fallback: just change .so to .dylib
        rpath_name="${dep%.so}.dylib"
      fi
      
      if [ -n "$rpath_name" ]; then
        install_name_tool -change "$dep" "$rpath_name" "$lib" 2>/dev/null || true
      fi
    done
    
    # Add rpaths to help find dependencies (ignore errors if rpath already exists)
    install_name_tool -add_rpath "@loader_path" "$lib" 2>/dev/null || true
    install_name_tool -add_rpath "$PROJECT_ROOT/bin/gmake/x64/Debug" "$lib" 2>&1 | grep -v "duplicate path" || true
    install_name_tool -add_rpath "$PROJECT_ROOT/bin/gmake/x64/Debug/lib" "$lib" 2>&1 | grep -v "duplicate path" || true
    install_name_tool -add_rpath "$PROJECT_ROOT/vendor/MiniXML" "$lib" 2>&1 | grep -v "duplicate path" || true
  done
  
  echo -e "${GREEN}✓ Library dependencies fixed${NC}"
  echo ""
fi

# Step 5: Verify build output
echo ""
echo -e "${YELLOW}Step 4: Verifying build output...${NC}"

if [ "$PROJECT" = "AsirikuyFrameworkAPI" ] || [ "$PROJECT" = "all" ]; then
  if [ -f "bin/gmake/x64/Debug/libAsirikuyFrameworkAPI.dylib" ] || [ -f "bin/gmake/x64/Debug/libAsirikuyFrameworkAPI.so" ]; then
    echo -e "${GREEN}✓ AsirikuyFrameworkAPI built successfully${NC}"
    ls -lh bin/gmake/x64/Debug/libAsirikuyFrameworkAPI.* 2>/dev/null | head -1
  else
    echo -e "${RED}✗ AsirikuyFrameworkAPI build failed - output not found${NC}"
  fi
fi

if [ "$PROJECT" = "UnitTests" ] || [ "$PROJECT" = "all" ]; then
  if [ -f "bin/gmake/x64/Debug/UnitTests" ]; then
    echo -e "${GREEN}✓ UnitTests built successfully${NC}"
    ls -lh bin/gmake/x64/Debug/UnitTests
  else
    echo -e "${YELLOW}⚠ UnitTests not found (may not be built)${NC}"
  fi
fi

if [ "$PROJECT" = "TradingStrategies" ] || [ "$PROJECT" = "all" ]; then
  if [ -f "bin/gmake/x64/Debug/lib/libtrading_strategies.dylib" ] || [ -f "bin/gmake/x64/Debug/lib/libtrading_strategies.so" ]; then
    echo -e "${GREEN}✓ TradingStrategies built successfully${NC}"
    ls -lh bin/gmake/x64/Debug/lib/libtrading_strategies.* 2>/dev/null | head -1
  else
    echo -e "${YELLOW}⚠ TradingStrategies not found (may not be built)${NC}"
  fi
fi

if [ "$PROJECT" = "CTesterFrameworkAPI" ] || [ "$PROJECT" = "all" ]; then
  # CTesterFrameworkAPI output is in bin/gmake/x64/Debug/ directory (one level up from lib/, same as AsirikuyFrameworkAPI)
  if [ -f "bin/gmake/x64/Debug/libCTesterFrameworkAPI.dylib" ] || [ -f "bin/gmake/x64/Debug/libCTesterFrameworkAPI.so" ]; then
    echo -e "${GREEN}✓ CTesterFrameworkAPI built successfully${NC}"
    ls -lh bin/gmake/x64/Debug/libCTesterFrameworkAPI.* 2>/dev/null | head -1
  else
    echo -e "${YELLOW}⚠ CTesterFrameworkAPI not found (may not be built)${NC}"
  fi
fi

echo ""
echo -e "${GREEN}=== Build Complete ===${NC}"

