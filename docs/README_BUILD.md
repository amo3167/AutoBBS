# AsirikuyFrameworkAPI Build Guide

This document provides instructions for building the `AsirikuyFrameworkAPI` shared library and `UnitTests` executable on macOS.

## Overview

The `AsirikuyFrameworkAPI` is the main entry point library for the Asirikuy Framework. It provides a unified API that Python, MQL (MetaTrader), and CTester can call. On Windows it generates `AsirikuyFrameworkAPI.dll`, and on macOS it should generate `libAsirikuyFrameworkAPI.dylib` (or `AsirikuyFrameworkAPI.dylib`).

**Architecture:**
- **AsirikuyFrameworkAPI** - Main entry point (what Python/MQL/CTester call)
  - Windows: `AsirikuyFrameworkAPI.dll`
  - macOS: `libAsirikuyFrameworkAPI.dylib` (or `AsirikuyFrameworkAPI.dylib`)
- **TradingStrategies** - Component library (used internally by AsirikuyFrameworkAPI)
  - Output: `libtrading_strategies.dylib` (dependency, not directly called)
- **UnitTests** - Test executable for validating AsirikuyFrameworkAPI functionality
  - Output: `UnitTests` (console application that runs all unit tests)

## Prerequisites

### Required Software

1. **macOS** (tested on macOS with ARM64 architecture)
2. **GCC/G++** compiler (usually comes with Xcode Command Line Tools)
3. **Make** (usually comes with Xcode Command Line Tools)
4. **Premake4** (for generating Makefiles)
5. **Boost C++ Libraries** (installed via Homebrew)
6. **cURL** (installed via Homebrew)

### Installing Dependencies

```bash
# Install Xcode Command Line Tools (if not already installed)
xcode-select --install

# Install Boost and cURL via Homebrew
brew install boost curl

# Set BOOST_ROOT environment variable
export BOOST_ROOT=/Users/andym/homebrew/opt/boost

# MiniXML is included in vendor/MiniXML (source code)
# It will be built automatically during the build process
```

### Premake4 Installation

Premake4 is required to generate the Makefiles. See `INSTALL_PREMAKE4.md` for detailed installation instructions.

**Quick Setup (using Docker):**
```bash
# Use the Docker wrapper script if premake4 is not installed natively
./premake4-docker.sh gmake
```

## Build Configurations

The build system supports four configurations:

| Configuration | Architecture | Build Type | Output Location |
|--------------|--------------|------------|-----------------|
| `debug32` | 32-bit | Debug | `bin/gmake/x32/Debug/lib/` |
| `release32` | 32-bit | Release | `bin/gmake/x32/Release/lib/` |
| `debug64` | 64-bit (ARM64) | Debug | `bin/gmake/x64/Debug/` |
| `release64` | 64-bit (ARM64) | Release | `bin/gmake/x64/Release/` |

## Building the Library

### Step 1: Generate Makefiles (if needed)

If you've modified `premake4.lua` or need to regenerate Makefiles:

```bash
cd /Users/andym/projects/AutoBBS
./premake4-docker.sh gmake
# Or if premake4 is installed natively:
premake4 gmake
```

### Step 2: Build MiniXML

**MiniXML must be built before linking AsirikuyFrameworkAPI:**

```bash
cd vendor/MiniXML
./configure
make
cd ../../..
```

This will create `libmxml.a` and `libmxml.dylib` in the `vendor/MiniXML` directory.

### Step 3: Build Dependencies

**AsirikuyFrameworkAPI depends on TradingStrategies and other components. The build system will automatically build dependencies, but you can also build them explicitly:**

```bash
cd build/gmake
export BOOST_ROOT=/Users/andym/homebrew/opt/boost

# Build all dependencies (TradingStrategies, AsirikuyCommon, Log, etc.)
# The make command will automatically build dependencies when building AsirikuyFrameworkAPI
make TradingStrategies config=debug64
make AsirikuyCommon config=debug64
make Log config=debug64
# ... (other dependencies)
```

### Step 4: Build AsirikuyFrameworkAPI

**Debug Build (64-bit):**
```bash
cd build/gmake
export BOOST_ROOT=/Users/andym/homebrew/opt/boost
make AsirikuyFrameworkAPI config=debug64
```

