#pragma once

#include <cstdint>
#include <vector>
#include <SDL_render.h>

//#include "ARenderer.h"
#include "../../engTools.h"

class CPU2D{
public:
	CPU2D(SDL_Texture* screentex, SDL_Renderer* renderer, int width, int height, std::vector<uint32_t>* screenbuffer); //constructor

	void Clear(uint32_t color);
	void Present(); // push pixels to texture and draw to screen
	void SetPixel(int x, int y, uint32_t color);
	void drawPoint(Position3d pos, int sizePx);
	SDL_Renderer* sdlRenderer;
	SDL_Texture* texture;
	std::vector<uint32_t>* bufMain; // Shaded pixel buffer
	int width;
	int height;
};
