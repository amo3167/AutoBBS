@echo off
REM Build script for C++ migration validation
REM Compiles only C++ files to verify migration work

call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"

echo.
echo ========================================
echo  C++ Migration Build (C files excluded)
echo ========================================
echo.

REM Clean previous C++ object files
if exist "E:\workspace\AutoBBS\tmp\vs2010\x32\Debug\TradingStrategies_CPP" (
    rmdir /s /q "E:\workspace\AutoBBS\tmp\vs2010\x32\Debug\TradingStrategies_CPP"
)
mkdir "E:\workspace\AutoBBS\tmp\vs2010\x32\Debug\TradingStrategies_CPP"

REM Delete old PDB file
if exist "E:\workspace\AutoBBS\bin\vs2010\x32\Debug\lib\TradingStrategies_CPP.pdb" (
    del /f "E:\workspace\AutoBBS\bin\vs2010\x32\Debug\lib\TradingStrategies_CPP.pdb"
)

echo Compiling C++ source files...
echo.

cl.exe /c ^
  /IE:\workspace\boost_1_49_0 ^
  /IE:\workspace\AutoBBS\vendor\STLSoft\include ^
  /IE:\workspace\AutoBBS\vendor\Pantheios\include ^
  /IE:\workspace\AutoBBS\dev\AsirikuyCommon\include ^
  /IE:\workspace\AutoBBS\dev\SymbolAnalyzer\include ^
  /IE:\workspace\AutoBBS\dev\Log\include ^
  /IE:\workspace\AutoBBS\dev\AsirikuyTechnicalAnalysis\include ^
  /IE:\workspace\AutoBBS\dev\OrderManager\include ^
  /IE:\workspace\AutoBBS\dev\TradingStrategies\include ^
  /IE:\workspace\AutoBBS\dev\TradingStrategies\include\strategies ^
  /IE:\workspace\AutoBBS\dev\AsirikuyEasyTrade\include ^
  /IE:\workspace\AutoBBS\dev\NTPClient\include ^
  /IE:\workspace\AutoBBS\dev\AsirikuyFrameworkAPI\include ^
  /ZI /JMC /nologo /W3 /WX- /diagnostics:column ^
  /Od /Oy- ^
  /D DEBUG /D _DEBUG /D WINDOWS /D _WINDOWS /D WIN32 /D _WIN32 /D _MBCS ^
  /D CPP_MIGRATION_IN_PROGRESS ^
  /EHsc /RTC1 /MTd /GS /Gy /fp:precise ^
  /Zc:wchar_t /Zc:forScope /Zc:inline ^
  /Fo"E:\workspace\AutoBBS\tmp\vs2010\x32\Debug\TradingStrategies_CPP\\" ^
  /Fd"E:\workspace\AutoBBS\bin\vs2010\x32\Debug\lib\TradingStrategies_CPP.pdb" ^
  /external:W3 /TP /analyze- /FC /errorReport:queue ^
  E:\workspace\AutoBBS\dev\TradingStrategies\src\AsirikuyStrategiesWrapper.cpp ^
  E:\workspace\AutoBBS\dev\TradingStrategies\src\BaseStrategy.cpp ^
  E:\workspace\AutoBBS\dev\TradingStrategies\src\Indicators.cpp ^
  E:\workspace\AutoBBS\dev\TradingStrategies\src\NumericLoggingHelpers.cpp ^
  E:\workspace\AutoBBS\dev\TradingStrategies\src\OrderBuilder.cpp ^
  E:\workspace\AutoBBS\dev\TradingStrategies\src\OrderManager.cpp ^
  E:\workspace\AutoBBS\dev\TradingStrategies\src\StrategyContext.cpp ^
  E:\workspace\AutoBBS\dev\TradingStrategies\src\StrategyFactory.cpp ^
  E:\workspace\AutoBBS\dev\TradingStrategies\src\StrategyTimeUtils.cpp ^
  E:\workspace\AutoBBS\dev\TradingStrategies\src\strategies\AutoBBSStrategy.cpp ^
  E:\workspace\AutoBBS\dev\TradingStrategies\src\strategies\AutoBBSWeeklyStrategy.cpp ^
  E:\workspace\AutoBBS\dev\TradingStrategies\src\strategies\RecordBarsStrategy.cpp ^
  E:\workspace\AutoBBS\dev\TradingStrategies\src\strategies\ScreeningStrategy.cpp ^
  E:\workspace\AutoBBS\dev\TradingStrategies\src\strategies\TakeOverStrategy.cpp ^
  E:\workspace\AutoBBS\dev\TradingStrategies\src\strategies\TrendLimitStrategy.cpp

if %ERRORLEVEL% neq 0 (
    echo.
    echo ========================================
    echo  C++ Compilation FAILED
    echo ========================================
    exit /b %ERRORLEVEL%
)

echo.
echo ========================================
echo  C++ Compilation SUCCESS
echo ========================================
echo.
echo Creating static library...

lib.exe /OUT:"E:\workspace\AutoBBS\bin\vs2010\x32\Debug\lib\TradingStrategies_CPP.lib" ^
  /NOLOGO /MACHINE:X86 ^
  E:\workspace\AutoBBS\tmp\vs2010\x32\Debug\TradingStrategies_CPP\*.obj

if %ERRORLEVEL% neq 0 (
    echo.
    echo ========================================
    echo  Library creation FAILED
    echo ========================================
    exit /b %ERRORLEVEL%
)

echo.
echo ========================================
echo  BUILD SUCCESSFUL
echo ========================================
echo.
echo Output: E:\workspace\AutoBBS\bin\vs2010\x32\Debug\lib\TradingStrategies_CPP.lib
echo.

exit /b 0
