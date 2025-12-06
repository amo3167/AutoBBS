# AutoBBS Build Scripts

## Quick Start

### For Daily Development
```batch
REM Fast incremental build (reuses cached objects)
.\scripts\build-parallel-simple.bat

REM Full clean rebuild
.\scripts\build-parallel-simple.bat clean
```

### For Release Packaging
```powershell
# After building, package artifacts into release folder
powershell -ExecutionPolicy Bypass -File .\scripts\create-release.ps1

# Or specify custom output directory
powershell -ExecutionPolicy Bypass -File .\scripts\create-release.ps1 -OutputDir "D:\my-releases"
```

This creates: `releases/AsirikuyFramework-{timestamp}/` with DLLs, libraries, headers, and docs organized for distribution.

### For Initial Setup
```batch
REM One-time vendor library build (only needed once)
.\scripts\build-vendors-once.bat

REM Sequential build (when you need predictable output or debugging)
.\scripts\build-sequential.bat
```

---

## Script Categories

### PRIMARY BUILD SCRIPTS (Use These)

#### `build-parallel-simple.bat`
- **Purpose**: Production build script with parallel compilation
- **Features**: 
  - Parallel Phase 1: 4 independent vendor libs
  - Parallel Phase 2: 4 independent core libs  
  - Sequential Phase 3: Dependent library chain
  - Uses MSBuild `/m` flag for true parallelism
- **Modes**:
  - `build-parallel-simple.bat` - Incremental (fast, ~2-3 min)
  - `build-parallel-simple.bat clean` - Full clean with vendor preservation (~4-5 min)
- **Status**: ✓ RECOMMENDED - Tested and working

#### `build-sequential.bat`
- **Purpose**: Simple, predictable sequential builds
- **Features**:
  - Builds one project at a time
  - No parallel execution
  - Best for debugging and understanding build issues
- **Build Time**: ~3-4 minutes (cached vendors), ~6-7 minutes (first build)
- **Status**: ✓ Tested and verified

#### `build-vendors-once.bat`
- **Purpose**: One-time vendor library initialization
- **Features**:
  - Builds TALib and MiniXML from source
  - Handles TALib_func special case (disables WholeProgramOptimization)
  - Skips rebuild if vendors already exist
- **Usage**: Run once at setup, then never again (unless vendors change)
- **Status**: ✓ Utility script

### RELEASE SCRIPTS

#### `create-release.ps1`
- **Purpose**: Package build artifacts into organized release folders with ZIP archive
- **Usage**: 
  - `powershell -ExecutionPolicy Bypass -File scripts\create-release.ps1`
  - `powershell -ExecutionPolicy Bypass -File scripts\create-release.ps1 -OutputDir "D:\releases"`
- **Output**: Creates in `releases/AsirikuyFramework-{timestamp}/`:
  - **Folder structure**:
    - `bin/` - DLLs and EXEs
    - `lib/` - Static libraries
    - `include/` - Header files (106+ headers from existing releases)
    - `docs/` - Documentation markdown files
    - `MANIFEST.json` - Build metadata
    - `README.txt` - Release information
  - **ZIP archive**: `AsirikuyFramework-{timestamp}.zip` (~0.7 MB) for easy distribution
- **Status**: ✓ Production - manual release packaging

### HELPER SCRIPTS

#### `update-toolset.ps1`
- **Purpose**: Update all .vcxproj files to use VS2022 toolset (v143)
- **Usage**: Called automatically by build scripts during premake4 regeneration
- **Manual usage**: `powershell -File scripts\update-toolset.ps1`

### CLEANUP/MAINTENANCE SCRIPTS

#### `clean-core.ps1`
- **Purpose**: Remove build artifacts while preserving source
- **Usage**: `powershell -File scripts\clean-core.ps1`

#### `migrate-artifacts.ps1`
- **Purpose**: Move lib/idb/pdb files to bin/release folder
- **Status**: Historical - already executed

#### `remove-unused-vendors.ps1`
- **Purpose**: Identify and remove unused vendor libraries
- **Status**: Historical - already executed

#### `upgrade_boost.ps1`
- **Purpose**: Update Boost library version
- **Usage**: `powershell -File scripts\upgrade_boost.ps1`

### SYSTEM TESTING SCRIPTS

#### `run_msbuild_*.bat`
- `run_msbuild_vs2022.bat` - Test VS2022 MSBuild
- `run_msbuild_retarget.bat` - Retarget projects to current toolset
- `run_msbuild_stlsoft_test.bat` - Test STLSoft library
- **Purpose**: Diagnostic tools for build system issues

#### `test_ntpclient.bat`
- **Purpose**: Test NTP client component
- **Usage**: For component-level testing

#### `list_windows_kits.bat`
- **Purpose**: List installed Windows SDK versions
- **Usage**: Diagnostic tool for compiler/SDK issues

### MISCELLANEOUS

