
@echo off

setlocal

if exist "%VS80COMNTOOLS%vsvars32.bat" (
    call "%VS80COMNTOOLS%vsvars32.bat" 
)else (
    call "%VS90COMNTOOLS%vsvars32.bat"
)

set COMPILER=cl /nologo /MT /O2 /W3 /c /D_CRT_SECURE_NO_DEPRECATE /I "../zlib"
set LIBER=lib /nologo

%COMPILER% *.c
%LIBER% /out:../libpng.lib *.obj
del *.obj

endlocal