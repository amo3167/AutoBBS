project "Gaul"
  location("../../build/" .. _ACTION .. "/projects")
  kind "StaticLib"
  language "C"
  files{
    "src/*.c",
	"src/*.h",
	"util/*.c",
	"src/gaul/*.h",
	"util/gaul/*.h",
  }
  vpaths{
	["Header Files"] = "**.h", 
	["Source Files"] = "**.c"
  }
