#pragma once

#include <SDL_render.h>
#include <SDL_surface.h>
#include <SDL_video.h>
#include <cstdint>
#include <vector>

#include "components/CPU2D.h"
#include "../globals.h"
#include "../material.h"

class cRenderer // Composite render class housing other specialised renderer objects
{
public:
	cRenderer(int width = globScreenwidth, int height = globScreenheight);
	~cRenderer();
	void resize(int newWidth, int newHeight); // Broken lol
	Hairline* hairline;

	void renderScene() const;
	void clear(Colour col);
	
	// Clear display buffer in a gradient from bottom to top of screen.
	// If fac values are selected, they should be 0 to 1, with bot being lower than top to retain the bottom/topness of specified colours.
	int width;
	int height;
private:
	// Pointers to our window and surface
	SDL_Surface* winSurface;
	SDL_Window* window;

	std::vector<uint32_t> bufScreen; 
	// This stuff is GPU
	SDL_Texture* screenTexture;
	SDL_Renderer* sdlRenderer;
};
