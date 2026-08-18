#pragma once

#include <SDL_render.h>
#include <SDL_surface.h>
#include <SDL_video.h>
#include <cstdint>
#include <vector>

#include "components/CPU2D.h"
#include "../globals.h"
#include "../material.h"

class LayoutElement;

class cRenderer // Composite render class housing other specialised renderer objects
{
public:
	cRenderer(int width = globScreenwidth, int height = globScreenheight);
	~cRenderer();
	void resize(int newWidth, int newHeight); // Broken lol
	Hairline* hairline;

	void renderScene();
	void clear(Colour col);
	
	int width;
	int height;

	std::vector<uint32_t> bufScreen;

	LayoutElement* UI;
private:
	// Pointers to our window and surface
	SDL_Surface* winSurface;
	SDL_Window* window;


	// This stuff is GPU
	SDL_Texture* screenTexture;
	SDL_Renderer* sdlRenderer;
};
