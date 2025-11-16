# Premake4 — Download & Quick Start

This project uses Premake4 to generate Visual Studio 2010 solutions for the TradingStrategies module.

Why this file
- The repository doesn't include a premake4 binary. Use this guide to download premake4.exe and run the quick build helper script.

Download premake4.exe
1. Visit the Premake 4.x releases page: https://github.com/premake/premake-4.x/releases
2. Download the `premake4.exe` binary for Windows. (If an official prebuilt binary isn't available, build premake4 from source using the instructions on the repo.)
3. Place `premake4.exe` in one of the following locations:
   - The repository root (recommended)
   - `dev/TradingStrategies/`
   - Any folder on your PATH

Quick build using PowerShell
1. Open a Visual Studio Developer Command Prompt (recommended) or a normal PowerShell session.
2. From the repository root run the helper script:

```powershell
.\scripts\run_build.ps1 -Configuration Debug
```

- Checks for `premake4.exe` (PATH or repo root)
- Runs `premake4.exe vs2010` to generate the solution into `build\vs2010`
- Attempts to locate `msbuild.exe` and runs it on the generated solution
- If `msbuild.exe` is not found, the script prints instructions to open the Developer Command Prompt or install Visual Studio Build Tools
What the script does
- Checks for `premake4.exe` (PATH or repo root)
- Runs `premake4.exe vs2010` to generate the solution into `build\vs2010`
- Attempts to locate `msbuild.exe` and runs it on the generated solution
- If `msbuild.exe` is not found, the script prints instructions to open the Developer Command Prompt or install Visual Studio Build Tools
- Optionally can auto-download `premake4.exe` into the repo root (pass `-AutoDownload`) and optionally verify SHA256 with `-PremakeSha256 <checksum>`
- Checks for `premake4.exe` (PATH or repo root)
- Runs `premake4.exe vs2010` to generate the solution into `build\vs2010`
- Attempts to locate `msbuild.exe` and runs it on the generated solution
- If `msbuild.exe` is not found, the script prints instructions to open the Developer Command Prompt or install Visual Studio Build Tools

Notes
- This project targets Visual Studio 2010 (`vs2010`). If you want to target a different Visual Studio version, update `dev/TradingStrategies/premake4.lua` and call premake with the appropriate action (for example `vs2019`), then update the msbuild path/command accordingly.
- The helper script is intentionally conservative: it won't download binaries automatically. Place `premake4.exe` manually to keep the repo free of binaries.

- If you want me to automatically download a premake4.exe during the script run, tell me and I can add an opt-in download step (PowerShell `Invoke-WebRequest`) with checksum verification.
Support
- The script supports an opt-in automatic download of `premake4.exe` using the `-AutoDownload` flag and `-PremakeUrl` to override the URL. To enable SHA256 verification, pass `-PremakeSha256 <hex>`.
- Example (downloads premake4 into repo root and verifies checksum):

```powershell
.\scripts\run_build.ps1 -AutoDownload -PremakeSha256 "<hex-sha256>" -Configuration Debug
```

If you'd like me to add an automatic, opt-in download step to the script as well, I've already implemented it; just run the example above.
- If you want me to automatically download a premake4.exe during the script run, tell me and I can add an opt-in download step (PowerShell `Invoke-WebRequest`) with checksum verification.
