# Windows Build Status - window-build Branch

## Overview
This document tracks the efforts to build the AutoBBS project on Windows. The project is primarily designed for Linux/Unix, which presents cross-platform compatibility challenges.

## Build Achievements

### ✅ Completed
1. **Premake4 project generation** - Successfully generated VS2010 solution files
   - 14 projects configured (3 TALib + 11 core projects)
   - Location: `build/vs2010/AsirikuyFramework.sln`

2. **Vendor library compilation**
   - **TALib_common.lib** ✓ Built successfully
   - **TALib_abstract.lib** ✓ Built successfully  
   - **TALib_func.lib** ✓ Built successfully
   - Located in: `vendor/TALib/`

3. **Core library compilation** (partial)
   - **AsirikuyCommon.lib** ✓ (some source files compiled)
   - **Log.lib** ✓ (some source files compiled)
   - **SymbolAnalyzer.lib** ✓ (some source files compiled)
   - **OrderManager.lib** ✓ (some source files compiled)
   - **AsirikuyTechnicalAnalysis.lib** ✓ (some source files compiled)

4. **Toolchain setup**
   - Installed MSVC v141 (VS2017 compatible toolset) in VS2022
   - Configured for Debug x64 platform
   - Windows SDK 10.0.26100.0 support

## Current Issues

### ❌ Platform-specific incompatibilities

#### 1. Unix-specific headers
**Status:** Blocking core compilation  
**Issue:** Core source files include Unix-only headers
```
fatal error C1083: Cannot open include file: 'sys/time.h'
```
**Files affected:** `core/AsirikuyCommon/src/AsirikuyLogger.c` and others

**Root cause:** The codebase was written for Linux/Unix and uses POSIX APIs

#### 2. Deprecated compiler flags
**Status:** Warning (non-blocking)
```
cl : Command line warning D9002: ignoring unknown option '-std=c++11'
cl : Command line warning D9035: option 'Gm' has been deprecated
```
**Fix:** Need to update premake4.lua to use Windows-compatible flags

#### 3. NTPClient Boost incompatibility
**Status:** Blocking compilation  
**Issue:** NTPClient uses newer Boost APIs (io_context, steady_timer) not available in Boost 1.49.0
```
error C2039: 'io_context': is not a member of 'boost::asio'
error C2039: 'steady_timer': is not a member of 'boost::asio'
```
**Root cause:** Code was updated for modern Boost but dependency is still on 1.49.0

## Configuration Changes Made

### 1. `premake4.lua`
- Made vendor includes conditional (only include if directory exists)
- Added check for GAUL vendor (currently missing)

### 2. `scripts/run_build.ps1`
- Updated to use `premake4-minimal.lua` option
- Added BOOST_ROOT environment variable handling

### 3. `vendor/TALib/premake4.lua`
- Added include directories for proper header resolution
- Added project dependencies (TALib_abstract depends on TALib_common, etc.)
- Paths added:
  - `c/include`
  - `c/src/ta_common`
  - `c/src/ta_abstract`
  - `c/src/ta_abstract/frames`
  - `c/src/ta_func`

## Build Command

To reproduce the current Windows build state:

```powershell
$env:BOOST_ROOT = "e:\workspace\boost_1_49_0"
& "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" `
  "e:\AutoBBS\build\vs2010\AsirikuyFramework.sln" `
  /p:Configuration=Debug `
  /p:Platform=x64 `
  /p:PlatformToolset=v141 `
  /p:WindowsTargetPlatformVersion=10.0.26100.0 `
  /v:minimal /m
```

## Recommendations

### Short-term: Fix compiler warnings
1. Remove `-std=c++11` flag from Unix-specific projects
2. Use `/std:c++11` or `/std:c++14` for MSVC

### Medium-term: Port Unix headers
1. Replace `sys/time.h` with `<chrono>` (C++ standard)
2. Replace POSIX APIs with Windows equivalents or platform-independent solutions
3. Create platform-specific wrapper headers

### Long-term: Use proper build target
**Recommended approach:** Build on Linux/WSL instead
- The project is designed for Unix/Linux
- Docker support available via `Dockerfile.premake4`
- WSL provides native Linux environment on Windows

Alternatively:
- Use CMake for cross-platform builds
- Maintain separate Windows compatibility layer
- Upgrade dependencies to modern versions

## Resources

- Boost 1.49.0 documentation: Limited API support
- VS2017 toolset (v141): Good compatibility with legacy code
- Windows 10 SDK: Modern headers may conflict with old code

## Next Steps

1. **Option A (Recommended):** Build on WSL/Docker
2. **Option B:** Create platform abstraction layer for Unix functions
3. **Option C:** Upgrade Boost to 1.60+ and fix NTPClient APIs

