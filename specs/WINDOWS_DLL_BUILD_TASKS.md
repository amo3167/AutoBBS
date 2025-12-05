# Windows DLL Build Tasks

## Overview
Tasks to complete Windows native build of AsirikuyFrameworkAPI.dll for MT4.

**Total Estimated Effort:** 4-5 hours  
**Priority:** CRITICAL  
**Branch:** `window-build`

---

## Phase 1: Code Analysis

### TASK-1.1: Scan for Unix-specific Headers
**Status:** NOT STARTED  
**Effort:** 30 minutes  
**Blocking:** TASK-2.x

**Description:**
Identify all files using Unix-specific headers that are incompatible with Windows.

**Search Patterns:**
```
#include <sys/
#include <unistd.h>
#include <pthread.h>
#include <arpa/
#include <netinet/
```

**Deliverable:**
- [ ] List of all files with Unix headers
- [ ] Document dependencies

**Acceptance Criteria:**
- All Unix header references identified
- Prioritized by build dependency order

---

### TASK-1.2: Map Boost API Usage in NTPClient
**Status:** NOT STARTED  
**Effort:** 20 minutes  
**Blocking:** TASK-3.1

**Description:**
Identify which Boost APIs are used in NTPClient and check availability in 1.49.0.

**Check:**
- `boost::asio::io_context` → exists as `io_service` in 1.49.0
- `boost::asio::steady_timer` → exists as `deadline_timer` in 1.49.0
- All resolver functions
- All network APIs

**Deliverable:**
- [ ] API compatibility matrix
- [ ] Affected lines of code

**Acceptance Criteria:**
- All Boost API calls documented
- Alternative APIs identified for each

---

## Phase 2: Critical Fixes

### TASK-2.1: Fix AsirikuyLogger.c
**Status:** NOT STARTED  
**Effort:** 15 minutes  
**Blocking:** Build (CRITICAL)

**Description:**
Replace Unix time headers with C++ standard library.

**File:** `core/AsirikuyCommon/src/AsirikuyLogger.c`

**Changes Required:**
```diff
- #include <sys/time.h>
+ #include <ctime>
+ #include <chrono>
```

**Deliverable:**
- [ ] File modified and tested to compile

**Acceptance Criteria:**
- No compilation errors for sys/time.h
- AsirikuyLogger.c compiles successfully
- Existing functionality preserved

---

### TASK-2.2: Find and Fix Other Unix Headers in Core
**Status:** NOT STARTED  
**Effort:** 1-2 hours  
**Blocking:** Build (CRITICAL)

**Description:**
Fix all remaining Unix header includes in core modules.

**Expected Files:**
- `core/Log/src/*.c`
- `core/SymbolAnalyzer/src/*.c`
- `core/AsirikuyTechnicalAnalysis/src/*.c`
- `core/OrderManager/src/*.c`
- `core/AsirikuyEasyTrade/src/*.c`

**Common Fixes:**
```
sys/time.h     → ctime, chrono
unistd.h       → windows.h (for Windows specific) or remove
pthread.h      → thread (C++ standard)
arpa/inet.h    → winsock2.h (Windows) or boost asio
sys/socket.h   → winsock2.h (Windows) or boost asio
```

**Process:**
1. Search for each Unix header pattern
2. Document Windows alternatives
3. Apply fixes file-by-file
4. Test compilation for each file

**Deliverable:**
- [ ] All Unix headers replaced
- [ ] Build succeeds for all core modules

**Acceptance Criteria:**
- Zero Unix header includes in source
- All core libraries compile without header errors
- No functionality regression

---

### TASK-2.3: Fix Compiler Flags in premake4.lua
**Status:** NOT STARTED  
**Effort:** 15 minutes  
**Blocking:** Build (not critical, warnings only)

**Description:**
Remove Unix-specific compiler flags for Windows builds.

**File:** `premake4.lua`

**Change:**
```lua
-- OLD (affects Windows):
buildoptions{"-std=c++11"}

-- NEW (platform-specific):
configuration{"not windows"}
  buildoptions{"-std=c++11"}
```

**Deliverable:**
- [ ] premake4.lua updated
- [ ] Projects regenerated

**Acceptance Criteria:**
- Windows builds don't get Unix compiler flags
- No command line warnings about unknown options
- Release build completes cleanly

---

