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