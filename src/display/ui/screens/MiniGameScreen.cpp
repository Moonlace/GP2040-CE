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
    buildGameList(options);
    for (uint8_t i = 0; i < gameCount; i++) {
        games[i]->configure(options, displayWidth, displayHeight);
        if (games[i]->id() == options.defaultGameId) {
            selectedGame = i;
        }
    }

    activeGame = nullptr;
    inputArmed = false;
    previousButtons = 0;
    previousDpad = 0;
    lastFrameTime = 0;
    getRenderer()->clearScreen();
}

void MiniGameScreen::shutdown() {
    clearElements();
}

void MiniGameScreen::buildGameList(const MiniGameOptions& options) {
    gameCount = 0;
    selectedGame = 0;
    if (!options.enabled) {
        return;
    }

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

MiniGame* MiniGameScreen::findGame(uint32_t id) {
    if (id == rhythmGame.id()) {
        return &rhythmGame;
    }
    return nullptr;
}

void MiniGameScreen::startSelectedGame(uint32_t now) {
    if (gameCount == 0) {
        return;
    }
    activeGame = games[selectedGame];
    activeGame->reset(now);
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

    if (activeGame == nullptr) {
        if (pressedButtons & GAMEPAD_MASK_B2) {
            return DisplayMode::CONFIG_INSTRUCTION;
        }
        if (gameCount > 0 && (pressedDpad & (GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT))) {
            selectedGame = selectedGame == 0 ? gameCount - 1 : selectedGame - 1;
        }
        if (gameCount > 0 && (pressedDpad & (GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT))) {
            selectedGame = (selectedGame + 1) % gameCount;
        }
        if (pressedButtons & GAMEPAD_MASK_B1) {
            startSelectedGame(now);
        }
    } else if (pressedButtons & GAMEPAD_MASK_S1) {
        activeGame = nullptr;
    } else {
        activeGame->update({state, pressedButtons, pressedDpad}, now);
    }

    return -1;
}

void MiniGameScreen::drawLauncher() {
    getRenderer()->drawText(4, 0, "Mini Games");
    if (gameCount == 0) {
        const uint8_t messageRow = displayHeight > 32 ? 3 : 2;
        const uint8_t controlsRow = displayHeight > 32 ? 7 : 3;
        getRenderer()->drawText(1, messageRow, "No games enabled");
        getRenderer()->drawText(0, controlsRow, "B2: Back");
        return;
    }

    const uint8_t titleRow = displayHeight > 32 ? 3 : 2;
    getRenderer()->drawText(1, titleRow, ">");
    getRenderer()->drawText(3, titleRow, games[selectedGame]->title());
    if (displayHeight > 32) {
        getRenderer()->drawText(0, 6, "Up/Down: Select");
        getRenderer()->drawText(0, 7, "B1: Play B2: Back");
    } else {
        getRenderer()->drawText(0, 3, "B1 Play / B2 Back");
    }
}

void MiniGameScreen::drawScreen() {
    if (activeGame != nullptr) {
        activeGame->render(*getRenderer());
    } else {
        drawLauncher();
    }
}
