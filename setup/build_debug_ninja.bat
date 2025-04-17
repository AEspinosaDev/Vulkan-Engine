@echo off

cd ..
mkdir build

REM 
set BUILD_DIR=build

REM 
where ninja >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo ✅ Ninja found. Generating build files with Ninja...
    cmake -G Ninja -S . -B %BUILD_DIR%
) else (
    echo ⚠️ Ninja not found. Falling back to Visual Studio 2022...
    cmake -G "Visual Studio 17 2022" -A x64 -S . -B %BUILD_DIR%
)

cmake --build build --config Debug

pause>nul
exit
