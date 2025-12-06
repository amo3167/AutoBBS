# AsirikuyFramework Build & Release Configuration Guide

**Last Updated:** December 6, 2025  
**Status:** Comprehensive build system with multi-configuration support

⚠️ **IMPORTANT**: For current build instructions, see `scripts/README.md`. The build scripts below reference legacy scripts that no longer exist. Use `scripts/build-parallel-simple.bat` or `scripts/build-sequential.bat` instead.

---

## 📋 Overview

This document describes the complete build and release system for AsirikuyFramework, including:

- **Windows builds** (MSBuild via Visual Studio 2019/2022)
- **macOS/Linux builds** (make/gmake)
- **Release packaging** with organized directory structures
- **Multi-configuration builds** (Debug/Release × x32/x64)
- **Artifact management** (DLLs, LIBs, headers, documentation)

---

## 🏗️ Directory Structure

### Source Directories

```
AutoBBS/
├── core/                          # Core framework source
│   ├── AsirikuyFrameworkAPI/      # Main DLL (exports 37 functions)
│   ├── TradingStrategies/         # Strategy engine
│   ├── CTesterFrameworkAPI/       # Backtester framework
│   ├── AsirikuyCommon/            # Common utilities
│   ├── AsirikuyEasyTrade/         # Trading utilities
│   ├── AsirikuyTechnicalAnalysis/ # TA-Lib wrapper
│   ├── NTPClient/                 # NTP time sync (Unix/macOS only)
│   └── ...
├── vendor/                        # Third-party libraries
│   ├── TALib/                     # Technical Analysis
│   ├── Gaul/                      # Genetic algorithms
│   ├── MiniXML/                   # XML parsing
│   └── ...
├── build/                         # Generated build files (premake output)
│   └── vs2010/                    # Visual Studio solution
└── premake4.lua                   # Build configuration
```

### Output Directories

#### Flat Layout (default for convenience)
```
bin/
├── AsirikuyFrameworkAPI.dll       # Main DLL (681 KB)
├── AsirikuyFrameworkAPI.lib       # Import library
├── trading_strategies.lib         # Trading engine
├── UnitTests.exe                  # Test executable
└── ...
```

#### Structured Layout (recommended for releases)
```
bin/
├── Release/x64/
│   ├── AsirikuyFrameworkAPI.dll
│   ├── AsirikuyFrameworkAPI.lib
│   └── ...
├── Release/Win32/
│   └── ...
├── Debug/x64/
│   └── ...
└── Debug/Win32/
    └── ...
```

#### Release Package Structure
```
releases/
└── AsirikuyFramework-20251206-143022/
    ├── bin/                       # Executables & DLLs
    │   ├── AsirikuyFrameworkAPI.dll
    │   ├── CTesterFrameworkAPI.dll
    │   └── ...
    ├── lib/                       # Static libraries
    │   ├── trading_strategies.lib
    │   └── ...
    ├── include/                   # Headers
    │   ├── AsirikuyFrameworkAPI/
    │   ├── TradingStrategies/
    │   └── ...
    ├── docs/                      # Documentation
    │   ├── API.md
    │   ├── README.md
    │   └── ...
    ├── MANIFEST.json              # Release metadata
    └── RELEASE.json               # Build information
```

---

## 🛠️ Build Scripts

### 1. Windows Build (PowerShell)

#### Quick Start

For builds, use the build scripts in the `scripts/` folder:

```batch
REM Release build - fast incremental
.\scripts\build-parallel-simple.bat

REM Full clean rebuild
.\scripts\build-parallel-simple.bat clean
```

Build outputs appear in `bin/vs2010/x64/Release/`:
- `AsirikuyFrameworkAPI.dll` - Main DLL
- `.lib` files - Static/import libraries
- `.idb`, `.pdb` files - Debug info

#### Parameters
| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `-Config` | string | `release64` | Build configuration |
| `-BuildType` | string | `All` | Debug, Release, or All |
| `-Platform` | string | `all` | x32, x64, or all |
| `-Clean` | switch | false | Clean before building |
| `-Release` | switch | false | Create release package |
| `-OutputDir` | string | `./releases` | Release output directory |
| `-Verbose` | switch | false | Detailed output |

#### Configurations
| Configuration | Meaning |
|--------------|---------|
| `debug32` | 32-bit Debug build |
| `debug64` | 64-bit Debug build |
| `release32` | 32-bit Release build |
| `release64` | 64-bit Release build (default) |

### 2. macOS/Linux Build (Bash)

