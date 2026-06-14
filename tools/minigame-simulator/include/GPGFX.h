/*
 * SPDX-License-Identifier: MIT
 */

#ifndef HOST_GPGFX_H_
#define HOST_GPGFX_H_

#include <cstdint>
#include <iosfwd>
#include <string>
#include <vector>

class GPGFX {
public:
	GPGFX(uint16_t width = 128, uint16_t height = 64);

	void clearScreen();
	void render() {}

	uint32_t getPixel(uint16_t x, uint16_t y) const;
	void drawPixel(uint16_t x, uint16_t y, uint32_t color);
	void drawText(uint16_t x, uint16_t y, std::string text, uint8_t invert = 0);
	void drawLine(
		uint16_t x1,
		uint16_t y1,
		uint16_t x2,
		uint16_t y2,
		uint32_t color,
		uint8_t filled
	);
	void drawRectangle(
		uint16_t x,
		uint16_t y,
		uint16_t right,
		uint16_t bottom,
		uint32_t color,
		uint8_t filled,
		double rotationAngle = 0
	);

	void present(std::ostream& output) const;
	size_t litPixelCount() const;
	const std::string& textRow(size_t row) const;

private:
	uint16_t width;
	uint16_t height;
	std::vector<uint8_t> pixels;
	std::vector<std::string> textRows;
};

#endif
