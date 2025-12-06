project "TradingStrategies"
  location("../../build/" .. _ACTION .. "/projects")
  -- Build as static lib on Windows (for linking into DLLs), shared lib on Unix
  configuration{"windows"}
    kind "StaticLib"
  configuration{"not windows"}
    kind "SharedLib"
  configuration{}
  language "C"
  targetname "TradingStrategies"
  files{
    "**.h", 
	"**.c", 
	"**.hpp"
  }
  -- Include directories for strategy modules
  -- Note: The "include" directory contains headers for all strategy modules:
  --   - strategies/common/ (StopLossManagement, RangeOrderManagement, TimeManagement)
  --   - strategies/macd/ (MACDDailyStrategy, MACDWeeklyStrategy, MACDOrderSplitting)
  --   - strategies/ichimoko/ (IchimokoDailyStrategy, IchimokoWeeklyStrategy, IchimokoOrderSplitting)
  --   - strategies/bbs/ (BBSBreakOutStrategy, BBSSwingStrategy, BBSOrderSplitting)
  --   - strategies/limit/ (LimitStrategy, LimitBBSStrategy, LimitOrderSplitting)
  --   - strategies/weekly/ (WeeklyAutoStrategy, WeeklyPivotStrategy, WeeklyOrderSplitting)
  --   - strategies/shortterm/ (ShortTermStrategy, ShortTermOrderSplitting)
  --   - strategies/misc/ (MiscStrategies, KeyKOrderSplitting)
  includedirs{
    "src",
    "include",
    "../AsirikuyCommon/include",
    "../Log/include",
    "../OrderManager/include",
    "../AsirikuyTechnicalAnalysis/include",
    "../AsirikuyEasyTrade/include"
  }
  defines{"_CRT_SECURE_NO_WARNINGS"}
  
  -- Dependencies (premake4 uses links instead of uses)
  links{
    "AsirikuyCommon",
    "Log",
    "OrderManager",
    "AsirikuyTechnicalAnalysis",
    "TALib_common",
    "TALib_abstract",
    "TALib_func",
    "SymbolAnalyzer",
    "AsirikuyEasyTrade"
  }
  -- Platform-specific library dependencies
  configuration{"not windows"}
    links{
      "NTPClient"
    }
  
  -- Platform-specific configurations
  configuration{"windows"}
    -- Windows DLL export configuration
    defines{"TRADING_STRATEGIES_EXPORTS"}
  
  configuration{"not windows"}
    -- Linux/macOS shared library configuration
    -- Add library paths for both Linux and macOS
    boostlib = (os.getenv("BOOST_ROOT") or "/usr/local") .. "/lib"
    libdirs{
      boostlib,
      "/Users/andym/homebrew/opt/curl/lib",
      "/usr/local/lib",
      "/usr/lib"
    }
    linkoptions{
      "-L" .. boostlib,
      "-L/Users/andym/homebrew/opt/curl/lib",
      "-L/usr/local/lib"
    }
    links{"curl"}  -- cURL needed for AsirikuyEasyTrade
    
  -- Target directory configurations (all libraries go to root bin/ directory)
  -- Use relative path from premake4.lua location (core/TradingStrategies/) to workspace root: ../../
  configuration{"windows", "x32", "Debug"}
    targetdir("../../bin/" .. _ACTION .. "/x32/Debug")
  configuration{"windows", "x64", "Debug"}
    targetdir("../../bin/" .. _ACTION .. "/x64/Debug")
  configuration{"windows", "x32", "Release"}
    targetdir("../../bin/" .. _ACTION .. "/x32/Release")
  configuration{"windows", "x64", "Release"}
    targetdir("../../bin/" .. _ACTION .. "/x64/Release")
  configuration{"macosx", "x32", "Debug"}
    targetdir("../../bin/" .. _ACTION .. "/x32/Debug/lib")
  configuration{"macosx", "x64", "Debug"}
    targetdir("../../bin/" .. _ACTION .. "/x64/Debug/lib")
  configuration{"macosx", "x32", "Release"}
    targetdir("../../bin/" .. _ACTION .. "/x32/Release/lib")
  configuration{"macosx", "x64", "Release"}
    targetdir("../../bin/" .. _ACTION .. "/x64/Release/lib")
  configuration{"linux", "x32", "Debug"}
    targetdir("../../bin/" .. _ACTION .. "/x32/Debug/lib")
  configuration{"linux", "x64", "Debug"}
    targetdir("../../bin/" .. _ACTION .. "/x64/Debug/lib")
  configuration{"linux", "x32", "Release"}
    targetdir("../../bin/" .. _ACTION .. "/x32/Release/lib")
  configuration{"linux", "x64", "Release"}
    targetdir("../../bin/" .. _ACTION .. "/x64/Release/lib")
    
  configuration{"macosx"}
    linkoptions{"-lc++", "-lboost_thread", "-lboost_chrono"}
    -- Rename .so to .dylib on macOS for clarity
    postbuildcommands{"mv " .. "bin/" .. _ACTION .. "/x64/Debug/lib/libtrading_strategies.so " .. "bin/" .. _ACTION .. "/x64/Debug/lib/libtrading_strategies.dylib 2>/dev/null || mv " .. "bin/" .. _ACTION .. "/x32/Debug/lib/libtrading_strategies.so " .. "bin/" .. _ACTION .. "/x32/Debug/lib/libtrading_strategies.dylib 2>/dev/null || mv " .. "bin/" .. _ACTION .. "/x64/Release/lib/libtrading_strategies.so " .. "bin/" .. _ACTION .. "/x64/Release/lib/libtrading_strategies.dylib 2>/dev/null || mv " .. "bin/" .. _ACTION .. "/x32/Release/lib/libtrading_strategies.so " .. "bin/" .. _ACTION .. "/x32/Release/lib/libtrading_strategies.dylib 2>/dev/null || true"}
    -- Pantheios removed - using standard logging instead
  configuration{"linux"}
    linkoptions{"-lc++", "-lboost_thread", "-lboost_chrono"}
    -- boost_system removed - not available in this Boost installation
