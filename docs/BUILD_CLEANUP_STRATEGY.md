# Build Artifact Cleanup & Retention Strategy

**Date:** December 6, 2025  
**Status:** Recommended best practices for Windows/macOS/Linux builds

---

## 📂 Build Directory Structure

After compilation, premake creates this structure:

```
build/
├── bin/              ← KEEP (Final binaries: .lib, .dll, .a, .so)
│   └── vs2010/x64/Release/
│       ├── AsirikuyFrameworkAPI.dll
│       ├── AsirikuyFrameworkAPI.lib
│       └── ...
├── tmp/              ← CLEAN (Intermediate objects: .obj, .o)
│   └── vs2010/x64/Release/
│       ├── TradingStrategies/
│       │   ├── AsirikuyStrategies.obj
│       │   ├── ATRPrediction.obj
│       │   └── ... (100+ object files)
│       └── ...
└── vs2010/           ← CONTAINS (Visual Studio solution files)
    ├── AsirikuyFramework.sln
    ├── projects/
    └── ...

bin/
├── vs2010/           ← KEEP (Final outputs from source build)
│   └── x64/Release/
│       ├── AsirikuyFrameworkAPI.dll
│       └── ...

releases/             ← KEEP (Release packages)
└── AsirikuyFramework-{timestamp}/
    ├── bin/
    ├── lib/
    ├── include/
    └── ...
```

---

## 🧹 Cleanup Policy Recommendations

### **Strategy 1: Development Builds (RECOMMENDED)**

```
After each build:
✓ Keep:    build/bin/        (final binaries)
✓ Keep:    build/vs2010/     (solution files for IDE)
✓ Keep:    bin/              (copy of final outputs)
✗ Clean:   build/tmp/        (object files not needed for development)
```

**Rationale:**
- Object files are 100-500MB, temporary compilation artifacts
- Don't contribute to functionality, only for incremental builds
- Developers can always rebuild if needed
- Saves disk space significantly
- Faster Git operations

**Disk Impact:**
- Object files alone: ~200-300 MB per configuration
- Without cleanup: 400-600 MB for 2 configurations
- With cleanup: <50 MB

---

### **Strategy 2: Release Builds (RECOMMENDED)**

```
After building and packaging:
✓ Keep:    bin/vs2010/x64/Release/     (final DLLs/LIBs)
✓ Keep:    releases/                    (packaged releases)
✗ Clean:   build/tmp/                   (not needed for release)
✗ Clean:   build/bin/ (debug configs)   (keep only needed arch)
```

**Rationale:**
- Release package contains everything needed
- Object files not needed for distribution
- Cleaner repository for production releases
- Customers only need DLLs/LIBs from `bin/` or `releases/`

---

### **Strategy 3: Continuous Integration (RECOMMENDED)**

```
Build -> Test -> Archive -> Clean

After each CI build:
✓ Archive:   artifacts/ (final binaries + PDBs)
✓ Archive:   releases/  (release packages)
✗ Clean:     build/tmp/ (intermediate objects)
✗ Clean:     build/bin/ (also in artifacts)
```

**Rationale:**
- Artifacts archived separately for each build
- Object files not needed in CI pipeline
- Prevents disk bloat on build servers
- Release packages independent of build artifacts

---

## 🎯 Recommended Default Behavior

### **For Development Workflow**

```
build-release.ps1
  └─> Builds successfully
  └─> Copies to bin/
  └─> Optionally cleans build/tmp/ (if -CleanBuild)
  └─> Keeps build/vs2010/ for IDE

Usage:
  .\build-release.ps1 -Config release64              # Normal build
  .\build-release.ps1 -Config release64 -CleanAfter  # Build + cleanup
```

### **For Release Packaging**

```
organize-release.ps1
  └─> Takes source: bin/vs2010/x64/Release/
  └─> Creates release package: releases/AsirikuyFramework-{timestamp}/
  └─> No impact on build artifacts
  └─> optionally: -CleanOldReleases (removes older packages)

Usage:
  .\organize-release.ps1 -SourceDir bin/vs2010        # Just package
  .\organize-release.ps1 -SourceDir bin/vs2010 \
    -KeepReleases 3                                     # Keep 3 latest
```

