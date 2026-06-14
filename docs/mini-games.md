# Mini Games

Mini games run as a screen inside the existing display add-on while the
controller is in Web Config mode. The display add-on remains the sole owner of
the OLED and switches between its normal screens and the mini-game screen.

## Runtime Flow

1. Enter Web Config mode normally.
2. Press `R1` on the OLED instruction screen to open Mini Games.
3. Use Up/Down to select an enabled game and press `B1` to start it.
4. The active game receives `update` and `render` calls for the complete
   framebuffer.
5. Press `S1` to return to the mini-game launcher.
6. Press `B2` from the launcher to return to the Web Config instruction screen.

This design does not alter controller boot processing, USB startup, or core 1
add-on loading. It also prevents the normal button display from drawing over an
active game.

## Adding a Game

Mini games are compiled into the firmware. The Web Configurator enables and
configures games already present in the firmware; it does not upload executable
code.

To add another game:

1. Implement the `MiniGame` interface under `headers/minigames` and
   `src/minigames`.
2. Give the game a stable, unique numeric ID.
3. Add its source file to `CMakeLists.txt`.
4. Add an instance and ID mapping to `MiniGameScreen::findGame`.
5. Add a default `MiniGameEntry` and any game-specific protobuf settings.
6. Expose those settings on the Mini Games configurator page.

Games should use fixed-size storage, avoid allocations in `update` and
`render`, and support both 128x32 and 128x64 display heights where practical.

## Desktop Testing

The standalone project in `tools/minigame-simulator` compiles the production
Rhythm Rush source against host-only display, input, clock, and configuration
stand-ins. It is separate from the firmware CMake project, so PC-only
dependencies cannot enter Pico builds.

The simulator includes an interactive build, deterministic tests, and a visual
autoplay bot that detects notes from the framebuffer. See
`tools/minigame-simulator/README.md` and
`docs/mini-game-build-and-test-guide.md`.

## Rhythm Rush

Rhythm Rush is the initial built-in game. Notes move down four lanes. Each lane
can be assigned to any gamepad button or direction on the Mini Games page; the
defaults are B1, B2, B3, and B4 from left to right. The page also configures
tempo from 60 to 240 BPM and three timing-window difficulty levels.
