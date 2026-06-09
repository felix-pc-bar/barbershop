#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <optional>

class pixelBuffer
{
public:
	int width;
	std::vector<bool> values;

	// stores delta position, for displaying only
	int displayDX;
	int displayDY;

	pixelBuffer() = default;
	pixelBuffer(int _width, int _height, int dDX = 0, int dDY = 0, bool fill = false);
	pixelBuffer(int _width, std::string b64data);

	void set(int x, int y, bool);
	int height();
	std::string bufB64();
private:
	// b64 translation written by claude
	static std::vector<uint8_t> pack(const std::vector<bool>& bits);
	static std::optional<std::vector<bool>> unpack(const std::vector<uint8_t>& data);
	static std::string b64Encode(const std::vector<uint8_t>& data);
	static std::vector<uint8_t> b64Decode(const std::string& s);
};
