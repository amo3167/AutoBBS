# Windows DLL Build Specification

## Overview
Complete the Windows build of AsirikuyFrameworkAPI.dll for MT4 integration by fixing platform-specific incompatibilities.

## Current Status
- ✅ Premake4 project generation complete
- ✅ TALib libraries compiled (common, abstract, func)
- ⚠️ Core projects partially compiled
- ❌ Build failures due to Unix-specific code
- ❌ NTPClient incompatible with Boost 1.49.0

## Build Objectives

### Primary Goal
Produce `AsirikuyFrameworkAPI.dll` (x64, Release) for MT4 terminal use

### Secondary Goals
1. All core libraries compile successfully
2. No platform-specific code remains
3. Build is reproducible on any Windows system with VS2022

## Technical Requirements

### Build Environment
- **OS:** Windows 10 or later
- **Compiler:** Visual Studio 2022 Community with v141 (VS2017) toolset
- **Boost:** 1.49.0 (already present at e:\workspace\boost_1_49_0)
- **Build System:** Premake4 + MSBuild
- **Output:** Release x64 DLL

### Build Output Structure
```
bin/vs2010/x64/Release/
├── AsirikuyFrameworkAPI.dll    (Main DLL for MT4)
├── AsirikuyFrameworkAPI.lib    (Import library)
├── AsirikuyCommon.lib
├── Log.lib
├── SymbolAnalyzer.lib
├── AsirikuyEasyTrade.lib
├── AsirikuyTechnicalAnalysis.lib
├── OrderManager.lib
├── TradingStrategies.lib
├── TALib_common.lib
├── TALib_abstract.lib
├── TALib_func.lib
└── MiniXML.lib
```

## Code Issues to Fix

### Issue 1: Unix Headers in AsirikuyLogger.c
**Severity:** CRITICAL (blocking core build)  
**File:** `core/AsirikuyCommon/src/AsirikuyLogger.c`  
**Problem:** Uses `sys/time.h` (Unix only)

**Current Code:**
```c
#include <sys/time.h>
```

**Fix:** Replace with C++ standard library
```c
#include <ctime>
#include <chrono>
```

**Impact:** Affects AsirikuyCommon library compilation

---

### Issue 2: Unix Headers in Other Core Files
**Severity:** CRITICAL (blocking core build)  
**Files:**
- `core/SymbolAnalyzer/src/*.c`
- `core/Log/src/*.c`
- Other Unix-specific includes

**Problem:** POSIX-specific headers not available on Windows

**Fix:** Use Windows APIs or C++ standard library alternatives

**Common Replacements:**
```c
// Old:
#include <unistd.h>        // Unix API
#include <sys/time.h>      // Unix time
#include <pthread.h>       // POSIX threads

// New (Windows):
#include <windows.h>       // Windows API
#include <ctime>           // Standard time
#include <thread>          // C++ threads
```

---

### Issue 3: NTPClient Boost 1.49.0 Incompatibility
**Severity:** HIGH (blocks AsirikuyFrameworkAPI)  
**File:** `core/NTPClient/include/NTPClient.hpp`  
**Problem:** Uses Boost 1.60+ API (io_context, steady_timer)

**Current Code:**
```cpp
boost::asio::io_context io_context_;
boost::asio::steady_timer deadline_;
```

**Issue:** These don't exist in Boost 1.49.0:
- `io_context` was called `io_service`
- `steady_timer` was called `deadline_timer`

**Fix Option A: Use older Boost APIs**
```cpp
boost::asio::io_service io_context_;  // Works in 1.49.0
boost::asio::deadline_timer deadline_; // Works in 1.49.0
```

**Fix Option B: Disable NTPClient for Windows**
```cpp
#ifdef _WIN32
  // Use Windows alternative or mock
#else
  // Use Boost ASIO
#endif
```

**Fix Option C: Create compatibility layer**
```cpp
// Wrapper in boost_compat.h
namespace boost_compat {
  #if BOOST_VERSION < 106000
    using io_context = boost::asio::io_service;
    using steady_timer = boost::asio::deadline_timer;
  #else
    using io_context = boost::asio::io_context;
    using steady_timer = boost::asio::steady_timer;
  #endif
}
```

