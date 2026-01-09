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
	Razor3D* razor3d;
	Hairline* hairline;
	void renderScene(Scene& scene) const;
	void clear(Colour col);
private:
	// Pointers to our window and surface
	SDL_Surface* winSurface;
	SDL_Window* window;

	std::vector<uint32_t> bufScreen; 
	// This stuff is GPU
	SDL_Texture* screenTexture;
	SDL_Renderer* sdlRenderer;
};