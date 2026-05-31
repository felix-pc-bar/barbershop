#pragma once
#include <vector>
#include "general3d.h"
#include "render/render.h"

class Game
{
public:
	Game();
	~Game();
	void run();
	//CPU3DRenderer* cpu3d;
	cRenderer* renderer;

private:
	std::vector<Scene> scenes;

	// Pointers to our window and surface
	//SDL_Surface* winSurface;
	//SDL_Window* window;
	//// This stuff is GPU
	//SDL_Texture* screenTexture;
	//SDL_Renderer* sdlRenderer;
};