**Release Build (64-bit):**
```bash
cd build/gmake
export BOOST_ROOT=/Users/andym/homebrew/opt/boost
make AsirikuyFrameworkAPI config=release64
```

**Build Both Configurations:**
```bash
cd build/gmake
export BOOST_ROOT=/Users/andym/homebrew/opt/boost
make AsirikuyFrameworkAPI config=debug64
make AsirikuyFrameworkAPI config=release64
```

### Step 5: Build UnitTests (Test Executable)

The `UnitTests` project builds a console application that runs all unit tests for AsirikuyFrameworkAPI and other framework components.

**Debug Build (64-bit):**
```bash
cd build/gmake
export BOOST_ROOT=/Users/andym/homebrew/opt/boost
make UnitTests config=debug64
```

**Release Build (64-bit):**
```bash
cd build/gmake
export BOOST_ROOT=/Users/andym/homebrew/opt/boost
make UnitTests config=release64
```

**Build Both Configurations:**
```bash
cd build/gmake
export BOOST_ROOT=/Users/andym/homebrew/opt/boost
make UnitTests config=debug64
make UnitTests config=release64
```

**Note:** The UnitTests executable will automatically run after building (via postbuildcommands). To run manually:

```bash
# Debug
cd bin/gmake/x64/Debug
DYLD_LIBRARY_PATH=.:./lib:../vendor/ChartDirector/lib/universal ./UnitTests

# Release
cd bin/gmake/x64/Release
DYLD_LIBRARY_PATH=.:./lib:../vendor/ChartDirector/lib/universal ./UnitTests
```

### Step 6: Build Both Libraries Together

To build both AsirikuyFrameworkAPI and UnitTests in one command:

**Debug Build:**
```bash
cd build/gmake
export BOOST_ROOT=/Users/andym/homebrew/opt/boost
make AsirikuyFrameworkAPI UnitTests config=debug64
```

**Release Build:**
```bash
cd build/gmake
export BOOST_ROOT=/Users/andym/homebrew/opt/boost
make AsirikuyFrameworkAPI UnitTests config=release64
```

**Build All (Both Configurations):**
```bash
cd build/gmake
export BOOST_ROOT=/Users/andym/homebrew/opt/boost
make AsirikuyFrameworkAPI UnitTests config=debug64
make AsirikuyFrameworkAPI UnitTests config=release64
```

### Build Output

After a successful build, the output files will be located at:

**AsirikuyFrameworkAPI Library:**
- **Debug:** `bin/gmake/x64/Debug/libAsirikuyFrameworkAPI.dylib`
- **Release:** `bin/gmake/x64/Release/libAsirikuyFrameworkAPI.dylib`

**UnitTests Executable:**
- **Debug:** `bin/gmake/x64/Debug/UnitTests`
- **Release:** `bin/gmake/x64/Release/UnitTests`

**Note:** 
- The library is built directly in the `bin/gmake/x64/Debug/` directory (not in a `lib/` subdirectory)
- The UnitTests executable is built in the same directory as the library
- UnitTests automatically runs after building (via postbuildcommands)

## Verification

### Check AsirikuyFrameworkAPI Library

```bash
# Verify file exists and check size
ls -lh bin/gmake/x64/Debug/libAsirikuyFrameworkAPI.dylib
ls -lh bin/gmake/x64/Release/libAsirikuyFrameworkAPI.dylib

# Verify file type
file bin/gmake/x64/Debug/libAsirikuyFrameworkAPI.dylib

# Check dependencies
otool -L bin/gmake/x64/Debug/libAsirikuyFrameworkAPI.dylib
```

### Check Exported Symbols

```bash
# List exported symbols (should include initInstanceC, c_runStrategy, etc.)
nm -gU bin/gmake/x64/Debug/libAsirikuyFrameworkAPI.dylib | head -20

# Count exported symbols
nm -gU bin/gmake/x64/Debug/libAsirikuyFrameworkAPI.dylib | wc -l
```

### Check UnitTests Executable

```bash
# Verify file exists and check size
ls -lh bin/gmake/x64/Debug/UnitTests
ls -lh bin/gmake/x64/Release/UnitTests

# Verify file type
file bin/gmake/x64/Debug/UnitTests

# Check dependencies
otool -L bin/gmake/x64/Debug/UnitTests

# Run tests manually (if not auto-run)
cd bin/gmake/x64/Debug
DYLD_LIBRARY_PATH=.:./lib:../../vendor/ChartDirector/lib/universal ./UnitTests
```

