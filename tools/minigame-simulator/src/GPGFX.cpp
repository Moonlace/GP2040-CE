/*
 * SPDX-License-Identifier: MIT
 */

#include "GPGFX.h"

#include <algorithm>
#include <cstdlib>
#include <ostream>

GPGFX::GPGFX(uint16_t width, uint16_t height) :
	width(width),
	height(height),
	pixels(static_cast<size_t>(width) * height, 0),
	textRows(std::max<size_t>(1, height / 8), "") {
}

void GPGFX::clearScreen() {
	std::fill(pixels.begin(), pixels.end(), uint8_t {0});
	std::fill(textRows.begin(), textRows.end(), "");
}

uint32_t GPGFX::getPixel(uint16_t x, uint16_t y) const {
	if (x >= width || y >= height) {
		return 0;
	}
	return pixels[static_cast<size_t>(y) * width + x];
}

void GPGFX::drawPixel(uint16_t x, uint16_t y, uint32_t color) {
	if (x >= width || y >= height) {
		return;
	}
	pixels[static_cast<size_t>(y) * width + x] = color == 0 ? 0 : 1;
}

void GPGFX::drawText(uint16_t x, uint16_t y, std::string text, uint8_t invert) {
	(void)invert;
	if (y >= textRows.size()) {
		return;
	}
	if (textRows[y].size() < x) {
		textRows[y].resize(x, ' ');
	}
	if (textRows[y].size() < x + text.size()) {
		textRows[y].resize(x + text.size(), ' ');
	}
	textRows[y].replace(x, text.size(), text);
}

void GPGFX::drawLine(
	uint16_t x1,
	uint16_t y1,
	uint16_t x2,
	uint16_t y2,
	uint32_t color,
	uint8_t filled
) {
	(void)filled;
	int x = x1;
	int y = y1;
	const int targetX = x2;
	const int targetY = y2;
	const int stepX = x < targetX ? 1 : -1;
	const int stepY = y < targetY ? 1 : -1;
	const int deltaX = std::abs(targetX - x);
	const int deltaY = -std::abs(targetY - y);
	int error = deltaX + deltaY;

	while (true) {
		drawPixel(static_cast<uint16_t>(x), static_cast<uint16_t>(y), color);
		if (x == targetX && y == targetY) {
			break;
		}
		const int doubledError = error * 2;
		if (doubledError >= deltaY) {
			error += deltaY;
			x += stepX;
		}
		if (doubledError <= deltaX) {
			error += deltaX;
			y += stepY;
		}
	}
}

void GPGFX::drawRectangle(
	uint16_t x,
	uint16_t y,
	uint16_t rectangleWidth,
	uint16_t rectangleHeight,
	uint32_t color,
	uint8_t filled,
	double rotationAngle
) {
	(void)rotationAngle;
	if (rectangleWidth == 0 || rectangleHeight == 0) {
		return;
	}

	if (filled) {
		for (uint16_t row = 0; row < rectangleHeight; row++) {
			drawLine(x, y + row, x + rectangleWidth - 1, y + row, color, 0);
		}
		return;
	}

	drawLine(x, y, x + rectangleWidth - 1, y, color, 0);
	drawLine(x, y, x, y + rectangleHeight - 1, color, 0);
	drawLine(
		x + rectangleWidth - 1,
		y,
		x + rectangleWidth - 1,
		y + rectangleHeight - 1,
		color,
		0
	);
	drawLine(
		x,
		y + rectangleHeight - 1,
		x + rectangleWidth - 1,
		y + rectangleHeight - 1,
		color,
		0
	);
}

void GPGFX::present(std::ostream& output) const {
	static const char density[] = " .:*#";
	const uint16_t blockWidth = 2;
	const uint16_t blockHeight = 4;

	output << "\x1b[H";
	if (!textRows.empty()) {
		output << textRows[0] << '\n';
	}

	for (uint16_t y = 8; y < height; y += blockHeight) {
		for (uint16_t x = 0; x < width; x += blockWidth) {
			uint8_t count = 0;
			for (uint16_t offsetY = 0; offsetY < blockHeight; offsetY++) {
				for (uint16_t offsetX = 0; offsetX < blockWidth; offsetX++) {
					count += static_cast<uint8_t>(getPixel(x + offsetX, y + offsetY) != 0);
				}
			}
			const size_t densityIndex = count == 0 ? 0 :
				std::min<size_t>(4, 1 + (count - 1) / 2);
			output << density[densityIndex];
		}
		output << '\n';
	}
}

size_t GPGFX::litPixelCount() const {
	return static_cast<size_t>(std::count(pixels.begin(), pixels.end(), 1));
}

const std::string& GPGFX::textRow(size_t row) const {
	static const std::string empty;
	return row < textRows.size() ? textRows[row] : empty;
}
