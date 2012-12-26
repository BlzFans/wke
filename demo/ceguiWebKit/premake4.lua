solution "ceguiWebKit"
   configurations { "Debug", "Release" }
   
   project "ceguiWebKit"
      kind "WindowedApp"
      language "C++"
      files { "*.h", "*.cpp" }
      defines { "CEGUI_STATIC", "_CRT_SECURE_NO_WARNINGS" }
      includedirs { "../cegui/include"} 
      targetdir "bin"
 
      configuration "Debug"
         defines { "_DEBUG" }
         flags { "WinMain", "Symbols", "StaticRuntime", "Unicode" }
         libdirs { "../lib", "../dependencies/lib/static" }
         
         links { "wke", "d3d9", "d3dx9", "freetype_D", "pcre_d", "expat_d", "SILLY_d", "libpng_d", "jpeg_d", "zlib_d", "Winmm", 
         "CEGUIBase_Static_d", "CEGUIDirect3D9Renderer_Static_d", "CEGUIExpatParser_Static_d", 
         "CEGUISILLYImageCodec_Static_d", "CEGUIFalagardWRBase_Static_d" }
         
         linkoptions { "/ignore:4099" }
         
         targetname "$(ProjectName)_d"
 
      configuration "Release"
         defines { "NDEBUG" }
         flags { "WinMain", "Optimize", "StaticRuntime", "Unicode"}    
         libdirs { "../lib", "../dependencies/lib/static" }
         
         links { "wke", "d3d9", "d3dx9", "freetype", "pcre", "expat", "SILLY", "libpng", "jpeg", "zlib", "Winmm",
         "CEGUIBase_Static", "CEGUIDirect3D9Renderer_Static", "CEGUIExpatParser_Static", 
         "CEGUISILLYImageCodec_Static", "CEGUIFalagardWRBase_Static" }
         
         targetname "$(ProjectName)"
