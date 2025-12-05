# Windows DLL Build - Quick Reference

## TL;DR

Build AsirikuyFrameworkAPI.dll for MT4 in 5 steps:

### 1. Fix Code Issues (30 min - 2 hours)

**Find Unix headers:**
```powershell
cd e:\AutoBBS
Get-ChildItem -Recurse -Filter "*.c" -o "*.h" | 
  Select-String -Pattern "#include <sys/|#include <unistd|#include <pthread"
```

**Key files to fix:**
- `core/AsirikuyCommon/src/AsirikuyLogger.c` - Replace `<sys/time.h>` with `<ctime>`
- `core/NTPClient/hpp/cpp` - Change `io_context` → `io_service`, `steady_timer` → `deadline_timer`
- All other `.c` files with Unix headers - Use Windows or C++ std alternatives

**Compiler flags:**
- Edit `premake4.lua`: Move `-std=c++11` inside `configuration{"not windows"}` block

### 2. Regenerate Projects (5 min)
```powershell
$env:BOOST_ROOT = "e:\workspace\boost_1_49_0"
cd e:\AutoBBS
.\premake4.exe --file=premake4.lua vs2010
```

### 3. Clean Build (30 min)
```powershell
$msbuild = "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
$env:BOOST_ROOT = "e:\workspace\boost_1_49_0"

& $msbuild "build\vs2010\AsirikuyFramework.sln" `
  /p:Configuration=Release `
  /p:Platform=x64 `
  /p:PlatformToolset=v141 `
  /p:WindowsTargetPlatformVersion=10.0.26100.0 `
  /v:minimal /m
```

### 4. Verify Output (5 min)
```powershell
Test-Path "e:\AutoBBS\bin\vs2010\x64\Release\AsirikuyFrameworkAPI.dll"
dumpbin /exports "e:\AutoBBS\bin\vs2010\x64\Release\AsirikuyFrameworkAPI.dll"
```

### 5. Test in MT4 (30 min)
Copy DLL to MT4 folder and load in test EA

---

## Common Fixes

### Fix: sys/time.h not found
```c
// BEFORE:
#include <sys/time.h>

// AFTER:
#include <ctime>
#include <chrono>
```

### Fix: unistd.h not found
```c
// BEFORE:
#include <unistd.h>

// AFTER (remove for most cases, or):
#ifdef _WIN32
  #include <windows.h>
#else
  #include <unistd.h>
#endif
```

### Fix: io_context not found in boost::asio
```cpp
// BEFORE (Boost 1.60+):
boost::asio::io_context io_context_;

// AFTER (Boost 1.49.0):
boost::asio::io_service io_context_;
```

### Fix: steady_timer not found in boost::asio
```cpp
// BEFORE (Boost 1.60+):
boost::asio::steady_timer deadline_;

// AFTER (Boost 1.49.0):
boost::asio::deadline_timer deadline_;
```

### Fix: Unknown compiler option -std=c++11
```lua
-- BEFORE (premake4.lua):
buildoptions{"-std=c++11"}

-- AFTER:
configuration{"not windows"}
  buildoptions{"-std=c++11"}
```

---

## Expected Warnings (OK to Ignore)

```
warning C4244: 'argument': conversion from 'double' to 'int'
warning D9035: option 'Gm' has been deprecated
```

These are normal for older code.

---

## Expected Build Outputs

If successful, you should see:
```
e:\AutoBBS\bin\vs2010\x64\Release\
  AsirikuyFrameworkAPI.dll      ← Main output for MT4
  AsirikuyFrameworkAPI.lib
  AsirikuyCommon.lib
  Log.lib
  SymbolAnalyzer.lib
  AsirikuyTechnicalAnalysis.lib
  OrderManager.lib
  TradingStrategies.lib
```

---

## Troubleshooting

### Error: Cannot open include file 'sys/time.h'
→ Fix: Replace with `<ctime>` and `<chrono>`

### Error: 'io_context': is not a member of 'boost::asio'
→ Fix: Change to `io_service` (Boost 1.49.0 name)

### Error: 'steady_timer': is not a member of 'boost::asio'
→ Fix: Change to `deadline_timer` (Boost 1.49.0 name)

### Error: Unknown compiler version
→ OK: This is a warning, build continues

### Build stops but no DLL created
→ Check: Look for actual error lines above (not just warnings)

---

## Documentation

- **Specification:** `specs/WINDOWS_DLL_BUILD_SPEC.md`
- **Tasks:** `specs/WINDOWS_DLL_BUILD_TASKS.md`
- **Strategy:** `docs/MT4_WINDOWS_BUILD_STRATEGY.md`
- **Status:** `docs/WINDOWS_BUILD_STATUS.md`

---

## Branch

All work on: **window-build** branch

Commit as you go:
```powershell
git add <file>
git commit -m "Fix: <description>"
```

---

## Support Checklist

✓ VS2022 with v141 toolset installed  
✓ Boost 1.49.0 at e:\workspace\boost_1_49_0  
✓ Git repository initialized  
✓ window-build branch checked out  

**Ready to start!**

