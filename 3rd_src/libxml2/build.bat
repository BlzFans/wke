@echo off

setlocal

if exist "%VS80COMNTOOLS%vsvars32.bat" (
    call "%VS80COMNTOOLS%vsvars32.bat" 
) else (
    call "%VS90COMNTOOLS%vsvars32.bat"
)

set COMPILER=cl /nologo /MT /O2 /W3 /c /D_CRT_SECURE_NO_DEPRECATE /DWIN32 /DLIBXML_XPATH_ENABLED /DLIBXML_OUTPUT_ENABLED /DLIBXML_PUSH_ENABLED /DLIBXML_TREE_ENABLED /DLIBXML_HTML_ENABLED /DNOLIBTOOL /DLIBXML_STATIC /I ./ /wd 4819 /wd 4996
set LIBER=lib /nologo

%COMPILER% *.c
%LIBER% /out:../libxml2.lib *.obj
del *.obj

endlocal