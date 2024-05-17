@echo off
setlocal

:: Ask for the rendering type
:renderingType
echo Please enter the rendering type (forward/deferred):
set /p renderType=
if "%renderType%"=="forward" (
    goto renderTypeSet
) else if "%renderType%"=="deferred" (
    goto renderTypeSet
) else (
    echo Invalid rendering type. Please enter either "forward" or "deferred".
    goto renderingType
)

:renderTypeSet
echo Rendering type set to %renderType%.

:: Ask for the antialiasing type
:antialiasingType
echo Please enter the antialiasing type (none/msaa4/msaa8/fxaa):
set /p aaType=
if "%aaType%"=="none" (
    goto aaTypeSet
) else if "%aaType%"=="msaa4" (
    goto aaTypeSet
) else if "%aaType%"=="msaa8" (
    goto aaTypeSet
) else if "%aaType%"=="fxaa" (
    goto aaTypeSet
) else (
    echo Invalid antialiasing type. Please enter either "none", "msaa4", "msaa8", or "fxaa".
    goto antialiasingType
)

:aaTypeSet
echo Antialiasing type set to %aaType%.

:: Ask for the gui
:enableGui
echo Enable GUI (true/false):
set /p enableGui=
if "%enableGui%"=="true" (
    goto enableGuiSet
) else if "%enableGui%"=="false" (
    goto enableGuiSet
) else (
    echo Invalid argument. Please enter either "true" or "false".
    goto enableGui
)

:enableGuiSet
echo GUI type set to %enableGui%.

:: Add a delay of 5 seconds (5000 milliseconds)
echo Launching the application...
timeout /t 2 /nobreak > nul

:: Call the executable with the collected arguments
.\build\examples\Release\RendererApp.exe -type %renderType% -aa %aaType% -gui %enableGui%

endlocal