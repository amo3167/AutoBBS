# Missing Vendor Directories

## Summary

The `vendor/` directory does not exist in the project. The `premake4.lua` file references **16 vendor projects** that need to be included.

## Missing Vendor Projects

Based on `premake4.lua` lines 207-221 and 234, the following vendor directories are expected:

### Core Libraries (15 projects)
1. **MiniXML** - XML parsing library
2. **dSFMT** - Double precision SIMD-oriented Fast Mersenne Twister (random number generator)
3. **FANN** - Fast Artificial Neural Network library
4. **TALib** - Technical Analysis Library (C implementation)
5. **Pantheios** - Logging library
6. **Shark** - Machine learning library
7. **DevIL** - Developer's Image Library (image loading)
8. **Jasper** - JPEG-2000 codec
9. **LibJPEG** - JPEG image library
10. **LibMNG** - Multiple-image Network Graphics library
11. **LibPNG** - PNG image library
12. **LittleCMS** - Color management system
13. **Zlib** - Compression library
14. **Gaul** - Genetic Algorithm Utility Library
15. **Waffles** - Machine learning toolkit

### Platform-Specific (1 project)
16. **curl** - HTTP client library (Windows only, line 234)

## Impact

When running `./docker-premake4.sh gmake`, you'll see errors like:
```
cannot open /workspace/vendor/MiniXML/premake4.lua: No such file or directory
```

This prevents the build system from generating complete Makefiles.

## Solutions

### Option 1: Add Missing Vendor Directories
If these libraries are required for your build:
1. Create the `vendor/` directory
2. Add each vendor library with its own `premake4.lua` file
3. Ensure each vendor project follows the expected structure

### Option 2: Make Vendor Includes Optional
Modify `premake4.lua` to conditionally include vendor projects only if they exist:

```lua
-- Example conditional include
if os.isdir("vendor/MiniXML") then
    include "vendor/MiniXML"
end
```

### Option 3: Comment Out Unused Vendors
If you don't need all vendor libraries, comment out the ones you don't use:

```lua
-- include "vendor/MiniXML"  -- Not needed for current build
-- include "vendor/Shark"    -- Not needed for current build
```

## Next Steps

1. **Determine which vendors are actually needed:**
   - Check which projects in `core/` actually depend on vendor libraries
   - Review include paths in `premake4.lua` to see which are referenced

2. **For TradingStrategies specifically:**
   - Check if `core/TradingStrategies` depends on any vendor libraries
   - If not, you may be able to build it independently

3. **Create a minimal build configuration:**
   - Start with just the projects you need
   - Add vendors as dependencies are discovered

## Checking Dependencies

To see which vendor libraries are actually used, check:
- Include paths in `premake4.lua` (lines 45-103)
- Link dependencies in individual project `premake4.lua` files
- Source code includes in `core/` projects

