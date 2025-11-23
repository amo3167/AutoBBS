project "Log"
  location("../../build/" .. _ACTION .. "/projects")
  kind "StaticLib"
  language "C"
  files{
    "**.h", 
	"**.c", 
	"**.hpp"
  }
  -- vpaths not supported in premake4 4.3
  includedirs{
    "src"
  }
  
  -- Target directory configurations (all libraries go to root bin/ directory)
  -- Use relative path from premake4.lua location (core/Log/) to workspace root: ../../
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