/*
 * SPDX-License-Identifier: MIT
 */

#include "minigames/rhythm_game.h"

#include <cstdio>
#include "gamepad.h"

namespace {
const uint8_t NOTE_PATTERN[] = {
    0, 1, 2, 3, 0, 2, 1, 3,
    3, 2, 1, 0, 1, 3, 0, 2,
};

int32_t elapsed(uint32_t now, uint32_t then) {
    return static_cast<int32_t>(now - then);
}
}

void RhythmGame::configure(const MiniGameOptions& options, uint16_t width, uint16_t height) {
    displayWidth = width;
    displayHeight = height;
    bpm = options.rhythmBpm < 60 ? 60 : (options.rhythmBpm > 240 ? 240 : options.rhythmBpm);
    difficulty = options.rhythmDifficulty < 1 ? 1 :
        (options.rhythmDifficulty > 3 ? 3 : options.rhythmDifficulty);
    laneButtons[0] = options.rhythmLane1Button;
    laneButtons[1] = options.rhythmLane2Button;
    laneButtons[2] = options.rhythmLane3Button;
    laneButtons[3] = options.rhythmLane4Button;
}

void RhythmGame::reset(uint32_t now) {
    for (Note& note : notes) {
        note = {};
    }
    score = 0;
    combo = 0;
    misses = 0;
    patternIndex = 0;
    nextSpawnTime = now + 250;
}

uint32_t RhythmGame::noteInterval() const {
    const uint32_t beat = 60000 / bpm;
    if (difficulty == 1) {
        return beat;
    }
    if (difficulty == 2) {
        return (beat * 3) / 4;
    }
    return beat / 2;
}

uint32_t RhythmGame::hitWindow() const {
    if (difficulty == 1) {
        return 180;
    }
    if (difficulty == 2) {
        return 140;
    }
    return 105;
}

void RhythmGame::spawnNote(uint32_t hitTime) {
    for (Note& note : notes) {
        if (!note.active) {
            note.active = true;
            note.lane = NOTE_PATTERN[patternIndex % (sizeof(NOTE_PATTERN) / sizeof(NOTE_PATTERN[0]))];
            note.hitTime = hitTime;
            patternIndex++;
            return;
        }
    }
}

void RhythmGame::hitLane(uint8_t lane, uint32_t now) {
    Note* best = nullptr;
    uint32_t bestDistance = static_cast<uint32_t>(-1);

    for (Note& note : notes) {
        if (!note.active || note.lane != lane) {
            continue;
        }
        const int32_t delta = elapsed(now, note.hitTime);
        const uint32_t distance = static_cast<uint32_t>(delta < 0 ? -delta : delta);
        if (distance < bestDistance) {
            best = &note;
            bestDistance = distance;
        }
    }

    if (best != nullptr && bestDistance <= hitWindow()) {
        best->active = false;
        combo++;
        score += 100 + (combo > 50 ? 50 : combo);
    } else {
        combo = 0;
    }
}

void RhythmGame::update(const MiniGameInput& input, uint32_t now) {
    while (elapsed(now, nextSpawnTime) >= 0) {
        spawnNote(nextSpawnTime + NOTE_TRAVEL_TIME_MS);
        nextSpawnTime += noteInterval();
    }

    const uint32_t pressedInputs =
        input.pressedButtons | (static_cast<uint32_t>(input.pressedDpad) << 16);
    for (uint8_t lane = 0; lane < LANE_COUNT; lane++) {
        if (pressedInputs & laneButtons[lane]) {
            hitLane(lane, now);
        }
    }

    for (Note& note : notes) {
        if (note.active && elapsed(now, note.hitTime) > static_cast<int32_t>(hitWindow())) {
            note.active = false;
            combo = 0;
            misses++;
        }
    }
}

void RhythmGame::render(GPGFX& display) {
    const uint16_t top = 9;
    const uint16_t hitY = displayHeight > 40 ? displayHeight - 9 : displayHeight - 5;
    const uint16_t laneWidth = displayWidth / LANE_COUNT;
    const uint16_t noteWidth = laneWidth > 8 ? laneWidth - 8 : 3;
    const uint32_t now = getMillis();

    char status[22];
    std::snprintf(status, sizeof(status), "S:%lu C:%u M:%u",
        static_cast<unsigned long>(score),
        static_cast<unsigned int>(combo),
        static_cast<unsigned int>(misses));
    display.drawText(0, 0, status);

    display.drawLine(0, hitY, displayWidth - 1, hitY, 1, 0);
    for (uint8_t lane = 1; lane < LANE_COUNT; lane++) {
        const uint16_t x = lane * laneWidth;
        display.drawLine(x, top, x, displayHeight - 1, 1, 0);
    }

    for (const Note& note : notes) {
        if (!note.active) {
            continue;
        }
        const int32_t untilHit = static_cast<int32_t>(note.hitTime - now);
        int32_t y = static_cast<int32_t>(hitY) -
            (untilHit * static_cast<int32_t>(hitY - top)) / static_cast<int32_t>(NOTE_TRAVEL_TIME_MS);
        if (y < static_cast<int32_t>(top)) {
            y = top;
        } else if (y > static_cast<int32_t>(displayHeight - 4)) {
            y = displayHeight - 4;
        }
        const uint16_t x = note.lane * laneWidth + 4;
        display.drawRectangle(x, static_cast<uint16_t>(y), noteWidth, 4, 1, 1);
    }
}
