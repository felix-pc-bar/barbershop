#include <iostream>
#include <cstdlib>
// #include <cstdint>
#include <chrono>

#include <SDL_events.h>
#include <SDL_keyboard.h>
#include <SDL_scancode.h>
#include <SDL_stdinc.h>
#include <SDL_timer.h>
#include <SDL_keycode.h>

#include "game.h"
#include "render/render.h"
#include "material.h"

#include "../buffer.h"

using std::endl, std::cout;
using dtclock = std::chrono::steady_clock;

Game::Game()
{
	this->renderer = new cRenderer();
	// this->renderer = nullptr;
}

void Game::run()
{
	if (this->renderer == nullptr)
	{
		std::cout << "Error: renderer not initialised. Exiting..." << std::endl;
		return;
	}
	const Uint8* gk; // Used to read off inputs
	SDL_Event event; // SDL event buffer

	pixelBuffer pxbuf(100, 100, 2);

	// ====
	// TIME
	// ====

	int frame = 0;
	auto lastTime = dtclock::now();
	int fpsLimit = 0;
	float fpsLimTick = 1.0f / fpsLimit;
	float gameTime = 0.0f; //Time since game start, use for framerate independent motion eg trig anim
	float dtFac = 1.0f; // dt as ratio; shouldn't change anything if you multiply with it and you're running at 60fps

	// ===========
	// SCENE SETUP
	// ===========

	float fpsRunningTotal = 0;
	int runningAvgPeriodFrames = 30;
	do
	{
		auto currentTime = dtclock::now();
		std::chrono::duration<float> elapsed = currentTime - lastTime;
		float dt = elapsed.count(); // Raw frametime (s)
		gameTime += dt;
		if (fpsLimit != 0 && dt < fpsLimTick)
		{
			SDL_Delay((fpsLimTick - dt) * 1000);
			dt += fpsLimTick - dt;
			currentTime = dtclock::now();
		}
		float dtMulti = dt / (1.0f / globfpsTarget);
		lastTime = currentTime;
		float fps = 1.0f / dt;
		dtFac = dt * 60.0f;
		fpsRunningTotal += fps;
		if (frame % runningAvgPeriodFrames == 0)
		{
			cout << fpsRunningTotal / (float)runningAvgPeriodFrames << endl;
			fpsRunningTotal = 0;
		}

		// Handle inputs
		gk = SDL_GetKeyboardState(NULL);
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT || gk[SDL_SCANCODE_ESCAPE]) {
				break;
			}
			if (event.type == SDL_MOUSEMOTION)
			{
				// mouse
			}
			if (event.type == SDL_MOUSEWHEEL)
			{
				// wheel
			}
			if (event.type == SDL_KEYDOWN && event.key.repeat == 0)
			{
				if (event.key.keysym.sym == SDLK_z)
				{
					// etc
				}
			}
		}

		// Key hold events here
		if (gk[SDL_SCANCODE_W])
		{
			// etc
		}

		this->renderer->clear({0x000000});
		this->renderer->renderScene(pxbuf);
		frame++;
	}
	while (event.type != SDL_QUIT && !gk[SDL_SCANCODE_ESCAPE]);

	return;
	// lol, lmao even
	system("pause");

	//SDL_Quit();

	return;
}

Game::~Game()
{
	//SDL_DestroyTexture(this->screenTexture);
}