---

### Issue 4: Compiler Flags
**Severity:** MEDIUM (warnings only, build continues)  
**File:** `premake4.lua`  
**Problem:** Unix-specific compiler flags (-std=c++11 for GCC)

**Current Code:**
```lua
buildoptions{"-std=c++11"}
```

**Fix:** Make platform-specific
```lua
configuration{"not windows"}
  buildoptions{"-std=c++11"}
configuration{"windows"}
  -- MSVC uses /std:c++11 or /std:c++14
```

---

## Dependency Chain

```
AsirikuyFrameworkAPI.dll (FINAL)
├── NTPClient
├── AsirikuyCommon ✓ (partially working)
├── Log ✓ (partially working)
├── SymbolAnalyzer ✓ (partially working)
├── AsirikuyTechnicalAnalysis ✓ (partially working)
├── OrderManager ✓ (partially working)
├── TradingStrategies
├── AsirikuyEasyTrade
├── MiniXML
├── curl
├── TALib_common ✓ (built)
├── TALib_abstract ✓ (built)
└── TALib_func ✓ (built)
```

**Critical Path:**
1. Fix AsirikuyCommon (needed by all)
2. Fix NTPClient (needed by AsirikuyFrameworkAPI)
3. Fix all dependencies
4. Build AsirikuyFrameworkAPI

## Build Process

### Step 1: Generate Projects
```powershell
$env:BOOST_ROOT = "e:\workspace\boost_1_49_0"
.\premake4.exe --file=premake4.lua vs2010
```

### Step 2: Build Release Configuration
```powershell
$msbuild = "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"

& $msbuild "build\vs2010\AsirikuyFramework.sln" `
  /p:Configuration=Release `
  /p:Platform=x64 `
  /p:PlatformToolset=v141 `
  /p:WindowsTargetPlatformVersion=10.0.26100.0 `
  /v:detailed `
  /m
```

### Step 3: Verify Output
```powershell
Get-ChildItem "bin\vs2010\x64\Release\" -Filter "*.dll"
Get-ChildItem "bin\vs2010\x64\Release\" -Filter "*.lib"
```

## Testing & Validation

### Unit 1: DLL Exists and Loads
```powershell
$dll = "e:\AutoBBS\bin\vs2010\x64\Release\AsirikuyFrameworkAPI.dll"
Test-Path $dll
Add-Type -Path $dll  # Should load without error
```

### Unit 2: Exports Present
```cpp
// Verify all required functions exist
initInstanceMQL4
initInstanceMQL5
mql4_runStrategy
mql5_runStrategy
// ... (all functions from AsirikuyFrameworkAPI.def)
```

### Unit 3: MT4 Integration
Load DLL in MT4 terminal and verify:
- Expert advisor can initialize
- Strategy can run
- No runtime errors

## Risk Assessment

| Risk | Impact | Mitigation |
|------|--------|-----------|
| Unix headers in other files | HIGH | Systematic search and replace |
| Boost API mismatch | MEDIUM | Use compatibility layer or upgrade Boost |
| Compiler flag issues | LOW | Test build, adjust flags |
| Missing vendor libs | LOW | Already compiled (TALib) |

## Success Criteria

✓ All source files compile without errors  
✓ AsirikuyFrameworkAPI.dll generated successfully  
✓ DLL loads in Windows without runtime errors  
✓ All exported functions accessible from MT4  
✓ No platform-specific code in source tree  
✓ Build reproducible on any Windows system  

## Timeline Estimate

| Task | Effort | Duration |
|------|--------|----------|
| Identify all Unix headers | 30 min | Grep search |
| Fix AsirikuyLogger | 15 min | Header replacement |
| Fix other core files | 1-2 hours | File-by-file fixes |
| Fix NTPClient | 30 min | Boost compatibility |
| Fix compiler flags | 15 min | Premake4 update |
| Clean rebuild | 30 min | Full build |
| Testing & validation | 1 hour | Functional tests |
| **Total** | **4-5 hours** | **Parallel work possible** |

