# Build System Setup Guide - C++ Migration

## Overview

This document provides step-by-step instructions for updating the TradingStrategies build system to support C++ compilation while maintaining C API compatibility.

**Target**: Mixed C/C++ compilation with C++17 standard  
**Build System**: Premake4 → Visual Studio 2010  
**Platform**: Windows x32 Release

---

## Current Build Configuration

### Current premake4.lua

```lua
project "TradingStrategies"
  location("../../build/" .. _ACTION .. "/projects")
  kind "StaticLib"
  language "C"
  files{
    "**.h", 
    "**.c", 
    "**.hpp"
  }
  vpaths{
    ["Header Files/*"] = "../../../dev/TradingStrategies/include/**.*", 
    ["Source Files/*"] = "../../../dev/TradingStrategies/src/**.*", 
    ["Unit Tests"]   = "../../../dev/TradingStrategies/tests/**.*"
  }
  includedirs{
    "src"
  }
  defines{"FANN_NO_DLL"}
```

**Key Points**:
- Language: C (needs to change to C++)
- Files: Includes `**.hpp` but no `**.cpp` (needs to add)
- Include dirs: Only "src" (needs C++ include paths)

---

## Updated Build Configuration

### Step 1: Update premake4.lua

**Location**: `dev/TradingStrategies/premake4.lua`

```lua
project "TradingStrategies"
  location("../../build/" .. _ACTION .. "/projects")
  kind "StaticLib"
  language "C++"  -- Changed from "C"
  
  -- C++ standard
  flags { "Unicode" }
  
  -- Enable C++17 features (VS2010 supports subset of C++11, but we can use what's available)
  -- Note: Full C++17 requires VS2017+, but we can use modern patterns within VS2010 limits
  
  files{
    "**.h",      -- C headers
    "**.c",      -- C source (legacy, will be removed during migration)
    "**.hpp",    -- C++ headers
    "**.cpp"     -- C++ source (new)
  }
  
  vpaths{
    ["Header Files/C"] = {"../../../dev/TradingStrategies/include/**.h"},
    ["Header Files/C++"] = {"../../../dev/TradingStrategies/include/**.hpp"},
    ["Source Files/C"] = {"../../../dev/TradingStrategies/src/**.c"},
    ["Source Files/C++"] = {"../../../dev/TradingStrategies/src/**.cpp"},
    ["Unit Tests"] = {"../../../dev/TradingStrategies/tests/**.*"}
  }
  
  includedirs{
    "src",
    "include",           -- Add C++ include directory
    "include/strategies" -- Add strategies include directory
  }
  
  defines{
    "FANN_NO_DLL",
    "CPP_MIGRATION_IN_PROGRESS"  -- Feature flag
  }
  
  -- C++ specific compiler flags
  buildoptions{
    "/EHsc",   -- Enable C++ exceptions
    "/GR",     -- Enable RTTI (Runtime Type Information)
    "/W3"      -- Warning level 3
  }
  
  -- Configuration-specific settings
  configuration "Debug"
    defines { "_DEBUG" }
    flags { "Symbols" }
    
  configuration "Release"
    defines { "NDEBUG" }
    flags { "Optimize" }
    optimize "Speed"
```

### Step 2: Key Changes Explained

| Change | Reason |
|--------|--------|
| `language "C++"` | Enable C++ compilation mode |
| `"**.cpp"` in files | Include C++ source files |
| `"include"` in includedirs | Add C++ header search path |
| `/EHsc` flag | Enable C++ exception handling |
| `/GR` flag | Enable RTTI for dynamic_cast, typeid |
| `CPP_MIGRATION_IN_PROGRESS` | Conditional compilation flag |

### Step 3: Visual Studio 2010 C++ Limitations

**VS2010 Supports** (C++03 + some C++11):
- ✅ Classes, templates, namespaces
- ✅ Smart pointers (via Boost or manual implementation)
- ✅ Virtual functions, inheritance
- ✅ STL containers (vector, map, etc.)
- ✅ Exception handling
- ✅ Operator overloading
- ⚠️ Limited C++11: nullptr, auto (limited), range-based for (NO)

