
@echo off

setlocal

if exist "%VS80COMNTOOLS%vsvars32.bat" (
    call "%VS80COMNTOOLS%vsvars32.bat" 
) else (
    call "%VS90COMNTOOLS%vsvars32.bat"
)

rem set COMPILER=cl /nologo /MT /Od /Zi /W3 /c /D_DEBUG /D_CRT_SECURE_NO_DEPRECATE /DWIN32 /DBUILDING_LIBCURL /DCURL_STATICLIB /DCURL_DISABLE_LDAP /DHAVE_LIBZ /DHAVE_ZLIB_H /DUSE_SSLEAY /DUSE_OPENSSL /I. /I include /I ../zlib /I ../
set COMPILER=cl /nologo /MT /O2 /W3 /c /DNDEBUG /D_CRT_SECURE_NO_DEPRECATE /DWIN32 /DBUILDING_LIBCURL /DCURL_STATICLIB /DCURL_DISABLE_LDAP /DHAVE_LIBZ /DHAVE_ZLIB_H /DUSE_SSLEAY /DUSE_OPENSSL /I. /I include /I ../zlib /I ../
set LIBER=lib /nologo

%COMPILER% *.c
%LIBER% /out:../libcurl_imp.lib *.obj
del *.obj

endlocal