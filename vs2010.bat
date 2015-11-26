@ECHO off
CALL "%VS100COMNTOOLS%vsvars32.bat"
CD "%~dp0vs2010"
wke.sln
EXIT