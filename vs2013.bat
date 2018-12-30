@ECHO off
CALL "%VS120COMNTOOLS%vsvars32.bat"
CD "%~dp0vs2013"
wke.sln
EXIT