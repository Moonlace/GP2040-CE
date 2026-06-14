/*
 * SPDX-License-Identifier: MIT
 */

#ifndef RHYTHM_GAME_H_
#define RHYTHM_GAME_H_

#include "minigames/minigame.h"

#ifndef MINI_GAME_ENABLED
#define MINI_GAME_ENABLED 1
#endif

#ifndef MINI_GAME_RHYTHM_BPM
#define MINI_GAME_RHYTHM_BPM 120
#endif

#ifndef MINI_GAME_RHYTHM_DIFFICULTY
#define MINI_GAME_RHYTHM_DIFFICULTY 2
#endif

#define MINI_GAME_RHYTHM_ID 1
#define MINI_GAME_RHYTHM_TITLE "Rhythm Rush"

class RhythmGame : public MiniGame {
    public:
        uint32_t id() const override { return MINI_GAME_RHYTHM_ID; }
        const char* title() const override { return MINI_GAME_RHYTHM_TITLE; }
        void configure(const MiniGameOptions& options, uint16_t width, uint16_t height) override;
        void reset(uint32_t now) override;
        void update(const MiniGameInput& input, uint32_t now) override;
        void render(GPGFX& display) override;

    private:
        struct Note {
            bool active;
            uint8_t lane;
            uint32_t hitTime;
        };

        static const uint8_t NOTE_CAPACITY = 16;
        static const uint8_t LANE_COUNT = 4;
        static const uint32_t NOTE_TRAVEL_TIME_MS = 1400;

        void spawnNote(uint32_t hitTime);
        void hitLane(uint8_t lane, uint32_t now);
        uint32_t noteInterval() const;
        uint32_t hitWindow() const;

        Note notes[NOTE_CAPACITY] {};
        uint16_t displayWidth = 128;
        uint16_t displayHeight = 64;
        uint32_t bpm = 120;
        uint32_t difficulty = 2;
        uint32_t laneButtons[LANE_COUNT] {};
        uint32_t nextSpawnTime = 0;
        uint32_t score = 0;
        uint16_t combo = 0;
        uint16_t misses = 0;
        uint8_t patternIndex = 0;
};

#endif
