
Web and Flash Embedded in 3D games, based on WebKit




Building wke

VS2005:
1 Install Visual Studio 2005 SP1
2 Install the Windows SDK 6.0
3 Open the vs2005/wke.sln solution file in Visual Studio and build the solution. 
  This can take from 30 minutes to 1 hours. More likely 40 minutes.

VS2008:
1 Install Visual Studio 2008 SP1
2 Install the Windows SDK 6.0 or newer
3 Open the vs2008/wke.sln solution file in Visual Studio and build the solution. 
  This can take from 30 minutes to 1 hours. More likely 40 minutes.

Run wke
The wkeBrowser.exe executable can be found at demo/bin/wkeBrowser.exe


Common Build Errors:

1. Cygwin exception::handle: Exception: STATUS_ACCESS_VIOLATION

   Close the Visual Studio, then relaunch and rebuild. This usually fixes the problem.