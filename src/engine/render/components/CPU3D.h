#pragma once

#include <cstdint>
#include <vector>
#include <SDL_render.h>

//#include "ARenderer.h"
#include "../../engTools.h"

class CPU3D{
public:
	CPU3D(SDL_Texture* screentex, SDL_Renderer* renderer, int width, int height, std::vector<uint32_t>* screenbuffer); //constructor

	void Clear(uint32_t color);
	void Present(); // push pixels to texture and draw to screen
	void drawScene(Scene& scene);
	void drawTri(Vertex3d& v1,Vertex3d& v2,Vertex3d& v3, Material& mat); // Draw a triangle in screen space
	//void SetPixel(int x, int y, uint32_t color);
	//void drawPoint(Position3d pos, int sizePx);
	SDL_Renderer* sdlRenderer;
	SDL_Texture* texture;
	std::vector<uint32_t>* bufMain; 
	std::vector<float> bufDepth; // Depth pixel buffer
	std::vector<bool> bufIsDrawn; //Whether the background has been shaded
	int width;
	int height;
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

