@echo off
REM Simple parallel build for AutoBBS - uses MSBuild /m flag for parallelism
REM Usage: build-parallel-simple.bat [clean]
REM   clean = perform full clean before build
REM   (default) = incremental build, skip clean

setlocal enabledelayedexpansion
set BOOST_ROOT=e:\workspace\boost_1_49_0
set DO_CLEAN=0

REM Check for clean parameter
if "%1"=="clean" set DO_CLEAN=1

REM Find MSBuild
echo Checking for MSBuild...
set "MSBUILD=C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
if not exist "%MSBUILD%" (
    echo ERROR: MSBuild not found
    exit /b 1
)
echo Found MSBuild

if %DO_CLEAN%==1 (
    echo ========================================================================
    echo   AutoBBS Parallel Build (CLEAN + BUILD^) - x64 and Win32
    echo ========================================================================
) else (
    echo ========================================================================
    echo   AutoBBS Parallel Build (INCREMENTAL^) - x64 and Win32
    echo ========================================================================
)
echo Using MSBuild with /m flag for parallel compilation
echo Building both x64 and Win32 platforms
echo.

pushd "%~dp0\.."
echo Current directory: %cd%

if %DO_CLEAN%==1 (
    echo.
    echo Cleaning build directories ^(preserving vendor libraries^)...
    set VENDOR_BACKUP_NEEDED_X64=0
    set VENDOR_BACKUP_NEEDED_WIN32=0
    
    REM Check x64 vendor libraries
    if exist "bin\vs2010\x64\Release\lib\TALib_common.lib" set VENDOR_BACKUP_NEEDED_X64=1
    if exist "bin\vs2010\x64\Release\lib\TALib_abstract.lib" set VENDOR_BACKUP_NEEDED_X64=1
    if exist "bin\vs2010\x64\Release\lib\TALib_func.lib" set VENDOR_BACKUP_NEEDED_X64=1
    if exist "bin\vs2010\x64\Release\lib\MiniXML.lib" set VENDOR_BACKUP_NEEDED_X64=1
    
    REM Check Win32 vendor libraries (premake uses x32 as platform name)
    if exist "bin\vs2010\x32\Release\lib\TALib_common.lib" set VENDOR_BACKUP_NEEDED_WIN32=1
    if exist "bin\vs2010\x32\Release\lib\TALib_abstract.lib" set VENDOR_BACKUP_NEEDED_WIN32=1
    if exist "bin\vs2010\x32\Release\lib\TALib_func.lib" set VENDOR_BACKUP_NEEDED_WIN32=1
    if exist "bin\vs2010\x32\Release\lib\MiniXML.lib" set VENDOR_BACKUP_NEEDED_WIN32=1

    REM Backup x64 vendor libraries
    if !VENDOR_BACKUP_NEEDED_X64!==1 (
        if not exist "tmp\vendor_backup_x64" mkdir tmp\vendor_backup_x64
        if exist "bin\vs2010\x64\Release\lib\TALib_common.lib" copy /y "bin\vs2010\x64\Release\lib\TALib_common.lib" tmp\vendor_backup_x64\ >nul
        if exist "bin\vs2010\x64\Release\lib\TALib_abstract.lib" copy /y "bin\vs2010\x64\Release\lib\TALib_abstract.lib" tmp\vendor_backup_x64\ >nul
        if exist "bin\vs2010\x64\Release\lib\TALib_func.lib" copy /y "bin\vs2010\x64\Release\lib\TALib_func.lib" tmp\vendor_backup_x64\ >nul
        if exist "bin\vs2010\x64\Release\lib\MiniXML.lib" copy /y "bin\vs2010\x64\Release\lib\MiniXML.lib" tmp\vendor_backup_x64\ >nul
    )
    
    REM Backup Win32 vendor libraries
    if !VENDOR_BACKUP_NEEDED_WIN32!==1 (
        if not exist "tmp\vendor_backup_win32" mkdir tmp\vendor_backup_win32
        if exist "bin\vs2010\x32\Release\lib\TALib_common.lib" copy /y "bin\vs2010\x32\Release\lib\TALib_common.lib" tmp\vendor_backup_win32\ >nul
        if exist "bin\vs2010\x32\Release\lib\TALib_abstract.lib" copy /y "bin\vs2010\x32\Release\lib\TALib_abstract.lib" tmp\vendor_backup_win32\ >nul
        if exist "bin\vs2010\x32\Release\lib\TALib_func.lib" copy /y "bin\vs2010\x32\Release\lib\TALib_func.lib" tmp\vendor_backup_win32\ >nul
        if exist "bin\vs2010\x32\Release\lib\MiniXML.lib" copy /y "bin\vs2010\x32\Release\lib\MiniXML.lib" tmp\vendor_backup_win32\ >nul
    )

    if exist bin\vs2010 rmdir /s /q bin\vs2010
    if exist tmp\vs2010 rmdir /s /q tmp\vs2010
    if exist build\vs2010\projects rmdir /s /q build\vs2010\projects

    REM Restore x64 vendor libraries
    if !VENDOR_BACKUP_NEEDED_X64!==1 (
        echo Restoring x64 vendor libraries...
        if not exist "bin\vs2010\x64\Release\lib" mkdir bin\vs2010\x64\Release\lib
        copy /y tmp\vendor_backup_x64\*.lib bin\vs2010\x64\Release\lib\ >nul
        rmdir /s /q tmp\vendor_backup_x64
    )
    
    REM Restore Win32 vendor libraries
    if !VENDOR_BACKUP_NEEDED_WIN32!==1 (
        echo Restoring Win32 vendor libraries...
        if not exist "bin\vs2010\x32\Release\lib" mkdir bin\vs2010\x32\Release\lib
        copy /y tmp\vendor_backup_win32\*.lib bin\vs2010\x32\Release\lib\ >nul
        rmdir /s /q tmp\vendor_backup_win32
    )
    echo Regenerating project files with premake4...
    premake4.exe --file=premake4.lua vs2010
    if !ERRORLEVEL! NEQ 0 (
        echo ERROR: Premake4 failed
        exit /b 1
    )
    echo Updating toolset to v143...
    powershell -ExecutionPolicy Bypass -File scripts\update-toolset.ps1
    echo.
) else (
    REM Check if projects exist for incremental build
    if not exist "build\vs2010\AsirikuyFramework.sln" (
        echo Generating project files with premake4...
        premake4.exe --file=premake4.lua vs2010
        if !ERRORLEVEL! NEQ 0 (
            echo ERROR: Premake4 failed
            exit /b 1
        )
        echo Updating toolset to v143...
        powershell -ExecutionPolicy Bypass -File scripts\update-toolset.ps1
        echo.
    )
)

