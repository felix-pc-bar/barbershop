#pragma once
#include <SDL.h>
#include "ARenderer.h"
#include "../logic2d.h"
#include "../engTools.h"

class CPURenderer : public ARenderer{
public:
	CPURenderer(SDL_Texture* screentex, SDL_Renderer* renderer, int width, int height); //constructor

	void Clear(uint32_t color) override;
	void Present(); // push pixels to texture and draw to screen
	void drawScene(Scene& scene) override;
	void drawTri(Vertex3d& v1,Vertex3d& v2,Vertex3d& v3, Material& mat) override; // Draw a triangle in screen space
	void SetPixel(int x, int y, uint32_t color) override;
	void drawPoint(Position3d pos, int sizePx) override;
private:
	SDL_Renderer* sdlRenderer;
	SDL_Texture* texture;
	std::vector<uint32_t> bufShaded; // Shaded pixel buffer
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

