project "MiniXML"
  location("../../build/" .. _ACTION .. "/projects")
  kind "StaticLib"
  language "C"
  files{
	"**.h", 
	"**.c"
  }
  excludes{
	"testmxml.c", 
	"mxmldoc.c"
  }
  vpaths{
	["Header Files"] = "**.h", 
	["Source Files"] = "**.c"
  }

