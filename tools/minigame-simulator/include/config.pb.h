/*
 * SPDX-License-Identifier: MIT
 */

#ifndef HOST_CONFIG_PB_H_
#define HOST_CONFIG_PB_H_

#include <cstdint>

struct MiniGameOptions {
	bool enabled = true;
	uint32_t defaultGameId = 1;
	uint32_t rhythmBpm = 120;
	uint32_t rhythmDifficulty = 2;
	uint32_t rhythmLane1Button = 1U << 0;
	uint32_t rhythmLane2Button = 1U << 1;
	uint32_t rhythmLane3Button = 1U << 2;
	uint32_t rhythmLane4Button = 1U << 3;
};

#endif
