# AsirikuyFramework Release Package

**Release:** AsirikuyFramework-20251206-083254  
**Generated:** 2025-12-06 08:32:54  
**Platform:** Windows x64  

## Contents

### Executables & Libraries

- **bin/** - DLL files (shared libraries)
  - AsirikuyFrameworkAPI.dll - Main framework DLL (37 exported functions)
  - Other framework DLLs

- **lib/** - LIB files (import/static libraries)
  - AsirikuyFrameworkAPI.lib - Import library for linking
  - trading_strategies.lib - Strategy engine
  - Other support libraries

### Headers

- **include/** - C/C++ header files organized by module
  - AsirikuyFrameworkAPI/ - Main framework headers
  - TradingStrategies/ - Strategy engine headers
  - AsirikuyCommon/ - Common utilities
  - And others...

### Documentation

- **docs/** - Markdown documentation files
  - API guides
  - Usage examples
  - Architecture documentation

## Integration Instructions

### 1. For MT4 Integration

Copy AsirikuyFrameworkAPI.dll to MT4's Experts folder:
`
C:\Program Files\MetaTrader 4\experts\
`

Create an Expert Advisor that imports and calls:
- initInstanceMQL4() - Initialize with MT4 context
- mql4_runStrategy() - Execute strategy
- deinitInstance() - Cleanup

### 2. For C/C++ Development

Link against the import library:
`
lib/AsirikuyFrameworkAPI.lib
`

Include headers from:
`
include/AsirikuyFrameworkAPI/
`

### 3. Exported Functions (37 total)

Key functions:
- Core: initInstanceC, initInstanceMQL4, initInstanceMQL5, deinitInstance
- Execution: c_runStrategy, mql4_runStrategy, mql5_runStrategy, jf_runStrategy
- Utilities: Symbol parsing, currency conversion, info queries

Full function list available in API documentation.

## Build Information

- **Compiler:** MSVC (Visual Studio 2022)
- **Target:** x64 PE32+
- **Size:** AsirikuyFrameworkAPI.dll is approximately 681 KB
- **Dependencies:** Windows SDK, no external runtime dependencies
- **Note:** Windows stubs provided for NTP and EasyTrade functionality

## License

See individual source files for licensing information.

## Support

For issues or questions, contact the development team.
