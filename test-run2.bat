@echo off
set PATH=C:\msys64\ucrt64\bin;C:\Windows\System32;%PATH%
cd /d C:\code\Backgammon\build
Backgammon.exe 2> err.log