## Phase 3: NTPClient Boost Fix

### TASK-3.1: Fix NTPClient Boost API Compatibility
**Status:** NOT STARTED  
**Effort:** 30 minutes  
**Blocking:** AsirikuyFrameworkAPI build

**Description:**
Update NTPClient to use Boost 1.49.0 compatible APIs.

**Files:**
- `core/NTPClient/include/NTPClient.hpp`
- `core/NTPClient/src/NTPClient.cpp`

**Replace:**
```cpp
// OLD (Boost 1.60+):
boost::asio::io_context io_context_;
boost::asio::steady_timer deadline_;
auto results = resolver.resolve(host, port, options);

// NEW (Boost 1.49.0):
boost::asio::io_service io_context_;
boost::asio::deadline_timer deadline_;
// Alternative resolve API compatible with 1.49.0
```

**Options:**
1. **Direct replacement** - Use 1.49.0 API names
2. **Compatibility layer** - Create wrappers
3. **Platform guard** - Conditionally compile

**Recommended:** Option 1 (simplest)

**Deliverable:**
- [ ] All io_context → io_service
- [ ] All steady_timer → deadline_timer
- [ ] Resolver API updated
- [ ] NTPClient.cpp compiles

**Acceptance Criteria:**
- NTPClient compiles without Boost errors
- No references to non-existent Boost classes
- Runtime behavior unchanged

---

## Phase 4: Build & Integration

### TASK-4.1: Clean Rebuild with Fixes
**Status:** NOT STARTED  
**Effort:** 30 minutes  
**Blocking:** Testing

**Description:**
Full rebuild after all source fixes applied.

**Steps:**
```powershell
$env:BOOST_ROOT = "e:\workspace\boost_1_49_0"

# Clean previous build
Remove-Item -Recurse "build\vs2010", "bin\vs2010" -ErrorAction SilentlyContinue

# Regenerate projects
.\premake4.exe --file=premake4.lua vs2010

# Build Release x64
$msbuild = "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
& $msbuild "build\vs2010\AsirikuyFramework.sln" `
  /p:Configuration=Release `
  /p:Platform=x64 `
  /p:PlatformToolset=v141 `
  /p:WindowsTargetPlatformVersion=10.0.26100.0 `
  /v:detailed `
  /m
```

**Deliverable:**
- [ ] Build completes without errors
- [ ] All libraries generated
- [ ] AsirikuyFrameworkAPI.dll created

**Acceptance Criteria:**
- Zero compilation errors
- All expected .lib and .dll files exist
- Build output in correct directory

---

### TASK-4.2: Verify DLL Structure
**Status:** NOT STARTED  
**Effort:** 15 minutes  
**Blocking:** Testing

**Description:**
Verify DLL has correct structure and exports.

**Checks:**
```powershell
# Check DLL exists
$dll = "e:\AutoBBS\bin\vs2010\x64\Release\AsirikuyFrameworkAPI.dll"
Test-Path $dll

# List exports (requires dumpbin or similar)
dumpbin /exports $dll

# Verify key exports:
# - initInstanceMQL4
# - initInstanceMQL5
# - mql4_runStrategy
# - mql5_runStrategy
# - jf_runStrategy
```

**Deliverable:**
- [ ] Export list documented
- [ ] All required functions present

**Acceptance Criteria:**
- DLL file exists and is valid
- All functions from .def file exported
- No unexpected symbols

---

### TASK-4.3: Load DLL in PowerShell
**Status:** NOT STARTED  
**Effort:** 10 minutes  
**Blocking:** Testing

**Description:**
Verify DLL loads without runtime errors.

**Test:**
```powershell
$dll = "e:\AutoBBS\bin\vs2010\x64\Release\AsirikuyFrameworkAPI.dll"

# Attempt to load
try {
    Add-Type -Path $dll
    Write-Host "DLL loaded successfully"
} catch {
    Write-Host "DLL load error: $_"
}
```

**Deliverable:**
- [ ] DLL loads without errors
- [ ] No missing dependencies

**Acceptance Criteria:**
- DLL loads in PowerShell
- No missing DLL dependencies
- No runtime initialization failures

---

## Phase 5: Documentation & Commit

### TASK-5.1: Update Build Documentation
**Status:** NOT STARTED  
**Effort:** 20 minutes  
**Blocking:** None

