project "CTesterFrameworkAPI"
  location("../../build/" .. _ACTION .. "/projects")
  kind "SharedLib"
  language "C"
  files {
	"src/**.c",
    "src/**.def",
	"include/**.h",
  }
  links{
	"AsirikuyFrameworkAPI",
	"Gaul",
	"Pantheios_core", 
	"Pantheios_utils", 
	"Pantheios_frontend", 
	"Pantheios_backend",
	"AsirikuyCommon", 
	"Log"
  }
  libdirs{
	"../../bin/**"
  }
  vpaths{
	["Header Files"] = "include/**.h", 
	["Source Files"] = "src/**.c"
  }
  configuration{"not windows"}
    excludes {
      "src/**.def"
    }
  configuration{"windows"}
    linkoptions{"/DEF:../../../dev/CTesterFrameworkAPI/src/CTesterFrameworkAPI.def"}
    links{"mpi"}
  configuration{"macosx"}
    links{"mpich", "pmpich", "mpl"}
  configuration{"linux"}
    links{"mpich", "mpl"}

