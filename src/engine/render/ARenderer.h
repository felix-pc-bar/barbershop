#pragma once
#include "../engTools.h"
#include <cstdint>

class ARenderer //Generic class outlining the functions of renderer components
{
public:
	virtual ~ARenderer() = default;
	virtual void Clear(uint32_t color) = 0;
	virtual void Present() = 0; // push pixels to texture and draw to screen
	virtual void drawScene(Scene& scene) = 0;
	virtual void drawTri(Vertex3d& v1,Vertex3d& v2,Vertex3d& v3, Material& mat) = 0; // Draw a triangle in screen space
	virtual void SetPixel(int x, int y, uint32_t color) = 0;
	virtual void drawPoint(Position3d pos, int sizePx) = 0;
};