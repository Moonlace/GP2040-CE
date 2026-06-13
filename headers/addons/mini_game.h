/*
 * SPDX-License-Identifier: MIT
 */

#ifndef MINI_GAME_ADDON_H_
#define MINI_GAME_ADDON_H_

#include "GPGFX.h"
#include "gpaddon.h"
#include "minigames/rhythm_game.h"

#ifndef MINI_GAME_ADDON_ENABLED
#define MINI_GAME_ADDON_ENABLED 1
#endif

#ifndef MINI_GAME_RHYTHM_BPM
#define MINI_GAME_RHYTHM_BPM 120
#endif

#ifndef MINI_GAME_RHYTHM_DIFFICULTY
#define MINI_GAME_RHYTHM_DIFFICULTY 2
#endif

#define MiniGameAddonName "MiniGame"

class MiniGameAddon : public GPAddon {
public:
    bool available() override;
    void setup() override;
    void preprocess() override {}
    void process() override;
    void postprocess(bool sent) override {}
    void reinit() override {}
    std::string name() override { return MiniGameAddonName; }

private:
    static const uint8_t MAX_REGISTERED_GAMES = 8;

    void buildGameList(const MiniGameOptions& options);
    void drawLauncher();
    void startSelectedGame(uint32_t now);
    MiniGame* findGame(uint32_t id);

    GPGFX* display = nullptr;
    GPGFX_DisplayTypeOptions displayOptions {};
    RhythmGame rhythmGame;
    MiniGame* games[MAX_REGISTERED_GAMES] {};
    MiniGame* activeGame = nullptr;
    uint8_t gameCount = 0;
    uint8_t selectedGame = 0;
    bool inputArmed = false;
    uint32_t previousButtons = 0;
    uint8_t previousDpad = 0;
    uint32_t lastFrameTime = 0;
    uint16_t displayWidth = 128;
    uint16_t displayHeight = 64;
};

#endif
