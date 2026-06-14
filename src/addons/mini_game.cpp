/*
 * SPDX-License-Identifier: MIT
 */

#include "addons/mini_game.h"

#include "addons/display.h"
#include "storagemanager.h"
#include "system.h"

namespace {
const uint32_t FRAME_TIME_MS = 33;
}

bool MiniGameAddon::available() {
    System::setMiniGameDisplayReady(false);
    const MiniGameOptions& options = Storage::getInstance().getAddonOptions().miniGameOptions;
    if (!System::isMiniGameMode() || !options.enabled) {
        return false;
    }

    display = new GPGFX();
    displayOptions = display->getAvailableDisplay(GPGFX_DisplayType::DISPLAY_TYPE_NONE);
    if (displayOptions.displayType == GPGFX_DisplayType::DISPLAY_TYPE_NONE) {
        delete display;
        display = nullptr;
        return false;
    }
    return true;
}

void MiniGameAddon::setup() {
    const DisplayOptions& options = Storage::getInstance().getDisplayOptions();
    const MiniGameOptions& miniGameOptions = Storage::getInstance().getAddonOptions().miniGameOptions;

    displayOptions.size = options.size;
    displayOptions.orientation = options.flip;
    displayOptions.inverted = options.invert;
    displayOptions.font.fontData = GP_Font_Standard;
    displayOptions.font.width = 6;
    displayOptions.font.height = 8;
    displayOptions.contrast = options.contrast;
    display->init(displayOptions);

    displayHeight = options.size == GPGFX_DisplaySize::SIZE_128x32 ? 32 : 64;
    buildGameList(miniGameOptions);
    for (uint8_t i = 0; i < gameCount; i++) {
        games[i]->configure(miniGameOptions, displayWidth, displayHeight);
        if (games[i]->id() == miniGameOptions.defaultGameId) {
            selectedGame = i;
        }
    }

    display->clearScreen();
    drawLauncher();
    display->render();
    System::setMiniGameDisplayReady(true);
}

void MiniGameAddon::buildGameList(const MiniGameOptions& options) {
    gameCount = 0;
    for (size_t i = 0; i < options.games_count && gameCount < MAX_REGISTERED_GAMES; i++) {
        if (!options.games[i].enabled) {
            continue;
        }
        MiniGame* game = findGame(options.games[i].gameId);
        if (game != nullptr) {
            games[gameCount++] = game;
        }
    }
}

MiniGame* MiniGameAddon::findGame(uint32_t id) {
    if (id == rhythmGame.id()) {
        return &rhythmGame;
    }
    return nullptr;
}

void MiniGameAddon::startSelectedGame(uint32_t now) {
    if (gameCount == 0) {
        return;
    }
    activeGame = games[selectedGame];
    activeGame->reset(now);
}

void MiniGameAddon::drawLauncher() {
    display->drawText(4, 0, "Mini Games");
    if (gameCount == 0) {
        display->drawText(1, 2, "No games enabled");
        display->drawText(1, displayHeight > 32 ? 7 : 3, "B2: Exit");
        return;
    }

    const uint8_t titleRow = displayHeight > 32 ? 3 : 2;
    display->drawText(1, titleRow, ">");
    display->drawText(3, titleRow, games[selectedGame]->title());
    if (displayHeight > 32) {
        display->drawText(0, 6, "Up/Down: Select");
        display->drawText(0, 7, "B1: Play B2: Exit");
    } else {
        display->drawText(0, 3, "B1 Play / B2 Exit");
    }
}

void MiniGameAddon::process() {
    const uint32_t now = getMillis();
    if (now - lastFrameTime < FRAME_TIME_MS) {
        return;
    }
    lastFrameTime = now;

    const GamepadState state = Storage::getInstance().GetProcessedGamepad()->state;
    if (!inputArmed) {
        previousButtons = state.buttons;
        previousDpad = state.dpad;
        if (state.buttons == 0 && state.dpad == 0) {
            inputArmed = true;
        }
    }

    MiniGameInput input {
        state,
        inputArmed ? state.buttons & ~previousButtons : 0,
        static_cast<uint8_t>(inputArmed ? state.dpad & ~previousDpad : 0),
    };
    previousButtons = state.buttons;
    previousDpad = state.dpad;

    if (activeGame == nullptr) {
        if (gameCount > 0 && (input.pressedDpad & (GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT))) {
            selectedGame = selectedGame == 0 ? gameCount - 1 : selectedGame - 1;
        }
        if (gameCount > 0 && (input.pressedDpad & (GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT))) {
            selectedGame = (selectedGame + 1) % gameCount;
        }
        if (input.pressedButtons & GAMEPAD_MASK_B1) {
            startSelectedGame(now);
        } else if (input.pressedButtons & GAMEPAD_MASK_B2) {
            System::requestMiniGameExit();
        }
    } else if (input.pressedButtons & GAMEPAD_MASK_S1) {
        activeGame = nullptr;
    } else {
        activeGame->update(input, now);
    }

    display->clearScreen();
    if (activeGame != nullptr) {
        activeGame->render(*display);
    } else {
        drawLauncher();
    }
    display->render();
}