REM Phase 1: Build vendor libraries in parallel (if needed)
echo ========================================================================
echo Building x64 Platform
echo ========================================================================
if not exist "bin\vs2010\x64\Release\lib\TALib_func.lib" (
    echo Building x64 vendor libraries in parallel...
    "%MSBUILD%" build\vs2010\AsirikuyFramework.sln ^
        /t:TALib_common;TALib_abstract;TALib_func;MiniXML ^
        /p:Configuration=Release /p:Platform=x64 /m /v:minimal /nologo
    if !ERRORLEVEL! NEQ 0 (
        echo ERROR: x64 Vendor build failed
        exit /b 1
    )
    echo.
)

echo ========================================================================
echo Building Win32 Platform
echo ========================================================================
if not exist "bin\vs2010\x32\Release\lib\TALib_func.lib" (
    echo Building Win32 vendor libraries in parallel...
    "%MSBUILD%" build\vs2010\AsirikuyFramework.sln ^
        /t:TALib_common;TALib_abstract;TALib_func;MiniXML ^
        /p:Configuration=Release /p:Platform=Win32 /m /v:minimal /nologo
    if !ERRORLEVEL! NEQ 0 (
        echo ERROR: Win32 Vendor build failed
        exit /b 1
    )
    echo.
)

REM Phase 2: Build independent core libs in parallel - x64
echo Building x64 independent libraries in parallel...
"%MSBUILD%" build\vs2010\AsirikuyFramework.sln ^
    /t:AsirikuyCommon;Log;SymbolAnalyzer;AsirikuyTechnicalAnalysis;NTPClient ^
    /p:Configuration=Release /p:Platform=x64 /p:BuildProjectReferences=false /m /v:minimal /nologo
if !ERRORLEVEL! NEQ 0 (
    echo ERROR: x64 Phase 2 build failed
    exit /b 1
)
echo.

REM Phase 3: Build dependent chain sequentially - x64
echo Building x64 dependent libraries...

"%MSBUILD%" build\vs2010\projects\OrderManager.vcxproj /p:Configuration=Release /p:Platform=x64 /p:BuildProjectReferences=false /v:minimal /nologo
if !ERRORLEVEL! NEQ 0 (
    echo ERROR: x64 OrderManager failed
    exit /b 1
)

"%MSBUILD%" build\vs2010\projects\AsirikuyEasyTrade.vcxproj /p:Configuration=Release /p:Platform=x64 /p:BuildProjectReferences=false /v:minimal /nologo
if !ERRORLEVEL! NEQ 0 (
    echo ERROR: x64 AsirikuyEasyTrade failed
    exit /b 1
)

"%MSBUILD%" build\vs2010\projects\TradingStrategies.vcxproj /p:Configuration=Release /p:Platform=x64 /p:BuildProjectReferences=false /v:minimal /nologo
if !ERRORLEVEL! NEQ 0 (
    echo ERROR: x64 TradingStrategies failed
    exit /b 1
)

echo.
echo Building x64 AsirikuyFrameworkAPI.dll...
"%MSBUILD%" build\vs2010\projects\AsirikuyFrameworkAPI.vcxproj /p:Configuration=Release /p:Platform=x64 /p:BuildProjectReferences=false /v:minimal /nologo
if !ERRORLEVEL! NEQ 0 (
    echo ERROR: x64 AsirikuyFrameworkAPI failed
    exit /b 1
)

