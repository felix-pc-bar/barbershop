#pragma once
#include "SDL_keycode.h"
#include "render/components/bmpfont.h"
#include "stubble/stubble.h"
#include "render/render.h"

#include <chrono>
#include <SDL_events.h>

class Game
{
public:
	Game();
	~Game();
	void run();
	void quit();
	cRenderer* renderer;
	StubbleParser* stubbleparser;
private:

	const Uint8* gk; // Used to read off inputs
	SDL_Event event; // SDL event buffer
	SDL_Keymod mods;

	// ====
	// TIME
	// ====

	int frame;
	std::chrono::steady_clock::time_point lastTime;
	int fpsLimit;
	float fpsLimTick;
	float gameTime; //Time since game start, use for framerate independent motion eg trig anim
	float dtFac; // dt as ratio; shouldn't change anything if you multiply with it and you're running at 60fps

	float fpsRunningTotal;
	int runningAvgPeriodFrames;

	// buff stuff
	std::vector<pixelBuffer*> pxbufs;

	std::vector<int> rulers; // These are horizontal lines to help with typography; the int stores the number of pixels above the bottom of the normal font the line should be drawn (respecting zoom)
	int previewRuleHeight;
	bool previewingRule; // For the rule being previewed before it is placed

	bmpFont* testFont;
	int defaultWidth;
	int defaultHeight;
	// these are the "camera" offset
	int dx;
	int dy;

	// zoom
	int scale;

	// in screen space,y-up
	int mousex;
	int mousey;

	// 0,0 when origin is at centre of screen; origin @ bottom left of screen -> (-ve,-ve)
	int originDX;
	int originDY;

	std::vector<Point2d> bufrels;
	bool lmbdown;
	bool mmbdown;

	enum class state
	{
		roughing_normal,
		tweaking_normal
	};

	enum class tool
	{
		pen,
		line,
		circle,
		placing_ruler
	};

	state currentState;
	tool currentTool;
	bool unsavedWork;

	// == Functions ==
	void dealFontBuffers(bmpFont* font); // Copies the buffers of a font into the pxbufs vector to be displayed, for inspection and editing
	void createUndoState();
};
