project "AsirikuyFrameworkAPI"
  location("../../build/" .. _ACTION .. "/projects")
  kind "SharedLib"
  language "C++"
  files{
    "**.h", 
	"**.c", 
    "**.rc",
    "**.def",
	"**.xml", 
	"**.hpp"
  }
  includedirs{
    "src",
    "../../../vendor/MiniXML"  -- MiniXML headers (mxml-private.h)
  }
  -- NTPClient link moved to platform-specific configurations below
  configuration{"windows"}
    links{ 
	  "MiniXML", 
	  -- "chartdir51", -- Removed: ChartDirector not available, Windows/Linux only
	  "AsirikuyCommon", 
	  "Log", 
	  "SymbolAnalyzer", 
	  "AsirikuyTechnicalAnalysis", 
	  "OrderManager", 
	  "AsirikuyEasyTrade", -- Stub implementation for Windows
	  "NTPClient", -- Now available on Windows with Boost 1.84.0
	  -- "curl", -- Not available on Windows
	  "TALib_common", 
	  "TALib_abstract", 
	  "TALib_func"
    }
    libdirs{
      "../../vendor/boost_1_84_0/stage/lib"
    }
  configuration{"windows", "x32"}
    -- Link TradingStrategies explicitly from bin directory
    linkoptions{"../../../bin/" .. _ACTION .. "/x32/Release/TradingStrategies.lib"}
    -- Boost 1.84 libraries with static runtime (x86 32-bit)
    links{
      "libboost_system-vc143-mt-s-x32-1_84",
      "libboost_thread-vc143-mt-s-x32-1_84",
      "libboost_chrono-vc143-mt-s-x32-1_84"
    }
  configuration{"windows", "x64"}
    -- Link TradingStrategies explicitly from bin directory
    linkoptions{"../../../bin/" .. _ACTION .. "/x64/Release/TradingStrategies.lib"}
    -- Boost 1.84 libraries with static runtime (x64 64-bit)
    links{
      "libboost_system-vc143-mt-s-x64-1_84",
      "libboost_thread-vc143-mt-s-x64-1_84",
      "libboost_chrono-vc143-mt-s-x64-1_84"
    }
  -- Determine boost directory for non-Windows prebuild commands
  if os.isdir("../../vendor/boost_1_84_0") then
    boostdir = "../../vendor/boost_1_84_0"
  elseif os.isdir("../../vendor/boost_1_49_0") then
    boostdir = "../../vendor/boost_1_49_0"
  else
    boostdir = os.getenv("BOOST_ROOT") or "/usr/local"
  end
  requiredBoostLibs = "--with-system --with-chrono --with-thread --with-date_time --with-regex --with-filesystem --with-serialization --with-test"  
  os.chdir("../..")
  cwd = os.getcwd()
  configuration{"not windows"}
    excludes{
      "**.rc",
      "**.def"
    }
  configuration{"windows"}
    linkoptions{"/DEF:../../../core/AsirikuyFrameworkAPI/src/AsirikuyFrameworkAPI.def"}
  configuration{"windows", "Release"}
    linkoptions{"/OPT:REF,ICF"}
  configuration{"not windows"}
    links{
	  "TradingStrategies",
      -- "chartdir51", -- Removed: ChartDirector not available, Windows/Linux only
	  "AsirikuyCommon",
	  "Log",
	  "SymbolAnalyzer",
      "AsirikuyEasyTrade",  
	  "OrderManager",
	  "AsirikuyTechnicalAnalysis",
      "mxml",  -- MiniXML library name is 'mxml', not 'MiniXML'  
	  "TALib_common",
	  "TALib_abstract",
	  "TALib_func",
	  "NTPClient",
	  "curl",
	  "boost_filesystem",
	  "boost_serialization",
	  "boost_thread",
	  -- "boost_system", -- Removed - not available
	  "boost_date_time",
	  "boost_atomic",
	  "pthread"
    }
    -- Add library paths for both Linux and macOS
    -- Note: These paths will be used at build time, not premake4 generation time
    boostlib = (os.getenv("BOOST_ROOT") or "/usr/local") .. "/lib"
    libdirs{
      boostlib,
      "/Users/andym/homebrew/opt/curl/lib",
      "/usr/local/lib",
      "/usr/lib",
      "../../../vendor/MiniXML"  -- MiniXML library path
    }
    linkoptions{
      "-L" .. boostlib,
      "-L/Users/andym/homebrew/opt/curl/lib",
      "-L/usr/local/lib",
      "-L../../../vendor/MiniXML"  -- MiniXML library path
    }
  configuration{"macosx"}
    -- Additional macOS-specific paths if needed
    boostlib = (os.getenv("BOOST_ROOT") or "/usr/local") .. "/lib"
    libdirs{
      boostlib,
      "/Users/andym/homebrew/opt/curl/lib",
      "/usr/local/lib",
      "/usr/lib"
    }
  if _ARGS[1] == "WITH_R" then
	links{"AsirikuyRWrapper"}
  end
  configuration{"linux"}
	links{"rt"}
  -- Windows
  configuration{"windows"}
    -- Windows builds should look in bin directory for dependencies
    libdirs{
      "../../../bin/" .. _ACTION .. "/x64/Release",
      "../../../bin/" .. _ACTION .. "/x64/Debug"
    }
  configuration{"windows", "x32", "Debug"}
    targetdir("../../../bin/" .. _ACTION .. "/x32/Debug")
    libdirs{"../../../bin/" .. _ACTION .. "/x32/Debug"}
  configuration{"windows", "x64", "Debug"}
    targetdir("../../../bin/" .. _ACTION .. "/x64/Debug")
    libdirs{"../../../bin/" .. _ACTION .. "/x64/Debug"}
  configuration{"windows", "x32", "Release"}
    targetdir("../../../bin/" .. _ACTION .. "/x32/Release")
    libdirs{"../../../bin/" .. _ACTION .. "/x32/Release"}
  configuration{"windows", "x64", "Release"}
    targetdir("../../../bin/" .. _ACTION .. "/x64/Release")
    libdirs{"../../../bin/" .. _ACTION .. "/x64/Release"}
  -- Not Windows
  configuration{"not windows", "x32", "Debug"}
    targetdir("bin/" .. _ACTION .. "/x32/Debug")
    prebuildcommands{"cd " .. boostdir .. " && ./b2 --with-atomic " .. requiredBoostLibs .. " --abbreviate-paths --build-dir=" .. cwd .. "/tmp --stagedir=" .. cwd .. "/bin/" .. _ACTION .. "/x32/Debug variant=debug link=static threading=multi runtime-link=static address-model=32 architecture=x86 stage"}
	configuration{"not windows", "x64", "Debug"}
		targetdir("bin/" .. _ACTION .. "/x64/Debug")
		-- Replace architecture=ia64 with architecture=x86 (portable 64-bit)
		prebuildcommands{"cd " .. boostdir .. " && ./b2 --with-atomic " .. requiredBoostLibs .. " --abbreviate-paths --build-dir=" .. cwd .. "/tmp --stagedir=" .. cwd .. "/bin/" .. _ACTION .. "/x64/Debug variant=debug link=static threading=multi runtime-link=static address-model=64 cxxflags=-fPIC architecture=x86 stage"}
  configuration{"not windows", "x32", "Release"}
    targetdir("bin/" .. _ACTION .. "/x32/Release")
    prebuildcommands{"cd " .. boostdir .. " && ./b2 --with-atomic " .. requiredBoostLibs .. " --abbreviate-paths --build-dir=" .. cwd .. "/tmp --stagedir=" .. cwd .. "/bin/" .. _ACTION .. "/x32/Release variant=release link=static threading=multi runtime-link=static address-model=32 architecture=x86 cxxflags=-march=i686 stage"}
	configuration{"not windows", "x64", "Release"}
		targetdir("bin/" .. _ACTION .. "/x64/Release")
		-- Replace architecture=ia64 with architecture=x86 for consistency
		prebuildcommands{"cd " .. boostdir .. " && ./b2 --with-atomic " .. requiredBoostLibs .. " --abbreviate-paths --build-dir=" .. cwd .. "/tmp --stagedir=" .. cwd .. "/bin/" .. _ACTION .. "/x64/Release variant=release link=static threading=multi runtime-link=static address-model=64 cxxflags=-fPIC architecture=x86 stage"}
  configuration{"linux"}
    linkoptions{"-rdynamic"}
    buildoptions{"-DBOOST_THREAD_USE_LIB"}
  -- macOS-specific targetdir configurations
  configuration{"macosx", "x32", "Debug"}
    targetdir("bin/" .. _ACTION .. "/x32/Debug")
    postbuildcommands{"cd " .. cwd .. " && mv bin/" .. _ACTION .. "/x32/Debug/libAsirikuyFrameworkAPI.so bin/" .. _ACTION .. "/x32/Debug/libAsirikuyFrameworkAPI.dylib 2>/dev/null || true"}
  configuration{"macosx", "x64", "Debug"}
    targetdir("bin/" .. _ACTION .. "/x64/Debug")
    postbuildcommands{"mv " .. "bin/" .. _ACTION .. "/x64/Debug/libAsirikuyFrameworkAPI.so " .. "bin/" .. _ACTION .. "/x64/Debug/libAsirikuyFrameworkAPI.dylib 2>/dev/null || true"}
  configuration{"macosx", "x32", "Release"}
    targetdir("bin/" .. _ACTION .. "/x32/Release")
    postbuildcommands{"cd " .. cwd .. " && mv bin/" .. _ACTION .. "/x32/Release/libAsirikuyFrameworkAPI.so bin/" .. _ACTION .. "/x32/Release/libAsirikuyFrameworkAPI.dylib 2>/dev/null || true"}
  configuration{"macosx", "x64", "Release"}
    targetdir("bin/" .. _ACTION .. "/x64/Release")
    postbuildcommands{"cd " .. cwd .. " && mv bin/" .. _ACTION .. "/x64/Release/libAsirikuyFrameworkAPI.so bin/" .. _ACTION .. "/x64/Release/libAsirikuyFrameworkAPI.dylib 2>/dev/null || true"}
	
