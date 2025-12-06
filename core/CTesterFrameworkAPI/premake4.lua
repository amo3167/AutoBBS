project "CTesterFrameworkAPI"
  location("../../build/" .. _ACTION .. "/projects")
  kind "SharedLib"
  language "C"
  files {
	"src/**.c",
    "src/**.def",
	"include/**.h",
  }
  includedirs{
    "src",
    "include",
    "../../../vendor/Gaul/src",  -- Gaul headers (gaul.h) - only for Linux/macOS
    "../../../vendor/Gaul/util/gaul"  -- Gaul utility headers (gaul_util.h)
  }
  links{
	"AsirikuyFrameworkAPI",
	-- Pantheios removed - using standard fprintf for logging
	-- "Pantheios_core", 
	-- "Pantheios_utils", 
	-- "Pantheios_frontend", 
	-- "Pantheios_backend",
	"AsirikuyCommon", 
	"Log"
  }
  -- Gaul library only on Linux/macOS (not available on Windows)
  configuration{"not windows"}
    links{"Gaul"}
  configuration{}
  libdirs{
	"../../bin/**",
	"../../../bin/" .. _ACTION .. "/x64/Debug/lib",  -- Gaul library location
	"../../../bin/" .. _ACTION .. "/x32/Debug/lib",
	"../../../bin/" .. _ACTION .. "/x64/Release",    -- AsirikuyFrameworkAPI.lib for x64 Release
	"../../../bin/" .. _ACTION .. "/x32/Release"     -- AsirikuyFrameworkAPI.lib for x32 Release
  }
  configuration{"macosx"}
    libdirs{
      (os.getenv("HOMEBREW_PREFIX") or "/opt/homebrew") .. "/opt/mpich/lib",
      "/Users/andym/homebrew/opt/mpich/lib",
      (os.getenv("HOMEBREW_PREFIX") or "/opt/homebrew") .. "/opt/libomp/lib",
      "/Users/andym/homebrew/opt/libomp/lib"
    }
    includedirs{
      (os.getenv("HOMEBREW_PREFIX") or "/opt/homebrew") .. "/opt/libomp/include",
      "/Users/andym/homebrew/opt/libomp/include"
    }
    buildoptions{
      "-Xpreprocessor", "-fopenmp",
      "-D_OPENMP"
    }
    linkoptions{
      "-L" .. ((os.getenv("HOMEBREW_PREFIX") or "/opt/homebrew") .. "/opt/mpich/lib"),
      "-L/Users/andym/homebrew/opt/mpich/lib",
      "-headerpad_max_install_names",
      "-Xpreprocessor", "-fopenmp",
      "-lomp"
    }
  configuration{"not windows"}
    excludes {
      "src/**.def"
    }
  configuration{"windows"}
    linkoptions{"/DEF:../../../core/CTesterFrameworkAPI/src/CTesterFrameworkAPI.def"}
    -- MPI not available on Windows by default
    -- links{"mpi"}
    buildoptions{"/openmp"}
    defines{"_OPENMP"}
  configuration{"macosx", "x64"}
    links{"mpi", "pmpi", "mpl"}
  configuration{"macosx", "x32"}
    links{"mpi", "pmpi", "mpl"}
  configuration{"linux"}
    links{"mpich", "mpl"}
    buildoptions{"-fopenmp", "-D_OPENMP"}
    linkoptions{"-fopenmp"}
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

