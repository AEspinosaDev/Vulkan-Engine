@echo off
setlocal

:: Packages one application into dist\<target>: bin, engine resources and its assets
:: Usage: package.bat --target <app> [--assets <dir>]
::   --assets overrides the directory declared with vkfw_app_assets(), relative to the repository root

:: Resolve the repository root before shift changes %0
set "ROOT=%~dp0.."
set "TARGET="
set "ASSETS="

:parseArgs
if "%~1"=="" goto argsParsed
if "%~1"=="--target" (
    set "TARGET=%~2"
    shift
    shift
    goto parseArgs
)
if "%~1"=="--assets" (
    set "ASSETS=%~2"
    shift
    shift
    goto parseArgs
)
echo Unknown argument: %~1
goto usage

:argsParsed
if "%TARGET%"=="" goto usage

pushd "%ROOT%"

cmake --preset distribution "-DPACKAGE_TARGET=%TARGET%" "-DPACKAGE_ASSETS=%ASSETS%" || goto failed
cmake --build build/distribution --target %TARGET% || goto failed

:: Start from a clean folder so files from previous packages do not linger
if exist "dist\%TARGET%" rmdir /s /q "dist\%TARGET%"
cmake --install build/distribution --prefix "dist/%TARGET%" || goto failed

echo.
echo Package ready: %CD%\dist\%TARGET%
popd
exit /b 0

:failed
echo Packaging %TARGET% failed
popd
exit /b 1

:usage
echo Usage: package.bat --target ^<app^> [--assets ^<dir^>]
exit /b 1