### **For Full Orchestration**

```
build-orchestrate.ps1
  └─> Action: build           → Compiles, optionally cleans tmp/
  └─> Action: release         → Builds + organizes + cleans
  └─> Action: clean           → Explicit cleanup phase
  └─> Action: all (default)   → Full workflow with final cleanup

Usage:
  .\build-orchestrate.ps1 -Action build      # Build + minimal cleanup
  .\build-orchestrate.ps1 -Action release    # Build + package + cleanup
  .\build-orchestrate.ps1 -Action clean      # Just cleanup
```

---

## 📋 Files to Keep vs Clean

### **ALWAYS KEEP**

```
bin/vs2010/x64/Release/
├── AsirikuyFrameworkAPI.dll       Essential DLL
├── AsirikuyFrameworkAPI.lib       Import library
├── trading_strategies.lib         Strategy engine
└── ...                            Other libraries

releases/
└── AsirikuyFramework-{timestamp}/  Packaged releases
    ├── bin/
    ├── lib/
    ├── include/
    ├── MANIFEST.json
    └── README.txt
```

### **ALWAYS CLEAN**

```
build/tmp/vs2010/x64/Release/
├── TradingStrategies/
│   ├── *.obj                    Object files (compiled code)
│   └── ...
├── AsirikuyCommon/
│   ├── *.obj
│   └── ...
└── ... (100+ object files per config)
```

### **OPTIONAL (Context Dependent)**

```
build/bin/vs2010/              Maybe (also in bin/)
├── x86/Debug/                 - If only release needed, clean
├── x64/Debug/                 - If only x64, clean
└── ...

.pdb files                      - Keep for debugging, clean for release
├── build/vs2010/*.pdb
└── bin/vs2010/x64/Release/*.pdb
```

---

## 💾 Disk Space Comparison

### **After 2-config build (debug64 + release64)**

| Storage | Keep All | Smart Clean | Release Only |
|---------|----------|-------------|--------------|
| build/tmp/ | ~300 MB | 0 MB | 0 MB |
| build/bin/ | ~25 MB | ~25 MB | 0 MB |
| bin/ | ~15 MB | ~15 MB | ~15 MB |
| releases/ | varies | varies | ~50 MB |
| **Total** | ~340 MB | ~40 MB | ~65 MB |

### **Savings with smart cleanup**
- **Development:** 8-9x smaller
- **CI/CD:** 5-6x smaller
- **Release:** Clean separation

---

## ⚙️ Script Configuration Options

### **build-release.ps1**

```powershell
# Default: Don't clean (developers may recompile)
.\build-release.ps1 -Config release64

# Clean intermediate objects after build
.\build-release.ps1 -Config release64 -CleanAfter

# Keep everything (for debugging)
.\build-release.ps1 -Config release64 -NoClean

# Clean before and after
.\build-release.ps1 -Config release64 -Clean -CleanAfter
```

### **organize-release.ps1**

```powershell
# Default: Keep all releases
.\organize-release.ps1 -SourceDir bin/vs2010

# Keep only 3 most recent releases, clean older ones
.\organize-release.ps1 -SourceDir bin/vs2010 -KeepReleases 3

# Clean all releases before packaging new one
.\organize-release.ps1 -SourceDir bin/vs2010 -CleanOldReleases
```

### **build-orchestrate.ps1**

```powershell
# Build only, minimal cleanup (don't remove tmp/)
.\build-orchestrate.ps1 -Action build

# Full release: build + organize + clean intermediate objects
.\build-orchestrate.ps1 -Action release -CleanBuild

# Just cleanup (no build)
.\build-orchestrate.ps1 -Action clean

# Full workflow: clean -> build -> test -> organize -> deep clean
.\build-orchestrate.ps1 -Action all -DeepClean
```

---

## 🔧 Implementation Details

### **Cleanup Levels**

**Level 1: Shallow Clean (Default after build)**
```
Remove:
  - build/tmp/          (object files)
Keep:
  - build/bin/          (DLLs, LIBs)
  - build/vs2010/       (solution files)
  - bin/                (outputs)
Size removed: ~200-300 MB per config
```

