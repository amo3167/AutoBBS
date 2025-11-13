# TradingStrategies Build Test - macOS

## Prerequisites

1. **premake4** or **premake5** installed
2. **BOOST_ROOT** environment variable set
3. All dependencies built (AsirikuyCommon, Log, OrderManager, etc.)

## Build Steps

### 1. Set Environment Variables

```bash
export BOOST_ROOT=/path/to/boost
```

### 2. Generate Build Files

From `/Users/andym/projects/AutoBBS`:

```bash
# Using premake4
premake4 gmake

# OR using premake5 (if compatible)
premake5 gmake
```

### 3. Build TradingStrategies

```bash
cd build/gmake
make TradingStrategies
```

### 4. Verify Output

The shared library should be created at:
- **Debug**: `bin/gmake/x64/Debug/lib/libtrading_strategies.dylib`
- **Release**: `bin/gmake/x64/Release/lib/libtrading_strategies.dylib`

### 5. Test Library Loading

```bash
# Check library exports
nm -gU bin/gmake/x64/Release/lib/libtrading_strategies.dylib | grep trading_strategies

# Expected exports:
# trading_strategies_run
# trading_strategies_free_output
# trading_strategies_get_error_message
# trading_strategies_get_version
# trading_strategies_init
# trading_strategies_cleanup
```

### 6. Test Python Loading

```python
import ctypes
lib = ctypes.CDLL('bin/gmake/x64/Release/lib/libtrading_strategies.dylib')
print("Library loaded successfully!")
```

## Expected Library Name

On macOS, the library will be named:
- `libtrading_strategies.dylib` (with `targetname "trading_strategies"`)

## Troubleshooting

### Issue: Library not found
- Check that the library is in the expected output directory
- Verify `targetname` is set correctly in premake4.lua

### Issue: Missing symbols
- Ensure all dependencies are built first
- Check that `uses` statements are correct

### Issue: Link errors
- Verify all dependency libraries are available
- Check include paths are correct

## Build Configuration Summary

- **Kind**: SharedLib
- **Target Name**: trading_strategies
- **Output**: libtrading_strategies.dylib (macOS)
- **Dependencies**: AsirikuyCommon, Log, OrderManager, AsirikuyTechnicalAnalysis, AsirikuyEasyTrade

