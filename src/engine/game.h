#pragma once
#include <vector>
#include "render/render.h"

class Game
{
public:
	Game();
	~Game();
	void run();
	void quit();
	//CPU3DRenderer* cpu3d;
	cRenderer* renderer;

private:
	// Pointers to our window and surface
	//SDL_Surface* winSurface;
	//SDL_Window* window;
	//// This stuff is GPU
	//SDL_Texture* screenTexture;
	//SDL_Renderer* sdlRenderer;
};
