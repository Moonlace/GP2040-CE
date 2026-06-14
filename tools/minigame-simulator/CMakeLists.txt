cmake_minimum_required(VERSION 3.20)

project(GP2040MiniGameSimulator LANGUAGES CXX)

add_executable(gp2040-minigame-sim
	src/GPGFX.cpp
	src/RhythmBot.cpp
	src/SimulatorClock.cpp
	src/main.cpp
	../../src/minigames/rhythm_game.cpp
)

target_compile_features(gp2040-minigame-sim PRIVATE cxx_std_17)

target_include_directories(gp2040-minigame-sim PRIVATE
	include
	../../headers
)

if(MSVC)
	target_compile_options(gp2040-minigame-sim PRIVATE /W4 /permissive-)
else()
	target_compile_options(gp2040-minigame-sim PRIVATE -Wall -Wextra -Wpedantic)
endif()

include(CTest)
if(BUILD_TESTING)
	add_test(
		NAME rhythm-game-self-test
		COMMAND gp2040-minigame-sim --self-test
	)
	add_test(
		NAME rhythm-game-bot-matrix
		COMMAND gp2040-minigame-sim --bot-matrix
	)
endif()
