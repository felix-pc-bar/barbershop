#pragma once

#include <cstdint>
#include <vector>
#include <SDL_render.h>

#include "../../general3d.h"

class Razor3D{
public:
	Razor3D(int width, int height, std::vector<uint32_t>* screenbuffer, bool renderdithered = false); //constructor

	void clear(uint32_t color);
	void drawTri(Vertex3d& v1,Vertex3d& v2,Vertex3d& v3, Material& mat, Camera* cam, const cRenderer* renderer = nullptr); // Draw a triangle in screen space

	std::vector<uint32_t>* bufMain; 
	std::vector<float> bufDepth; // Depth pixel buffer
	std::vector<bool> bufIsDrawn; //Whether the background has been shaded

	int width;
	int height;
	bool dither;
	static inline constexpr std::uint8_t bayer8x8[8][8] = {
	{  2, 193,  48, 239,  14, 205,  60, 251 },
	{129,  66, 177, 112, 141,  78, 189, 124 },
	{ 34, 225,  18, 209,  44, 235,  30, 221 },
	{161,  98, 145,  82, 173, 110, 157,  94 },
	{ 10, 201,  56, 247,   6, 197,  52, 243 },
	{137,  74, 185, 120, 133,  70, 181, 116 },
	{ 42, 233,  26, 217,  38, 229,  22, 213 },
	{169, 106, 153,  90, 165, 102, 149,  86 }
	};	
};

struct TriangleToRender 
{
	Vertex3d v1, v2, v3;
	float distanceToCamera;
	Material material;
	TriangleToRender(const Vertex3d& a, const Vertex3d& b, const Vertex3d& c, const Position3d& camPos, Material* mat);
};

struct PointToRender 
{
	Position3d pos;
	float distanceToCamera;
	Material material;
	PointToRender(Position3d Pos, const Position3d& camPos, Material* mat);
};

