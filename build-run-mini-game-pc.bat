@echo off
setlocal
pushd "%~dp0"

if /I "%~1"=="build" goto build_only
if /I "%~1"=="test" goto test
if /I "%~1"=="help" goto usage
if /I "%~1"=="--help" goto usage

set "BPM=%~1"
set "DIFFICULTY=%~2"
set "HEIGHT=%~3"
if "%BPM%"=="" set "BPM=120"
if "%DIFFICULTY%"=="" set "DIFFICULTY=2"
if "%HEIGHT%"=="" set "HEIGHT=64"

call tools\minigame-simulator\run.cmd ^
	-Bpm %BPM% ^
	-Difficulty %DIFFICULTY% ^
	-Height %HEIGHT%
goto complete

:build_only
call tools\minigame-simulator\run.cmd -BuildOnly
goto complete

:test
call tools\minigame-simulator\run.cmd -Test
goto complete

:usage
echo Usage:
echo   %~nx0 [bpm] [difficulty] [height]
echo   %~nx0 build
echo   %~nx0 test
echo.
echo Examples:
echo   %~nx0
echo   %~nx0 180 3 64
echo   %~nx0 build
popd
exit /b 0

:complete
set "RESULT=%ERRORLEVEL%"
if not "%RESULT%"=="0" echo PC mini-game command failed with exit code %RESULT%.
popd
exit /b %RESULT%