#### Quick Start
```bash
# Release build (x64)
./build-complete.sh release64

# Release with package
./build-complete.sh -r release64

# Clean build with verbose output
./build-complete.sh -c -v release64
```

#### Usage
```bash
./build-complete.sh [OPTIONS] [CONFIG] [PROJECT]
```

#### Options
| Option | Description |
|--------|------------|
| `-c, --clean` | Clean build directories |
| `-r, --release` | Create release package |
| `-v, --verbose` | Verbose output |
| `--no-vendor` | Skip vendor libraries |
| `-h, --help` | Show help |

### 3. Unified Build Orchestration (PowerShell)

The most comprehensive option for Windows:

```powershell
# Full build with release package
.\build-orchestrate.ps1 -Action all -AllConfigs -Parallel -OutputLayout structured

# Just release existing build
.\build-orchestrate.ps1 -Action release -OutputLayout flat

# Clean and verify
.\build-orchestrate.ps1 -Action clean -AllConfigs
.\build-orchestrate.ps1 -Action test
```

#### Actions
| Action | What It Does |
|--------|------------|
| `clean` | Remove build artifacts |
| `build` | Compile everything |
| `release` | Build + organize + package |
| `test` | Verify artifacts exist and report |
| `all` | Clean → build → organize → release → test |

#### Output Organization
| Option | Behavior |
|--------|----------|
| `flat` | All artifacts in `bin/` root |
| `structured` | Organized by config: `bin/Release/x64/` |
| `both` | Both layouts created |

---

## 📊 Build Artifacts

### Windows (x64 Release)

| Artifact | Type | Size | Location | Notes |
|----------|------|------|----------|-------|
| `AsirikuyFrameworkAPI.dll` | DLL | 681 KB | `bin/vs2010/x64/Release/` | Main API, 37 exports |
| `AsirikuyFrameworkAPI.lib` | Import Lib | 10.5 KB | `bin/vs2010/x64/Release/` | Linking support |
| `trading_strategies.lib` | Static Lib | 525 KB | `bin/vs2010/x64/Release/` | Strategy engine |
| `AsirikuyEasyTrade.lib` | Static Lib | 19 KB | `bin/vs2010/x64/Release/` | Trading utils (Windows stubs) |

### macOS/Linux (x64 Release)

| Artifact | Type | Location | Notes |
|----------|------|----------|-------|
| `libAsirikuyFrameworkAPI.so` | Shared | `bin/gmake/x64/Release/lib/` | Linux |
| `libAsirikuyFrameworkAPI.dylib` | Shared | `bin/gmake/x64/Release/lib/` | macOS |
| `libtrading_strategies.a` | Static | `bin/gmake/x64/Release/lib/` | Strategy engine |
| `libasiriuy_common.a` | Static | `bin/gmake/x64/Release/lib/` | Common utilities |
| `libntp_client.a` | Static | `bin/gmake/x64/Release/lib/` | NTP time sync (Unix) |

---

## 🔧 Configuration Options

### Environment Variables

Set before building:

```powershell
# Windows
$env:BOOST_ROOT = "e:\workspace\boost_1_49_0"

# macOS/Linux
export BOOST_ROOT=/Users/user/homebrew/opt/boost
export MAKE_JOBS=8
```

### Compiler Settings

Controlled by `premake4.lua`:

```lua
-- Windows
configuration{"windows"}
  buildoptions{"/Zc:noexceptTypes-"}  -- VS2022 compatibility
  defines{"NOMINMAX"}                 -- Prevent min/max macros

-- macOS
configuration{"macosx"}
  includedirs{"/opt/local/include"}
  defines{"DARWIN", "unix", "UNIX"}

-- Linux
configuration{"linux", "x64"}
  buildoptions{"-fPIC", "-O2", "-march=native"}
```

---

## 📦 Release Package Contents

When using `-Release` flag:

```
AsirikuyFramework-20251206-143022/
├── bin/
│   ├── AsirikuyFrameworkAPI.dll        # Main DLL (37 exports)
│   ├── CTesterFrameworkAPI.dll         # Backtester API
│   └── UnitTests.exe                   # Test suite
├── lib/
│   ├── AsirikuyFrameworkAPI.lib        # Import library
│   ├── trading_strategies.lib          # Strategy engine
│   └── AsirikuyEasyTrade.lib           # Easy trade utilities
├── include/
│   ├── AsirikuyFrameworkAPI/           # Main headers
│   ├── TradingStrategies/              # Strategy headers
│   ├── AsirikuyCommon/                 # Common headers
│   └── ...
├── docs/
│   ├── API.md                          # API documentation
│   ├── README.md                       # Getting started
│   ├── INSTALL_GUIDE.md                # Installation
│   └── ...
├── MANIFEST.json                       # Release metadata
└── README.txt                          # Quick reference
```

