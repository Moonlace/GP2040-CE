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
--bot
--bot-matrix
--bot-duration-ms 2000..
```

Run the deterministic smoke test with:

```powershell
.\tools\minigame-simulator\run.cmd -Test
```

## Autoplay bot

The bot tests Rhythm Rush through the same public input and rendered display
surface used by the simulator. It does not inspect the game's note array. It
watches the framebuffer for notes reaching each lane's hit line, presses B1
through B4, and verifies the exact perfect-play score, combo, and miss count.

Run one configuration:

```powershell
.\tools\minigame-simulator\run.cmd -Bot -Bpm 180 -Difficulty 3 -Height 64
```

Run the full 18-case matrix across 60, 120, and 240 BPM, all three difficulty
levels, and both display heights:

```powershell
.\tools\minigame-simulator\run.cmd -BotMatrix
```

This simulator tests game behavior and rendering calls. Real hardware is still
required for dual-core startup, GPIO boot selection, physical OLED drivers,
flash persistence, and USB behavior.
