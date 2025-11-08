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
  vpaths{
	["Header Files/*"] = "../../../dev/AsirikuyFrameworkAPI/include/**.*", 
	["Source Files/*"] = "../../../dev/AsirikuyFrameworkAPI/src/**.*", 
	["Resource Files"] = "../../../dev/AsirikuyFrameworkAPI/res/**.*", 
	["Config Files"]   = "../../../dev/AsirikuyFrameworkAPI/config/**.*", 
	["Unit Tests"]     = "../../../dev/AsirikuyFrameworkAPI/tests/**.*"
  }
  includedirs{
    "src"
  }
  uses "NTPClient"
  configuration{"windows"}
    links{ 
	  "MiniXML", 
	  "chartdir51", 
	  "Jasper", 
	  "LibJPEG", 
	  "LibMNG", 
	  "LibPNG", 
	  "LittleCMS", 
	  "Zlib", 
	  "DevIL", 
	  "Shark", 
	  "AsirikuyCommon", 
	  "Log", 
	  "SymbolAnalyzer", 
	  "AsirikuyTechnicalAnalysis", 
	  "OrderManager", 
	  "TradingStrategies",
	  "AsirikuyEasyTrade",
	  "Waffles",
	  "curl",
	  "TALib_common", 
	  "TALib_abstract", 
	  "TALib_func",
	  "Pantheios_frontend",
	  "Pantheios_backend",
	  "Pantheios_utils", 
	  "Pantheios_core",
	  "dSFMT",
	  "FANN"
    }
  boostdir = os.getenv("BOOST_ROOT")
  requiredBoostLibs = "--with-system --with-chrono --with-thread --with-date_time --with-regex --with-filesystem --with-serialization --with-test"  
  os.chdir("../..")
  cwd = os.getcwd()
  configuration{"not windows"}
    excludes{
      "**.rc",
      "**.def"
    }
  configuration{"windows"}
    linkoptions{"/DEF:../../../dev/AsirikuyFrameworkAPI/src/AsirikuyFrameworkAPI.def"}
  configuration{"windows", "Release"}
    linkoptions{"/OPT:REF,ICF"}
  configuration{"not windows"}
    links{
	  "TradingStrategies",
      "chartdir51",
	  "DevIL",
	  "AsirikuyCommon",
	  "Log",
	  "SymbolAnalyzer",
      "AsirikuyEasyTrade",  
      "Waffles",
	  "OrderManager",
	  "AsirikuyTechnicalAnalysis",
      "Shark",
      "MiniXML",  
	  "Jasper",
	  "LibJPEG",
	  "LibMNG",
	  "LibPNG",
	  "LittleCMS",
	  "Zlib",	  
	  "TALib_common",
	  "TALib_abstract",
	  "TALib_func",
	  "FANN",	
	  "dSFMT",	  
	  "NTPClient",
	  "Pantheios_core",
	  "Pantheios_frontend",
	  "Pantheios_backend",
	  "Pantheios_utils",
	  "curl",
	  "boost_prg_exec_monitor",
	  "boost_regex",
	  "boost_test_exec_monitor",
	  "boost_unit_test_framework",
	  "boost_filesystem",
	  "boost_wserialization",
	  "boost_serialization",
	  "boost_thread",
	  "boost_system",
	  "boost_date_time",
	  "boost_atomic",
	  "pthread"
    }
  if _ARGS[1] == "WITH_R" then
	links{"AsirikuyRWrapper"}
  end
  configuration{"linux"}
	links{"rt"}
  -- Windows
  configuration{"windows", "x32", "Debug"}
    targetdir("bin/" .. _ACTION .. "/x32/Debug")
    prelinkcommands{"cd " .. boostdir, "b2.exe " .. requiredBoostLibs .. " --abbreviate-paths --build-dir=" .. cwd .. "/tmp --stagedir=" .. cwd .. "/bin/" .. _ACTION .. "/x32/Debug variant=debug link=static threading=multi runtime-link=static address-model=32 architecture=x86 stage 2>nul 1>nul"}
  configuration{"windows", "x32", "Release"}
    targetdir("bin/" .. _ACTION .. "/x32/Release")
    prelinkcommands{"cd " .. boostdir, "b2.exe " .. requiredBoostLibs .. " --abbreviate-paths --build-dir=" .. cwd .. "/tmp --stagedir=" .. cwd .. "/bin/" .. _ACTION .. "/x32/Release variant=release link=static threading=multi runtime-link=static address-model=32 architecture=x86 stage 2>nul 1>nul"}
  configuration{"windows", "x64", "Debug"}
    targetdir("bin/" .. _ACTION .. "/x64/Debug")
    prelinkcommands{"cd " .. boostdir, "b2.exe " .. requiredBoostLibs .. " --abbreviate-paths --build-dir=" .. cwd .. "/tmp --stagedir=" .. cwd .. "/bin/" .. _ACTION .. "/x64/Debug variant=debug link=static threading=multi runtime-link=static address-model=64 stage 2>nul 1>nul"}
  configuration{"windows", "x64", "Release"}
    targetdir("bin/" .. _ACTION .. "/x64/Release")
    prelinkcommands{"cd " .. boostdir, "b2.exe " .. requiredBoostLibs .. " --abbreviate-paths --build-dir=" .. cwd .. "/tmp --stagedir=" .. cwd .. "/bin/" .. _ACTION .. "/x64/Release variant=release link=static threading=multi runtime-link=static address-model=64 architecture=ia64 stage 2>nul 1>nul"}
  -- Not Windows
  configuration{"not windows", "x32", "Debug"}
    targetdir("bin/" .. _ACTION .. "/x32/Debug")
    prebuildcommands{"cd " .. boostdir .. " && ./b2 --with-atomic " .. requiredBoostLibs .. " --abbreviate-paths --build-dir=" .. cwd .. "/tmp --stagedir=" .. cwd .. "/bin/" .. _ACTION .. "/x32/Debug variant=debug link=static threading=multi runtime-link=static address-model=32 architecture=x86 stage"}
  configuration{"not windows", "x64", "Debug"}
    targetdir("bin/" .. _ACTION .. "/x64/Debug")
    prebuildcommands{"cd " .. boostdir .. " && ./b2 --with-atomic " .. requiredBoostLibs .. " --abbreviate-paths --build-dir=" .. cwd .. "/tmp --stagedir=" .. cwd .. "/bin/" .. _ACTION .. "/x64/Debug variant=debug link=static threading=multi runtime-link=static address-model=64 cxxflags=-fPIC architecture=ia64 stage"}
  configuration{"not windows", "x32", "Release"}
    targetdir("bin/" .. _ACTION .. "/x32/Release")
    prebuildcommands{"cd " .. boostdir .. " && ./b2 --with-atomic " .. requiredBoostLibs .. " --abbreviate-paths --build-dir=" .. cwd .. "/tmp --stagedir=" .. cwd .. "/bin/" .. _ACTION .. "/x32/Release variant=release link=static threading=multi runtime-link=static address-model=32 architecture=x86 cxxflags=-march=i686 stage"}
  configuration{"not windows", "x64", "Release"}
    targetdir("bin/" .. _ACTION .. "/x64/Release")
    prebuildcommands{"cd " .. boostdir .. " && ./b2 --with-atomic " .. requiredBoostLibs .. " --abbreviate-paths --build-dir=" .. cwd .. "/tmp --stagedir=" .. cwd .. "/bin/" .. _ACTION .. "/x64/Release variant=release link=static threading=multi runtime-link=static address-model=64 cxxflags=-fPIC architecture=ia64 stage"}
  configuration{"linux"}
    linkoptions{"-rdynamic"}
    buildoptions{"-DBOOST_THREAD_USE_LIB"}	
	
