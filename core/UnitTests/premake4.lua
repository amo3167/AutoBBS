project "UnitTests"
  location("../../build/" .. _ACTION .. "/projects")
  kind "ConsoleApp"
  language "C++"
  files{
    "**.hpp", 
	"**.cpp"
  }
  links{ "NTPClient" }
  includedirs{
    "../AsirikuyCommon/tests", 
    "../AsirikuyFrameworkAPI/tests", 
    "../AsirikuyTechnicalAnalysis/tests", 
    "../Log/tests", 
    "../NTPClient/tests", 
    "../OrderManager/tests", 
    "../SymbolAnalyzer/tests", 
    "../TradingStrategies/tests"
  }
  links{
    -- Do not change the order of these libraries,
    -- otherwise the GCC on Linux will complain
    "AsirikuyFrameworkAPI",
	"AsirikuyTechnicalAnalysis",
	"AsirikuyEasyTrade",
	"TradingStrategies",
	"SymbolAnalyzer",
	"OrderManager",
	"TALib_func",
	-- Pantheios removed - using standard fprintf for logging
	-- "Pantheios_core",
	-- "Pantheios_utils",
	-- "chartdir51", -- Removed: ChartDirector not available, Windows/Linux only
  }
  configuration{"windows", "Release"}
    linkoptions{"/OPT:REF,ICF"}
  configuration{"not windows"}
    libdirs{
      os.getenv("BOOST_ROOT") .. "/lib"
    }
    linkoptions{
      "-L" .. os.getenv("BOOST_ROOT") .. "/lib"
    }
    links{
      "boost_serialization",
      "boost_unit_test_framework"
    }
  os.chdir("../..")
  -- Windows
  configuration{"windows", "x32", "Debug"}
    targetdir("bin/" .. _ACTION .. "/x32/Debug")
    postbuildcommands{"cd " .. cwd .. "/bin/" .. _ACTION .. "/x32/Debug/", "UnitTests.exe"}
  configuration{"windows", "x64", "Debug"}
    targetdir("bin/" .. _ACTION .. "/x64/Debug")
    postbuildcommands{"cd " .. cwd .. "/bin/" .. _ACTION .. "/x64/Debug", "UnitTests.exe"}
  configuration{"windows", "x32", "Release"}
    targetdir("bin/" .. _ACTION .. "/x32/Release")
    postbuildcommands{"cd " .. cwd .. "/bin/" .. _ACTION .. "/x32/Release", "UnitTests.exe"}
  configuration{"windows", "x64", "Release"}
    targetdir("bin/" .. _ACTION .. "/x64/Release")
    postbuildcommands{"cd " .. cwd .. "/bin/" .. _ACTION .. "/x64/Release", "UnitTests.exe"}
  -- Linux
  configuration{"linux"}
    linkoptions{"-rdynamic"}
  configuration{"linux", "x32", "Debug"}
    targetdir("bin/" .. _ACTION .. "/x32/Debug")
    postbuildcommands{"LD_LIBRARY_PATH=" .. cwd .. "/bin/" .. _ACTION .. "/x32/Debug/lib:" .. cwd .. "/bin/" .. _ACTION .. "/x32/Debug/UnitTests"}
  configuration{"linux", "x64", "Debug"}
    targetdir("bin/" .. _ACTION .. "/x64/Debug")
    postbuildcommands{"LD_LIBRARY_PATH=" .. cwd .. "/bin/" .. _ACTION .. "/x64/Debug/lib:" .. cwd .. "/bin/" .. _ACTION .. "/x64/Debug/UnitTests"}
  configuration{"linux", "x32", "Release"}
    targetdir("bin/" .. _ACTION .. "/x32/Release")
    postbuildcommands{"LD_LIBRARY_PATH=" .. cwd .. "/bin/" .. _ACTION .. "/x32/Release/lib:" .. cwd .. "/bin/" .. _ACTION .. "/x32/Release/UnitTests"}
  configuration{"linux", "x64", "Release"}
    targetdir("bin/" .. _ACTION .. "/x64/Release")
    postbuildcommands{"LD_LIBRARY_PATH=" .. cwd .. "/bin/" .. _ACTION .. "/x64/Release/lib:" .. cwd .. "/bin/" .. _ACTION .. "/x64/Release/UnitTests"}
  -- Mac OSX
  configuration{"macosx", "x32", "Debug"}
    targetdir("bin/" .. _ACTION .. "/x32/Debug")
    postbuildcommands{"DYLD_LIBRARY_PATH=" .. cwd .. "/bin/" .. _ACTION .. "/x32/Debug/lib:" .. cwd .. "/bin/" .. _ACTION .. "/x32/Debug/UnitTests"}
  configuration{"macosx", "x32", "Release"}
    targetdir("bin/" .. _ACTION .. "/x32/Release")
    postbuildcommands{"DYLD_LIBRARY_PATH=" .. cwd .. "/bin/" .. _ACTION .. "/x32/Release/lib:" .. cwd .. "/bin/" .. _ACTION .. "/x32/Release/UnitTests"}
  configuration{"macosx", "x64", "Debug"}
    targetdir("bin/" .. _ACTION .. "/x64/Debug")
    postbuildcommands{"DYLD_LIBRARY_PATH=" .. cwd .. "/bin/" .. _ACTION .. "/x64/Debug/lib:" .. cwd .. "/bin/" .. _ACTION .. "/x64/Debug/UnitTests"}
  configuration{"macosx", "x64", "Release"}
    targetdir("bin/" .. _ACTION .. "/x64/Release")
    postbuildcommands{"DYLD_LIBRARY_PATH=" .. cwd .. "/bin/" .. _ACTION .. "/x64/Release/lib:" .. cwd .. "/bin/" .. _ACTION .. "/x64/Release/UnitTests"}

