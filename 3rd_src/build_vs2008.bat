@setlocal

@call "%VS90COMNTOOLS%vsvars32.bat"

@cd libcurl

@call build.bat

@cd ..\zlib

@call build.bat

@endlocal