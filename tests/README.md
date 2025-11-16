# Tests Directory

This directory contains test scripts and utilities for the AutoBBS project.

## Files

### `test_asirikuy_framework_api.py`

**Status**: ⚠️ **For Future Reference** (Migration Deferred)

**Purpose**: Proof-of-concept test script to verify that `AsirikuyFrameworkAPI` can be loaded from Python and that `__stdcall` functions work on macOS.

**What it tests**:
1. Library loading (`libAsirikuyFrameworkAPI.dylib`)
2. Simple function call (`getFrameworkVersion`) - no `__stdcall`
3. `__stdcall` function call (`c_runStrategy`) - verifies `__stdcall` works on macOS

**Key Finding**: 
- `__stdcall` is **ignored** on macOS/Linux (Unix systems use standard C calling convention)
- `AsirikuyFrameworkAPI` can be used from Python on macOS without special `__stdcall` handling
- Functions can be called normally with `ctypes`

**Current Status**:
- Migration to `AsirikuyFrameworkAPI` has been **DEFERRED** (see `specs/001-trading-strategies-python-refactor/MIGRATION_DEFERRED.md`)
- CTester continues to use `CTesterFrameworkAPI` directly
- This test is preserved for future reference if migration is reconsidered

**Usage**:
```bash
cd /Users/andym/projects/AutoBBS
python3 tests/test_asirikuy_framework_api.py
```

**Note**: Requires `libAsirikuyFrameworkAPI.dylib` to be built and located at `bin/gmake/x64/Debug/libAsirikuyFrameworkAPI.dylib`

