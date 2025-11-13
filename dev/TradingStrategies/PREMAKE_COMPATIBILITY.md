# Premake4 vs Premake5 Compatibility

## Issue Found

When running `premake5 --file=premake4.lua gmake`, we get:

```
Error: invalid value 'StaticRuntime' for flags
```

## Root Cause

The main `premake4.lua` file uses `flags{"StaticRuntime", "Unsafe"}` which is premake4 syntax. Premake5 doesn't support `StaticRuntime` flag.

## Solutions

### Option 1: Install Premake4 (Recommended)

Premake4 is the original version and fully supports the existing build scripts.

**Download Premake4:**
- Official: https://premake.github.io/download.html
- Or build from source: https://github.com/premake/premake-4.x

**Install:**
```bash
# Download premake4 binary for macOS
# Place in PATH or use directly
```

### Option 2: Fix Premake5 Compatibility

Update the main `premake4.lua` to be compatible with premake5:

1. Remove or comment out `StaticRuntime` flag
2. Replace `os.get()` with `os.target()` or `os.host()`
3. Test build generation

**Changes needed in `/Users/andym/projects/AutoBBS/premake4.lua`:**

```lua
-- Change this:
flags{"StaticRuntime", "Unsafe"}

-- To this (premake5 compatible):
flags{"Unsafe"}
-- Note: StaticRuntime is not needed for shared libraries anyway
```

```lua
-- Change this:
if os.get() == "windows" and not os.isfile("b2.exe") then
elseif os.get() ~= "windows" and not os.isfile("./b2") then

-- To this:
if os.target() == "windows" and not os.isfile("b2.exe") then
elseif os.target() ~= "windows" and not os.isfile("./b2") then
```

### Option 3: Use Premake5 with Workarounds

1. Create a wrapper script that handles the compatibility
2. Or manually fix the generated Makefiles if needed

## Current Status

- ✅ Premake5 is installed
- ✅ Boost is installed (`/Users/andym/homebrew/opt/boost`)
- ⚠️ Compatibility issues prevent automatic build generation
- ✅ TradingStrategies premake4.lua is correct (no issues there)

## Recommendation

For now, **TradingStrategies/premake4.lua is correctly configured**. The issue is in the main `premake4.lua` file. 

**Next Steps:**
1. Either install premake4, OR
2. Fix the main premake4.lua for premake5 compatibility, OR  
3. Test if TradingStrategies can be built independently (if possible)

## Quick Test

To test if TradingStrategies build config is correct (ignoring main premake issues):

```bash
# Set BOOST_ROOT
export BOOST_ROOT=$(brew --prefix boost)

# The TradingStrategies/premake4.lua itself should be fine
# The issue is in the root premake4.lua that includes all projects
```

