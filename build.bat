@echo off
REM Build script for Kitbash C++ implementation on Windows
REM Usage: build.bat [clean|release|debug]

setlocal enabledelayedexpansion

set BUILD_TYPE=Release
set CLEAN_BUILD=false

REM Parse command line arguments
:parse_args
if "%~1"=="" goto end_parse
if /i "%~1"=="clean" (
    set CLEAN_BUILD=true
    shift
    goto parse_args
)
if /i "%~1"=="debug" (
    set BUILD_TYPE=Debug
    shift
    goto parse_args
)
if /i "%~1"=="release" (
    set BUILD_TYPE=Release
    shift
    goto parse_args
)
echo Unknown option: %~1
echo Usage: %0 [clean^|release^|debug]
exit /b 1

:end_parse

echo Building Kitbash C++ implementation...
echo Build type: %BUILD_TYPE%

REM Clean build directory if requested
if "%CLEAN_BUILD%"=="true" (
    echo Cleaning build directory...
    if exist build rmdir /s /q build
)

REM Create build directory
if not exist build mkdir build
cd build

REM Configure with CMake
echo Configuring with CMake...
cmake -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ..
if errorlevel 1 (
    echo CMake configuration failed!
    exit /b 1
)

REM Build
echo Building...
cmake --build . --config %BUILD_TYPE%
if errorlevel 1 (
    echo Build failed!
    exit /b 1
)

echo.
echo Build complete!
if "%BUILD_TYPE%"=="Debug" (
    echo Executable: build\src\Debug\kitbash.exe
) else (
    echo Executable: build\src\Release\kitbash.exe
)
echo.
echo Usage examples:
echo   kitbash.exe --help
echo   kitbash.exe base.obj addition.obj
echo   kitbash.exe -s -o merged.obj base.obj addition.obj

endlocal