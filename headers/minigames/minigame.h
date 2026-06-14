/*
 * SPDX-License-Identifier: MIT
 */

#ifndef MINI_GAME_H_
#define MINI_GAME_H_

#include <cstdint>

#include "GPGFX.h"
#include "config.pb.h"
#include "gamepad/GamepadState.h"

struct MiniGameInput {
    GamepadState state;
    uint32_t pressedButtons;
    uint8_t pressedDpad;
};

class MiniGame {
    public:
        virtual ~MiniGame() {}
        virtual uint32_t id() const = 0;
        virtual const char* title() const = 0;
        virtual void configure(const MiniGameOptions& options, uint16_t width, uint16_t height) = 0;
        virtual void reset(uint32_t now) = 0;
        virtual void update(const MiniGameInput& input, uint32_t now) = 0;
        virtual void render(GPGFX& display) = 0;
};

#endif
