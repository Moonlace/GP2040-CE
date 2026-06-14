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
    RhythmGame rhythmGame;
    bool gameEnabled = false;
    bool gameActive = false;
    bool inputArmed = false;
    uint32_t previousButtons = 0;
    uint8_t previousDpad = 0;
    uint32_t lastFrameTime = 0;
    uint16_t displayWidth = 128;
    uint16_t displayHeight = 64;
};

#endif
