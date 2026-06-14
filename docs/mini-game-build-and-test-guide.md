# Mini Game Build and Test Guide

This guide covers three workflows:

1. Adding mini-game code and building it into GP2040-CE firmware.
2. Building and running the automated mini-game bot.
3. Building and playing a mini game on a PC.

Run all commands from the repository root unless a section says otherwise.
The Windows examples assume the toolchain is installed under
`%USERPROFILE%\.pico-sdk`.

## Windows Batch Files

Three batch files at the repository root provide the shortest path for the
main workflows:

```text
build-mini-game-firmware.bat
build-run-mini-game-bot.bat
build-run-mini-game-pc.bat
```

They can be launched from Command Prompt or PowerShell. Each script changes to
the repository root automatically, so it also works when launched by
double-clicking or from another directory.

## Prerequisites

Firmware builds require:

- CMake 3.31.x
- Ninja
- Arm GNU Toolchain 14.2.Rel1
- Raspberry Pi Pico SDK 2.2.0 with submodules
- Git
- Python
- Node.js and npm for rebuilding the embedded Web Configurator

The PC simulator additionally requires Visual Studio 2022 or Visual Studio
Build Tools with the C++ tools installed.

Verify the main tools:

```powershell
cmake --version
ninja --version
arm-none-eabi-g++ --version
node --version
npm --version
```

## Add a Mini Game

Mini games are compiled into the firmware. The Web Configurator enables and
configures games that are already present in the firmware; it does not upload
executable game code to the controller.

### Implement the game

1. Add a header under `headers/minigames`.
2. Derive the game class from `MiniGame`.
3. Implement `id`, `title`, `configure`, `reset`, `update`, and `render`.
4. Add the implementation under `src/minigames`.
5. Use a stable and unique numeric game ID.

The interface is defined in:

```text
headers/minigames/minigame.h
```

Games should avoid dynamic allocation during `update` and `render`, use
fixed-size state where practical, and support both 128x32 and 128x64 displays.

### Register the game

Update `MiniGameAddon` so the launcher can find the game:

1. Include the game header in `headers/addons/mini_game.h`.
2. Add a game instance as a `MiniGameAddon` member.
3. Map the stable game ID in `MiniGameAddon::findGame`.
4. Add the game source file to the firmware source list in the root
   `CMakeLists.txt`.

If the game needs settings:

1. Add fields to `MiniGameOptions` or an appropriate game-specific protobuf
   message in `proto/config.proto`.
2. Add defaults and migrations in the storage/configuration code.
3. Expose the settings through `src/webconfig.cpp`.
4. Add mock API data under `www/server/app.js`.
5. Add controls to `www/src/Pages/MiniGamesPage.tsx`.

The existing Rhythm Rush implementation is a complete reference:

```text
headers/minigames/rhythm_game.h
src/minigames/rhythm_game.cpp
```

## Build Firmware

### Batch file

Build Pico firmware, including all registered mini-game add-ons and games:

```bat
build-mini-game-firmware.bat
```

The default is a full `Pico` build. A full build updates submodules, rebuilds
the Web Configurator, embeds it, and compiles the firmware:

```bat
build-mini-game-firmware.bat full Pico
```

After at least one successful full build, use fast mode to reuse the existing
embedded Web Configurator:

```bat
build-mini-game-firmware.bat fast Pico
```

The second argument can be another board directory name under `configs`.
Firmware artifacts are written under `build`.

### Find the built firmware

When the build succeeds, the final lines printed by the batch file include the
absolute path beginning with `Firmware:`. The general output location is:

```text
<repository>\build\GP2040-CE_<version>_<board>.uf2
```

For this checkout's default Pico build, the current file is:

```text
E:\GP2040\build\GP2040-CE_0.0.0_Pico.uf2
```

The `0.0.0` version is normal for a local development build that is not based on a
release tag. It does not mean the build failed.

Build with the configuration for the actual controller board whenever one exists:

```bat
build-mini-game-firmware.bat full <BoardConfig>
```

The board configuration names are the directory names under `configs`. A generic
`Pico` UF2 may use the wrong pins or peripherals for a custom controller board.

### Install the UF2 on the controller

Back up the controller configuration from the Web Configurator before flashing,
especially when changing board configurations or testing development firmware.

1. Disconnect the controller from USB.
2. Hold the physical `BOOTSEL` button on the RP2040 controller board.
3. Connect the controller to the PC while continuing to hold `BOOTSEL`.
4. Release the button when Windows mounts a drive named `RPI-RP2`.
5. Copy the matching `.uf2` file from the `build` directory to the root of `RPI-RP2`.
6. Wait for the copy to finish. The drive disappears and the controller restarts automatically.

Do not disconnect the controller while Windows is copying the UF2.

If the existing GP2040-CE Web Configurator is available, its reboot menu provides
another route into the bootloader:

1. Open the Web Configurator for the connected controller.
2. Open the reboot menu and select `USB (BOOTSEL)`.
3. Wait for the `RPI-RP2` drive to appear.
4. Copy the matching UF2 to that drive and wait for the controller to restart.

After the new firmware boots, open the Web Configurator's `Mini Games` page, enable
the add-on and desired games, configure the four Rhythm Rush lane buttons, select
the boot-entry button, save, and reboot. Hold that configured boot button while
powering on or rebooting to let the mini-game add-on take ownership of the display.