#### `copy_strategy_results.sh`
- **Purpose**: Copy strategy backtest results (shell script)
- **Platform**: Linux/Git Bash

#### `README_COPY_RESULTS.md`
- **Purpose**: Documentation for copying strategy results

---

## Build System Architecture

### Dependency Graph
```
Vendor Libraries (parallel):
├── TALib_common
├── TALib_abstract
├── TALib_func (special: /p:WholeProgramOptimization=false)
└── MiniXML

Independent Core Libraries (parallel):
├── AsirikuyCommon
├── Log
├── SymbolAnalyzer
└── AsirikuyTechnicalAnalysis

Dependent Chain (sequential):
├── OrderManager (depends on: AsirikuyCommon, Log)
├── AsirikuyEasyTrade (depends on: OrderManager, AsirikuyCommon)
├── TradingStrategies (depends on: AsirikuyEasyTrade, SymbolAnalyzer)
└── AsirikuyFrameworkAPI.dll (depends on: TradingStrategies, all libs)
```

### Key Optimizations
- **Vendor Library Caching**: Automatically backs up and restores vendor libs during clean
- **Dependency Optimization**: Uses `/p:BuildProjectReferences=false` to prevent cascading rebuilds
- **Toolset Management**: Automatically updates projects to v143 (VS2022) toolset
- **Parallel Compilation**: MSBuild `/m` flag for multi-core utilization

---

## Common Tasks

### First Time Setup
```batch
REM Clone repo, navigate to AutoBBS folder
cd AutoBBS

REM One-time vendor build
.\scripts\build-vendors-once.bat

REM Full parallel build
.\scripts\build-parallel-simple.bat clean
```

### Daily Development Build
```batch
REM Fast incremental parallel build
.\scripts\build-parallel-simple.bat
```

### Clean Rebuild
```batch
REM Full clean with vendor preservation
.\scripts\build-parallel-simple.bat clean
```

### Debug Specific Issue
```batch
REM Sequential build for clear error output
.\scripts\build-sequential.bat
```

### Update Project Files After premake4 Changes
```batch
REM Regenerate and update toolset
powershell -File scripts\update-toolset.ps1
```

---

### Removed Scripts (Why)

The following temporary/experimental scripts were deleted for cleanliness:
- `build-parallel.bat` - First iteration with syntax issues
- `build-parallel-v2.bat` - Second iteration incomplete
- `build-parallel-final.bat` - Abandoned experiment
- `build-parallel-clean.bat` - Superseded by build-parallel-simple.bat
- `build-clean.bat` - Basic version (replaced by build-parallel-simple.bat)
- `build-orchestrate.ps1` - 537-line PowerShell experiment (never used)
- `build-release.ps1` - 520-line PowerShell experiment (never committed, incomplete)
- `build.ps1` - Early PowerShell attempt
- `organize-release.ps1` - Not needed
- `run_build.ps1` - Obsolete

**Note on Releases**: The `releases/` folder contains release packages. These were generated by the experimental `build-release.ps1` script (now deleted) or manually created. For future releases, the build outputs in `bin/vs2010/x64/Release/` are the primary artifacts. Organize them manually or create a simple packaging script if needed.

---

## Tips & Tricks

### Fastest Possible Build
```batch
REM Incremental parallel - only rebuilds changed files
.\scripts\build-parallel-simple.bat
```
Expected time: **2-3 minutes** (with cached vendors)

### Slowest But Cleanest Build
```batch
REM Full clean - wipes everything but preserves vendors
.\scripts\build-parallel-simple.bat clean
```
Expected time: **4-5 minutes**

### Debug Build Errors
```batch
REM Sequential output - easy to trace errors
.\scripts\build-sequential.bat
```

### Rebuild Only TradingStrategies
```batch
cd build\vs2010
MSBuild.exe AsirikuyFramework.sln /t:TradingStrategies ^
  /p:Configuration=Release /p:Platform=x64 ^
  /p:BuildProjectReferences=false /m
```

### Check What's Being Built
```batch
REM Run with verbose output
.\scripts\build-sequential.bat
```
Much clearer output than parallel version.

---

## Environment Setup

Required:
- Visual Studio 2022 Community (MSBuild 17.x)
- Premake4 4.3
- BOOST_ROOT set to your Boost installation

Optional:
- PowerShell 5.1+ (for cleanup scripts)

---

## Status Summary

| Script | Status | Usage |
|--------|--------|-------|
| build-parallel-simple.bat | ✓ Production | Daily builds |
| build-sequential.bat | ✓ Production | Debugging |
| build-vendors-once.bat | ✓ Utility | One-time setup |
| update-toolset.ps1 | ✓ Helper | Auto-called |
| clean-core.ps1 | ✓ Utility | Manual cleanup |
| upgrade_boost.ps1 | ✓ Utility | Boost updates |
| Test scripts | ✓ Utility | Diagnostics |

---

Last Updated: December 6, 2025
