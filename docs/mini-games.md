# Mini Game Add-on

The mini-game add-on is an alternate boot mode that gives one add-on exclusive
ownership of the configured OLED. The normal display add-on and other core 1
output add-ons are not loaded in this mode.

## Runtime flow

1. Core 0 reads the boot GPIO mask before normal controller operation starts.
2. A matching mini-game mask selects `System::BootMode::MINIGAME`.
3. Core 0 continues to scan and process controller inputs, but does not start
   TinyUSB or send controller reports.
4. Core 1 loads only `MiniGameAddon`, which initializes the OLED and presents
   the launcher.
5. Selecting a game calls its `reset`, `update`, and `render` methods. Pressing
   S1 returns to the launcher; B2 exits the launcher and reboots to controller
   mode.

Input remains disarmed until the boot combination is released. This prevents
the held boot button from immediately starting or exiting a game.

## Adding a game

Mini games are compiled into the firmware. This avoids loading executable code
from configuration storage and keeps memory use deterministic.

To add another game:

1. Implement the `MiniGame` interface in `headers/minigames`.
2. Add its source file to `CMakeLists.txt`.
3. Give it a stable numeric game ID.
4. Add an instance and ID mapping to `MiniGameAddon::findGame`.
5. Add its default `MiniGameEntry` and any game-specific protobuf settings.
6. Expose those settings on the Mini Games configurator page.

Games should use fixed-size storage, avoid allocations in `update` and
`render`, and support both 128x64 and 128x32 display heights where practical.

## Desktop testing

The standalone project in `tools/minigame-simulator` compiles the production
rhythm-game source against host-only display, input, clock, and configuration
stand-ins. It is not referenced by the firmware CMake project, so desktop
dependencies cannot enter Pico builds.

See `tools/minigame-simulator/README.md` for build, interactive controls, and
self-test commands. The simulator also includes a visual autoplay bot that
detects notes from the framebuffer and validates perfect-play results across a
tempo, difficulty, and display-size matrix.

For complete firmware, simulator, and bot build instructions, see
`docs/mini-game-build-and-test-guide.md`.

## Rhythm Rush

Rhythm Rush is the initial built-in game. Notes move down four lanes. Each lane
can be assigned to any gamepad button or direction in the Mini Games
configurator; the defaults are B1, B2, B3, and B4 from left to right. The
configurator also controls tempo from 60 to 240 BPM and provides three
timing-window difficulty levels.
