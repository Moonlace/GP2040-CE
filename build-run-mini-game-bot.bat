@echo off
setlocal
pushd "%~dp0"

set "MODE=%~1"
if "%MODE%"=="" set "MODE=matrix"

if /I "%MODE%"=="matrix" goto matrix
if /I "%MODE%"=="single" goto single
if /I "%MODE%"=="test" goto test
goto usage

:matrix
set "DURATION=%~2"
if "%DURATION%"=="" set "DURATION=20"
call tools\minigame-simulator\run.cmd -BotMatrix -DurationSeconds %DURATION%
goto complete

:single
set "BPM=%~2"
set "DIFFICULTY=%~3"
set "HEIGHT=%~4"
set "DURATION=%~5"
if "%BPM%"=="" set "BPM=120"
if "%DIFFICULTY%"=="" set "DIFFICULTY=2"
if "%HEIGHT%"=="" set "HEIGHT=64"
if "%DURATION%"=="" set "DURATION=20"
call tools\minigame-simulator\run.cmd -Bot ^
	-Bpm %BPM% ^
	-Difficulty %DIFFICULTY% ^
	-Height %HEIGHT% ^
	-DurationSeconds %DURATION%
goto complete

:test
call tools\minigame-simulator\run.cmd -Test
goto complete

:usage
echo Usage:
echo   %~nx0 matrix [duration-seconds]
echo   %~nx0 single [bpm] [difficulty] [height] [duration-seconds]
echo   %~nx0 test
popd
exit /b 2

:complete
set "RESULT=%ERRORLEVEL%"
if not "%RESULT%"=="0" echo Mini-game bot failed with exit code %RESULT%.
popd
exit /b %RESULT%
