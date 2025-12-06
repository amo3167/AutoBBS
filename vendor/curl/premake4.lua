project "curl"
  location("../../build/" .. _ACTION .. "/projects")
  kind "StaticLib"
  language "C++"
  files{
    "include/**.h", 
	"src/**.c"
  }
  vpaths{
	["Header Files/*"] = "../../../vendor/curl/include/**.h", 
	["Source Files/*"] = "../../../vendor/curl/src/**.c"
  }
  configuration{"vs*"}
    defines{"_CRT_SECURE_NO_WARNINGS"}
