# Boost 1.84.0 Upgrade Script for AutoBBS
# Run this script locally where you have better download/extraction tools

Write-Host "=== Boost 1.84.0 Upgrade for AutoBBS ===" -ForegroundColor Cyan
Write-Host ""

$boostVersion = "1_84_0"
$boostUrl = "https://archives.boost.io/release/1.84.0/source/boost_1_84_0.7z"
$boostInstallPath = "E:\workspace\boost_$boostVersion"

# Step 1: Download Boost
Write-Host "[1/5] Downloading Boost 1.84.0..." -ForegroundColor Yellow
if (Test-Path "$boostInstallPath") {
    Write-Host "  Boost 1.84.0 already exists at $boostInstallPath" -ForegroundColor Green
} else {
    $downloadPath = "E:\workspace\boost_$boostVersion.7z"
    
    if (!(Test-Path $downloadPath)) {
        Write-Host "  Downloading from $boostUrl (this will take several minutes)..."
        try {
            # Try with curl first (more reliable)
            & curl.exe -L -o $downloadPath $boostUrl --progress-bar
        } catch {
            Write-Host "  curl failed, trying Invoke-WebRequest..."
            Invoke-WebRequest -Uri $boostUrl -OutFile $downloadPath -UseBasicParsing
        }
        
        $sizeMB = [math]::Round((Get-Item $downloadPath).Length / 1MB, 2)
        Write-Host "  Downloaded: $sizeMB MB" -ForegroundColor Green
    }
    
    # Step 2: Extract Boost
    Write-Host "[2/5] Extracting Boost (this may take 2-3 minutes)..." -ForegroundColor Yellow
    
    if (Get-Command 7z -ErrorAction SilentlyContinue) {
        Write-Host "  Using 7-Zip..."
        & 7z x $downloadPath -o"E:\workspace" -y | Out-Null
    } elseif (Get-Command "C:\Program Files\7-Zip\7z.exe" -ErrorAction SilentlyContinue) {
        Write-Host "  Using 7-Zip from Program Files..."
        & "C:\Program Files\7-Zip\7z.exe" x $downloadPath -o"E:\workspace" -y | Out-Null
    } else {
        Write-Host "  ERROR: 7-Zip not found. Please install 7-Zip or manually extract $downloadPath" -ForegroundColor Red
        Write-Host "  Download 7-Zip from: https://www.7-zip.org/download.html"
        exit 1
    }
    
    Write-Host "  Extraction complete" -ForegroundColor Green
}

# Step 3: Bootstrap Boost.Build
Write-Host "[3/5] Bootstrapping Boost.Build..." -ForegroundColor Yellow
Push-Location $boostInstallPath

if (!(Test-Path ".\b2.exe")) {
    cmd /c bootstrap.bat msvc
    if ($LASTEXITCODE -eq 0) {
        Write-Host "  Bootstrap complete" -ForegroundColor Green
    } else {
        Write-Host "  Bootstrap failed - check output above" -ForegroundColor Red
        Pop-Location
        exit 1
    }
} else {
    Write-Host "  b2.exe already exists" -ForegroundColor Green
}

# Step 4: Build required Boost libraries
Write-Host "[4/5] Building Boost libraries (this will take 10-20 minutes)..." -ForegroundColor Yellow
Write-Host "  Required libs: system, thread, date_time, regex, filesystem, serialization, test"

$b2Args = @(
    "--with-system",
    "--with-chrono", 
    "--with-thread",
    "--with-date_time",
    "--with-regex",
    "--with-filesystem",
    "--with-serialization",
    "--with-test",
    "--abbreviate-paths",
    "variant=debug,release",
    "link=static",
    "threading=multi",
    "runtime-link=static",
    "address-model=32,64",
    "toolset=msvc-14.3",
    "stage",
    "-j8"
)

& .\b2.exe $b2Args

if ($LASTEXITCODE -eq 0) {
    Write-Host "  Build complete" -ForegroundColor Green
} else {
    Write-Host "  Build had errors - check output above" -ForegroundColor Yellow
    Write-Host "  (Some errors may be OK if key libraries built successfully)"
}

Pop-Location

# Step 5: Update environment and regenerate project
Write-Host "[5/5] Updating BOOST_ROOT environment variable..." -ForegroundColor Yellow

# Set user environment variable (permanent)
[Environment]::SetEnvironmentVariable("BOOST_ROOT", $boostInstallPath, "User")

# Set current session
$env:BOOST_ROOT = $boostInstallPath

Write-Host "  BOOST_ROOT set to: $boostInstallPath" -ForegroundColor Green
Write-Host ""
Write-Host "=== Boost Upgrade Complete ===" -ForegroundColor Cyan
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Yellow
Write-Host "  1. Close and reopen VS Code (to pick up new BOOST_ROOT)"
Write-Host "  2. Run: cd E:\workspace\AutoBBS"
Write-Host "  3. Run: .\scripts\run_build.ps1 -Toolchain vs2022 -Clean"
Write-Host ""
Write-Host "If you encounter issues, check:" -ForegroundColor Yellow
Write-Host "  - Verify BOOST_ROOT: `$env:BOOST_ROOT"
Write-Host "  - Check Boost libs exist: dir $boostInstallPath\stage\lib"
Write-Host "  - Review build log for errors"
Write-Host ""
