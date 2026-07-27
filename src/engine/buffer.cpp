#include <iostream>
#include <optional>
#include <array>

#include "buffer.h"

static constexpr char kB64Chars[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

pixelBuffer::pixelBuffer(int _width, int _height, int dDX, int dDY, bool fill)
{
	if (fill == true)
	{
		for (long int i = 0; i < _width * _height; i++)
		{
			this->values.emplace_back(i % 2);
		}
	}
	else
	{
		this->width = _width;
		for (long int i = 0; i < _width * _height; i++)
		{
			this->values.emplace_back(fill);
		}
	}
	this->width = _width;
	this->displayDX = dDX;
	this->displayDY = dDY;
}

pixelBuffer::pixelBuffer(int _width, std::string b64data)
{
	this->displayDX = 0;
	this->displayDY = 0;
	this->width = _width;
	auto returned = unpack(b64Decode(b64data));
	if (!returned.has_value())
	{
		std::cout << "Error: failed to build pixelBuffer from b64 data; returning empty buffer..." << std::endl;
		return;
	}
	this->values = returned.value();
}

void pixelBuffer::set(int x, int y, bool value)
{
	if (y * this->width + x < this->values.size() && x < this->width && x >= 0) 
	{
		this->values[y * this->width + x] = value;
	}
	return;
}

int pixelBuffer::height()
{
	return static_cast<int>(this->values.size() / this->width);
}

// Pack bits MSB-first into bytes, prepend 4-byte LE bit count
std::vector<uint8_t> pixelBuffer::pack(const std::vector<bool>& bits) {
	uint32_t n = static_cast<uint32_t>(bits.size());
	std::vector<uint8_t> out;
	out.reserve(4 + (n + 7) / 8);

	out.push_back( n        & 0xFF);
	out.push_back((n >>  8) & 0xFF);
	out.push_back((n >> 16) & 0xFF);
	out.push_back((n >> 24) & 0xFF);

	for (uint32_t i = 0; i < n; i += 8) {
		uint8_t byte = 0;
		for (int b = 0; b < 8 && (i + b) < n; ++b)
			if (bits[i + b]) byte |= uint8_t(0x80) >> b;
		out.push_back(byte);
	}
	return out;
}

std::optional<std::vector<bool>> pixelBuffer::unpack(const std::vector<uint8_t>& data) {
	if (data.size() < 4)
	{
		std::cout << "too short for header" << std::endl;
		return std::nullopt;
	}
	uint32_t n = data[0]
		| (uint32_t(data[1]) <<  8)
		| (uint32_t(data[2]) << 16)
		| (uint32_t(data[3]) << 24);

	if (data.size() < 4 + (n + 7) / 8)
	{
		std::cout << "data truncated" << std::endl;
		return std::nullopt;
	}

	std::vector<bool> bits(n);
	for (uint32_t i = 0; i < n; ++i)
		bits[i] = (data[4 + i / 8] >> (7 - i % 8)) & 1;

	return bits;
}

std::string pixelBuffer::b64Encode(const std::vector<uint8_t>& data) {
	std::string out;
	out.reserve(((data.size() + 2) / 3) * 4);

	for (size_t i = 0; i < data.size(); i += 3) {
		uint32_t v = uint32_t(data[i]) << 16;
		if (i + 1 < data.size()) v |= uint32_t(data[i + 1]) << 8;
		if (i + 2 < data.size()) v |= uint32_t(data[i + 2]);

		out += kB64Chars[(v >> 18) & 0x3F];
		out += kB64Chars[(v >> 12) & 0x3F];
		out += (i + 1 < data.size()) ? kB64Chars[(v >> 6) & 0x3F] : '=';
		out += (i + 2 < data.size()) ? kB64Chars[(v >> 0) & 0x3F] : '=';
	}
	return out;
}

std::vector<uint8_t> pixelBuffer::b64Decode(const std::string& s) {
	// Thread-safe static init (C++11 guarantee)
	static const auto kTable = []() {
		std::array<int8_t, 256> t;
		t.fill(-1);
		for (int i = 0; i < 64; ++i)
			t[uint8_t(kB64Chars[i])] = int8_t(i);
		return t;
	}();

	std::vector<uint8_t> out;
	out.reserve(s.size() * 3 / 4);

	uint32_t accum = 0;
	int pending = 0;
	for (char c : s) {
		if (c == '=') break;
		int8_t v = kTable[uint8_t(c)];
		if (v < 0) continue;           // tolerates whitespace
		accum = (accum << 6) | uint8_t(v);
		pending += 6;
		if (pending >= 8) {
			pending -= 8;
			out.push_back(uint8_t(accum >> pending));
			accum &= (1u << pending) - 1;
		}
	}
	return out;
}

std::string pixelBuffer::bufB64()
{
	return b64Encode(pack(this->values));
}
