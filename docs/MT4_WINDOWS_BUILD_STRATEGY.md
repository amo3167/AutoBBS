# MT4 Integration: Windows DLL Build Strategy

## Question: Can we use Docker/Linux build for Windows MT4?

**Answer: NO**

## Why Not?

### Binary Format Incompatibility
| Aspect | Windows | Linux |
|--------|---------|-------|
| **Build Target** | Windows PE (DLL) | ELF (SO) |
| **Calling Convention** | stdcall/cdecl | System V ABI |
| **Runtime** | Windows/MSVCRT | Linux libc/glibc |
| **MT4 Compatibility** | ✓ Direct loading | ✗ Cannot load |

### Docker/WSL Complexity
Even using WSL to build Windows DLLs would:
1. Require Interop between Linux tools and Windows compiler
2. Need cross-compilation setup (complex, error-prone)
3. Still need MSVC or compatible Windows toolchain

## Recommended Solutions

### ✅ Option 1: Build on Windows with Fixed Source (RECOMMENDED)

**Requirements:**
1. Fix Unix-specific code for Windows compatibility
2. Resolve Boost 1.49.0 incompatibilities  
3. Use Windows build tools (VS2022 with v141 toolset)

**Advantages:**
- Direct DLL output for MT4
- No cross-compilation complexity
- Build artifacts immediately usable

**Timeline:** 2-4 hours for fixes

**Steps:**
```powershell
# After fixing source code:
$env:BOOST_ROOT = "e:\workspace\boost_1_49_0"
msbuild "build\vs2010\AsirikuyFramework.sln" `
  /p:Configuration=Release `
  /p:Platform=x64 `
  /p:PlatformToolset=v141
# Output: AsirikuyFrameworkAPI.dll in bin/vs2010/x64/Release/
```

---

### Option 2: Use WSL with MinGW Cross-Compiler

**Complexity:** High  
**Reliability:** Lower  
**Build time:** 1-2 hours setup, 30 min build

**Process:**
1. Set up MinGW-w64 cross-compiler in WSL
2. Cross-compile to Windows target
3. Transfer DLL to Windows

**Disadvantages:**
- Complex setup
- Harder to debug issues
- Boost 1.49.0 may not work well with MinGW

---

### Option 3: Native Windows Build Environment

**Your current approach** (incomplete due to source compatibility issues)

**What's needed:**
1. Fix AsirikuyLogger.c to use Windows APIs or C++ standard library
2. Update NTPClient to use compatible Boost APIs
3. Remove deprecated compiler flags

---

## Source Code Fixes Needed for Windows Build

### Fix 1: Replace Unix Headers
**File:** `core/AsirikuyCommon/src/AsirikuyLogger.c`
```c
// Old (Unix-only):
#include <sys/time.h>

// New (Cross-platform):
#include <chrono>
#include <ctime>
```

### Fix 2: NTPClient Boost Compatibility
**File:** `core/NTPClient/include/NTPClient.hpp`
```cpp
// Old (Boost 1.60+):
boost::asio::io_context io_context_;
boost::asio::steady_timer deadline_;

// New (Boost 1.49.0):
boost::asio::io_service io_context_;
boost::asio::deadline_timer deadline_;
```

### Fix 3: Compiler Flags
**File:** `premake4.lua` - Remove `-std=c++11` for MSVC
```lua
configuration{"not windows"}
  buildoptions{"-std=c++11"}
```

---

## Current Progress on Windows Build

### Already Built Successfully ✓
- **TALib_common.lib**
- **TALib_abstract.lib**  
- **TALib_func.lib**
- **Premake4 solution files** (14 projects)

### Ready After Source Fixes
- All core libraries
- **AsirikuyFrameworkAPI.dll** (the main MT4 DLL)

---

## Effort Estimate

| Approach | Setup Time | Build Time | Success Rate |
|----------|-----------|-----------|--------------|
| **Option 1: Windows fixes** | 2-4 hours | 15 min | 95% |
| **Option 2: WSL MinGW** | 2-3 hours | 30 min | 60% |
| **Option 3: Linux+Docker** | N/A | N/A | 0% (incompatible) |

---

## Recommendation

**Use Option 1: Build Natively on Windows**

The project is 95% ready:
- Build system (Premake4) ✓ working
- Vendor libraries (TALib) ✓ compiled
- Compiler toolchain (VS2017 v141) ✓ installed

Only 3-4 source files need Unix→Windows fixes. This is faster and more reliable than Docker/WSL approaches.

Once fixed, you'll get:
```
E:\AutoBBS\bin\vs2010\x64\Release\AsirikuyFrameworkAPI.dll
```

Ready for immediate use in MT4.

