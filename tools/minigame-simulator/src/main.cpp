/*
 * SPDX-License-Identifier: MIT
 */

#include "GPGFX.h"
#include "RhythmBot.h"
#include "SimulatorClock.h"
#include "gamepad.h"
#include "minigames/rhythm_game.h"

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#endif

namespace {
struct SimulatorOptions {
	uint32_t bpm = 120;
	uint32_t difficulty = 2;
	uint16_t height = 64;
	uint32_t botDurationMs = 20000;
	bool selfTest = false;
	bool bot = false;
	bool botMatrix = false;
};

class Keyboard {
public:
	Keyboard() {
#ifdef _WIN32
		const HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
		DWORD mode = 0;
		if (output != INVALID_HANDLE_VALUE && GetConsoleMode(output, &mode)) {
			SetConsoleMode(output, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
		}
#else
		tcgetattr(STDIN_FILENO, &originalTermios);
		termios raw = originalTermios;
		raw.c_lflag &= static_cast<tcflag_t>(~(ICANON | ECHO));
		tcsetattr(STDIN_FILENO, TCSANOW, &raw);
		originalFlags = fcntl(STDIN_FILENO, F_GETFL, 0);
		fcntl(STDIN_FILENO, F_SETFL, originalFlags | O_NONBLOCK);
#endif
	}

	~Keyboard() {
#ifndef _WIN32
		tcsetattr(STDIN_FILENO, TCSANOW, &originalTermios);
		fcntl(STDIN_FILENO, F_SETFL, originalFlags);
#endif
	}

	bool read(char& value) {
#ifdef _WIN32
		if (!_kbhit()) {
			return false;
		}
		value = static_cast<char>(_getch());
		return true;
#else
		return ::read(STDIN_FILENO, &value, 1) == 1;
#endif
	}

private:
#ifndef _WIN32
	termios originalTermios {};
	int originalFlags = 0;
#endif
};

uint32_t buttonForKey(char key) {
	switch (key) {
		case 'a':
		case 'A':
		case '1':
			return GAMEPAD_MASK_B1;
		case 's':
		case 'S':
		case '2':
			return GAMEPAD_MASK_B2;
		case 'd':
		case 'D':
		case '3':
			return GAMEPAD_MASK_B3;
		case 'f':
		case 'F':
		case '4':
			return GAMEPAD_MASK_B4;
		default:
			return 0;
	}
}

uint32_t parseUnsigned(const char* value, const char* optionName) {
	char* end = nullptr;
	const unsigned long parsed = std::strtoul(value, &end, 10);
	if (end == value || *end != '\0') {
		throw std::runtime_error(std::string("Invalid value for ") + optionName);
	}
	return static_cast<uint32_t>(parsed);
}

SimulatorOptions parseOptions(int argc, char* argv[]) {
	SimulatorOptions options;
	for (int i = 1; i < argc; i++) {
		const std::string argument = argv[i];
		if (argument == "--self-test") {
			options.selfTest = true;
		} else if (argument == "--bot") {
			options.bot = true;
		} else if (argument == "--bot-matrix") {
			options.botMatrix = true;
		} else if (argument == "--bot-duration-ms" && i + 1 < argc) {
			options.botDurationMs = parseUnsigned(argv[++i], "--bot-duration-ms");
		} else if (argument == "--bpm" && i + 1 < argc) {
			options.bpm = parseUnsigned(argv[++i], "--bpm");
		} else if (argument == "--difficulty" && i + 1 < argc) {
			options.difficulty = parseUnsigned(argv[++i], "--difficulty");
		} else if (argument == "--height" && i + 1 < argc) {
			options.height = static_cast<uint16_t>(parseUnsigned(argv[++i], "--height"));
		} else if (argument == "--help" || argument == "-h") {
			std::cout
				<< "Usage: gp2040-minigame-sim [options]\n"
				<< "  --bpm 60..240\n"
				<< "  --difficulty 1..3\n"
				<< "  --height 32|64\n"
				<< "  --self-test\n"
				<< "  --bot\n"
				<< "  --bot-matrix\n"
				<< "  --bot-duration-ms 2000..\n";
			std::exit(0);
		} else {
			throw std::runtime_error("Unknown or incomplete option: " + argument);
		}
	}

	if (options.bpm < 60 || options.bpm > 240) {
		throw std::runtime_error("--bpm must be between 60 and 240");
	}
	if (options.difficulty < 1 || options.difficulty > 3) {
		throw std::runtime_error("--difficulty must be between 1 and 3");
	}
	if (options.height != 32 && options.height != 64) {
		throw std::runtime_error("--height must be 32 or 64");
	}
	if (options.botDurationMs < 2000) {
		throw std::runtime_error("--bot-duration-ms must be at least 2000");
	}
	return options;
}

MiniGameOptions makeGameOptions(const SimulatorOptions& simulatorOptions) {
	MiniGameOptions options;
	options.rhythmBpm = simulatorOptions.bpm;
	options.rhythmDifficulty = simulatorOptions.difficulty;
	return options;
}

bool statusContains(const GPGFX& display, const std::string& expected) {
	return display.textRow(0).find(expected) != std::string::npos;
}

int runSelfTest() {
	RhythmGame game;
	GPGFX display(128, 64);
	const SimulatorOptions simulatorOptions;
	MiniGameOptions gameOptions = makeGameOptions(simulatorOptions);
	gameOptions.rhythmLane1Button = GAMEPAD_MASK_L1;
	game.configure(gameOptions, 128, 64);
	game.reset(0);

	MiniGameInput input {};
	setSimulatedMillis(250);
	game.update(input, getMillis());
	display.clearScreen();
	game.render(display);
	if (display.litPixelCount() == 0 || !statusContains(display, "S:0 C:0 M:0")) {
		std::cerr << "Self-test failed: first note did not render\n";
		return 1;
	}

	input.pressedButtons = GAMEPAD_MASK_B1;
	setSimulatedMillis(1650);
	game.update(input, getMillis());
	display.clearScreen();
	game.render(display);
	if (!statusContains(display, "S:0 C:0 M:0")) {
		std::cerr << "Self-test failed: old lane button remained active\n";
		return 1;
	}

	input.pressedButtons = GAMEPAD_MASK_L1;
	game.update(input, getMillis());
	display.clearScreen();
	game.render(display);
	if (!statusContains(display, "S:101 C:1 M:0")) {
		std::cerr << "Self-test failed: remapped lane button did not score\n";
		return 1;
	}

	setSimulatedMillis(1650);
	input.pressedButtons = GAMEPAD_MASK_B1;
	game.update(input, getMillis());
	display.clearScreen();
	game.render(display);
	if (!statusContains(display, "S:101 C:1 M:0")) {
		std::cerr << "Self-test failed: timed lane hit did not score\n";
		return 1;
	}

	setSimulatedMillis(4000);
	input.pressedButtons = 0;
	game.update(input, getMillis());
	display.clearScreen();
	game.render(display);
	if (statusContains(display, "M:0")) {
		std::cerr << "Self-test failed: expired notes were not counted as misses\n";
		return 1;
	}

	GPGFX shortDisplay(128, 32);
	game.configure(makeGameOptions(simulatorOptions), 128, 32);
	game.reset(0);
	setSimulatedMillis(250);
	game.update(input, getMillis());
	shortDisplay.clearScreen();
	game.render(shortDisplay);
	if (shortDisplay.litPixelCount() == 0) {
		std::cerr << "Self-test failed: 128x32 rendering produced no pixels\n";
		return 1;
	}

	std::cout << "Rhythm game self-test passed\n";
	return 0;
}

int runBot(const SimulatorOptions& options) {
	const RhythmBotResult result = runRhythmBot(
		options.bpm,
		options.difficulty,
		options.height,
		options.botDurationMs
	);
	printRhythmBotResult(result, std::cout);
	return result.passed ? 0 : 1;
}

int runBotMatrix(const SimulatorOptions& options) {
	const uint32_t tempos[] = {60, 120, 240};
	const uint32_t difficulties[] = {1, 2, 3};
	const uint16_t heights[] = {32, 64};
	uint32_t passed = 0;
	uint32_t total = 0;

	std::cout
		<< "Rhythm Rush visual autoplay matrix ("
		<< options.botDurationMs << " ms simulated per case)\n";
	for (uint16_t height : heights) {
		for (uint32_t difficulty : difficulties) {
			for (uint32_t bpm : tempos) {
				const RhythmBotResult result = runRhythmBot(
					bpm,
					difficulty,
					height,
					options.botDurationMs
				);
				printRhythmBotResult(result, std::cout);
				passed += result.passed ? 1U : 0U;
				total++;
			}
		}
	}
	std::cout << "Bot matrix: " << passed << '/' << total << " cases passed\n";
	return passed == total ? 0 : 1;
}

int runInteractive(const SimulatorOptions& simulatorOptions) {
	RhythmGame game;
	GPGFX display(128, simulatorOptions.height);
	game.configure(makeGameOptions(simulatorOptions), 128, simulatorOptions.height);
	game.reset(0);

	Keyboard keyboard;
	const auto startedAt = std::chrono::steady_clock::now();
	bool running = true;
	std::cout << "\x1b[2J";

	while (running) {
		const auto elapsed = std::chrono::steady_clock::now() - startedAt;
		setSimulatedMillis(static_cast<uint32_t>(
			std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count()
		));

		MiniGameInput input {};
		char key = 0;
		while (keyboard.read(key)) {
			if (key == 'q' || key == 'Q') {
				running = false;
			} else if (key == 'r' || key == 'R') {
				game.reset(getMillis());
			} else {
				input.pressedButtons |= buttonForKey(key);
			}
		}

		game.update(input, getMillis());
		display.clearScreen();
		game.render(display);
		display.present(std::cout);
		std::cout
			<< "A/S/D/F or 1/2/3/4: hit lanes | R: restart | Q: quit\n"
			<< "BPM " << simulatorOptions.bpm
			<< " | difficulty " << simulatorOptions.difficulty
			<< " | display 128x" << simulatorOptions.height << "   \n"
			<< std::flush;
		std::this_thread::sleep_for(std::chrono::milliseconds(33));
	}

	std::cout << "\n";
	return 0;
}
}

int main(int argc, char* argv[]) {
	try {
		const SimulatorOptions options = parseOptions(argc, argv);
		if (options.selfTest) {
			return runSelfTest();
		}
		if (options.botMatrix) {
			return runBotMatrix(options);
		}
		if (options.bot) {
			return runBot(options);
		}
		return runInteractive(options);
	} catch (const std::exception& error) {
		std::cerr << error.what() << '\n';
		return 2;
	}
}
