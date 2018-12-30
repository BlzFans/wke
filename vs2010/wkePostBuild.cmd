mkdir 2>NUL ..\demo\bin
mkdir 2>NUL ..\demo\libwke

xcopy /D /F /R /Y "%TARGETPATH%" ..\demo\bin\
xcopy /D /F /R /Y "%CONFIGURATIONBUILDDIR%\lib\wke.lib" ..\demo\libwke\
xcopy /D /F /R /Y "..\wke\wke.h" ..\demo\libwke\
xcopy /D /F /R /Y "%TARGETPATH%" ..\demo\libwke\