@echo off
setlocal EnableExtensions

set "BUILD_DIR=build"
set "CONFIG=Release"
set "QT_PREFIX="
set "GENERATOR="
set "CLEAN=0"
set "CMAKE_EXE="

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
if /I "%~1"=="-clean" (
  set "CLEAN=1"
  shift
  goto :parse_args
)
echo Unknown argument: %~1
goto :usage

:after_parse
set "SCRIPT_DIR=%~dp0"
if "%SCRIPT_DIR:~-1%"=="\" set "SCRIPT_DIR=%SCRIPT_DIR:~0,-1%"
set "BUILD_PATH=%SCRIPT_DIR%\%BUILD_DIR%"
set "LOCAL_UCRT_BIN=%SCRIPT_DIR%\tools\msys64\msys64\ucrt64\bin"
set "LOCAL_QT_PREFIX=%SCRIPT_DIR%\tools\msys64\msys64\ucrt64"

if exist "%LOCAL_UCRT_BIN%\cmake.exe" (
  set "CMAKE_EXE=%LOCAL_UCRT_BIN%\cmake.exe"
  set "PATH=%LOCAL_UCRT_BIN%;%PATH%"
) else (
  where cmake >nul 2>nul
  if errorlevel 1 (
    echo [ERROR] cmake not found in PATH.
    exit /b 1
  )
  set "CMAKE_EXE=cmake"
)

if "%QT_PREFIX%"=="" (
  if exist "%LOCAL_QT_PREFIX%\lib\cmake\Qt5" (
    set "QT_PREFIX=%LOCAL_QT_PREFIX%"
  )
)

if not "%QT_PREFIX%"=="" (
  if not exist "%QT_PREFIX%" (
    echo [ERROR] Qt path not found: %QT_PREFIX%
    exit /b 1
  )
  if defined CMAKE_PREFIX_PATH (
    set "CMAKE_PREFIX_PATH=%QT_PREFIX%;%CMAKE_PREFIX_PATH%"
  ) else (
    set "CMAKE_PREFIX_PATH=%QT_PREFIX%"
  )
)

if /I not "%CONFIG%"=="Debug" if /I not "%CONFIG%"=="Release" (
  echo [ERROR] -config must be Debug or Release.
  exit /b 1
)

if "%CLEAN%"=="1" (
  if exist "%BUILD_PATH%" rmdir /s /q "%BUILD_PATH%"
)

if not defined GENERATOR (
  where ninja >nul 2>nul
  if errorlevel 1 (
    set "GENERATOR=Visual Studio 17 2022"
  ) else (
    set "GENERATOR=Ninja"
  )
)

echo Running CMake configure...
if /I "%GENERATOR:~0,14%"=="Visual Studio" (
  "%CMAKE_EXE%" -S "%SCRIPT_DIR%" -B "%BUILD_PATH%" -G "%GENERATOR%" -A x64
) else (
  "%CMAKE_EXE%" -S "%SCRIPT_DIR%" -B "%BUILD_PATH%" -G "%GENERATOR%" -DCMAKE_CXX_COMPILER=g++
)
if errorlevel 1 (
  echo [ERROR] CMake configure failed.
  exit /b 1
)

echo Running build (%CONFIG%)...
"%CMAKE_EXE%" --build "%BUILD_PATH%" --config %CONFIG%
if errorlevel 1 (
  echo [ERROR] Build failed.
  exit /b 1
)

if exist "%BUILD_PATH%\Backgammon.exe" (
  echo Build succeeded. Executable: %BUILD_PATH%\Backgammon.exe
  exit /b 0
)
if exist "%BUILD_PATH%\%CONFIG%\Backgammon.exe" (
  echo Build succeeded. Executable: %BUILD_PATH%\%CONFIG%\Backgammon.exe
  exit /b 0
)

echo Build finished, but executable was not found in common paths.
exit /b 0

:usage
echo Build script
echo.
echo Usage:
echo   build.bat [-buildDir build] [-config Release^|Debug] [-qtPrefixPath QtPath] [-generator Name] [-clean]
echo.
echo Examples:
echo   build.bat
echo   build.bat -config Debug
echo   build.bat -qtPrefixPath C:\Qt\5.15.2\msvc2019_64
echo   build.bat -generator "Visual Studio 17 2022"
exit /b 0
