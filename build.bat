@echo off
echo Compiling the project...

mkdir build
cd build

cmake ..
cmake --build .

echo.
echo Press any key to close this window...
pause>nul

exit
