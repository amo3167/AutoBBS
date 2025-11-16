# Installing Premake4 on macOS (ARM64)

## Overview

Premake4 is required for full compatibility with the existing build scripts in this project. The main `premake4.lua` file uses premake4-specific syntax (like `StaticRuntime` flag and `os.get()`) that is not compatible with premake5.

## Installation Options

### Option 1: Build from Source (Recommended for ARM64)

Since pre-built binaries for ARM64 macOS are not available, you'll need to build premake4 from source:

1. **Clone the premake4 repository:**
   ```bash
   cd /tmp
   git clone --depth 1 https://github.com/premake/premake-4.x.git
   cd premake-4.x
   ```

2. **Install Lua (if not already installed):**
   ```bash
   brew install lua
   ```

3. **Build premake4:**
   - The build process requires a working premake4 to bootstrap itself
   - You may need to use premake5 with modifications, or manually create a Makefile
   - See the BUILD.txt file in the premake4 source for detailed instructions

4. **Install the binary:**
   ```bash
   cp bin/release/premake4 /usr/local/bin/premake4
   # Or to your homebrew bin:
   cp bin/release/premake4 ~/homebrew/bin/premake4
   chmod +x /usr/local/bin/premake4
   ```

### Option 2: Use Intel Binary with Rosetta 2 (Not Recommended)

The pre-built Intel binaries (i386/x86_64) are not compatible with ARM64 macOS, even with Rosetta 2, as they are 32-bit.

### Option 3: Use Docker (Alternative)

You could use a Docker container with a Linux environment that has premake4 pre-built:

```bash
docker run -it -v $(pwd):/workspace ubuntu:20.04
# Inside container:
apt-get update && apt-get install -y premake4
```

## Verification

After installation, verify premake4 works:

```bash
premake4 --version
# Should output: premake4 (Premake Build Script Generator) 4.4-beta5
```

## Current Status

- ✅ Premake5 is installed (`/Users/andym/homebrew/bin/premake5`)
- ⚠️ Premake4 is not installed
- ⚠️ Pre-built ARM64 binaries are not available
- ✅ Lua is installed (can be used for building)

## Next Steps

1. Build premake4 from source (see Option 1 above)
2. Or modify the main `premake4.lua` to be compatible with premake5 (see `core/TradingStrategies/PREMAKE_COMPATIBILITY.md`)

## Notes

- The `TradingStrategies/premake4.lua` file is already compatible with both premake4 and premake5
- The main project `premake4.lua` requires premake4 for full compatibility
- Building premake4 from source on ARM64 requires some manual work due to the bootstrap requirement

