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
};

#endif
