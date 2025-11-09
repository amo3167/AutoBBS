param(
    [string]$Configuration = "Debug",
    [string]$Action = "vs2010",
    [ValidateSet('vs2010','vs2022')] [string]$Toolchain = 'vs2010',
    [switch]$Clean,
    [switch]$ForceLocalPremake,
    [switch]$AutoDownload,
    [string]$PremakeUrl = "https://github.com/premake/premake-4.x/releases/download/4.4/premake4.exe",
    [string]$PremakeSha256 = ""  # Optional: provide SHA256 checksum to verify downloaded binary
)

function Write-Info($msg) { Write-Host "[INFO] $msg" -ForegroundColor Cyan }
function Write-Warn($msg) { Write-Host "[WARN] $msg" -ForegroundColor Yellow }
function Write-ErrorAndExit($msg) { Write-Host "[ERROR] $msg" -ForegroundColor Red; exit 1 }

Push-Location -Path (Split-Path -Parent $MyInvocation.MyCommand.Definition)
$repoRoot = Resolve-Path ".." | Select-Object -ExpandProperty Path
Push-Location $repoRoot

Write-Info "Repository root: $repoRoot"

# Look for premake4.exe in PATH or repo root
$premakeName = "premake4.exe"
$premakeInPath = $null -ne (Get-Command $premakeName -ErrorAction SilentlyContinue)
$premakeInRepo = Test-Path (Join-Path $repoRoot $premakeName)

if ($ForceLocalPremake) {
    if (-not $premakeInRepo) { Write-Warn "ForceLocalPremake set but $premakeName not found in repo root." }
    $premakePath = Join-Path $repoRoot $premakeName
} elseif ($premakeInPath) {
    $premakePath = (Get-Command $premakeName).Path
} elseif ($premakeInRepo) {
    $premakePath = Join-Path $repoRoot $premakeName
} else {
    $premakePath = $null
}

# If not found and AutoDownload requested, try to download premake into repo root
if (-not $premakePath -and $AutoDownload) {
    Write-Info "AutoDownload requested. Attempting to download premake from: $PremakeUrl"
    $outFile = Join-Path $repoRoot $premakeName
    try {
        Invoke-WebRequest -Uri $PremakeUrl -OutFile $outFile -UseBasicParsing -ErrorAction Stop
        Write-Info "Downloaded $premakeName to $outFile"
        if ($PremakeSha256 -ne "") {
            Write-Info "Verifying SHA256 checksum..."
            $hash = (Get-FileHash -Path $outFile -Algorithm SHA256).Hash.ToLower()
            if ($hash -ne $PremakeSha256.ToLower()) {
                Write-ErrorAndExit "Checksum verification failed for downloaded premake: expected $PremakeSha256, got $hash"
            } else {
                Write-Info "Checksum verified OK"
            }
        } else {
            Write-Warn "No checksum provided. It's recommended to verify the binary before use."
        }
        $premakePath = $outFile
    } catch {
        Write-Warn "Failed to download premake: $($_.Exception.Message)"
        $premakePath = $null
    }
}

if (-not $premakePath) {
    Write-Warn "Skipping premake run. You can place premake4.exe in the repo root and re-run this script."
} else {
    Write-Info "Using premake: $premakePath"
    Write-Info "Running: $premakePath $Action"
    & $premakePath $Action
    if ($LASTEXITCODE -ne 0) { Write-ErrorAndExit "premake failed with exit code $LASTEXITCODE" }
}

# Locate generated solution
$slnPath = Join-Path $repoRoot (Join-Path "build" (Join-Path $Action "AsirikuyFramework.sln"))
if (-not (Test-Path $slnPath)) {
    Write-Warn "Solution not found at expected path: $slnPath"
    Write-Warn "If premake generated a different solution name, update the script or open the build folder manually."
    Pop-Location; Pop-Location
    exit 0
}

# Locate msbuild.exe (string path)
$msbuildPath = $null
$msbuildCmdInfo = Get-Command msbuild.exe -ErrorAction SilentlyContinue
if ($msbuildCmdInfo) { $msbuildPath = $msbuildCmdInfo.Path }

