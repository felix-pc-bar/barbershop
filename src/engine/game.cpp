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
#include "globals.h"
#include "render/render.h"
#include "material.h"

#include "buffer.h"

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

	// ====
	// TIME
	// ====

	int frame = 0;
	auto lastTime = dtclock::now();
	int fpsLimit = 0;
	float fpsLimTick = 1.0f / fpsLimit;
	float gameTime = 0.0f; //Time since game start, use for framerate independent motion eg trig anim
	float dtFac = 1.0f; // dt as ratio; shouldn't change anything if you multiply with it and you're running at 60fps

	// buff stuff
	std::vector<bufData> pxbufs;
	pxbufs.emplace_back(bufData{{63, 63, 0}, 0, 0});
	pxbufs.emplace_back(bufData{{151, 163, 0}, 100, 300});

	// these are the "camera" offset
	int dx = 0;
	int dy = 0;

	// zoom
	int scale = 1;

	// in screen space,y-up
	int mousex = 0;
	int mousey = 0;

	std::vector<Point2d> bufrels;
	bufrels.emplace_back();

	bool lmbdown = false;
	bool mmbdown = false;

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
			// cout << fpsRunningTotal / (float)runningAvgPeriodFrames << endl;
			fpsRunningTotal = 0;
		}

		// Handle inputs
		gk = SDL_GetKeyboardState(NULL);
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT || gk[SDL_SCANCODE_ESCAPE]) 
			{
				break;
			}
			if (event.button.button == SDL_BUTTON_MIDDLE && event.type == SDL_MOUSEBUTTONDOWN) { mmbdown = true; }
			if (event.button.button == SDL_BUTTON_MIDDLE && event.type == SDL_MOUSEBUTTONUP) { mmbdown = false; }
			if (event.button.button == SDL_BUTTON_LEFT && event.type == SDL_MOUSEBUTTONDOWN) { lmbdown = true; }
			if (event.button.button == SDL_BUTTON_LEFT && event.type == SDL_MOUSEBUTTONUP) { lmbdown = false; }
			if (event.type == SDL_MOUSEMOTION)
			{
				// mouse
				// std::cout << "Mouse Motion Detected - "
				// 	<< "x: " << event.motion.x
				// 	<< ", y: " << event.motion.y << '\n';
				if (mmbdown)
				{
					dx += event.motion.xrel;
					dy -= event.motion.yrel;
				}

				mousex = event.motion.x;
				mousey = globScreenheight - event.motion.y;
			}
			if (lmbdown)
			{
				for (auto& bd : pxbufs)
				{
					int mouserelx = mousex - (dx + (bd.offsetx * scale));
					int mouserely = mousey - (dy + (bd.offsety * scale));
					bd.pb.set( mouserelx / scale, mouserely / scale, 1);
				}
			}
			if (event.type == SDL_MOUSEWHEEL)
			{
				// wheel
				if (event.wheel.y > 0)
				{
					// try to zoom on centre of screen
					dx += (dx - (this->renderer->width / 2)) / (2 * scale);
					dy += (dy - (this->renderer->height / 2)) / (2 * scale);
					scale++;
				}
				if (event.wheel.y < 0 && scale > 1)
				{
					dx -= (dx - (this->renderer->width / 2)) / (2 * scale);
					dy -= (dy - (this->renderer->height / 2)) / (2 * scale);
					scale--;
				}
			}
			if (event.type == SDL_KEYDOWN && event.key.repeat == 0)
			{
				if (event.key.keysym.sym == SDLK_z)
				{
					// etc
				}
				// if (event.key.keysym.sym == SDLK_z) { scale++; }
				// if (event.key.keysym.sym == SDLK_x && scale > 1) { scale--; }
			}
		}

		// Key hold events here
		// if (gk[SDL_SCANCODE_LEFT]) { dx -= 20 * dtFac; }
		// if (gk[SDL_SCANCODE_RIGHT]) { dx += 20 * dtFac; }
		// if (gk[SDL_SCANCODE_DOWN]) { dy -= 20 * dtFac; }
		// if (gk[SDL_SCANCODE_UP]) { dy += 20 * dtFac; }



		this->renderer->clear({0xFF202020});
		for (auto bd : pxbufs)
		{
			this->renderer->hairline->transformPixelBuffer(bd.pb, dx + (bd.offsetx * scale), dy + (bd.offsety * scale), scale, scale > 10); //add borders at higher scale
		}
		this->renderer->renderScene();
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
