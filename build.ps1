param(
    [string]$Config = "debug64",
    [string]$Project = "AsirikuyFrameworkAPI",
    [switch]$Clean,
    [switch]$Verbose,
    [switch]$Help
)

function Write-Info($msg) { Write-Host $msg -ForegroundColor Cyan }
function Write-Warn($msg) { Write-Host $msg -ForegroundColor Yellow }
function Write-Err($msg) { Write-Host $msg -ForegroundColor Red }

if ($Help) {
    Write-Host "Usage: .\\build.ps1 [-Config <debug32|debug64|release32|release64>] [-Project <name|all>] [-Clean] [-Verbose] [-Help]"
    exit 0
}

$repoRoot = Resolve-Path "." | Select-Object -ExpandProperty Path
Push-Location $repoRoot

if (-not $env:BOOST_ROOT) {
    $defaultBoost = "e:\\workspace\\boost_1_49_0"
    if (Test-Path $defaultBoost) { $env:BOOST_ROOT = $defaultBoost; Write-Info "BOOST_ROOT=$env:BOOST_ROOT" } else { Write-Warn "BOOST_ROOT not set" }
}

if ($Clean) {
    foreach ($p in @("build\\vs2010","bin\\vs2010","tmp\\vs2010","build\\gmake","bin\\gmake","tmp\\gmake")) { if (Test-Path $p) { Write-Info "Cleaning $p"; Remove-Item -LiteralPath $p -Recurse -Force -ErrorAction SilentlyContinue } }
}

$premakeExe = Join-Path $repoRoot "premake4.exe"
if (Test-Path $premakeExe) {
    $premakeArgs = "--file=premake4.lua vs2010"
    Write-Info "Running premake4.exe $premakeArgs"
    & $premakeExe --file=premake4.lua vs2010
    if ($LASTEXITCODE -ne 0) { Write-Err "premake failed ($LASTEXITCODE)"; Pop-Location; exit $LASTEXITCODE }
} else {
    Write-Warn "premake4.exe not found in repo root"
}

$slnPath = Join-Path $repoRoot "build\\vs2010\\AsirikuyFramework.sln"
if (-not (Test-Path $slnPath)) { Write-Err "Solution not found: $slnPath"; Pop-Location; exit 1 }

switch ($Config.ToLower()) {
    "debug32"   { $configuration = "Debug";   $platform = "Win32" }
    "debug64"   { $configuration = "Debug";   $platform = "x64" }
    "release32" { $configuration = "Release"; $platform = "Win32" }
    "release64" { $configuration = "Release"; $platform = "x64" }
    default      { $configuration = "Debug";   $platform = "x64" }
}

$msbuild = (Get-Command msbuild.exe -ErrorAction SilentlyContinue)
if ($null -eq $msbuild) {
    $possible = @(
        "C:\\Program Files\\Microsoft Visual Studio\\2022\\MSBuild\\Current\\Bin\\MSBuild.exe",
        "C:\\Program Files (x86)\\Microsoft Visual Studio\\2022\\MSBuild\\Current\\Bin\\MSBuild.exe",
        "C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\MSBuild\\Current\\Bin\\MSBuild.exe",
        "C:\\Program Files\\Microsoft Visual Studio\\2022\\Professional\\MSBuild\\Current\\Bin\\MSBuild.exe",
        "C:\\Program Files\\Microsoft Visual Studio\\2022\\Enterprise\\MSBuild\\Current\\Bin\\MSBuild.exe"
    )
    foreach ($p in $possible) { if (Test-Path $p) { $msbuildPath = $p; break } }
} else { $msbuildPath = $msbuild.Path }
if (-not $msbuildPath) { Write-Err "msbuild.exe not found"; Pop-Location; exit 1 }

$verbosity = if ($Verbose) { "/v:normal" } else { "/v:minimal" }
$target = if ($Project -and ($Project -ne "all")) { "/t:$Project" } else { "" }
$cmdArgs = @("$slnPath","/m","/p:Configuration=$configuration","/p:Platform=$platform","/p:TrackFileAccess=false",$verbosity)
if ($target -ne "") { $cmdArgs += $target }

Write-Info "Building $slnPath [$configuration,$platform] $([string]::Join(' ', $cmdArgs))"
& $msbuildPath @cmdArgs
$exit = $LASTEXITCODE
if ($exit -ne 0) { Write-Err "msbuild failed ($exit)"; Pop-Location; exit $exit }

Write-Info "Verifying outputs"
Get-ChildItem -Recurse -ErrorAction SilentlyContinue -Path (Join-Path $repoRoot "bin") -File -Include *.dll,*.lib | Select-Object -First 20 | ForEach-Object { Write-Host $_.FullName }

Pop-Location