### Test Loading with Python

```python
import ctypes
import os

# Get absolute path to the library
lib_path = os.path.abspath('bin/gmake/x64/Debug/libAsirikuyFrameworkAPI.dylib')

# Load the library (matching Windows: AsirikuyFrameworkAPI.dll)
lib = ctypes.CDLL(lib_path)

print('Library loaded successfully!')
print(f'Library handle: {lib}')

# Test calling a function (e.g., getFrameworkVersion)
lib.getFrameworkVersion.argtypes = [ctypes.POINTER(ctypes.c_int), 
                                      ctypes.POINTER(ctypes.c_int), 
                                      ctypes.POINTER(ctypes.c_int)]
major = ctypes.c_int()
minor = ctypes.c_int()
bugfix = ctypes.c_int()
lib.getFrameworkVersion(ctypes.byref(major), ctypes.byref(minor), ctypes.byref(bugfix))
print(f'Framework version: {major.value}.{minor.value}.{bugfix.value}')
```

## Build Differences: Debug vs Release

| Feature | Debug | Release |
|--------|-------|---------|
| **Compiler Flags** | `-g -DDEBUG -D_DEBUG` | `-Os -O2 -fomit-frame-pointer` |
| **Optimization** | `-O2` | `-Os -O2` (aggressive) |
| **Debug Symbols** | Yes | No |
| **File Size (AsirikuyFrameworkAPI)** | ~371KB | ~350KB (estimated) |
| **File Size (UnitTests)** | Larger (with debug info) | Smaller (optimized) |
| **Use Case** | Development, debugging, testing | Production, performance |

## Platform-Specific Notes

### macOS (.dylib)

- **Extension:** `.dylib`
- **Linker Flags:** `-dynamiclib -install_name @rpath/libtrading_strategies.dylib`
- **Architecture:** ARM64 (Apple Silicon) or x86_64 (Intel)

### Linux (.so)

To build for Linux, run premake4 and make on a Linux system:
- **Extension:** `.so`
- **Linker Flags:** `-shared`

### Windows (.dll)

To build for Windows, use Visual Studio with the generated project files:
- **Extension:** `.dll`
- **Defines:** `TRADING_STRATEGIES_EXPORTS`

## Troubleshooting

### Common Issues

1. **"premake4 not found"**
   - Use the Docker wrapper: `./premake4-docker.sh gmake`
   - Or install premake4 natively (see `INSTALL_PREMAKE4.md`)

2. **"BOOST_ROOT not set"**
   ```bash
   export BOOST_ROOT=/Users/andym/homebrew/opt/boost
   ```

3. **"error: invalid argument '-std=c++11' not allowed with 'C'"**
   - This has been fixed in the Makefiles. If you see this, regenerate Makefiles with premake4.

4. **"symbol(s) not found for architecture arm64"**
   - Ensure all dependencies are built for the same architecture
   - Check that library paths are correct in the Makefile

5. **Duplicate symbol errors**
   - Functions should be defined only once or made `static` if used in a single file
   - Check for duplicate function definitions across source files
   - **Note:** All duplicate symbols in TradingStrategies have been resolved

6. **"mxml-private.h file not found"**
   - Build MiniXML first: `cd vendor/MiniXML && ./configure && make`
   - Ensure `vendor/MiniXML/config.h` exists (generated by `configure`)

7. **"library 'boost_system' not found"**
   - This has been resolved by removing the non-existent `boost_system` dependency
   - The build now uses only `boost_thread`, `boost_chrono`, `boost_date_time`, and `boost_atomic`

8. **Pantheios/STLSoft compilation errors**
   - **Resolved:** All Pantheios logging has been replaced with standard `fprintf(stderr, ...)` calls
   - Pantheios dependencies have been removed from the build system

### Clean Build

To clean build artifacts:

```bash
cd build/gmake
make clean config=debug64
make clean config=release64
```

Or clean specific projects:

```bash
cd build/gmake
make AsirikuyFrameworkAPI clean config=debug64
make UnitTests clean config=debug64
```

Or remove the entire build directory:

```bash
rm -rf bin/gmake tmp/gmake build/gmake
```

## Dependencies