**VS2010 Does NOT Support** (C++11/14/17):
- ❌ `auto` with complex types (limited inference)
- ❌ Range-based for loops (`for (auto& x : vec)`)
- ❌ Lambda expressions (NO)
- ❌ `constexpr`
- ❌ `nullptr` (use NULL or 0)
- ❌ Move semantics (`std::move`, `&&`)
- ❌ Variadic templates
- ❌ `std::unique_ptr`, `std::shared_ptr` (use Boost)

**Workarounds**:
- Use Boost 1.55 (available in vendor/) for smart pointers
- Use explicit iterators instead of range-based for
- Use function pointers instead of lambdas
- Use NULL instead of nullptr
- Manual move semantics if needed

---

## Directory Structure Changes

### Step 4: Create C++ Directory Structure

**Current Structure**:
```
TradingStrategies/
├── include/
│   └── strategies/
│       ├── RecordBars.h
│       ├── AutoBBS.h
│       └── ... (C headers)
├── src/
│   ├── AsirikuyStrategies.c
│   └── strategies/
│       ├── RecordBars.c
│       └── ... (C sources)
└── tests/
```

**New Structure** (add C++ alongside C):
```
TradingStrategies/
├── include/
│   ├── strategies/              # Existing C headers
│   │   ├── RecordBars.h
│   │   └── ...
│   ├── StrategyContext.hpp      # New C++ headers
│   ├── IStrategy.hpp
│   ├── BaseStrategy.hpp
│   ├── StrategyFactory.hpp
│   ├── Indicators.hpp
│   ├── OrderBuilder.hpp
│   └── OrderManager.hpp
├── src/
│   ├── AsirikuyStrategies.c     # Existing C (to be replaced)
│   ├── AsirikuyStrategiesWrapper.cpp  # New C++ wrapper
│   ├── StrategyContext.cpp      # New C++ implementations
│   ├── StrategyFactory.cpp
│   ├── Indicators.cpp
│   ├── OrderBuilder.cpp
│   ├── OrderManager.cpp
│   └── strategies/
│       ├── RecordBars.c         # Existing C (will be removed)
│       └── RecordBarsStrategy.cpp # New C++ (will replace .c)
└── tests/
    ├── StrategyFactoryTest.cpp  # New C++ tests
    └── ...
```

**Commands** (in PowerShell):
```powershell
# Create C++ header directories (already exist, but ensure)
New-Item -ItemType Directory -Force -Path "e:\workspace\AutoBBS\dev\TradingStrategies\include"

# Create C++ strategies include directory (for future strategy classes)
New-Item -ItemType Directory -Force -Path "e:\workspace\AutoBBS\dev\TradingStrategies\include\strategies_cpp"

# Verify directories
Get-ChildItem -Directory "e:\workspace\AutoBBS\dev\TradingStrategies\include"
```

---

## Build Process

### Step 5: Regenerate Visual Studio Solution

**Commands**:
```powershell
# Navigate to AutoBBS root
cd e:\workspace\AutoBBS

# Run premake4 to regenerate Visual Studio solution
.\premake4.exe vs2010

# Verify generated solution
ls build\vs2010\AsirikuyFramework.sln
```

### Step 6: Build in Visual Studio

**Option A: Visual Studio GUI**
1. Open `build\vs2010\AsirikuyFramework.sln`
2. Select Configuration: "Release" | "Win32"
3. Right-click TradingStrategies project → Build
4. Verify no errors

**Option B: MSBuild Command Line**
```powershell
# Set up Visual Studio environment
& "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" x86

# Build TradingStrategies project
msbuild build\vs2010\projects\TradingStrategies.vcxproj /p:Configuration=Release /p:Platform=Win32

# Build entire solution
msbuild build\vs2010\AsirikuyFramework.sln /p:Configuration=Release /p:Platform=Win32
```

---

## Testing Mixed C/C++ Compilation

