/*
 * SPDX-License-Identifier: MIT
 */

#ifndef HOST_GAMEPAD_STATE_H_
#define HOST_GAMEPAD_STATE_H_

#include <cstdint>

#define GAMEPAD_MASK_UP    (1U << 0)
#define GAMEPAD_MASK_DOWN  (1U << 1)
#define GAMEPAD_MASK_LEFT  (1U << 2)
#define GAMEPAD_MASK_RIGHT (1U << 3)

#define GAMEPAD_MASK_B1 (1U << 0)
#define GAMEPAD_MASK_B2 (1U << 1)
#define GAMEPAD_MASK_B3 (1U << 2)
#define GAMEPAD_MASK_B4 (1U << 3)
#define GAMEPAD_MASK_S1 (1U << 8)

struct GamepadState {
	uint8_t dpad = 0;
	uint32_t buttons = 0;
};

#endif
