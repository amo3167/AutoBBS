project "Pantheios_core"
  location("../../build/" .. _ACTION .. "/projects")
  kind "StaticLib"
  language "C++"
  files{
    "include/pantheios/**.h", 
	"include/pantheios/**.hpp", 
	"src/core/**.*"
  }
  vpaths{
	["Header Files/*"] = "../../../vendor/Pantheios/include/pantheios/**.*", 
	["Source Files"] = "../../../vendor/Pantheios/src/core/**.*"
  }

project "Pantheios_utils"
  location("../../build/" .. _ACTION .. "/projects")
  kind "StaticLib"
  language "C++"
  files{
    "include/pantheios/util/**.h", 
	"src/util/**.c", 
	"src/util/**.cpp"
  }
  vpaths{
	["Header Files/*"] = "../../../vendor/Pantheios/include/pantheios/util/**.h", 
	["Source Files"] = {"**.c", "**.cpp"}
  }

project "Pantheios_frontend"
  location("../../build/" .. _ACTION .. "/projects")
  kind "StaticLib"
  language "C"
  files{
    "include/pantheios/frontend.h", 
	"include/pantheios/frontends/fe.simple.h", 
	"src/frontends/fe.simple.c"
  }
  vpaths{
	["Header Files"] = "**.h", 
	["Source Files"] = "**.c"
  }

project "Pantheios_backend"
  location("../../build/" .. _ACTION .. "/projects")
  kind "StaticLib"
  language "C++"
  files{
    "include/pantheios/backend.h", 
	"include/pantheios/backends/bec.file.h", 
	"src/backends/be/be.file.c", 
	"src/backends/bec.file.cpp"
  }
  vpaths{
	["Header Files"] = "**.h", 
	["Source Files"] = {"**.c", "**.cpp"}
  }
  
