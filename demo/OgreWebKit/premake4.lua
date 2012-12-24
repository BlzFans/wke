solution "OgreWebKit"
   configurations { "Debug", "Release" }
   
   project "OgreWebKit"
      kind "WindowedApp"
      language "C++"
      files { "*.h", "*.cpp" }
      includedirs { "$(OGRE_HOME)/include/ogre", "$(OGRE_HOME)/include/ois", "$(OGRE_HOME)/boost", "../libwke" } 
      defines { "BOOST_ALL_NO_LIB", "_CRT_SECURE_NO_WARNINGS" }
      targetdir "bin"
      buildoptions { "/wd4251", "/wd4275", "/wd4193" }
 
      configuration "Debug"
         defines { "_DEBUG" }
         flags { "WinMain", "Symbols" }
         libdirs { "$(OGRE_HOME)/lib/Debug", "../libwke" }
         links { "wke", "OgreMain_d", "OIS_d" }
         targetname "$(ProjectName)_d"
 
      configuration "Release"
         defines { "NDEBUG" }
         flags { "WinMain", "Optimize" }    
         libdirs { "$(OGRE_HOME)/lib/Release", "../libwke" }
         links { "wke", "OgreMain", "OIS" }
         targetname "$(ProjectName)"
