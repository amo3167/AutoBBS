# Library Files Analysis

**Date**: December 2024  
**Question**: Do we need the .so files in ctester/?

## Current Situation

### Files in `ctester/` directory:
- `libAsirikuyFrameworkAPI.so` - Linux shared library
- `libCTesterFrameworkAPI.so` - Linux shared library
- `libchartdir.so.5.1` - ChartDir library (Linux)
- `libchartdir.so.6.0` - ChartDir library (Linux)
- `libiomp5.so` - Intel OpenMP library (Linux)

### Files in `bin/gmake/x64/Debug/`:
- `libCTesterFrameworkAPI.dylib` - macOS shared library (actual build output)
- `libAsirikuyFrameworkAPI.dylib` - macOS shared library (actual build output)

## Analysis

### Platform-Specific Libraries

**macOS (Darwin)**:
- Uses `.dylib` files (not `.so`)
- CTester loads: `libCTesterFrameworkAPI.dylib` (line 69 in `asirikuy_strategy_tester.py`)
- Actual libraries are in: `bin/gmake/x64/Debug/`

**Linux**:
- Uses `.so` files
- CTester loads: `libCTesterFrameworkAPI.so` (line 67)

**Windows**:
- Uses `.dll` files
- CTester loads: `CTesterFrameworkAPI` (line 65)

### Library Loading

The `loadLibrary()` function in `include/asirikuy.py`:
```python
def loadLibrary(library):
    if os.name == 'nt':
        return windll.LoadLibrary(library)
    elif os.name == 'posix':
        return cdll.LoadLibrary(library)
    else:
        return None
```

`cdll.LoadLibrary()` searches:
1. Current directory (where Python script is run from)
2. System library paths (LD_LIBRARY_PATH on Linux, DYLD_LIBRARY_PATH on macOS)
3. Standard system library directories

## Answer: Do We Need .so Files in ctester/?

### On macOS: **NO**

The `.so` files in `ctester/` are:
- ❌ **Linux libraries** (not compatible with macOS)
- ❌ **Not used** on macOS (CTester loads `.dylib` files)
- ❌ **Leftover artifacts** from Linux development/testing
- ❌ **Should be removed** or ignored

### On Linux: **MAYBE**

If you're running CTester on Linux:
- ✅ `.so` files would be needed
- ⚠️ But they should be in the proper location (not necessarily in `ctester/`)
- ⚠️ Should match the actual build output location

## Recommendations

### 1. Remove .so Files from ctester/ (macOS)

Since you're on macOS:
```bash
cd ctester
rm -f *.so
```

**Rationale**:
- Not needed on macOS
- Clutters the directory
- Can cause confusion
- Actual libraries are in `bin/gmake/x64/Debug/`

### 2. Update .gitignore

Add to `.gitignore`:
```gitignore
# Platform-specific library files (use build output instead)
*.so
*.dylib
```

**Rationale**:
- Libraries are build artifacts
- Should be in build output directories
- Platform-specific (shouldn't be in repo)
- Can be rebuilt from source

### 3. Update Library Loading (Optional)

Consider updating `loadLibrary()` to search build directories:
```python
def loadLibrary(library):
    # Try current directory first (for compatibility)
    # Then try build output directories
    possible_paths = [
        library,  # Current directory
        f"../bin/gmake/x64/Debug/{library}",
        f"../bin/gmake/x64/Release/{library}",
    ]
    
    for path in possible_paths:
        if os.path.exists(path):
            if os.name == 'nt':
                return windll.LoadLibrary(path)
            elif os.name == 'posix':
                return cdll.LoadLibrary(path)
    
    # Fallback to system search
    if os.name == 'nt':
        return windll.LoadLibrary(library)
    elif os.name == 'posix':
        return cdll.LoadLibrary(library)
    else:
        return None
```

### 4. Create Symbolic Links (Alternative)

If you want libraries in `ctester/` for convenience:
```bash
cd ctester
ln -s ../bin/gmake/x64/Debug/libCTesterFrameworkAPI.dylib .
ln -s ../bin/gmake/x64/Debug/libAsirikuyFrameworkAPI.dylib .
```

**Rationale**:
- Keeps libraries accessible
- Points to actual build output
- No duplication
- Updates automatically when rebuilt

## Summary

**For macOS development:**
- ❌ **Remove `.so` files** from `ctester/` (they're Linux libraries)
- ✅ **Use `.dylib` files** from `bin/gmake/x64/Debug/`
- ✅ **Add to `.gitignore`** to prevent committing library files
- ⚠️ **Consider** updating library loading to use build directories

**For Linux development:**
- ✅ **Keep `.so` files** (but in proper location)
- ✅ **Build libraries** and place in appropriate directory
- ✅ **Update `.gitignore`** to exclude platform-specific libraries

## Action Items

1. [ ] Remove `.so` files from `ctester/` (macOS)
2. [ ] Update `.gitignore` to exclude `*.so` and `*.dylib`
3. [ ] Verify CTester can load libraries from build directory
4. [ ] Document library location requirements

