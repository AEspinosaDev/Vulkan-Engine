@echo off
echo Compiling the project...

cd ..
mkdir build
cd build

cmake ..
cmake --build . --config Debug


echo.
echo Press any key to close this window...
pause>nul

exit
