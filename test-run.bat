@echo off
set PATH=C:\msys64\ucrt64\bin;%PATH%
cd /d C:\code\Backgammon\build
echo Starting... > ..\debug.log
Backgammon.exe >> ..\debug.log 2>&1
echo Done >> ..\debug.log
