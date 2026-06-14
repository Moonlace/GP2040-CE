/*
 * SPDX-License-Identifier: MIT
 */

#ifndef _MINIGAMESCREEN_H_
#define _MINIGAMESCREEN_H_

#include "GPGFX_UI_widgets.h"
#include "minigames/rhythm_game.h"

class MiniGameScreen : public GPScreen {
    public:
        MiniGameScreen() {}
        MiniGameScreen(GPGFX* renderer) { setRenderer(renderer); }
        virtual ~MiniGameScreen() {}
        virtual int8_t update();
        virtual void init();
        virtual void shutdown();

    protected:
        virtual void drawScreen();

    private:
        static const uint8_t MAX_REGISTERED_GAMES = 8;

        void buildGameList(const MiniGameOptions& options);
        void drawLauncher();
        void startSelectedGame(uint32_t now);
        MiniGame* findGame(uint32_t id);

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
