@echo off
setlocal EnableExtensions

set "BUILD_DIR=build"
set "CONFIG=Release"
set "BUILD_IF_MISSING=1"
set "QT_PREFIX="
set "GENERATOR="

if /I "%~1"=="/?" goto :usage
if /I "%~1"=="-h" goto :usage
if /I "%~1"=="--help" goto :usage

:parse_args
if "%~1"=="" goto :after_parse
if /I "%~1"=="-buildDir" (
  set "BUILD_DIR=%~2"
  shift
  shift
  goto :parse_args
)
if /I "%~1"=="-config" (
  set "CONFIG=%~2"
  shift
  shift
  goto :parse_args
)
if /I "%~1"=="-buildIfMissing" (
  set "BUILD_IF_MISSING=1"
  shift
  goto :parse_args
)
if /I "%~1"=="-qtPrefixPath" (
  set "QT_PREFIX=%~2"
  shift
  shift
  goto :parse_args
)
if /I "%~1"=="-generator" (
  set "GENERATOR=%~2"
  shift
  shift
  goto :parse_args
)
echo Unknown argument: %~1
goto :usage

:after_parse
set "SCRIPT_DIR=%~dp0"
if "%SCRIPT_DIR:~-1%"=="\" set "SCRIPT_DIR=%SCRIPT_DIR:~0,-1%"
set "BUILD_PATH=%SCRIPT_DIR%\%BUILD_DIR%"
set "LOCAL_UCRT_BIN=C:\msys64\ucrt64\bin"
set "EXE=%BUILD_PATH%\Backgammon.exe"
if not exist "%EXE%" set "EXE=%BUILD_PATH%\%CONFIG%\Backgammon.exe"

if exist "%EXE%" goto :run
if not "%BUILD_IF_MISSING%"=="1" (
  echo [ERROR] Executable not found. Run build.bat first.
  exit /b 1
)

if not exist "%SCRIPT_DIR%\build.bat" (
  echo [ERROR] build.bat not found.
  exit /b 1
)

echo Executable not found. Starting build...
if not "%QT_PREFIX%"=="" (
  if not "%GENERATOR%"=="" (
    call "%SCRIPT_DIR%\build.bat" -buildDir "%BUILD_DIR%" -config %CONFIG% -qtPrefixPath "%QT_PREFIX%" -generator "%GENERATOR%"
  ) else (
    call "%SCRIPT_DIR%\build.bat" -buildDir "%BUILD_DIR%" -config %CONFIG% -qtPrefixPath "%QT_PREFIX%"
  )
) else (
  if not "%GENERATOR%"=="" (
    call "%SCRIPT_DIR%\build.bat" -buildDir "%BUILD_DIR%" -config %CONFIG% -generator "%GENERATOR%"
  ) else (
    call "%SCRIPT_DIR%\build.bat" -buildDir "%BUILD_DIR%" -config %CONFIG%
  )
)
if errorlevel 1 exit /b 1

set "EXE=%BUILD_PATH%\Backgammon.exe"
if not exist "%EXE%" set "EXE=%BUILD_PATH%\%CONFIG%\Backgammon.exe"
if not exist "%EXE%" (
  echo [ERROR] Executable still not found after build.
  exit /b 1
)

:run
if exist "%LOCAL_UCRT_BIN%\Qt5Core.dll" set "PATH=%LOCAL_UCRT_BIN%;%PATH%"
echo Launching: %EXE%
start "" "%EXE%"
exit /b 0

:usage
echo Run script
echo.
echo Usage:
echo   run.bat [-buildDir build] [-config Release^|Debug] [-buildIfMissing] [-qtPrefixPath QtPath] [-generator Name]
echo.
echo Examples:
echo   run.bat
echo   run.bat -config Debug
echo   run.bat -buildIfMissing -qtPrefixPath C:\Qt\5.15.2\msvc2019_64
exit /b 0
