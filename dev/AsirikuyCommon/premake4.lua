project "AsirikuyCommon"
  location("../../build/" .. _ACTION .. "/projects")
  kind "StaticLib"
  language "C++"
  files{
    "**.h", 
	"**.c", 
	"**.hpp", 
	"**.cpp"
  }
  -- vpaths not supported in premake4 4.3
  includedirs{
    "src"
  }
  -- Enable C++11 for Boost compatibility
  -- Since language is "C++", all files are compiled as C++, so C++11 is fine
  buildoptions{"-std=c++11"}
  --pchheader "Precompiled.h"
  --pchsource "Precompiled.c"
  --pchheader "Precompiled.hpp"
  --pchsource "Precompiled.cpp"