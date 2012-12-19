@echo off

cd libcurl
call build.bat

cd ..\zlib
call build.bat

cd ..\cairo
call build.bat

cd ..\libpng
call build.bat

cd ..\libxml2
call build.bat

cd ..\libxslt
call build.bat