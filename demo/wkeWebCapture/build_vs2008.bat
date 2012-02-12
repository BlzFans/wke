@setlocal

@call "%VS90COMNTOOLS%vsvars32.bat"

@set COMPILER=cl /nologo /MT /O2 /W3 /c /DNDEBUG /D_CRT_SECURE_NO_DEPRECATE
@set LINKER=link /nologo

@%COMPILER% *.c
@%LINKER% /out:../bin/wkeWebCapture.exe *.obj ../libwke/wke.lib user32.lib
@del *.obj

@endlocal
