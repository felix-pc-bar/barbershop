#pragma once

#include <SDL_render.h>
#include <SDL_surface.h>
#include <SDL_video.h>
#include <cstdint>
#include <vector>

#include "components/CPU2D.h"
#include "components/CPU3D.h"
#include "../engTools.h"

class cRenderer
{
public:
	cRenderer();
	~cRenderer();
	CPU3D* cpu3d;
	CPU2D* cpu2d;
	void renderScene(Scene& scene) const;
private:
	// Pointers to our window and surface
	SDL_Surface* winSurface;
	SDL_Window* window;

	std::vector<uint32_t> bufScreen; 
	// This stuff is GPU
	SDL_Texture* screenTexture;
	SDL_Renderer* sdlRenderer;
};