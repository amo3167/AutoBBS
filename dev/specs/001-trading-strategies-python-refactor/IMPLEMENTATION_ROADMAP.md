# TradingStrategies Refactoring - Implementation Roadmap

## Quick Start Summary

This refactoring enables the TradingStrategies C library to be called from Python, removing DLL dependencies and enabling integration with Python trading platforms via broker REST APIs.

## What Has Been Created

1. **REFACTORING_PLAN.md** - Comprehensive plan with 3 options
2. **include/TradingStrategiesPythonAPI.h** - Python-friendly C API header
3. **python/trading_strategies.py** - Python wrapper using ctypes
4. **python/example_usage.py** - Example integration with broker API
5. **python/__init__.py** - Python package initialization

## Recommended Approach: Option 1 (Minimal Refactor)

**Why**: Fastest to implement, minimal risk, maintains existing code structure.

## Implementation Steps

### Phase 1: Create C API Implementation (Priority: HIGH)

**File**: `src/TradingStrategiesPythonAPI.c`

**Tasks**:
1. Implement `trading_strategies_run()` function
2. Convert `PythonStrategyInput` → `StrategyParams`
3. Call existing `runStrategy(StrategyParams*)`
4. Convert `StrategyResults` → `PythonStrategyOutput`
5. Implement memory management
6. Implement `trading_strategies_free_output()`
7. Implement error message mapping

**Estimated Time**: 2-3 days

**Dependencies**:
- Need to understand `StrategyParams` structure fully
- Need to understand `StrategyResults` structure
- Need to map settings array indices

### Phase 2: Update Build System (Priority: HIGH)

**File**: `premake4.lua`

**Changes**:
```lua
project "TradingStrategies"
  kind "SharedLib"  -- Change from StaticLib
  targetname "trading_strategies"  -- Library name
  -- Add new source files
  files{
    "**.h",
    "**.c",
    "include/TradingStrategiesPythonAPI.h",  -- NEW
    "src/TradingStrategiesPythonAPI.c",       -- NEW
  }
```

**Estimated Time**: 1-2 hours

### Phase 3: Test C API (Priority: HIGH)

**Tasks**:
1. Create simple C test program
2. Test with minimal input
3. Verify output structure
4. Test memory management
5. Test error handling

**Estimated Time**: 1 day

### Phase 4: Python Integration Testing (Priority: MEDIUM)

**Tasks**:
1. Build shared library
2. Test Python wrapper loading
3. Test with mock data
4. Test signal extraction
5. Test error handling

**Estimated Time**: 1 day

### Phase 5: Broker API Integration (Priority: MEDIUM)

**Tasks**:
1. Implement real broker API client
2. Integrate with TradingPlatform class
3. Test order placement
4. Test order modification
5. Test order closing

**Estimated Time**: 2-3 days (depends on broker API)

### Phase 6: Remove DLL Dependencies (Priority: LOW)

**Tasks**:
1. Remove `__stdcall` calling conventions (if any in TradingStrategies)
2. Remove Windows-specific code (if any)
3. Update documentation
4. Clean up unused files

**Estimated Time**: 1 day

## Key Implementation Details

### Settings Array Mapping

The `settings` array in `StrategyParams` uses indexed access. You'll need to map Python dictionary keys to array indices. Example:

```c
// In AsirikuyDefines.h or similar
#define ACCOUNT_RISK_PERCENT 0
#define MAX_OPEN_ORDERS 1
#define IS_BACKTESTING 2
// ... etc
```

Create a mapping function:
```c
static int get_setting_index(const char* name) {
    // Map setting name to index
    if (strcmp(name, "ACCOUNT_RISK_PERCENT") == 0) return ACCOUNT_RISK_PERCENT;
    // ... etc
}
```

### Rates Data Structure

The current `RatesBuffers` structure supports multiple timeframes. For Python API, you have two options:

1. **Option A**: Pass one timeframe at a time, call multiple times
2. **Option B**: Extend `PythonStrategyInput` to support multiple timeframes

**Recommendation**: Start with Option A, extend to Option B if needed.

### Memory Management

**Critical**: Python manages memory for input arrays. C library manages memory for output arrays. Use `trading_strategies_free_output()` to free output memory.