# If user requested VS2022 prefer its developer command prompt and MSBuild
if ($Toolchain -eq 'vs2022') {
    # Common VS2022 community path; user may have different edition (Enterprise/Professional)
    $vs2022DevCmd = "C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\Common7\\Tools\\VsDevCmd.bat"
    if (-not (Test-Path $vs2022DevCmd)) {
        # Try Professional/Enterprise
        $vs2022DevCmd = "C:\\Program Files\\Microsoft Visual Studio\\2022\\Professional\\Common7\\Tools\\VsDevCmd.bat"
        if (-not (Test-Path $vs2022DevCmd)) {
            $vs2022DevCmd = "C:\\Program Files\\Microsoft Visual Studio\\2022\\Enterprise\\Common7\\Tools\\VsDevCmd.bat"
        }
    }
    if (Test-Path $vs2022DevCmd) {
        Write-Info "Found VS2022 devcmd: $vs2022DevCmd"
        # Use MSBuild from VS2022 after invoking devcmd in a subshell
        # Try a few likely MSBuild locations (Program Files and Program Files (x86))
        $possibleMsbuild = @(
            "C:\\Program Files\\Microsoft Visual Studio\\2022\\MSBuild\\Current\\Bin\\MSBuild.exe",
            "C:\\Program Files (x86)\\Microsoft Visual Studio\\2022\\MSBuild\\Current\\Bin\\MSBuild.exe",
            "C:\\Program Files\\Microsoft Visual Studio\\2022\\Professional\\MSBuild\\Current\\Bin\\MSBuild.exe",
            "C:\\Program Files\\Microsoft Visual Studio\\2022\\Enterprise\\MSBuild\\Current\\Bin\\MSBuild.exe"
        )
        foreach ($p in $possibleMsbuild) { if (Test-Path $p) { $msbuildPath = $p; break } }
        if (-not $msbuildPath) { Write-Warn "VS2022 MSBuild not found at expected paths, will fall back to msbuild on PATH if available." }
    } else {
        Write-Warn "VS2022 dev command prompt (VsDevCmd.bat) not found. Falling back to msbuild on PATH." 
    }
}
if ($Toolchain -eq 'vs2022' -and (Test-Path $vs2022DevCmd)) {
    Write-Info "Invoking VS2022 dev environment and building (PlatformToolset=v143)"
    if (-not $msbuildPath) { $msbuildPath = "msbuild.exe" }
    # If requested, clean tmp/bin directories that may contain older PDBs incompatible with a newer toolset
    if ($Clean) {
        $tmpPath = Join-Path $repoRoot "tmp\vs2010"
        $binPath = Join-Path $repoRoot "bin\vs2010"
        if (Test-Path $tmpPath) {
            Write-Info "Cleaning tmp path: $tmpPath"
            Remove-Item -LiteralPath $tmpPath -Recurse -Force -ErrorAction SilentlyContinue
        }
        if (Test-Path $binPath) {
            Write-Info "Cleaning bin path: $binPath"
            Remove-Item -LiteralPath $binPath -Recurse -Force -ErrorAction SilentlyContinue
        }
    }

    $cmd = "call `"$vs2022DevCmd`" && `"$msbuildPath`" `"$slnPath`" /m /p:Configuration=$Configuration /p:PlatformToolset=v143 /p:TrackFileAccess=false"
    cmd /c $cmd
    $exit = $LASTEXITCODE
} else {
    if (-not $msbuildPath) {
        Write-Warn "msbuild.exe not found on PATH. Open a Visual Studio Developer Command Prompt or install Visual Studio Build Tools."
        Write-Info "You can still generate solution with premake and build manually in Visual Studio."
        Write-Info "Generated solution path: $slnPath"
        Pop-Location; Pop-Location
        exit 0
    }
    Write-Info "Building solution with msbuild: $msbuildPath"
    & $msbuildPath $slnPath /m /p:Configuration=$Configuration /p:TrackFileAccess=false
    $exit = $LASTEXITCODE
}
if ($exit -ne 0) { Write-ErrorAndExit "msbuild failed with exit code $exit" }

Write-Info "Build finished successfully."

Pop-Location; Pop-Location