### AsirikuyFrameworkAPI Dependencies

The AsirikuyFrameworkAPI library depends on:

- **TradingStrategies** - Trading strategy implementations (component library)
- **AsirikuyCommon** - Common utilities and data structures
- **Log** - Logging functionality
- **OrderManager** - Order management
- **AsirikuyTechnicalAnalysis** - Technical analysis functions
- **AsirikuyEasyTrade** - Trading operations
- **NTPClient** - Network time protocol client
- **TALib_common, TALib_abstract, TALib_func** - Technical Analysis Library
- **SymbolAnalyzer** - Symbol analysis
- **MiniXML** - XML parsing library (source in `vendor/MiniXML`, built during build)
- **Boost** - C++ libraries (thread, chrono, date_time, atomic)
- **cURL** - HTTP client library

All dependencies must be built before building AsirikuyFrameworkAPI.

### UnitTests Dependencies

The UnitTests executable depends on:
- **AsirikuyFrameworkAPI** - Main framework library (must be built first)
- **AsirikuyTechnicalAnalysis** - Technical analysis functions
- **AsirikuyEasyTrade** - Trading operations
- **TradingStrategies** - Trading strategy implementations
- **SymbolAnalyzer** - Symbol analysis
- **OrderManager** - Order management
- **Boost** - C++ libraries (unit_test_framework, serialization)
- All vendor libraries that AsirikuyFrameworkAPI depends on

**Note:** 
- **Pantheios has been removed** - All logging now uses standard `fprintf(stderr, ...)` calls
- On Windows, AsirikuyFrameworkAPI also depends on many vendor libraries (ChartDirector, DevIL, etc.), but on macOS/Linux some of these may be optional or replaced with system libraries
- MiniXML source is included in `vendor/MiniXML` and must be built before linking (run `make` in that directory)
- UnitTests will automatically build all required dependencies when you build it

## Project Structure

```
AutoBBS/
├── core/
│   └── TradingStrategies/
│       ├── src/              # Source files
│       ├── include/          # Header files
│       └── premake4.lua      # Build configuration
├── build/
│   └── gmake/                # Generated Makefiles
├── bin/
│   └── gmake/                # Build output
│       └── x64/
│           ├── Debug/         # Debug builds (libAsirikuyFrameworkAPI.dylib)
│           └── Release/      # Release builds (libAsirikuyFrameworkAPI.dylib)
└── tmp/
    └── gmake/                # Temporary build files
```

## Additional Resources

- See `INSTALL_PREMAKE4.md` for Premake4 installation instructions
- Check individual project `premake4.lua` files for project-specific build settings

## Notes

- **Consistency with Windows:** On Windows, Python/MQL/CTester call `AsirikuyFrameworkAPI.dll`. On macOS, they should call `libAsirikuyFrameworkAPI.dylib` for consistency.
- **Component vs Main Library:** `TradingStrategies` is a component library used internally. The main entry point is `AsirikuyFrameworkAPI`.
- **Pantheios Removed:** All Pantheios logging has been replaced with standard `fprintf(stderr, ...)` calls throughout the codebase. This eliminates STLSoft/Pantheios compilation issues on macOS.
- **MiniXML:** The MiniXML library source is included in `vendor/MiniXML` and must be built before linking (`cd vendor/MiniXML && ./configure && make`).
- **Build Status:** The library successfully builds on macOS ARM64. All source files compile, and linking succeeds with proper dependencies.
- All duplicate symbol issues in TradingStrategies have been resolved
- The library is ready for use with Python `ctypes`, MQL, and CTester

## Python Integration

**For Python code that should match Windows behavior:**

Python/MQL/CTester should call `AsirikuyFrameworkAPI`, not `TradingStrategies` directly:

```python
import ctypes

# Windows: AsirikuyFrameworkAPI.dll
# macOS: libAsirikuyFrameworkAPI.dylib (or AsirikuyFrameworkAPI.dylib)
lib = ctypes.CDLL('libAsirikuyFrameworkAPI.dylib')

# Use the CTester interface (c_runStrategy) or MQL interface
# This matches what Windows uses
```

**Note:** There is also a newer Python-specific API (`TradingStrategiesPythonAPI`) that calls `libtrading_strategies.dylib` directly, but for consistency with MQL/CTester, use `AsirikuyFrameworkAPI`.

