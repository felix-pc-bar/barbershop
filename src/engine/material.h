#pragma once

#include <string>
#include <algorithm>
#include <cstdint>
#include <unordered_map>

class Colour // Store colour as decimal fractions of RGB
{
public:
	Colour();
	Colour(float r, float g, float b);
	Colour(float r, float g, float b, float a);
	Colour(uint32_t col);
	Colour(const std::string& gruvName); // Gruv colour from name
	uint32_t raw() const;
	friend Colour operator*(const Colour& c1, const float val);
	Colour& operator*=(const float val);

	
	float red;
	float green;
	float blue;
	float alpha;
private:
	static const std::unordered_map<std::string, Colour>& gruvCols() {
		static const std::unordered_map<std::string, Colour> table = {
			//bg
			{"darkred", Colour(0xFFcc241d)},
			{"darkgreen", Colour(0xFF98971a)},
			{"darkyellow", Colour(0xFFd79921)},
			{"darkblue", Colour(0xFF458588)},
			{"darkpurple", Colour(0xFFb16286)},
			{"darkaqua", Colour(0xFF689d6a)},
			{"gray5", Colour(0xFFa89984)},
			{"gray6", Colour(0xFF928374)},
			{"red", Colour(0xFFfb4934)},
			{"green", Colour(0xFFb8bb26)},
			{"yellow", Colour(0xFFfabd2f)},
			{"blue", Colour(0xFF83a598)},
			{"purple", Colour(0xFFd3869b)},
			{"aqua", Colour(0xFF8ec07c)},
			{"fg", Colour(0xFFebdbb2)},
			{"black", Colour(0xFF1d2021)}, //bg0_h
			{"grey", Colour(0xFF282828)},
			{"grey1", Colour(0xFF3c3836)},
			{"grey2", Colour(0xFF504945)},
			{"grey3", Colour(0xFF665c54)},
			{"grey4", Colour(0xFF7c6f64)},
			// gray5
			{"darkorange", Colour(0xFFd65d0e)},
			{"orange", Colour(0xFFfe8019)},
			// bg0_s
			{"white4", Colour(0xFFa89984)},
			{"white3", Colour(0xFFbdae93)},
			{"white2", Colour(0xFFd5c4a1)},
			{"white1", Colour(0xFFebdbb2)},
			{"white", Colour(0xFFfbf1c7)}
		};
		return table;
	};
};

class Material
{
public:
	Material();
	Material(Colour col, int pointSize = 0, bool shade = true, bool allowDebugVis = true, uint16_t ditherValue = 128);
	int pointWidth; //Size of squares to draw for points in scene
	Colour colour;
	bool shadeMat; //Whether to shade this material
	bool omitDbg; //Don't change shading in debug
	// Dither stuff
	uint16_t ditherValue; // set value offset for dithering (0-255, 128 by default)
};