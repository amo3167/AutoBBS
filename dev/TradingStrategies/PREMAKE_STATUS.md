# Premake5 Status Check

## Premake Status

### ✅ Premake5 Available

**Location**: `/Users/andym/homebrew/bin/premake5`  
**Version**: 5.0.0-beta6  
**Status**: ⚠️ Installed but has compatibility issues

### ⚠️ Compatibility Issues

Premake5 has **incompatibilities** with premake4.lua syntax:
- ❌ `StaticRuntime` flag is not supported in premake5
- ❌ `os.get()` is deprecated (should use `os.target()` or `os.host()`)
- ⚠️ Some premake4 syntax may not work correctly

### Recommendation

**Option 1: Use Premake4** (Recommended)
- Install premake4 for full compatibility
- Download from: https://premake.github.io/download.html
- Or build from source

**Option 2: Fix Premake5 Compatibility**
- Update `premake4.lua` to remove `StaticRuntime` flag
- Update deprecated `os.get()` calls
- Test thoroughly

**Option 3: Test with Premake5 Anyway**
- Some projects may still work despite warnings
- Test the build and fix issues as they arise

## Requirements

### 1. BOOST_ROOT Environment Variable

The build system **requires** `BOOST_ROOT` to be set:

```bash
export BOOST_ROOT=/path/to/boost
```

**Check if set:**
```bash
echo $BOOST_ROOT
```

**Find Boost (if installed via Homebrew):**
```bash
brew --prefix boost
# Output: /Users/andym/homebrew
# So BOOST_ROOT should be: /Users/andym/homebrew
```

**Current Status:**
- ✅ Boost is installed via Homebrew
- ✅ Location: `/Users/andym/homebrew/Cellar/boost/1.89.0/`
- ✅ Set BOOST_ROOT: `export BOOST_ROOT=$(brew --prefix boost)`

### 2. Generate Build Files

Once `BOOST_ROOT` is set:

```bash
cd /Users/andym/projects/AutoBBS
premake5 --file=premake4.lua gmake
```

This will generate Makefiles in `build/gmake/`

### 3. Build TradingStrategies

```bash
cd build/gmake
make TradingStrategies
```

## Verification

### Check Premake5
```bash
premake5 --version
# Should output: premake5 (Premake Build Script Generator) 5.0.0-beta6
```

### Test Build Generation
```bash
cd /Users/andym/projects/AutoBBS
export BOOST_ROOT=/path/to/boost  # Set this first!
premake5 --file=premake4.lua gmake
ls -la build/gmake/  # Should show Makefile and project files
```

## Expected Output

After successful build generation:
- `build/gmake/Makefile` - Main makefile
- `build/gmake/TradingStrategies.make` - TradingStrategies project makefile
- `bin/gmake/x64/Release/lib/libtrading_strategies.dylib` - Built library (after compilation)

## Next Steps

1. **Set BOOST_ROOT** (if not already set)
2. **Generate build files**: `premake5 --file=premake4.lua gmake`
3. **Build TradingStrategies**: `cd build/gmake && make TradingStrategies`
4. **Verify library**: Check for `libtrading_strategies.dylib` in output directory

## Notes

- Premake5 is compatible with premake4.lua syntax
- The build system will exit early if BOOST_ROOT is not set
- All dependencies must be built before TradingStrategies can be built
- The library will be named `libtrading_strategies.dylib` on macOS