### Step 7: Create Test C++ File

**Create**: `dev/TradingStrategies/src/test_cpp.cpp`

```cpp
// Test C++ compilation
#include <string>
#include <vector>

namespace trading {
    class TestClass {
    public:
        TestClass() {}
        std::string getName() const { return "test"; }
    };
}

// Ensure C linkage works
extern "C" {
    int test_cpp_compilation() {
        trading::TestClass obj;
        return obj.getName().length();
    }
}
```

**Build** and verify no errors.

### Step 8: Verify C API Compatibility

**Test**: Ensure existing C functions still work

```c
// In AsirikuyStrategies.c (existing C code)
AsirikuyReturnCode runStrategy(StrategyParams* pParams) {
    // Existing C code should compile without issues
    return getStrategyFunctions(pParams, &runStrategyFunc);
}
```

**Expected**: No compilation errors, C code compiles in C++ mode.

---

## Common Issues and Solutions

### Issue 1: C++ Name Mangling

**Problem**: C functions called from MQL get name-mangled in C++

**Solution**: Use `extern "C"` for all DLL exports

```cpp
extern "C" {
    __declspec(dllexport) int mql4_runStrategy(...) {
        // Implementation
    }
}
```

### Issue 2: C Headers Not Compatible with C++

**Problem**: C headers use C-specific syntax

**Solution**: Wrap C includes with `extern "C"`

```cpp
extern "C" {
    #include "AsirikuyDefines.h"
    #include "Base.h"
}
```

### Issue 3: Compiler Warnings in C Code

**Problem**: C code generates C++ warnings (e.g., implicit casts)

**Solution**: 
- Fix warnings incrementally
- Use compiler pragma to suppress temporarily:

```cpp
#pragma warning(push)
#pragma warning(disable: 4996)  // Disable specific warning
#include "legacy_c_header.h"
#pragma warning(pop)
```

### Issue 4: Boost Libraries Not Found

**Problem**: Boost headers not found for smart pointers

**Solution**: Add Boost include directory

```lua
includedirs{
    "src",
    "include",
    "../../vendor/Boost"  -- Add Boost from vendor/
}
```

---

## Validation Checklist

After updating build system:

- [ ] premake4.lua updated with C++ language
- [ ] `**.cpp` files included in build
- [ ] C++ include directories added
- [ ] Solution regenerated successfully
- [ ] TradingStrategies project builds (C only, no C++ yet)
- [ ] Test C++ file compiles successfully
- [ ] Mixed C/C++ compilation works
- [ ] Existing C functions still work
- [ ] DLL exports still work (no name mangling)
- [ ] No new compiler warnings introduced
- [ ] Build time within acceptable range

---

## Rollback Plan

If build system changes cause issues:

1. **Revert premake4.lua**:
   ```powershell
   git checkout HEAD -- dev/TradingStrategies/premake4.lua
   ```

2. **Regenerate solution**:
   ```powershell
   .\premake4.exe vs2010
   ```

3. **Rebuild**:
   ```powershell
   msbuild build\vs2010\AsirikuyFramework.sln /t:Rebuild /p:Configuration=Release
   ```

---

## Next Steps

After build system setup complete:

1. **Create First C++ Class**: StrategyContext
2. **Test Compilation**: Ensure C++ compiles
3. **Create C API Wrapper**: Wrapper for runStrategy()
4. **Validate**: Ensure DLL still works with MQL

---

## Additional Resources

- [Premake4 Documentation](https://github.com/premake/premake-4.x/wiki)
- [Visual Studio 2010 C++ Compiler](https://docs.microsoft.com/en-us/previous-versions/visualstudio/visual-studio-2010/)
- [Boost 1.55 Documentation](https://www.boost.org/doc/libs/1_55_0/)
- [Mixing C and C++](https://isocpp.org/wiki/faq/mixing-c-and-cpp)

---

**Document Version**: 1.0  
**Status**: Ready for Implementation  
**Last Updated**: November 9, 2025  
**Owner**: Phase 1 Implementation Team
