@setlocal

@set COMPILER=cl /nologo /MT /O2 /W3 /c /DNDEBUG /D_CRT_SECURE_NO_DEPRECATE
@set LIBER=lib /nologo

@%COMPILER% *.c
@%LIBER% /out:../zlib.lib *.obj
@del *.obj

@endlocal