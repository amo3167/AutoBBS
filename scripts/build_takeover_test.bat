@echo off
setlocal enabledelayedexpansion
REM Build and run integration smoke test for TakeOverStrategy

call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"

set OUT_DIR=E:\workspace\AutoBBS\tmp\vs2010\x32\Debug\TakeOver_Test
REM Output exe to local OUT_DIR to avoid potential permission issues in bin tree
set BIN_DIR=%OUT_DIR%
set LINK_EXE=link.exe
REM Fallback absolute path if link.exe not on PATH after VsDevCmd
if not exist "%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC" goto skip_vs_path
for /f "delims=" %%I in ('dir /b /ad "%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC"') do set MSVC_VER=%%I
if defined MSVC_VER (
  set LINK_EXE="%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\%MSVC_VER%\bin\Hostx64\x64\link.exe"
)
:skip_vs_path

if exist "%OUT_DIR%" rmdir /s /q "%OUT_DIR%"
mkdir "%OUT_DIR%"
if not exist "%BIN_DIR%" mkdir "%BIN_DIR%"

echo.
echo ========================================
echo  TakeOverStrategy Integration Test Build
echo ========================================
echo.

cl.exe /c /IE:\workspace\AutoBBS\dev\AsirikuyCommon\include /IE:\workspace\AutoBBS\dev\TradingStrategies\include /IE:\workspace\AutoBBS\dev\TradingStrategies\include\strategies /D DEBUG /D _DEBUG /D WINDOWS /D WIN32 /D CPP_MIGRATION_IN_PROGRESS /EHsc /MTd /W3 /nologo /Fo"%OUT_DIR%\\" E:\workspace\AutoBBS\dev\TradingStrategies\src\BaseStrategy.cpp E:\workspace\AutoBBS\dev\TradingStrategies\src\StrategyContext.cpp E:\workspace\AutoBBS\dev\TradingStrategies\src\strategies\TakeOverStrategy.cpp E:\workspace\AutoBBS\dev\TradingStrategies\tests\TakeOverStrategyTest.cpp

if %ERRORLEVEL% neq 0 (
  echo Build failed during compilation phase.
  exit /b %ERRORLEVEL%
)

echo Linking standalone test executable with debug static CRT...
link.exe /OUT:"%BIN_DIR%\TakeOverStrategyTest.exe" "%OUT_DIR%\BaseStrategy.obj" "%OUT_DIR%\StrategyContext.obj" "%OUT_DIR%\TakeOverStrategy.obj" "%OUT_DIR%\TakeOverStrategyTest.obj" /SUBSYSTEM:CONSOLE /NOLOGO

if %ERRORLEVEL% neq 0 (
  echo Link failed with ERRORLEVEL=!ERRORLEVEL!
  type "%OUT_DIR%\link.log" 2>nul | findstr /C:"error LNK" | more +0
  exit /b !ERRORLEVEL!
)

echo.
echo Link succeeded - checking for executable...
if exist "%BIN_DIR%\TakeOverStrategyTest.exe" (
  echo SUCCESS: Executable found at %BIN_DIR%\TakeOverStrategyTest.exe
  dir "%BIN_DIR%\TakeOverStrategyTest.exe"
  echo.
  echo Running test immediately before antivirus can quarantine...
  cd /d "%BIN_DIR%"
  TakeOverStrategyTest.exe 2>&1
  set TEST_EXIT=!ERRORLEVEL!
  echo.
  echo Test completed with exit code !TEST_EXIT!
  exit /b !TEST_EXIT!
) else (
  echo ERROR: Executable not found after link succeeded!
  echo This usually means antivirus software deleted the exe.
  echo Please add exclusion for: %BIN_DIR%
  dir "%BIN_DIR%" /a
  exit /b 1
)

