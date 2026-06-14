@echo off
setlocal
pushd "%~dp0"

set "BUILD_MODE=%~1"
set "BOARD=%~2"

if "%BUILD_MODE%"=="" set "BUILD_MODE=full"
if "%BOARD%"=="" set "BOARD=Pico"

if /I not "%BUILD_MODE%"=="full" if /I not "%BUILD_MODE%"=="fast" goto usage

set "CMAKE_EXE=%USERPROFILE%\.pico-sdk\tools\cmake\cmake-3.31.8-windows-x86_64\bin\cmake.exe"
set "NINJA_EXE=%USERPROFILE%\.pico-sdk\tools\ninja\1.13.2\ninja.exe"
set "ARM_BIN=%USERPROFILE%\.pico-sdk\toolchain\bin"
set "PICO_SDK_PATH=%USERPROFILE%\.pico-sdk\sdk\2.2.0"
set "PICO_TOOLCHAIN_PATH=%USERPROFILE%\.pico-sdk\toolchain"
set "GP2040_BOARDCONFIG=%BOARD%"
set "SKIP_SUBMODULES=TRUE"

if not exist "%CMAKE_EXE%" (
	where cmake.exe >nul 2>nul || goto missing_cmake
	set "CMAKE_EXE=cmake.exe"
)
if not exist "%NINJA_EXE%" (
	where ninja.exe >nul 2>nul || goto missing_ninja
	set "NINJA_EXE=ninja.exe"
)
if not exist "%PICO_SDK_PATH%\pico_sdk_init.cmake" goto missing_sdk
if not exist "%ARM_BIN%\arm-none-eabi-g++.exe" goto missing_arm

set "PATH=%ARM_BIN%;%PATH%"

if /I "%BUILD_MODE%"=="full" (
	echo Updating repository submodules...
	git submodule update --init --recursive
	if errorlevel 1 goto failed
	set "SKIP_WEBBUILD="
) else (
	if not exist "lib\httpd\fsdata.c" goto missing_web
	set "SKIP_WEBBUILD=TRUE"
)

echo.
echo Building GP2040-CE mini-game firmware
echo   Board: %BOARD%
echo   Mode:  %BUILD_MODE%
echo.

"%CMAKE_EXE%" -S . -B build -G Ninja ^
	-DCMAKE_BUILD_TYPE=Release ^
	-DCMAKE_MAKE_PROGRAM="%NINJA_EXE%"
if errorlevel 1 goto failed

"%CMAKE_EXE%" --build build --parallel
if errorlevel 1 goto failed

echo.
echo Firmware build completed.
set "UF2_FILE="
for %%F in ("build\GP2040-CE_*_%BOARD%.uf2") do if exist "%%~fF" set "UF2_FILE=%%~fF"
if not defined UF2_FILE goto missing_output

echo Firmware: %UF2_FILE%
echo.
echo Install on the controller:
echo   1. Disconnect the controller from USB.
echo   2. Hold the controller board's BOOTSEL button.
echo   3. Connect USB, then release BOOTSEL when the RPI-RP2 drive appears.
echo   4. Copy the UF2 file shown above to the RPI-RP2 drive.
echo   5. Wait for the drive to disappear and the controller to restart.
echo.
echo Web Configurator alternative: choose Reboot ^> USB (BOOTSEL), then copy the UF2.
popd
exit /b 0

:usage
echo Usage: %~nx0 [full^|fast] [board]
echo.
echo Examples:
echo   %~nx0
echo   %~nx0 full Pico
echo   %~nx0 fast Pico
popd
exit /b 2

:missing_cmake
echo CMake was not found. Install the GP2040-CE toolchain first.
goto failed

:missing_ninja
echo Ninja was not found. Install the GP2040-CE toolchain first.
goto failed

:missing_sdk
echo Pico SDK 2.2.0 was not found at "%PICO_SDK_PATH%".
goto failed

:missing_arm
echo Arm GNU Toolchain was not found at "%ARM_BIN%".
goto failed

:missing_web
echo Fast mode requires lib\httpd\fsdata.c from a previous full build.
echo Run "%~nx0 full %BOARD%" first.
goto failed

:missing_output
echo Build completed, but no UF2 matching board "%BOARD%" was found under "%CD%\build".
goto failed

:failed
set "RESULT=%ERRORLEVEL%"
if "%RESULT%"=="0" set "RESULT=1"
echo.
echo Firmware build failed with exit code %RESULT%.
popd
exit /b %RESULT%