**Level 2: Standard Clean (After release)**
```
Remove:
  - build/tmp/          (all object files)
  - build/bin/          (redundant, already in bin/)
  - Debug configs       (if only release needed)
Keep:
  - bin/vs2010/x64/Release/  (final artifacts)
  - releases/                (packaged releases)
Size removed: ~350-500 MB
```

**Level 3: Deep Clean (Release build)**
```
Remove:
  - Everything except releases/
  - All intermediate files
Keep:
  - releases/AsirikuyFramework-{timestamp}/
  - Source code
Size removed: ~400+ MB
Rationale: Only release packages matter for distribution
```

---

## 📊 Recommended Workflow by Use Case

### **Personal Development**

```powershell
# Minimal cleanup - keep build artifacts for fast rebuilds
.\build-release.ps1 -Config debug64
# Then just run/test
# Clean manually when disk full:
.\build-orchestrate.ps1 -Action clean
```

**Disk usage:** ~100-150 MB (with object files)

### **Team Development**

```powershell
# Build + shallow cleanup by default
.\build-release.ps1 -Config release64 -CleanAfter
# Each build: ~50 MB disk usage
# Prevents accidental commits of huge object files
```

**Disk usage:** ~50 MB per build

### **CI/CD Pipeline**

```powershell
# Build -> Test -> Archive -> Deep Clean
.\build-orchestrate.ps1 -Action all -DeepClean -OutputLayout flat
# After pipeline completes, only release packages remain
```

**Disk usage:** ~50 MB (final package only)

### **Release Management**

```powershell
# Old release packages cleaned automatically
.\organize-release.ps1 -SourceDir bin/vs2010 -KeepReleases 5
# Keeps last 5 releases, removes older ones
```

**Disk usage:** ~250 MB (5 × 50 MB packages)

---

## ✅ Best Practices

1. **Default to shallow cleanup** - Removes obvious temp files, keeps build artifacts
2. **Automatic cleanup on release** - When creating release packages, clean old builds
3. **Make cleanup optional** - Developers can opt-in to aggressive cleanup
4. **Archive before cleanup** - CI/CD archives artifacts before cleaning
5. **Keep releases separate** - Never clean `releases/` folder automatically
6. **Document cleanup levels** - Make it clear what each option removes
7. **Use git-friendly sizes** - Small repo sizes better for version control
8. **Enable cleanup by default in CI/CD** - Build servers have limited disk

---

## 🚀 Next Steps

### **Updates Needed**

1. Add `-CleanAfter` parameter to `build-release.ps1`
2. Add `-KeepReleases` parameter to `organize-release.ps1`
3. Add `clean` action to `build-orchestrate.ps1`
4. Add cleanup documentation to inline help
5. Create cleanup test scenarios

### **Quick Summary Table**

| Scenario | Action | Cleanup |
|----------|--------|---------|
| Development | `build-release.ps1` | No (default) |
| Team Build | `build-release.ps1 -CleanAfter` | Shallow |
| Release | `organize-release.ps1` | Auto archive |
| CI/CD | `build-orchestrate.ps1 -Action all -DeepClean` | Deep |
| Disk Cleanup | `build-orchestrate.ps1 -Action clean` | All levels |

---

## 📝 Summary

**Recommended Default Behavior:**

```
Source code builds   → in place (via premake)
↓
Outputs generated    → build/bin/ and bin/
↓
Option: -CleanAfter? 
  ├─ Yes  → Remove build/tmp/ (object files)
  └─ No   → Keep for faster rebuilds (developer choice)
↓
Release packages     → releases/ (independent)
↓
Old releases?        → -KeepReleases 3 (auto cleanup)
```

**The answer:**
- **After build:** Keep `bin/` and `build/vs2010/`, optionally clean `build/tmp/`
- **For releases:** Package in `releases/`, auto-manage old packages
- **For CI/CD:** Clean after archiving artifacts
- **For developers:** Choice to keep or clean intermediate files

This balances disk space, rebuild speed, and production cleanliness.