---

## 🚀 Typical Workflows

### Workflow 1: Development Build

```powershell
# Debug build for current development
.\build-release.ps1 -Config debug64 -Verbose
```

Output: `bin/vs2010/x64/Debug/`

### Workflow 2: Testing All Configurations

```powershell
# Build all configs in parallel
.\build-orchestrate.ps1 -Action build -AllConfigs -Parallel -Verbose
```

Creates: `bin/Debug/x32/`, `bin/Debug/x64/`, `bin/Release/x32/`, `bin/Release/x64/`

### Workflow 3: Release to Distribution

```powershell
# Full release build
.\build-orchestrate.ps1 -Action all -AllConfigs `
    -OutputLayout structured `
    -ReleaseOutput D:\releases
```

Output: Organized release package with all artifacts, headers, and docs

### Workflow 4: Quick CI/CD Pipeline

```bash
#!/bin/bash
# Linux/macOS CI build
./build-complete.sh -c release64      # Clean build
./build-complete.sh -r release64      # With release package
tar czf artifact.tar.gz releases/     # Archive for deployment
```

---

## 📋 Build Requirements

### Windows
- **Visual Studio 2019+** or **2022 Community Edition**
- **MSBuild 17.0+**
- **Boost 1.49+** (set `BOOST_ROOT`)
- **Windows SDK**

### macOS
- **Xcode Command Line Tools** (`xcode-select --install`)
- **Homebrew** (for Boost)
- **GNU Make**

```bash
brew install boost cmake
```

### Linux
- **GCC/Clang**
- **GNU Make**
- **Boost development libraries**

```bash
# Ubuntu/Debian
apt-get install build-essential libboost-all-dev cmake
```

---

## 🔍 Troubleshooting

### Build Fails with "BOOST_ROOT not set"

```powershell
# Windows
$env:BOOST_ROOT = "C:\path\to\boost"

# Or permanently
[Environment]::SetEnvironmentVariable("BOOST_ROOT", "C:\path\to\boost", "User")
```

### premake4 Not Found

```powershell
# Download from: https://premake.github.io/download.html
# Place premake4.exe in repo root
```

### Old Artifacts in Output

Use `-Clean` flag:
```powershell
.\build-release.ps1 -Config release64 -Clean
```

### Parallel Build Issues

If experiencing issues:
```powershell
# Run serially (removes /m flag)
.\build-release.ps1 -Config release64 -Verbose
```

---

## 📚 Output Examples

### Build Report (auto-generated)

```markdown
# Build Report
Generated: 2025-12-06 14:30:22

## Summary
- Total Configurations: 4
- Successful: 4
- Failed: 0

## Details
- debug32: ✅ PASSED
- debug64: ✅ PASSED
- release32: ✅ PASSED
- release64: ✅ PASSED

## Artifacts
- AsirikuyFrameworkAPI.dll (681 KB)
- trading_strategies.lib (525 KB)
- ...
```

### Log Files

- `build-report.md` - Summary of last build
- `build-{timestamp}.log` - Detailed build log
- `releases/AsirikuyFramework-{timestamp}/MANIFEST.json` - Release metadata

---

## 🔄 Continuous Integration

### GitHub Actions Example

```yaml
name: Build and Release

on: [push, pull_request]

jobs:
  build:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v2
      - name: Build Release
        run: |
          $env:BOOST_ROOT = "..."
          .\build-orchestrate.ps1 -Action all -AllConfigs -Parallel
      - name: Upload Artifacts
        uses: actions/upload-artifact@v2
        with:
          name: AsirikuyFramework
          path: releases/
```

---

## 📝 Notes

- **Windows DLL**: x64 PE32+, 37 exported functions
- **Linux/macOS**: Full functionality including NTP client
- **Stub Libraries**: Windows stubs provided for curl (EasyTrade) and Boost ASIO (NTP)
- **Release Packages**: Include headers, docs, and all artifacts needed for integration

---

**For more information, see:**
- `docs/WINDOWS_BUILD_STATUS.md` - Windows build status
- `docs/WINDOWS_DLL_BUILD_TASKS.md` - Build task tracking
- `specs/WINDOWS_DLL_BUILD_TASKS.md` - Detailed specifications
