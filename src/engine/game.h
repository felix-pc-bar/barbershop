#pragma once
#include <vector>
#include "engTools.h"
#include "SDL.h"
#include "render/ARenderer.h"

class Game
{
public:
	Game();
	~Game();
	void run();
	ARenderer* currentRenderer;
private:
	std::vector<Scene> scenes;

	// Pointers to our window and surface
	SDL_Surface* winSurface;
	SDL_Window* window;
	// This stuff is GPU
	SDL_Texture* screenTexture;
	SDL_Renderer* sdlRenderer;
};