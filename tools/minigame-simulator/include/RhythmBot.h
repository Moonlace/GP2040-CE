/*
 * SPDX-License-Identifier: MIT
 */

#ifndef RHYTHM_BOT_H_
#define RHYTHM_BOT_H_

#include <cstdint>
#include <iosfwd>
#include <string>

struct RhythmBotResult {
	uint32_t bpm = 0;
	uint32_t difficulty = 0;
	uint16_t height = 0;
	uint32_t durationMs = 0;
	uint32_t expectedHits = 0;
	uint32_t score = 0;
	uint32_t combo = 0;
	uint32_t misses = 0;
	bool passed = false;
	std::string failure;
};

RhythmBotResult runRhythmBot(
	uint32_t bpm,
	uint32_t difficulty,
	uint16_t height,
	uint32_t durationMs
);

void printRhythmBotResult(const RhythmBotResult& result, std::ostream& output);

#endif
