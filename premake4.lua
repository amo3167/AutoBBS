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
  -- Check if BOOST_ROOT is set and bootstrapped
  boostdir = os.getenv("BOOST_ROOT")

  if boostdir == nil then
      printf("Set the environment variable BOOST_ROOT first!")
      return
  else
    -- Only bootstrap if boost directory exists and b2 is not present
    -- Skip bootstrap in Docker or if boost directory doesn't exist
    if os.isdir(boostdir) then
      cwd = os.getcwd()
      os.chdir(boostdir)
      if os.get() == "windows" and not os.isfile("b2.exe") then
        if os.execute then
          os.execute("bootstrap.bat")
        end
      elseif os.get() ~= "windows" and not os.isfile("./b2") then
        -- Skip bootstrap in Docker - boost should already be built
        -- This is just for generating build files, not building
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
    -- Header directories
	if _ARGS[1] == "WITH_R" then
		rfolder = os.getenv("R_HOME")
		includedirs{"/usr/share/R/include/",
					"/usr/lib/R/site-library/Rcpp/include",
					"/usr/lib/R/site-library/RInside/include"}
	end
	includedirs{
      boostdir .. "/include",
	  "vendor/STLSoft/include", 
	  "vendor/Pantheios/include", 
	  "vendor/Pantheios/include/pantheios/backends", 
	  "vendor/Pantheios/include/pantheios/frontends", 
	  "vendor/MiniXML", 
	  "vendor/MiniXML/vcnet", 
	  "vendor/TALib/c/include", 
	  "vendor/TALib/c/src/ta_common", 
      "vendor/TALib/c/src/ta_abstract", 
      "vendor/TALib/c/src/ta_abstract/frames", 
      "vendor/TALib/c/src/ta_tables", 
	  "vendor/dSFMT", 
	  "vendor/FANN/src", 
	  "vendor/FANN/src/include", 
	  "vendor/ChartDirector/include", 
	  "vendor/Shark/include", 
	  "vendor/Shark/include/Array", 
	  "vendor/Shark/include/EALib", 
	  "vendor/Shark/include/FileUtil", 
	  "vendor/Shark/include/Fuzzy", 
	  "vendor/Shark/include/LinAlg", 
	  "vendor/Shark/include/Mixture", 
	  "vendor/Shark/include/MOO-EALib", 
	  "vendor/Shark/include/ReClaM", 
	  "vendor/Shark/include/Rng", 
	  "vendor/Shark/include/TimeSeries", 
	  "vendor/DevIL/include", 
	  "vendor/DevIL/src-IL/include", 
	  "vendor/DevIL/src-ILU/include", 
	  "vendor/DevIL/src-ILUT/include", 
	  "vendor/Jasper/src/libjasper/include", 
	  "vendor/Jasper/src/libjasper/include/jasper", 
	  "vendor/LibJPEG", 
	  "vendor/LibMNG", 
	  "vendor/LibPNG", 
	  "vendor/LittleCMS/include", 
	  "vendor/Zlib", 
	  "vendor/Gaul/src",
	  "vendor/Gaul/src/gaul",
	  "vendor/Gaul/util/gaul",
	  "vendor/MPICH2/include",
	  "vendor/Waffles/src",
	  "dev/AsirikuyCommon/include", 
	  "dev/SymbolAnalyzer/include", 
	  "dev/Log/include", 
	  "dev/AsirikuyTechnicalAnalysis/include", 
	  "dev/OrderManager/include", 
	  "dev/TradingStrategies/include", 
	  "dev/TradingStrategies/include/strategies", 
	  "dev/AsirikuyEasyTrade/include",
	  "dev/NTPClient/include", 
	  "dev/AsirikuyFrameworkAPI/include", 
	  "dev/AsirikuyFrameworkAPI/include/MQL", 
	  "dev/AsirikuyFrameworkAPI/include/jforex",
	  "dev/AsirikuyFrameworkAPI/include/CTester",
	  "dev/UnitTests/include",
	  "dev/CTesterFrameworkAPI/include"
	}
	
	if _ARGS[1] == "WITH_R" then
		includedirs{"dev/AsirikuyRWrapper/include"}
	end
	
    -- Global build settings
	flags{"StaticRuntime", "Unsafe"}
	-- STLSoft configuration (needed for cross-compilation)
	-- For 32-bit builds, use LLP64 model (Windows-like: int=4, long=4, long long=8)
	defines{
		"__LLP64__",
		"_STLSOFT_SIZEOF_SHORT=2",
		"_STLSOFT_SIZEOF_INT=4",
		"_STLSOFT_SIZEOF_LONG=4",
		"_STLSOFT_SIZEOF_LONG_LONG=8"
	}
    -- Build type specific settings
    configuration{"Debug"}
      flags{"Symbols"}
	  defines{"DEBUG", "_DEBUG"}
    configuration{"Release"}
	  defines{"NDEBUG"}
	  flags{"OptimizeSize", "NoFramePointer"}
    -- OS specific settings
	configuration{"windows"}
      includedirs{
	    "vendor/curl/include/curl",
	    "vendor/curl/include"
      }
	  defines{"WINDOWS", "_WINDOWS"}
	configuration{"windows", "x32"}
	  defines{"WIN32", "_WIN32"}
    configuration{"windows", "Release"}
	  buildoptions{"/Ox", "/GL", "/GF", "/Gm-"}
	  linkoptions{"/LTCG"}
	configuration{"macosx"}
      includedirs{"/opt/local/include"}
      libdirs{"/opt/local/lib"}
	  defines{"DARWIN", "unix", "UNIX"}
    configuration{"macosx", "x32"}
      -- Override default architecture for 32-bit builds on macOS
      -- Remove unsupported armv4t architecture flag by setting ARCH to empty
      -- premake4 will use this to override the default ARCH variable
      buildoptions{"-m32"}
      -- Note: ARCH variable is set by premake4, we need to override it in the Makefile
    configuration{"linux", "x32"}
      buildoptions{"-march=i686"}
    configuration{"linux", "x64"}
      buildoptions{"-fPIC", "-O2", "-march=native"}
    -- Individual builds
	configuration{"windows", "x32", "Debug"}
	  targetdir("bin/" .. _ACTION .. "/x32/Debug/lib")
	  implibdir("bin/" .. _ACTION .. "/x32/Debug/lib")
	  libdirs{"bin/" .. _ACTION .. "/x32/Debug/lib", "vendor/ChartDirector/lib/x32", "vendor/MPICH2/lib"}
	configuration{"linux", "x32", "Debug"}
	  targetdir("bin/" .. _ACTION .. "/x32/Debug/lib")
	  if _ARGS[1] == "WITH_R" then  
		libdirs{"bin/" .. _ACTION .. "/x32/Debug/lib", "vendor/ChartDirector/lib/linux/x32", "vendor/MPICH2/lib/linux/x32","/usr/lib/R/lib/", "/usr/local/lib/R/site-library/Rcpp/lib", "/usr/local/lib/R/site-library/RInside/lib"}
	  else
		libdirs{"bin/" .. _ACTION .. "/x32/Debug/lib", "vendor/ChartDirector/lib/linux/x32", "vendor/MPICH2/lib/linux/x32"}
	  end  
	configuration{"macosx", "x32", "Debug"}
	  targetdir("bin/" .. _ACTION .. "/x32/Debug/lib")
	  libdirs{"bin/" .. _ACTION .. "/x32/Debug/lib", "vendor/ChartDirector/lib/universal", "vendor/MPICH2/lib/universal/x32"}
	configuration{"windows", "x64", "Debug"}
	  targetdir("bin/" .. _ACTION .. "/x64/Debug/lib")
	  implibdir("bin/" .. _ACTION .. "/x64/Debug/lib")
	  libdirs{"bin/" .. _ACTION .. "/x64/Debug/lib", "vendor/ChartDirector/lib/x64", "vendor/MPICH2/lib", "C:/Program Files/Microsoft SDKs/Windows/v7.1/Lib/x64"}
	configuration{"linux", "x64", "Debug"}
	  targetdir("bin/" .. _ACTION .. "/x64/Debug/lib")
	  if _ARGS[1] == "WITH_R" then  
		libdirs{"bin/" .. _ACTION .. "/x64/Debug/lib", "vendor/ChartDirector/lib/linux/x64", "vendor/MPICH2/lib/linux/x64","/usr/lib/R/lib/", "/usr/local/lib/R/site-library/Rcpp/lib", "/usr/local/lib/R/site-library/RInside/lib"}
	  else
		libdirs{"bin/" .. _ACTION .. "/x64/Debug/lib", "vendor/ChartDirector/lib/linux/x64", "vendor/MPICH2/lib/linux/x64"}
	  end
	configuration{"macosx", "x64", "Debug"}
	  targetdir("bin/" .. _ACTION .. "/x64/Debug/lib")
	  libdirs{"bin/" .. _ACTION .. "/x64/Debug/lib", "vendor/ChartDirector/lib/universal", "vendor/MPICH2/lib/universal/x64"}
	configuration{"windows", "x32", "Release"}
	  targetdir("bin/" .. _ACTION .. "/x32/Release/lib")
	  implibdir("bin/" .. _ACTION .. "/x32/Release/lib")
	  libdirs{"bin/" .. _ACTION .. "/x32/Release/lib", "vendor/ChartDirector/lib/x32", "vendor/MPICH2/lib"}
	configuration{"linux", "x32", "Release"}
	  targetdir("bin/" .. _ACTION .. "/x32/Release/lib")
	  if _ARGS[1] == "WITH_R" then  
		libdirs{"bin/" .. _ACTION .. "/x32/Release/lib", "vendor/ChartDirector/lib/linux/x32", "vendor/MPICH2/lib/linux/x32","/usr/lib/R/lib/", "/usr/local/lib/R/site-library/Rcpp/lib", "/usr/local/lib/R/site-library/RInside/lib"}
	  else
		libdirs{"bin/" .. _ACTION .. "/x32/Release/lib", "vendor/ChartDirector/lib/linux/x32", "vendor/MPICH2/lib/linux/x32"}
	  end	  
	configuration{"macosx", "x32", "Release"}
	  targetdir("bin/" .. _ACTION .. "/x32/Release/lib")
	  libdirs{"bin/" .. _ACTION .. "/x32/Release/lib", "vendor/ChartDirector/lib/universal", "vendor/MPICH2/lib/universal/x32"}
	configuration{"windows", "x64", "Release"}
	  targetdir("bin/" .. _ACTION .. "/x64/Release/lib")
	  implibdir("bin/" .. _ACTION .. "/x64/Release/lib")
	  libdirs{"bin/" .. _ACTION .. "/x64/Release/lib", "vendor/ChartDirector/lib/x64", "vendor/MPICH2/lib", "C:/Program Files/Microsoft SDKs/Windows/v7.1/Lib/x64"}
	configuration{"linux", "x64", "Release"}
	  targetdir("bin/" .. _ACTION .. "/x64/Release/lib")
	  if _ARGS[1] == "WITH_R" then  
		libdirs{"bin/" .. _ACTION .. "/x64/Release/lib", "vendor/ChartDirector/lib/linux/x64", "vendor/MPICH2/lib/linux/x64","/usr/lib/R/lib/", "/usr/local/lib/R/site-library/Rcpp/lib", "/usr/local/lib/R/site-library/RInside/lib"}
	  else
		libdirs{"bin/" .. _ACTION .. "/x64/Release/lib", "vendor/ChartDirector/lib/linux/x64", "vendor/MPICH2/lib/linux/x64"}
	  end
	configuration{"macosx", "x64", "Release"}
	  targetdir("bin/" .. _ACTION .. "/x64/Release/lib")
	  libdirs{"bin/" .. _ACTION .. "/x64/Release/lib", "vendor/ChartDirector/lib/universal", "vendor/MPICH2/lib/universal/x64"}
		
	if _ARGS[1] == "WITH_R" then	
		include "dev/AsirikuyRWrapper"
    end
	
	-- Projects
	-- Vendor libraries (include only if they exist)
	if os.isdir("vendor/MiniXML") then include "vendor/MiniXML" end
	if os.isdir("vendor/dSFMT") then include "vendor/dSFMT" end
	if os.isdir("vendor/FANN") then include "vendor/FANN" end
	if os.isdir("vendor/TALib") then include "vendor/TALib" end
	if os.isdir("vendor/Pantheios") then include "vendor/Pantheios" end
	if os.isdir("vendor/Shark") then include "vendor/Shark" end
	if os.isdir("vendor/DevIL") then include "vendor/DevIL" end
	if os.isdir("vendor/Jasper") then include "vendor/Jasper" end
	if os.isdir("vendor/LibJPEG") then include "vendor/LibJPEG" end
	if os.isdir("vendor/LibMNG") then include "vendor/LibMNG" end
	if os.isdir("vendor/LibPNG") then include "vendor/LibPNG" end
	if os.isdir("vendor/LittleCMS") then include "vendor/LittleCMS" end
	if os.isdir("vendor/Zlib") then include "vendor/Zlib" end
	if os.isdir("vendor/Gaul") then include "vendor/Gaul" end
	if os.isdir("vendor/Waffles") then include "vendor/Waffles" end
	include "dev/AsirikuyCommon"
	include "dev/Log"
	include "dev/SymbolAnalyzer"
	include "dev/AsirikuyEasyTrade"
	include "dev/AsirikuyTechnicalAnalysis"
	include "dev/OrderManager"
	include "dev/TradingStrategies"
	include "dev/NTPClient"
	include "dev/AsirikuyFrameworkAPI"
	include "dev/CTesterFrameworkAPI"
	include "dev/UnitTests"
    if os.get() == "windows" then
	  if os.isdir("vendor/curl") then include "vendor/curl" end
    end
end
