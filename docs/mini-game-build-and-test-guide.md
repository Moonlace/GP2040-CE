# Mini Game Build and Test Guide

This guide covers building mini games into GP2040-CE firmware, running the
automated bot, and running Rhythm Rush on a PC. Run commands from the repository
root unless stated otherwise.

## Windows Batch Files

The repository root contains:

```text
build-mini-game-firmware.bat
build-run-mini-game-bot.bat
build-run-mini-game-pc.bat
```

Each script changes to the repository root automatically.

## Prerequisites

Firmware builds require CMake 3.31.x, Ninja, Arm GNU Toolchain 14.2.Rel1,
Raspberry Pi Pico SDK 2.2.0 with submodules, Git, Python, Node.js, and npm.
The PC simulator also requires Visual Studio 2022 or Visual Studio Build Tools
with C++ support.

The batch files expect the Pico toolchain under `%USERPROFILE%\.pico-sdk` and
fall back to CMake and Ninja from `PATH`.

## Add a Mini Game

1. Add a class derived from `MiniGame` under `headers/minigames`.
2. Implement it under `src/minigames`.
3. Implement `id`, `title`, `configure`, `reset`, `update`, and `render`.
4. Use a stable, unique numeric game ID.
5. Add the source file to the root `CMakeLists.txt`.
6. Add an instance and ID mapping to `MiniGameScreen::findGame`.

If the game needs settings:

1. Add fields to `MiniGameOptions` or a game-specific message in
   `proto/config.proto`.
2. Initialize defaults in `src/config_utils.cpp`.
3. Expose the values through `src/webconfig.cpp`.
4. Add mock values under `www/server/app.js`.
5. Add controls to `www/src/Pages/MiniGamesPage.tsx`.

Rhythm Rush is the reference implementation:

```text
headers/minigames/rhythm_game.h
src/minigames/rhythm_game.cpp
headers/display/ui/screens/MiniGameScreen.h
src/display/ui/screens/MiniGameScreen.cpp
```

Games should avoid dynamic allocation during `update` and `render`, use
fixed-size state where practical, and support 128x32 and 128x64 displays.

## Build Firmware

Run a full build when protobuf, API, or Web Configurator files change:

```bat
build-mini-game-firmware.bat full Pico
```

After a successful full build, a firmware-only rebuild can reuse the embedded
Web Configurator:

```bat
build-mini-game-firmware.bat fast Pico
```

Replace `Pico` with a directory name under `configs` for the target board. For
the Haute42 COSMOX M-Lite configuration:

```bat
build-mini-game-firmware.bat full Haute42COSMOXMLite
```

The script prints the absolute UF2 path when it succeeds. Artifacts use this
general location:

```text
<repository>\build\GP2040-CE_<version>_<board>.uf2
```

A local development build may use version `0.0.0`; that does not indicate a
failed build.

### Install the UF2

Back up the controller configuration before flashing.

1. Disconnect the controller.
2. Hold its physical `BOOTSEL` button while connecting USB.
3. Release it when Windows mounts the `RPI-RP2` drive.
4. Copy the matching UF2 from `build` to the root of `RPI-RP2`.
5. Wait for the drive to disappear and the controller to restart.

Do not disconnect the controller while the UF2 is copying. The existing Web
Configurator can also enter the bootloader through `Reboot > USB (BOOTSEL)`.

### Open Mini Games on the Controller

1. Enter Web Config mode using the controller's normal method.
2. Open the browser configurator and configure games on the `Mini Games` page.
3. Save the settings.
4. On the controller OLED instruction screen, press `R1`.
5. Use Up/Down to select a game and `B1` to play.
6. Press `S1` to return to the launcher and `B2` to return to the instruction
   screen.

Rhythm Rush lane controls default to B1, B2, B3, and B4 and can be reassigned
on the Mini Games page.

## Build and Run the PC Mini Game

The PC simulator is a separate CMake project under
`tools/minigame-simulator`. Its Windows-only code is not referenced by the
firmware build.

Build and launch with default tuning:

```bat
build-run-mini-game-pc.bat
```

Launch with BPM, difficulty, and display height:

```bat
build-run-mini-game-pc.bat 180 3 64
```

Build without launching:

```bat
build-run-mini-game-pc.bat build
```

Build and run all PC tests:

```bat
build-run-mini-game-pc.bat test
```

Interactive controls:

- `A`, `S`, `D`, `F`: lanes 1 through 4
- `1`, `2`, `3`, `4`: alternate lane controls
- `R`: restart
- `Q`: quit

## Build and Run the Bot

The bot reads the simulated OLED framebuffer and emits public game inputs. It
does not inspect private note state.

Build and run the 18-case matrix:

```bat
build-run-mini-game-bot.bat
```

Set each case duration:

```bat
build-run-mini-game-bot.bat matrix 60
```

Run one BPM, difficulty, height, and duration configuration:

```bat
build-run-mini-game-bot.bat single 180 3 64 20
```

Run all CTest cases:

```bat
build-run-mini-game-bot.bat test
```

The matrix covers 60, 120, and 240 BPM; difficulties 1 through 3; and 128x32
and 128x64 displays. A passing case reports exact scoring, full combo, and zero
misses.

## What Still Requires Hardware

The simulator and bot cover mini-game timing, scoring, input behavior, and
rendering calls. Hardware is still required to validate the physical OLED and
I2C driver, flash-backed settings, USB behavior, and navigation between the Web
Config display screens.
