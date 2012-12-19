@echo off

setlocal

if exist "%VS80COMNTOOLS%vsvars32.bat" (
    call "%VS80COMNTOOLS%vsvars32.bat" 
) else (
    call "%VS90COMNTOOLS%vsvars32.bat"
)

set COMPILER=cl /nologo /MT /O2 /W3 /c /D_CRT_SECURE_NO_DEPRECATE /DWIN32 /D_REENTRANT /DNDEBUG /D_WINDOWS /D_MBCS /DLIBXML_STATIC /DLIBXML_XPATH_ENABLED /DLIBXML_TREE_ENABLED /DLIBXML_OUTPUT_ENABLED /DLIBXML_DOCB_ENABLED /DLIBXML_HTML_ENABLED /DLIBXSLT_STATIC /I ./ /I ../ /I ../libxml2 /wd 4819
set LIBER=lib /nologo

%COMPILER% *.c
%LIBER% /out:../libxslt.lib *.obj
del *.obj

endlocal