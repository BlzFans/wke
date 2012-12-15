@echo off

setlocal

if exist "%VS80COMNTOOLS%vsvars32.bat" (
    call "%VS80COMNTOOLS%vsvars32.bat" 
) else (
    call "%VS90COMNTOOLS%vsvars32.bat"
)

rem set COMPILER=cl /nologo /MT /Od /Zi /W3 /c /TC /DPIXMAN_NO_TLS /D_CRT_SECURE_NO_DEPRECATE /DCAIRO_WIN32_STATIC_BUILD /DDISABLE_SOME_FLOATING_POINT -DPACKAGE=pixman-1 -DPACKAGE_VERSION="" -DPACKAGE_BUGREPORT="" /I src /I pixman /I ../libpng /I ../zlib /wd 4146 /wd 4244 /wd 4996 /wd 4819 /wd 4700
set COMPILER=cl /nologo /MT /O2 /W3 /c /TC /DPIXMAN_NO_TLS /D_CRT_SECURE_NO_DEPRECATE /DCAIRO_WIN32_STATIC_BUILD /DDISABLE_SOME_FLOATING_POINT -DPACKAGE=pixman-1 -DPACKAGE_VERSION="" -DPACKAGE_BUGREPORT="" /I src /I pixman /I ../libpng /I ../zlib /wd 4146 /wd 4244 /wd 4996 /wd 4819 /wd 4700
set LIBER=lib /nologo

%COMPILER% src/*.c
%COMPILER% src/win32/*.c
%COMPILER% pixman/pixman-access-accessors.c
%COMPILER% pixman/pixman-access.c
%COMPILER% pixman/pixman-bits-image.c
%COMPILER% pixman/pixman-combine32.c
%COMPILER% pixman/pixman-combine64.c
%COMPILER% pixman/pixman-conical-gradient.c
%COMPILER% pixman/pixman-cpu.c
%COMPILER% pixman/pixman-noop.c
%COMPILER% pixman/pixman-edge-accessors.c
%COMPILER% pixman/pixman-edge.c
%COMPILER% pixman/pixman-fast-path.c
%COMPILER% pixman/pixman-general.c
%COMPILER% pixman/pixman-gradient-walker.c
%COMPILER% pixman/pixman-image.c
%COMPILER% pixman/pixman-implementation.c
%COMPILER% pixman/pixman-linear-gradient.c
%COMPILER% pixman/pixman-matrix.c
%COMPILER% pixman/pixman-mmx.c
%COMPILER% pixman/pixman-radial-gradient.c
%COMPILER% pixman/pixman-region16.c
%COMPILER% pixman/pixman-region32.c
%COMPILER% pixman/pixman-solid-fill.c
%COMPILER% pixman/pixman-timer.c
%COMPILER% pixman/pixman-trap.c
%COMPILER% pixman/pixman-utils.c
%COMPILER% pixman/pixman.c
%LIBER% /out:../cairo.lib *.obj
del *.obj

endlocal