echo.
echo Building x64 CTesterFrameworkAPI.dll...
"%MSBUILD%" build\vs2010\projects\CTesterFrameworkAPI.vcxproj /p:Configuration=Release /p:Platform=x64 /p:BuildProjectReferences=false /v:minimal /nologo
if !ERRORLEVEL! NEQ 0 (
    echo ERROR: x64 CTesterFrameworkAPI failed
    exit /b 1
)

echo.
echo ========================================================================
echo Building Win32 Platform Components
echo ========================================================================

REM Phase 2: Build independent core libs in parallel - Win32
echo Building Win32 independent libraries in parallel...
"%MSBUILD%" build\vs2010\AsirikuyFramework.sln ^
    /t:AsirikuyCommon;Log;SymbolAnalyzer;AsirikuyTechnicalAnalysis;NTPClient ^
    /p:Configuration=Release /p:Platform=Win32 /p:BuildProjectReferences=false /m /v:minimal /nologo
if !ERRORLEVEL! NEQ 0 (
    echo ERROR: Win32 Phase 2 build failed
    exit /b 1
)
echo.

REM Phase 3: Build dependent chain sequentially - Win32
echo Building Win32 dependent libraries...

"%MSBUILD%" build\vs2010\projects\OrderManager.vcxproj /p:Configuration=Release /p:Platform=Win32 /p:BuildProjectReferences=false /v:minimal /nologo
if !ERRORLEVEL! NEQ 0 (
    echo ERROR: Win32 OrderManager failed
    exit /b 1
)

"%MSBUILD%" build\vs2010\projects\AsirikuyEasyTrade.vcxproj /p:Configuration=Release /p:Platform=Win32 /p:BuildProjectReferences=false /v:minimal /nologo
if !ERRORLEVEL! NEQ 0 (
    echo ERROR: Win32 AsirikuyEasyTrade failed
    exit /b 1
)

"%MSBUILD%" build\vs2010\projects\TradingStrategies.vcxproj /p:Configuration=Release /p:Platform=Win32 /p:BuildProjectReferences=false /v:minimal /nologo
if !ERRORLEVEL! NEQ 0 (
    echo ERROR: Win32 TradingStrategies failed
    exit /b 1
)

echo.
echo Building Win32 AsirikuyFrameworkAPI.dll...
"%MSBUILD%" build\vs2010\projects\AsirikuyFrameworkAPI.vcxproj /p:Configuration=Release /p:Platform=Win32 /p:BuildProjectReferences=false /v:minimal /nologo
if !ERRORLEVEL! NEQ 0 (
    echo ERROR: Win32 AsirikuyFrameworkAPI failed
    exit /b 1
)

echo.
echo Building Win32 CTesterFrameworkAPI.dll...
"%MSBUILD%" build\vs2010\projects\CTesterFrameworkAPI.vcxproj /p:Configuration=Release /p:Platform=Win32 /p:BuildProjectReferences=false /v:minimal /nologo
if !ERRORLEVEL! NEQ 0 (
    echo ERROR: Win32 CTesterFrameworkAPI failed
    exit /b 1
)

echo.
echo ========================================================================
echo   Build Complete!
echo ========================================================================
echo.
echo x64 Build Artifacts:
if exist "bin\vs2010\x64\Release\AsirikuyFrameworkAPI.dll" (
    echo   [OK] bin\vs2010\x64\Release\AsirikuyFrameworkAPI.dll
) else (
    echo   [MISSING] bin\vs2010\x64\Release\AsirikuyFrameworkAPI.dll
)
if exist "bin\vs2010\x64\Release\CTesterFrameworkAPI.dll" (
    echo   [OK] bin\vs2010\x64\Release\CTesterFrameworkAPI.dll
) else (
    echo   [MISSING] bin\vs2010\x64\Release\CTesterFrameworkAPI.dll
)
echo.
echo Win32 Build Artifacts:
if exist "bin\vs2010\x32\Release\AsirikuyFrameworkAPI.dll" (
    echo   [OK] bin\vs2010\x32\Release\AsirikuyFrameworkAPI.dll
) else (
    echo   [MISSING] bin\vs2010\x32\Release\AsirikuyFrameworkAPI.dll
)
if exist "bin\vs2010\x32\Release\CTesterFrameworkAPI.dll" (
    echo   [OK] bin\vs2010\x32\Release\CTesterFrameworkAPI.dll
) else (
    echo   [MISSING] bin\vs2010\x32\Release\CTesterFrameworkAPI.dll
)
echo.

REM Verify all required DLLs exist
set BUILD_FAILED=0
if not exist "bin\vs2010\x64\Release\AsirikuyFrameworkAPI.dll" set BUILD_FAILED=1
if not exist "bin\vs2010\x64\Release\CTesterFrameworkAPI.dll" set BUILD_FAILED=1
if not exist "bin\vs2010\x32\Release\AsirikuyFrameworkAPI.dll" set BUILD_FAILED=1
if not exist "bin\vs2010\x32\Release\CTesterFrameworkAPI.dll" set BUILD_FAILED=1

if %BUILD_FAILED%==1 (
    echo ERROR: One or more DLLs missing!
    exit /b 1
)

echo All builds completed successfully at %date% %time%

endlocal
exit /b 0
