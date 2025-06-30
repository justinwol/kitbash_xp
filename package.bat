@echo off
REM Package script for KITBASH release distributions
REM Usage: package.bat [version]
REM Example: package.bat 0.1.0

setlocal enabledelayedexpansion

REM Set version from command line argument or default
if "%~1"=="" (
    set VERSION=0.1.0
    echo No version specified, using default: %VERSION%
) else (
    set VERSION=%~1
    echo Using version: %VERSION%
)

set BUILD_DIR=build\src\Release

echo.
echo ========================================
echo  KITBASH Release Packaging Script
echo ========================================
echo Version: %VERSION%
echo Build directory: %BUILD_DIR%
echo.

REM Check if build directory exists
if not exist "%BUILD_DIR%" (
    echo ERROR: Build directory not found: %BUILD_DIR%
    echo Please run build.bat first to compile the project.
    echo.
    pause
    exit /b 1
)

REM Check if required files exist
if not exist "%BUILD_DIR%\kitbash.exe" (
    echo ERROR: kitbash.exe not found in %BUILD_DIR%
    echo Please ensure the project built successfully.
    echo.
    pause
    exit /b 1
)

if not exist "%BUILD_DIR%\kitbash_core.lib" (
    echo ERROR: kitbash_core.lib not found in %BUILD_DIR%
    echo Please ensure the project built successfully.
    echo.
    pause
    exit /b 1
)

REM Create dist directory
echo Creating distribution directory...
if not exist dist mkdir dist

REM Clean previous packages for this version
if exist "dist\kitbash-cli-v%VERSION%" rmdir /s /q "dist\kitbash-cli-v%VERSION%"
if exist "dist\kitbash-sdk-v%VERSION%" rmdir /s /q "dist\kitbash-sdk-v%VERSION%"
if exist "dist\kitbash-cli-v%VERSION%-windows-x64.zip" del "dist\kitbash-cli-v%VERSION%-windows-x64.zip"
if exist "dist\kitbash-sdk-v%VERSION%-windows-x64.zip" del "dist\kitbash-sdk-v%VERSION%-windows-x64.zip"

REM Package CLI
echo.
echo Packaging CLI distribution...
mkdir "dist\kitbash-cli-v%VERSION%"
copy "%BUILD_DIR%\kitbash.exe" "dist\kitbash-cli-v%VERSION%\" >nul
copy "kitbash_cli\README.md" "dist\kitbash-cli-v%VERSION%\" >nul
copy "LICENSE" "dist\kitbash-cli-v%VERSION%\" >nul

REM Package SDK  
echo Packaging SDK distribution...
mkdir "dist\kitbash-sdk-v%VERSION%"
copy "%BUILD_DIR%\kitbash_core.lib" "dist\kitbash-sdk-v%VERSION%\" >nul
copy "src\kitbash.h" "dist\kitbash-sdk-v%VERSION%\" >nul
copy "kitbash_sdk\README.md" "dist\kitbash-sdk-v%VERSION%\" >nul
copy "LICENSE" "dist\kitbash-sdk-v%VERSION%\" >nul

REM Create ZIP files using PowerShell
echo Creating ZIP archives...
cd dist
powershell -Command "Compress-Archive -Path 'kitbash-cli-v%VERSION%' -DestinationPath 'kitbash-cli-v%VERSION%-windows-x64.zip' -Force" >nul 2>&1
if errorlevel 1 (
    echo ERROR: Failed to create CLI ZIP archive
    cd ..
    pause
    exit /b 1
)

powershell -Command "Compress-Archive -Path 'kitbash-sdk-v%VERSION%' -DestinationPath 'kitbash-sdk-v%VERSION%-windows-x64.zip' -Force" >nul 2>&1
if errorlevel 1 (
    echo ERROR: Failed to create SDK ZIP archive
    cd ..
    pause
    exit /b 1
)
cd ..

REM Display results
echo.
echo ========================================
echo  PACKAGING COMPLETE
echo ========================================
echo.
echo Packages created in dist\ directory:
echo   kitbash-cli-v%VERSION%-windows-x64.zip
echo   kitbash-sdk-v%VERSION%-windows-x64.zip
echo.

REM Get file sizes
for %%f in ("dist\kitbash-cli-v%VERSION%-windows-x64.zip") do (
    set /a CLI_SIZE=%%~zf/1024
    echo CLI package size: !CLI_SIZE! KB
)

for %%f in ("dist\kitbash-sdk-v%VERSION%-windows-x64.zip") do (
    set /a SDK_SIZE=%%~zf/1024
    echo SDK package size: !SDK_SIZE! KB
)

echo.
echo Next steps for release:
echo 1. Test the packages by extracting and running them
echo 2. Create git tag: git tag -a v%VERSION% -m "Release version %VERSION%"
echo 3. Push tag: git push origin v%VERSION%
echo 4. Create GitHub release and upload the ZIP files
echo.
echo Ready for release!

endlocal