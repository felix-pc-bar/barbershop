#pragma once
#include "render/render.h"

class Game
{
public:
	Game();
	~Game();
	void run();
	void quit();
	cRenderer* renderer;
};
