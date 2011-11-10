@setlocal

@call "%VS80COMNTOOLS%vsvars32.bat"

@cd libcurl

@call build.bat

@cd ..\zlib

@call build.bat

@endlocal