project "NTPClient"
  location("../../build/" .. _ACTION .. "/projects")
  kind "StaticLib"
  language "C++"
  files{
    "**.hpp", 
	"**.cpp"
  }
  includedirs{
    "src"
  }
  -- Enable C++11 for Boost compatibility
  buildoptions{"-std=c++11"}
  -- Boost dependency (handled via BOOST_ROOT)
  -- links{ "Boost" } -- Commented out as Boost is linked via system
