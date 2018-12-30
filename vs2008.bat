@ECHO off
CALL "%VS90COMNTOOLS%vsvars32.bat"
CD "%~dp0vs2008"
wke.sln
EXIT