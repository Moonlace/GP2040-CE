/*
 * SPDX-License-Identifier: MIT
 */

#include "SimulatorClock.h"

namespace {
uint32_t simulatedMillis = 0;
}

void setSimulatedMillis(uint32_t value) {
	simulatedMillis = value;
}

uint32_t getMillis() {
	return simulatedMillis;
}
