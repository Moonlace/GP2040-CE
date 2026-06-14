/*
 * SPDX-License-Identifier: MIT
 */

#include "RhythmBot.h"

#include "GPGFX.h"
#include "SimulatorClock.h"
#include "gamepad.h"
#include "minigames/rhythm_game.h"

#include <cstdio>
#include <iomanip>
#include <ostream>

namespace {
const uint16_t DISPLAY_WIDTH = 128;
const uint8_t LANE_COUNT = 4;
const uint32_t FIRST_HIT_TIME_MS = 1650;
const uint32_t NOTE_TRAVEL_TIME_MS = 1400;
const uint32_t SIMULATION_STEP_MS = 5;

uint32_t noteInterval(uint32_t bpm, uint32_t difficulty) {
	const uint32_t beat = 60000 / bpm;
	if (difficulty == 1) {
		return beat;
	}
	if (difficulty == 2) {
		return (beat * 3) / 4;
	}
	return beat / 2;
}

uint32_t visualHitLineLead(uint16_t height) {
	const uint16_t top = 9;
	const uint16_t hitY = height > 40 ? height - 9 : height - 5;
	return (NOTE_TRAVEL_TIME_MS - 1) / (hitY - top);
}

uint32_t expectedHitCount(
	uint32_t bpm,
	uint32_t difficulty,
	uint16_t height,
	uint32_t durationMs
) {
	const uint32_t observableUntil = durationMs + visualHitLineLead(height);
	if (observableUntil < FIRST_HIT_TIME_MS) {
		return 0;
	}
	return 1 + (observableUntil - FIRST_HIT_TIME_MS) / noteInterval(bpm, difficulty);
}

uint32_t expectedScore(uint32_t hits) {
	const uint32_t cappedHits = hits < 50 ? hits : 50;
	const uint32_t comboBonus = (cappedHits * (cappedHits + 1)) / 2;
	const uint32_t cappedBonus = hits > 50 ? (hits - 50) * 50 : 0;
	return hits * 100 + comboBonus + cappedBonus;
}

uint32_t detectLaneButtons(const GPGFX& display, uint16_t height) {
	const uint16_t hitY = height > 40 ? height - 9 : height - 5;
	const uint16_t sampleY = hitY + 3;
	const uint16_t laneWidth = DISPLAY_WIDTH / LANE_COUNT;
	const uint32_t buttons[LANE_COUNT] = {
		GAMEPAD_MASK_B1,
		GAMEPAD_MASK_B2,
		GAMEPAD_MASK_B3,
		GAMEPAD_MASK_B4,
	};

	uint32_t pressedButtons = 0;
	for (uint8_t lane = 0; lane < LANE_COUNT; lane++) {
		const uint16_t sampleX = lane * laneWidth + laneWidth / 2;
		if (display.getPixel(sampleX, sampleY) != 0) {
			pressedButtons |= buttons[lane];
		}
	}
	return pressedButtons;
}

bool readStatus(
	const GPGFX& display,
	uint32_t& score,
	uint32_t& combo,
	uint32_t& misses
) {
	unsigned long parsedScore = 0;
	unsigned int parsedCombo = 0;
	unsigned int parsedMisses = 0;
	const int matched =
#ifdef _MSC_VER
		sscanf_s(
#else
		std::sscanf(
#endif
		display.textRow(0).c_str(),
		"S:%lu C:%u M:%u",
		&parsedScore,
		&parsedCombo,
		&parsedMisses
	);
	if (matched != 3) {
		return false;
	}

	score = static_cast<uint32_t>(parsedScore);
	combo = parsedCombo;
	misses = parsedMisses;
	return true;
}
}

RhythmBotResult runRhythmBot(
	uint32_t bpm,
	uint32_t difficulty,
	uint16_t height,
	uint32_t durationMs
) {
	RhythmBotResult result;
	result.bpm = bpm;
	result.difficulty = difficulty;
	result.height = height;
	result.durationMs = durationMs;
	result.expectedHits = expectedHitCount(bpm, difficulty, height, durationMs);

	MiniGameOptions options;
	options.rhythmBpm = bpm;
	options.rhythmDifficulty = difficulty;

	RhythmGame game;
	GPGFX display(DISPLAY_WIDTH, height);
	game.configure(options, DISPLAY_WIDTH, height);
	game.reset(0);

	for (uint32_t now = 0; now <= durationMs; now += SIMULATION_STEP_MS) {
		setSimulatedMillis(now);
		MiniGameInput input {};
		game.update(input, now);
		display.clearScreen();
		game.render(display);

		input.pressedButtons = detectLaneButtons(display, height);
		if (input.pressedButtons != 0) {
			game.update(input, now);
			display.clearScreen();
			game.render(display);
		}
	}

	if (!readStatus(display, result.score, result.combo, result.misses)) {
		result.failure = "unable to parse the rendered status line";
		return result;
	}

	const uint32_t targetScore = expectedScore(result.expectedHits);
	if (result.misses != 0) {
		result.failure = "bot allowed one or more notes to expire";
	} else if (result.combo != result.expectedHits) {
		result.failure = "final combo does not match the expected note count";
	} else if (result.score != targetScore) {
		result.failure = "score does not match the exact perfect-play score";
	} else {
		result.passed = true;
	}
	return result;
}

void printRhythmBotResult(const RhythmBotResult& result, std::ostream& output) {
	output
		<< (result.passed ? "PASS" : "FAIL")
		<< "  BPM " << std::setw(3) << result.bpm
		<< "  difficulty " << result.difficulty
		<< "  128x" << result.height
		<< "  hits " << std::setw(3) << result.combo
		<< '/' << std::setw(3) << result.expectedHits
		<< "  score " << std::setw(6) << result.score
		<< "  misses " << result.misses;
	if (!result.failure.empty()) {
		output << "  " << result.failure;
	}
	output << '\n';
}