**Pattern**:
```c
// In trading_strategies_run()
output->signal_types = malloc(signals_count * sizeof(int));
// ... allocate other arrays
// Store pointer in _internal_data for cleanup
```

### Error Handling

Map all `AsirikuyReturnCode` values to human-readable strings:
```c
const char* trading_strategies_get_error_message(int return_code) {
    switch (return_code) {
        case SUCCESS: return "Success";
        case NULL_POINTER: return "Null pointer error";
        case INVALID_PARAMETER: return "Invalid parameter";
        // ... etc
        default: return "Unknown error";
    }
}
```

## Testing Strategy

### Unit Tests (C)

Create `tests/TradingStrategiesPythonAPITests.c`:
- Test input conversion
- Test output conversion
- Test error cases
- Test memory management

### Integration Tests (Python)

Create `python/tests/test_trading_strategies.py`:
- Test library loading
- Test with mock data
- Test signal extraction
- Test error handling
- Test memory leaks

### End-to-End Tests

Create `python/tests/test_broker_integration.py`:
- Test with real broker API (or mock)
- Test order placement
- Test order modification
- Test order closing

## Common Issues

### Issue 1: Library Not Found

**Solution**: Ensure library path is correct in `trading_strategies.py`:
```python
_lib_paths = [
    os.path.join(os.path.dirname(__file__), "..", "build", f"libtrading_strategies{_lib_ext}"),
    # Add more paths as needed
]
```

### Issue 2: Memory Leaks

**Solution**: Always call `trading_strategies_free_output()` after processing output. Use Python's `try/finally` pattern.

### Issue 3: Settings Array Mismatch

**Solution**: Create comprehensive mapping between Python settings dict and C array indices. Document all required settings.

### Issue 4: Rates Data Format

**Solution**: Standardize rates data format. Use consistent timeframe naming (M1, M5, H1, D1, etc.).

## Migration Checklist

### Before Starting
- [ ] Review `REFACTORING_PLAN.md`
- [ ] Understand `StrategyParams` structure
- [ ] Understand `StrategyResults` structure
- [ ] Identify all required settings
- [ ] Set up build environment

### Implementation
- [ ] Create `TradingStrategiesPythonAPI.c`
- [ ] Implement input conversion
- [ ] Implement output conversion
- [ ] Implement memory management
- [ ] Update build system
- [ ] Build shared library

### Testing
- [ ] Create C unit tests
- [ ] Test with minimal data
- [ ] Test with real data
- [ ] Test error cases
- [ ] Test Python wrapper
- [ ] Test broker integration

### Documentation
- [ ] Document API
- [ ] Document settings mapping
- [ ] Create usage examples
- [ ] Update README

### Cleanup
- [ ] Remove DLL-specific code
- [ ] Remove unused files
- [ ] Update documentation
- [ ] Code review

## Next Actions

1. **Review the plan** - Ensure it meets your requirements
2. **Clarify questions** - Resolve any ambiguities
3. **Start Phase 1** - Implement C API
4. **Iterate** - Test and refine

## Questions to Answer

Before starting implementation, clarify:

1. **Settings Mapping**: Do you have a complete list of all settings indices?
2. **Rates Data**: Which timeframes are required? All or subset?
3. **File I/O**: Keep file I/O as optional fallback or remove completely?
4. **State Persistence**: How should strategy state be persisted in Python?
5. **Thread Safety**: Are strategies thread-safe? Need concurrent execution?
6. **Broker API**: Which broker(s) will you integrate with?

## Support

For questions or issues:
1. Review `REFACTORING_PLAN.md` for detailed options
2. Check `include/TradingStrategiesPythonAPI.h` for API reference
3. Review `python/example_usage.py` for usage patterns

## Timeline Estimate

- **Phase 1-3** (C API + Build + Testing): 4-5 days
- **Phase 4** (Python Testing): 1 day
- **Phase 5** (Broker Integration): 2-3 days
- **Phase 6** (Cleanup): 1 day

**Total**: 8-10 days for complete implementation

**Minimum Viable Product**: Phases 1-4 (6-7 days) - Basic Python integration working