If a configured GPIO boot combination does not match, holding `S2` while connecting
USB remains a fallback for entering Web Configurator mode. From normal controller
mode, release all buttons briefly and then hold `S2 + B3 + B4` for four seconds to
reboot into Web Configurator mode.

On the Haute42 COSMOX Lite, `S2` is Start. `B3` and `B4` are the first two
buttons on the upper action-button row: `X` then `Y` in XInput labels, or `Y`
then `X` in Nintendo Switch labels.

### Initialize dependencies

```powershell
git submodule update --init --recursive

$env:PICO_SDK_PATH="$env:USERPROFILE\.pico-sdk\sdk\2.2.0"
$env:PICO_TOOLCHAIN_PATH="$env:USERPROFILE\.pico-sdk\toolchain"
$env:GP2040_BOARDCONFIG="Pico"
```

Change `Pico` to another directory name under `configs` when building for a
different board.

### Full build

The full build regenerates the Web Configurator and embeds it in the firmware.
Use this after changing game settings, protobuf definitions, API endpoints, or
Web Configurator files.

```powershell
$env:SKIP_SUBMODULES="TRUE"
Remove-Item Env:SKIP_WEBBUILD -ErrorAction SilentlyContinue

cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

For the `Pico` board, the resulting firmware is:

```text
build/GP2040-CE_<version>_Pico.uf2
```

### Faster firmware-only rebuild

Use this only after a successful full web build has generated
`lib/httpd/fsdata.c`:

```powershell
$env:SKIP_SUBMODULES="TRUE"
$env:SKIP_WEBBUILD="TRUE"

cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

Unset the variable before the next full build:

```powershell
Remove-Item Env:SKIP_WEBBUILD -ErrorAction SilentlyContinue
```

## Build and Run the PC Mini Game

The PC simulator is a separate CMake project under
`tools/minigame-simulator`. It is not included by the root firmware
`CMakeLists.txt`, so its Windows-only code and headers cannot enter the Pico
build.

The simulator compiles the production `src/minigames/rhythm_game.cpp` file
unchanged against host-only display, input, clock, and configuration adapters.

### Batch file

Build and launch with default tuning:

```bat
build-run-mini-game-pc.bat
```

Tune the rhythm game with positional arguments for BPM, difficulty, and display
height:

```bat
build-run-mini-game-pc.bat 180 3 64
```

Build without launching the interactive game:

```bat
build-run-mini-game-pc.bat build
```

Build and run all PC tests:

```bat
build-run-mini-game-pc.bat test
```

Build and launch:

```powershell
.\tools\minigame-simulator\run.cmd
```

Launch with custom settings:

```powershell
.\tools\minigame-simulator\run.cmd -Bpm 180 -Difficulty 3 -Height 64
```

Controls:

- `A`, `S`, `D`, `F`: lanes 1 through 4
- `1`, `2`, `3`, `4`: alternate lane controls
- `R`: restart the game
- `Q`: quit

### Build manually

The wrapper is recommended because it discovers Visual Studio, CMake, and
Ninja automatically. The equivalent manual commands from a Visual Studio
Developer PowerShell are:

```powershell
cmake -S tools/minigame-simulator `
  -B tools/minigame-simulator/build `
  -G Ninja `
  -DCMAKE_BUILD_TYPE=Release

cmake --build tools/minigame-simulator/build --parallel

.\tools\minigame-simulator\build\gp2040-minigame-sim.exe
```

## Build and Run the Bot

The autoplay bot tests through the simulated OLED framebuffer and public game
inputs. It detects notes reaching the rendered hit line and emits B1 through
B4. It does not read the game's private note array.

### Batch file

Build the bot and run the complete matrix:

```bat
build-run-mini-game-bot.bat
```

The following is equivalent and sets each matrix case to 60 simulated seconds:

```bat
build-run-mini-game-bot.bat matrix 60
```

Run one tuned bot configuration. Arguments are BPM, difficulty, display
height, and duration in seconds:

```bat
build-run-mini-game-bot.bat single 180 3 64 20
```

Build the simulator and run all CTest cases:

```bat
build-run-mini-game-bot.bat test
```

### Run one configuration

```powershell
.\tools\minigame-simulator\run.cmd `
  -Bot `
  -Bpm 180 `
  -Difficulty 3 `
  -Height 64 `
  -DurationSeconds 20
```

A passing result reports:

- the expected number of notes hit
- the exact perfect-play score
- a combo equal to the number of hits
- zero misses

### Run the complete matrix

```powershell
.\tools\minigame-simulator\run.cmd -BotMatrix
```

The matrix runs 18 combinations:

- 60, 120, and 240 BPM
- difficulty levels 1, 2, and 3
- 128x32 and 128x64 displays

The command returns a nonzero exit code if any case fails.

Change the simulated duration for every matrix case:

```powershell
.\tools\minigame-simulator\run.cmd -BotMatrix -DurationSeconds 60
```

### Run all PC tests

```powershell
.\tools\minigame-simulator\run.cmd -Test
```

This builds the simulator and runs CTest. The current suite contains:

1. A deterministic rhythm-game smoke test.
2. The 18-case visual autoplay matrix.

## What Still Requires Hardware

The PC simulator and bot cover mini-game timing, scoring, input behavior, and
rendering calls. A real RP2040 board is still required to validate:

- boot-button GPIO detection
- dual-core startup and add-on ownership
- the physical OLED and I2C driver
- flash-backed settings persistence
- USB behavior
- entering and leaving mini-game mode on the controller
