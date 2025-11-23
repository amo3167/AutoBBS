# Vendor Libraries Analysis

## Summary

**Good News**: Most vendor libraries are **NOT required** for building `TradingStrategies`! They're only needed for the full `AsirikuyFrameworkAPI` build.

## What TradingStrategies Actually Needs

### ✅ Direct Dependencies (No Vendors!)
- `AsirikuyCommon` - ✅ Exists
- `Log` - ✅ Exists  
- `OrderManager` - ✅ Exists
- `AsirikuyTechnicalAnalysis` - ✅ Exists
- `AsirikuyEasyTrade` - ✅ Exists

### ⚠️ Optional/Commented Dependencies
- **Pantheios** - Used for logging in source code, but many calls are commented out
- **FANN** - Has `FANN_NO_DLL` define, suggesting it might be optional

## Vendor Libraries Breakdown

### Required for Full Framework (AsirikuyFrameworkAPI)
These are only needed if building the full framework API:

1. **Pantheios** - Logging library (used in TradingStrategies but can be replaced)
2. **ChartDirector** - Charting library (Windows/Linux only)
3. **DevIL** - Image loading
4. **TALib** - Technical Analysis (might be used)
5. **FANN** - Neural networks (optional based on `FANN_NO_DLL`)
6. **Shark/Waffles** - Machine learning (optional)
7. **Image libraries** (JPEG, PNG, MNG) - Only for image processing
8. **MiniXML** - XML parsing (only if XML needed)
9. **Zlib** - Compression (usually available as system library)
10. **dSFMT** - Random number generator (can use alternatives)
11. **Gaul** - Genetic algorithms (optional)
12. **MPICH2** - MPI library (only for parallel processing)
13. **STLSoft** - C++ utilities (might be optional)
14. **curl** - HTTP client (Windows only, can use system libcurl)

## Recommendations

### Option 1: Build TradingStrategies Only (Recommended)
**You can build TradingStrategies WITHOUT any vendor libraries!**

1. Make vendor includes conditional in `premake4.lua`
2. Build only the projects you need
3. Replace Pantheios logging with standard logging or remove it

### Option 2: Use System Libraries Instead
Many of these have modern system equivalents:

- **Zlib** → `brew install zlib` (usually already installed)
- **curl** → `brew install curl` (usually already installed)  
- **PNG/JPEG** → System libraries available
- **TALib** → Can install via `brew install ta-lib` or use system version

### Option 3: Download Old Vendors (Not Recommended)
- These libraries are very old (10+ years)
- May have security vulnerabilities
- Hard to maintain
- Better to use modern alternatives

## Action Plan

### For TradingStrategies Build:

1. **Comment out vendor includes** in `premake4.lua` (lines 207-221)
2. **Make Pantheios optional** - replace with standard logging or remove
3. **Build only needed projects**:
   ```lua
   -- Comment out vendor includes
   -- include "vendor/MiniXML"
   -- include "vendor/dSFMT"
   -- ... etc
   
   -- Keep only what you need
   include "core/AsirikuyCommon"
   include "core/Log"
   include "core/OrderManager"
   include "core/AsirikuyTechnicalAnalysis"
   include "core/AsirikuyEasyTrade"
   include "core/TradingStrategies"
   ```

### For Full Framework Build:

If you need the full framework later, you can:
1. Install modern equivalents via Homebrew
2. Use system libraries where possible
3. Only download vendors that have no modern alternative

## Next Steps

Would you like me to:
1. ✅ Create a minimal `premake4.lua` that builds only TradingStrategies?
2. ✅ Replace Pantheios logging with standard logging?
3. ✅ Set up conditional vendor includes?

