-- Minimal premake4.lua for building TradingStrategies only
-- This version excludes vendor libraries that are not needed

-- Handle action
if _ACTION == "clean" then
  os.rmdir("bin")
  os.rmdir("build")
  os.rmdir("tmp")
  os.rmdir("doc")
elseif _ACTION == "doc" then
  os.outputof("doxygen Doxyfile")
elseif _ACTION == nil then
  -- No action was provided
else
  -- Check if BOOST_ROOT is set
  boostdir = os.getenv("BOOST_ROOT")

  if boostdir == nil then
      printf("Set the environment variable BOOST_ROOT first!")
      return
  else
    -- Skip boost bootstrap in Docker or if directory doesn't exist
    if os.isdir(boostdir) then
      cwd = os.getcwd()
      os.chdir(boostdir)
      if os.get() == "windows" and not os.isfile("b2.exe") then
        if os.execute then
          os.execute("bootstrap.bat")
        end
      elseif os.get() ~= "windows" and not os.isfile("./b2") then
        -- Skip bootstrap in Docker - boost should already be built
      end
      os.chdir(cwd)
    end
  end
  
  -- The main definition of the solution starts here
  solution "AsirikuyFramework"
    -- Build directories
	location("build/" .. _ACTION)
	objdir("tmp/" .. _ACTION)
    -- Supported platforms and configurations
	platforms{"x32", "x64"}
	configurations{"Debug", "Release"}
    -- Header directories (minimal - only what's needed)
	includedirs{
      boostdir,
      boostdir .. "/include",
	  "vendor/TALib/c/include",
	  "vendor/MiniXML",  -- MiniXML headers (mxml-private.h)
	  "vendor/Gaul/src",  -- Gaul headers (gaul.h)
	  "vendor/Gaul/util/gaul",  -- Gaul utility headers (gaul_util.h)
	  "core/AsirikuyCommon/include", 
	  "core/SymbolAnalyzer/include", 
	  "core/Log/include", 
	  "core/AsirikuyTechnicalAnalysis/include", 
	  "core/OrderManager/include", 
	  "core/TradingStrategies/include", 
	  "core/TradingStrategies/include/strategies", 
	  "core/AsirikuyEasyTrade/include",
	  "core/NTPClient/include", 
	  "core/AsirikuyFrameworkAPI/include", 
	  "core/AsirikuyFrameworkAPI/include/MQL", 
	  "core/AsirikuyFrameworkAPI/include/jforex",
	  "core/AsirikuyFrameworkAPI/include/CTester",
	  "core/UnitTests/include",
	  "core/CTesterFrameworkAPI/include"
	}
	
    -- Global build settings
	flags{"StaticRuntime", "Unsafe"}
    -- Build type specific settings
    configuration{"Debug"}
      flags{"Symbols"}
	  defines{"DEBUG", "_DEBUG"}
    configuration{"Release"}
	  defines{"NDEBUG"}
	  flags{"OptimizeSize", "NoFramePointer"}
    -- OS specific settings
	configuration{"macosx"}
      includedirs{"/opt/local/include"}
      libdirs{"/opt/local/lib"}
	  defines{"DARWIN", "unix", "UNIX"}
    configuration{"linux", "x32"}
      buildoptions{"-march=i686"}
    configuration{"linux", "x64"}
      buildoptions{"-fPIC", "-O2", "-march=native"}
    -- Individual builds
	configuration{"macosx", "x64", "Debug"}
	  targetdir("bin/" .. _ACTION .. "/x64/Debug/lib")
	  libdirs{"bin/" .. _ACTION .. "/x64/Debug/lib"}
	configuration{"macosx", "x64", "Release"}
	  targetdir("bin/" .. _ACTION .. "/x64/Release/lib")
	  libdirs{"bin/" .. _ACTION .. "/x64/Release/lib"}
	
	-- Vendor projects (must be built before core projects that depend on them)
	include "vendor/TALib"
	include "vendor/Gaul"
	
	-- Core projects
	include "core/AsirikuyCommon"
	include "core/Log"
	include "core/SymbolAnalyzer"
	include "core/AsirikuyEasyTrade"
	include "core/AsirikuyTechnicalAnalysis"
	include "core/OrderManager"
	include "core/TradingStrategies"
	include "core/NTPClient"
	include "core/AsirikuyFrameworkAPI"
	include "core/CTesterFrameworkAPI"
	include "core/UnitTests"
end

