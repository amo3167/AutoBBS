project "AsirikuyEasyTrade"
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