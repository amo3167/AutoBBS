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
    echo   AutoBBS Parallel Build (CLEAN + BUILD^)
    echo ========================================================================
) else (
    echo ========================================================================
    echo   AutoBBS Parallel Build (INCREMENTAL^)
    echo ========================================================================
)
echo Using MSBuild with /m flag for parallel compilation
echo.

pushd "%~dp0\.."
echo Current directory: %cd%

if %DO_CLEAN%==1 (
    echo.
    echo Cleaning build directories ^(preserving vendor libraries^)...
    set VENDOR_BACKUP_NEEDED=0
    if exist "bin\vs2010\x64\Release\lib\TALib_common.lib" set VENDOR_BACKUP_NEEDED=1
    if exist "bin\vs2010\x64\Release\lib\TALib_abstract.lib" set VENDOR_BACKUP_NEEDED=1
    if exist "bin\vs2010\x64\Release\lib\TALib_func.lib" set VENDOR_BACKUP_NEEDED=1
    if exist "bin\vs2010\x64\Release\lib\MiniXML.lib" set VENDOR_BACKUP_NEEDED=1

    if !VENDOR_BACKUP_NEEDED!==1 (
        if not exist "tmp\vendor_backup" mkdir tmp\vendor_backup
        if exist "bin\vs2010\x64\Release\lib\TALib_common.lib" copy /y "bin\vs2010\x64\Release\lib\TALib_common.lib" tmp\vendor_backup\ >nul
        if exist "bin\vs2010\x64\Release\lib\TALib_abstract.lib" copy /y "bin\vs2010\x64\Release\lib\TALib_abstract.lib" tmp\vendor_backup\ >nul
        if exist "bin\vs2010\x64\Release\lib\TALib_func.lib" copy /y "bin\vs2010\x64\Release\lib\TALib_func.lib" tmp\vendor_backup\ >nul
        if exist "bin\vs2010\x64\Release\lib\MiniXML.lib" copy /y "bin\vs2010\x64\Release\lib\MiniXML.lib" tmp\vendor_backup\ >nul
    )

    if exist bin\vs2010 rmdir /s /q bin\vs2010
    if exist tmp\vs2010 rmdir /s /q tmp\vs2010
    if exist build\vs2010\projects rmdir /s /q build\vs2010\projects

    if !VENDOR_BACKUP_NEEDED!==1 (
        echo Restoring vendor libraries...
        if not exist "bin\vs2010\x64\Release\lib" mkdir bin\vs2010\x64\Release\lib
        copy /y tmp\vendor_backup\*.lib bin\vs2010\x64\Release\lib\ >nul
        rmdir /s /q tmp\vendor_backup
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
if not exist "bin\vs2010\x64\Release\lib\TALib_func.lib" (
    echo Building vendor libraries in parallel...
    "%MSBUILD%" build\vs2010\AsirikuyFramework.sln ^
        /t:TALib_common;TALib_abstract;TALib_func;MiniXML ^
        /p:Configuration=Release /p:Platform=x64 /m /v:minimal /nologo
    if !ERRORLEVEL! NEQ 0 (
        echo ERROR: Vendor build failed
        exit /b 1
    )
    echo.
)

REM Phase 2: Build independent core libs in parallel
echo Building independent libraries in parallel...
"%MSBUILD%" build\vs2010\AsirikuyFramework.sln ^
    /t:AsirikuyCommon;Log;SymbolAnalyzer;AsirikuyTechnicalAnalysis ^
    /p:Configuration=Release /p:Platform=x64 /p:BuildProjectReferences=false /m /v:minimal /nologo
if !ERRORLEVEL! NEQ 0 (
    echo ERROR: Phase 1 build failed
    exit /b 1
)
echo.

REM Phase 3: Build dependent chain sequentially
echo Building dependent libraries...

"%MSBUILD%" build\vs2010\projects\OrderManager.vcxproj /p:Configuration=Release /p:Platform=x64 /p:BuildProjectReferences=false /v:minimal /nologo
if !ERRORLEVEL! NEQ 0 (
    echo ERROR: OrderManager failed
    exit /b 1
)

"%MSBUILD%" build\vs2010\projects\AsirikuyEasyTrade.vcxproj /p:Configuration=Release /p:Platform=x64 /p:BuildProjectReferences=false /v:minimal /nologo
if !ERRORLEVEL! NEQ 0 (
    echo ERROR: AsirikuyEasyTrade failed
    exit /b 1
)

"%MSBUILD%" build\vs2010\projects\TradingStrategies.vcxproj /p:Configuration=Release /p:Platform=x64 /p:BuildProjectReferences=false /v:minimal /nologo
if !ERRORLEVEL! NEQ 0 (
    echo ERROR: TradingStrategies failed
    exit /b 1
)

echo.
echo Building AsirikuyFrameworkAPI.dll...
"%MSBUILD%" build\vs2010\projects\AsirikuyFrameworkAPI.vcxproj /p:Configuration=Release /p:Platform=x64 /p:BuildProjectReferences=false /v:minimal /nologo
if !ERRORLEVEL! NEQ 0 (
    echo ERROR: AsirikuyFrameworkAPI failed
    exit /b 1
)

echo.
echo Building CTesterFrameworkAPI.dll...
"%MSBUILD%" build\vs2010\projects\CTesterFrameworkAPI.vcxproj /p:Configuration=Release /p:Platform=x64 /p:BuildProjectReferences=false /v:minimal /nologo
if !ERRORLEVEL! NEQ 0 (
    echo ERROR: CTesterFrameworkAPI failed
    exit /b 1
)

echo.
echo ========================================================================
echo   Build Complete!
echo ========================================================================
echo.
if exist "bin\vs2010\x64\Release\AsirikuyFrameworkAPI.dll" (
    echo DLL: bin\vs2010\x64\Release\AsirikuyFrameworkAPI.dll
)
if exist "bin\vs2010\x64\Release\CTesterFrameworkAPI.dll" (
    echo DLL: bin\vs2010\x64\Release\CTesterFrameworkAPI.dll
)
if not exist "bin\vs2010\x64\Release\AsirikuyFrameworkAPI.dll" (
    echo ERROR: AsirikuyFrameworkAPI.dll not found!
    exit /b 1
)
if not exist "bin\vs2010\x64\Release\CTesterFrameworkAPI.dll" (
    echo ERROR: CTesterFrameworkAPI.dll not found!
    exit /b 1
)
echo Build completed at %date% %time%

endlocal
exit /b 0
