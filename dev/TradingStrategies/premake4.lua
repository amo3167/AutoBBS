project "TradingStrategies"
  location("../../build/" .. _ACTION .. "/projects")
  kind "StaticLib"
  language "C++"  -- Changed from "C" to enable C++ compilation
  
  -- File patterns: include both C and C++ files
  files{
    "**.h",      -- C headers
    "**.c",      -- C source (legacy, will be removed during migration)
    "**.hpp",    -- C++ headers
    "**.cpp"     -- C++ source (new)
  }
  
  -- Virtual paths for better organization in IDE
  vpaths{
    ["Header Files/C"] = {"../../../dev/TradingStrategies/include/**.h"},
    ["Header Files/C++"] = {"../../../dev/TradingStrategies/include/**.hpp"},
    ["Source Files/C"] = {"../../../dev/TradingStrategies/src/**.c"},
    ["Source Files/C++"] = {"../../../dev/TradingStrategies/src/**.cpp"},
    ["Unit Tests"] = {"../../../dev/TradingStrategies/tests/**.*"}
  }
  
  -- Include directories: add C++ include paths
  includedirs{
    "src",
    "include",                -- C++ header directory
    "include/strategies"      -- Strategy headers
  }
  
  -- Preprocessor defines
  defines{
    "FANN_NO_DLL",
    "CPP_MIGRATION_IN_PROGRESS"  -- Feature flag for conditional compilation
  }
  
  -- C++ specific compiler flags for Visual Studio
  buildoptions{
    "/EHsc",   -- Enable C++ exception handling
    "/GR"      -- Enable RTTI (Runtime Type Information)
  }
  
  -- Configuration-specific settings
  configuration "Debug"
    defines { "_DEBUG" }
    flags { "Symbols" }
    
  configuration "Release"
    defines { "NDEBUG" }
    flags { "Optimize" }

