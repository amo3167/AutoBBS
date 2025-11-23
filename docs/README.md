# AutoBBS

## Build System

### Requirements

- **Visual Studio 2022** (Community, Professional, or Enterprise)
  - Platform Toolset: v143
  - Windows SDK: 10.0.26100.0 or higher
- **Boost 1.49** (located at `e:\workspace\boost_1_49_0`)
  - Set `BOOST_ROOT` environment variable before building
- **premake4** (included in repository root)

### Build Instructions

1. **Set Boost environment variable** (PowerShell):
   ```powershell
   $env:BOOST_ROOT='e:\workspace\boost_1_49_0'
   ```

2. **Generate Visual Studio 2010 solution** (compatible with VS2022):
   ```powershell
   .\premake4.exe --file=premake4.lua vs2010
   ```

3. **Build with MSBuild**:
   ```powershell
   msbuild build\vs2010\AsirikuyFramework.sln /p:Configuration=Debug /p:Platform=Win32
   ```

   Or open `build\vs2010\AsirikuyFramework.sln` in Visual Studio 2022.

### Recent Modernization (November 2025)

The build system has been upgraded to support modern toolchains while maintaining compatibility:

#### Toolchain Updates
- **Compiler**: Upgraded from VS2010 to VS2022 (MSVC v143)
- **Windows SDK**: Retargeted to 10.0.26100.0
- **C++ Standard**: C++03 (default for compatibility with Boost 1.49)

#### Dependency Patches
- **STLSoft**: Patched `vendor/STLSoft/include/stlsoft/stlsoft.h` to accept modern MSVC versions by adjusting `_MSC_VER` version guard
- **Boost**: Retained v1.49 for C++03 compatibility (upgrade to 1.84+ requires C++11 minimum)

#### Project Changes
- **Removed**: Shark machine learning library (unmaintained, incompatible with modern toolchains)
- **Restored**: NTPClient (previously disabled, now builds successfully)
- **Threading**: Uses `boost::mutex` and `boost::thread` (C++11 `<thread>`/`<mutex>` unavailable in C++03 mode)

#### Build Scripts
- Added `scripts/build.ps1` for automated builds
- Added `scripts/upgrade_boost.ps1` for future Boost upgrades (requires C++ standard migration)

### Known Issues

- **Boost 1.84 Upgrade**: Blocked pending C++11/C++17 migration
  - Boost 1.84+ requires C++11 minimum
  - Current premake4 setup generates C++03-compatible projects
  - Requires premake script updates or migration to premake5

- **Deprecated Warnings**: 
  - `/Gm` (minimal rebuild) flag deprecated but retained for premake4 compatibility
  - Can be suppressed or removed in future premake updates

### Future Work

1. **C++ Standard Migration**: Upgrade codebase to C++17 to enable:
   - Boost 1.84+ upgrade
   - Modern STL concurrency primitives (`std::thread`, `std::mutex`)
   - Better compiler optimizations and diagnostics

2. **Build System**: Consider migrating from premake4 to premake5 or CMake for:
   - Native C++ standard support
   - Better Visual Studio 2022 integration
   - Modern toolchain feature detection