**Description:**
Update Windows build documentation with successful process.

**Files to Update:**
- `docs/WINDOWS_BUILD_STATUS.md` - Update status
- `README.md` - Add Windows build section
- `docs/BUILD_GUIDE.md` - Create if needed

**Deliverable:**
- [ ] Complete build process documented
- [ ] Troubleshooting guide added

**Acceptance Criteria:**
- Clear step-by-step instructions
- Known issues documented
- Reproducible by another developer

---

### TASK-5.2: Commit Changes to window-build Branch
**Status:** NOT STARTED  
**Effort:** 10 minutes  
**Blocking:** PR review

**Description:**
Commit all source fixes and documentation.

**Commits:**
```
commit 1: Fix Unix headers in core modules
commit 2: Fix NTPClient Boost 1.49.0 compatibility
commit 3: Fix compiler flags in premake4.lua
commit 4: Update build documentation
```

**Deliverable:**
- [ ] All changes committed
- [ ] Branch ready for PR review

**Acceptance Criteria:**
- All changes in git history
- Commit messages clear and descriptive
- No untracked files

---

## Phase 6: Testing

### TASK-6.1: MT4 Integration Test
**Status:** NOT STARTED  
**Effort:** 1-2 hours  
**Blocking:** Release

**Description:**
Verify DLL works with actual MT4 terminal.

**Process:**
1. Copy DLL to MT4 Experts folder
2. Create test EA that loads AsirikuyFrameworkAPI
3. Initialize via initInstanceMQL4()
4. Call mql4_runStrategy()
5. Verify results

**Deliverable:**
- [ ] MT4 successfully loads DLL
- [ ] Functions callable and return data
- [ ] No crashes or errors

**Acceptance Criteria:**
- DLL loads in MT4 without errors
- Test EA runs strategy successfully
- Results are reasonable

---

### TASK-6.2: Performance Benchmark
**Status:** NOT STARTED  
**Effort:** 30 minutes  
**Optional:** Yes

**Description:**
Baseline performance measurements.

**Metrics:**
- DLL load time
- Strategy initialization time
- Single strategy run time
- Memory usage

**Deliverable:**
- [ ] Baseline metrics document

**Acceptance Criteria:**
- Performance data captured
- No unexpected slowdowns

---

## Dependency Matrix

```
TASK-1.1 (Scan) → TASK-2.2 (Fix)
TASK-1.2 (Boost) → TASK-3.1 (Fix)
TASK-2.1 (Logger) ↓
TASK-2.2 (Core)   ├→ TASK-4.1 (Build) → TASK-4.2 (Verify) → TASK-4.3 (Load) → TASK-6.1 (MT4)
TASK-2.3 (Flags) ↓
TASK-3.1 (NTP)   ↓
    ↓
TASK-5.1 (Docs) → TASK-5.2 (Commit)
```

---

## Execution Plan

### Day 1 (3-4 hours)
- [ ] TASK-1.1: Header scan (30 min)
- [ ] TASK-1.2: Boost API map (20 min)
- [ ] TASK-2.1: Fix AsirikuyLogger (15 min)
- [ ] TASK-2.2: Fix remaining headers (1-2 hours)
- [ ] TASK-2.3: Fix compiler flags (15 min)

### Day 2 (1-2 hours)
- [ ] TASK-3.1: Fix NTPClient (30 min)
- [ ] TASK-4.1: Clean rebuild (30 min)
- [ ] TASK-4.2: Verify DLL (15 min)
- [ ] TASK-4.3: Load test (10 min)

### Day 3 (1-2 hours)
- [ ] TASK-5.1: Update docs (20 min)
- [ ] TASK-5.2: Commit (10 min)
- [ ] TASK-6.1: MT4 test (1 hour)

---

## Risk Mitigation

| Risk | Mitigation |
|------|-----------|
| Unknown Unix deps | Regular grep searches for patterns |
| Boost API gaps | Create compatibility layer if needed |
| Build failures | Track error messages, rebuild incrementally |
| MT4 incompatibility | Test with simple EA first |

---

## Success Metrics

✓ All tasks completed  
✓ Zero compilation errors  
✓ AsirikuyFrameworkAPI.dll created and exports verified  
✓ DLL loads successfully in Windows  
✓ DLL works with MT4 terminal  
✓ All changes committed to window-build branch  

