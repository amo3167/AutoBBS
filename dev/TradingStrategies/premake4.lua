project "TradingStrategies"
  location("../../build/" .. _ACTION .. "/projects")
  kind "SharedLib"
  language "C"
  targetname "trading_strategies"
  files{
    "**.h", 
	"**.c", 
	"**.hpp"
  }
  includedirs{
    "src",
    "../AsirikuyCommon/include",
    "../Log/include",
    "../OrderManager/include",
    "../AsirikuyTechnicalAnalysis/include",
    "../AsirikuyEasyTrade/include"
  }
  defines{"FANN_NO_DLL"}
  
  -- Dependencies (premake4 uses links instead of uses)
  links{
    "AsirikuyCommon",
    "Log",
    "OrderManager",
    "AsirikuyTechnicalAnalysis",
    "AsirikuyEasyTrade",
    "NTPClient",
    "TALib_common",
    "TALib_abstract",
    "TALib_func",
    "SymbolAnalyzer"
  }
  
  -- Platform-specific configurations
  configuration{"windows"}
    -- Windows DLL export configuration
    defines{"TRADING_STRATEGIES_EXPORTS"}
  
  configuration{"not windows"}
    -- Linux/macOS shared library configuration
    -- No special configuration needed for standard C calling convention
    -- On macOS, this will generate libtrading_strategies.dylib
    -- On Linux, this will generate libtrading_strategies.so
    -- Link against C++ standard library and Boost for dependencies
    configuration{"macosx"}
      libdirs{os.getenv("BOOST_ROOT") .. "/lib"}
      linkoptions{"-lc++", "-L" .. os.getenv("BOOST_ROOT") .. "/lib", "-lboost_thread", "-lboost_chrono", "-lcurl"}
      -- Pantheios removed - using standard logging instead
    configuration{"linux"}
      libdirs{os.getenv("BOOST_ROOT") .. "/lib"}
      linkoptions{"-lc++", "-lboost_thread", "-lboost_system", "-lboost_chrono"}

