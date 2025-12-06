project "TALib_common"
  location("../../build/" .. _ACTION .. "/projects")
  kind "StaticLib"
  language "C"
  includedirs{"c/include", "c/src/ta_common"}
  files{
	"c/include/ta_common.h", 
	"c/include/ta_defs.h", 
	"c/src/ta_common/**.h", 
	"c/src/ta_common/**.c"
  }
  vpaths{
	["Header Files"] = "**.h", 
	["Source Files"] = "**.c"
  }
  
project "TALib_abstract"
  location("../../build/" .. _ACTION .. "/projects")
  kind "StaticLib"
  language "C"
  includedirs{"c/include", "c/src/ta_common", "c/src/ta_abstract", "c/src/ta_abstract/frames"}
  links{"TALib_common"}
  files{
    "c/include/ta_abstract.h", 
	"c/src/ta_abstract/**.h", 
	"c/src/ta_abstract/**.c"
  }
  excludes{"c/src/ta_abstract/excel_glue.c"}
  vpaths{
	["Header Files"] = "**.h", 
	["Source Files"] = "**.c"
  }
  
project "TALib_func"
  location("../../build/" .. _ACTION .. "/projects")
  kind "StaticLib"
  language "C"
  includedirs{"c/include", "c/src/ta_common", "c/src/ta_func"}
  links{"TALib_common", "TALib_abstract"}
  files{
	"c/include/ta_func.h", 
	"c/include/ta_libc.h", 
	"c/src/ta_func/**.h", 
	"c/src/ta_func/**.c"
  }
  vpaths{
	["Header Files"] = "**.h", 
	["Source Files"] = "**.c"
  }
  