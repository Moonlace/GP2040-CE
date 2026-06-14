/*
 * SPDX-License-Identifier: MIT
 */

#include "MiniGameScreen.h"

#include "gamepad.h"
#include "storagemanager.h"

namespace {
const uint32_t FRAME_TIME_MS = 33;
}

void MiniGameScreen::init() {
    const MiniGameOptions& options = Storage::getInstance().getAddonOptions().miniGameOptions;
    const GPGFX_DisplayMetrics* metrics = getRenderer()->getDriver()->getMetrics();

    displayWidth = metrics->width;
    displayHeight = metrics->height;
    gameEnabled = false;
    for (size_t i = 0; i < options.games_count; i++) {
        if (options.games[i].gameId == rhythmGame.id() && options.games[i].enabled) {
            gameEnabled = options.enabled;
            rhythmGame.configure(options, displayWidth, displayHeight);
            break;
        }
    }

    gameActive = false;
    inputArmed = false;
    previousButtons = 0;
    previousDpad = 0;
    lastFrameTime = 0;
    getRenderer()->clearScreen();
}

void MiniGameScreen::shutdown() {
    clearElements();
}

int8_t MiniGameScreen::update() {
    const uint32_t now = getMillis();
    if (now - lastFrameTime < FRAME_TIME_MS) {
        return -1;
    }
    lastFrameTime = now;

    const GamepadState state = getGamepad()->state;
    if (!inputArmed) {
        previousButtons = state.buttons;
        previousDpad = state.dpad;
        if (state.buttons == 0 && state.dpad == 0) {
            inputArmed = true;
        }
    }

    const uint32_t pressedButtons =
        inputArmed ? state.buttons & ~previousButtons : 0;
    const uint8_t pressedDpad =
        static_cast<uint8_t>(inputArmed ? state.dpad & ~previousDpad : 0);
    previousButtons = state.buttons;
    previousDpad = state.dpad;

    if (!gameActive) {
        if (pressedButtons & GAMEPAD_MASK_B2) {
            return DisplayMode::CONFIG_INSTRUCTION;
        }
        if (gameEnabled && (pressedButtons & GAMEPAD_MASK_B1)) {
            rhythmGame.reset(now);
            gameActive = true;
        }
    } else if (pressedButtons & GAMEPAD_MASK_S1) {
        gameActive = false;
    } else {
        rhythmGame.update({state, pressedButtons, pressedDpad}, now);
    }

    return -1;
}

void MiniGameScreen::drawScreen() {
    if (gameActive) {
        rhythmGame.render(*getRenderer());
        return;
    }

    getRenderer()->drawText(4, 0, "Mini Games");
    if (gameEnabled) {
        getRenderer()->drawText(1, 3, "> Rhythm Rush");
        getRenderer()->drawText(0, 6, "B1: Play");
    } else {
        getRenderer()->drawText(1, 3, "No games enabled");
    }
    getRenderer()->drawText(0, 7, "B2: Back");
}
