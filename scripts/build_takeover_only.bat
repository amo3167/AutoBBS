@echo off
REM Minimal C++ build - TakeOverStrategy validation only
REM Compiles only files needed to verify TakeOverStrategy migration

call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"

echo.
echo ========================================
echo  TakeOverStrategy Migration Validation
echo ========================================
echo.

REM Clean
if exist "E:\workspace\AutoBBS\tmp\vs2010\x32\Debug\TakeOver_CPP" (
    rmdir /s /q "E:\workspace\AutoBBS\tmp\vs2010\x32\Debug\TakeOver_CPP"
)
mkdir "E:\workspace\AutoBBS\tmp\vs2010\x32\Debug\TakeOver_CPP"

if exist "E:\workspace\AutoBBS\bin\vs2010\x32\Debug\lib\TakeOver_CPP.pdb" (
    del /f "E:\workspace\AutoBBS\bin\vs2010\x32\Debug\lib\TakeOver_CPP.pdb"
)

echo Compiling TakeOverStrategy and dependencies...
echo.

cl.exe /c ^
  /IE:\workspace\boost_1_49_0 ^
  /IE:\workspace\AutoBBS\vendor\STLSoft\include ^
  /IE:\workspace\AutoBBS\vendor\Pantheios\include ^
  /IE:\workspace\AutoBBS\vendor\TALib\c\include ^
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
  /Fo"E:\workspace\AutoBBS\tmp\vs2010\x32\Debug\TakeOver_CPP\\" ^
  /Fd"E:\workspace\AutoBBS\bin\vs2010\x32\Debug\lib\TakeOver_CPP.pdb" ^
  /external:W3 /TP /analyze- /FC /errorReport:queue ^
  E:\workspace\AutoBBS\dev\TradingStrategies\src\BaseStrategy.cpp ^
  E:\workspace\AutoBBS\dev\TradingStrategies\src\StrategyContext.cpp ^
  E:\workspace\AutoBBS\dev\TradingStrategies\src\StrategyFactory.cpp ^
  E:\workspace\AutoBBS\dev\TradingStrategies\src\strategies\TakeOverStrategy.cpp

if %ERRORLEVEL% neq 0 (
    echo.
    echo ========================================
    echo  TakeOverStrategy COMPILATION FAILED
    echo ========================================
    exit /b %ERRORLEVEL%
)

echo.
echo ========================================
echo  TakeOverStrategy COMPILATION SUCCESS
echo ========================================
echo.
echo Compiled files:
echo   - BaseStrategy.cpp
echo   - StrategyContext.cpp  
echo   - StrategyFactory.cpp
echo   - TakeOverStrategy.cpp
echo.
echo This validates that the TakeOverStrategy
echo C++ migration is syntactically correct.
echo.

exit /b 0
