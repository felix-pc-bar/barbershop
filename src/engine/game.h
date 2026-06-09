#pragma once
#include "stubble/stubble.h"
#include "render/render.h"

class Game
{
public:
	Game();
	~Game();
	void run();
	void quit();
	cRenderer* renderer;
	StubbleParser* stubbleparser;
};
