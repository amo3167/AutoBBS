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
	-- Pantheios removed - using standard fprintf for logging
	-- "Pantheios_core", 
	-- "Pantheios_utils", 
	-- "Pantheios_frontend", 
	-- "Pantheios_backend",
	"AsirikuyCommon", 
	"Log"
  }
  libdirs{
	"../../bin/**"
  }
  configuration{"macosx"}
    libdirs{
      (os.getenv("HOMEBREW_PREFIX") or "/opt/homebrew") .. "/opt/mpich/lib",
      "/Users/andym/homebrew/opt/mpich/lib"
    }
  configuration{"not windows"}
    excludes {
      "src/**.def"
    }
  configuration{"windows"}
    linkoptions{"/DEF:../../../core/CTesterFrameworkAPI/src/CTesterFrameworkAPI.def"}
    links{"mpi"}
  configuration{"macosx"}
    links{"mpich", "pmpich", "mpl"}
  configuration{"linux"}
    links{"mpich", "mpl"}
  os.chdir("../..")
  cwd = os.getcwd()
  -- Windows
  configuration{"windows", "x32", "Debug"}
    targetdir("bin/" .. _ACTION .. "/x32/Debug")
  configuration{"windows", "x64", "Debug"}
    targetdir("bin/" .. _ACTION .. "/x64/Debug")
  configuration{"windows", "x32", "Release"}
    targetdir("bin/" .. _ACTION .. "/x32/Release")
  configuration{"windows", "x64", "Release"}
    targetdir("bin/" .. _ACTION .. "/x64/Release")
  -- Linux
  configuration{"linux", "x32", "Debug"}
    targetdir("bin/" .. _ACTION .. "/x32/Debug")
  configuration{"linux", "x64", "Debug"}
    targetdir("bin/" .. _ACTION .. "/x64/Debug")
  configuration{"linux", "x32", "Release"}
    targetdir("bin/" .. _ACTION .. "/x32/Release")
  configuration{"linux", "x64", "Release"}
    targetdir("bin/" .. _ACTION .. "/x64/Release")
  -- macOS
  configuration{"macosx", "x32", "Debug"}
    targetdir("bin/" .. _ACTION .. "/x32/Debug")
  configuration{"macosx", "x64", "Debug"}
    targetdir("bin/" .. _ACTION .. "/x64/Debug")
  configuration{"macosx", "x32", "Release"}
    targetdir("bin/" .. _ACTION .. "/x32/Release")
  configuration{"macosx", "x64", "Release"}
    targetdir("bin/" .. _ACTION .. "/x64/Release")

