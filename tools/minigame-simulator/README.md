# Mini Game Simulator

This is a standalone host build for exercising mini-game logic on a desktop.
It is intentionally not included by the firmware's root `CMakeLists.txt`.

The simulator compiles the production
`src/minigames/rhythm_game.cpp` file unchanged. Desktop-only stand-ins for the
display, clock, gamepad state, and generated configuration types live under
this directory and take precedence only in this target's include path.

## Build on Windows

```powershell
.\tools\minigame-simulator\run.cmd
```

The script locates the CMake and Ninja copies installed with the Pico
toolchain, loads the Visual Studio C++ build environment, builds the standalone
target, and launches it.

Use `-Bpm`, `-Difficulty`, and `-Height` to override the defaults.

Controls:

- `A`, `S`, `D`, `F` or `1`, `2`, `3`, `4`: hit lanes
- `R`: restart
- `Q`: quit

Options:

```text
--bpm 60..240
--difficulty 1..3
--height 32|64
--self-test
```

Run the deterministic smoke test with:

```powershell
.\tools\minigame-simulator\run.cmd -Test
```

This simulator tests game behavior and rendering calls. Real hardware is still
required for dual-core startup, GPIO boot selection, physical OLED drivers,
flash persistence, and USB behavior.
