#include <algorithm>
#include <cmath>
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

#include "../../external/tinyfiledialogs/tinyfiledialogs.c"

#include "game.h"
#include "SDL.h"
#include "general3d.h"
#include "globals.h"
#include "render/render.h"
#include "material.h"
#include "stubble/stubble.h"
#include "helpers/stringy.h"

#include "buffer.h"

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

	StubbleParser sp;

	// buff stuff
	std::vector<pixelBuffer> pxbufs;
	pxbufs.emplace_back(pixelBuffer(64, 64, 0, 0, 0));
	// pxbufs.emplace_back(pixelBuffer(151, 163,  100, 300, 0));

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

	enum class state
	{
		normal
	};
	state currentState = state::normal;
	bool unsavedWork = false;

	float fpsRunningTotal = 0;
	int runningAvgPeriodFrames = 30;
	for (;;)
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
				if (unsavedWork)
				{
					std::string response;
					std::cout << "You have unsaved work. Really quit? (y/N)\n";
					std::cin >> response;
					if (helpers::toLower(response) == "y") { return; }
				}
				else { return; }
			}
			if (event.button.button == SDL_BUTTON_MIDDLE && event.type == SDL_MOUSEBUTTONDOWN) { mmbdown = true; }
			if (event.button.button == SDL_BUTTON_MIDDLE && event.type == SDL_MOUSEBUTTONUP) { mmbdown = false; }
			if (event.button.button == SDL_BUTTON_LEFT && event.type == SDL_MOUSEBUTTONDOWN) { lmbdown = true; }
			if (event.button.button == SDL_BUTTON_LEFT && event.type == SDL_MOUSEBUTTONUP) { lmbdown = false; }
			if (event.type == SDL_MOUSEMOTION)
			{
				// mouse
				if (mmbdown)
				{
					dx += event.motion.xrel;
					dy -= event.motion.yrel;
				}

				mousex = event.motion.x;
				mousey = globScreenheight - event.motion.y;
			}

			if (currentState == state::normal)
			{
				if (lmbdown)
				{
					for (auto& pb : pxbufs)
					{
						int mouserelx = mousex - (dx + (pb.displayDX * scale));
						int mouserely = mousey - (dy + (pb.displayDY * scale));
						pb.set( mouserelx / scale, mouserely / scale, true);
					}
				}
				if (event.type == SDL_MOUSEWHEEL)
				{
					// wheel
					if (event.wheel.y > 0)
					{
						// TODO: zoom on centre of screen
						scale = std::round(scale * 1.75);
					}
					if (event.wheel.y < 0 && scale > 1)
					{
						scale = std::max(static_cast<int>(std::round(scale / 1.75)), 1);
					}
				}

				// Key hold events here

				// if (gk[SDL_SCANCODE_LEFT]) { dx -= 20 * dtFac; }
				// if (gk[SDL_SCANCODE_RIGHT]) { dx += 20 * dtFac; }
				// if (gk[SDL_SCANCODE_DOWN]) { dy -= 20 * dtFac; }
				// if (gk[SDL_SCANCODE_UP]) { dy += 20 * dtFac; }

				if (event.type == SDL_KEYDOWN && event.key.repeat == 0)
				{
					if (event.key.keysym.sym == SDLK_s)
					{
						const char* fp = tinyfd_saveFileDialog(
							"Save font",
							"font.stbbl",
							0,
							NULL,
							NULL
						);
						sp.stbExport(&pxbufs[0], fp);
					}
					if (event.key.keysym.sym == SDLK_l)
					{
						const char* filters[] = { "*.stbbl" };
						
						const char* file = tinyfd_openFileDialog(
							"Open font",
							"",
							1,
							filters,
							"Stubble files",
							0
						);
						if (file != NULL)
						{
							auto returned = sp.import(file);
							if (returned.has_value())
							{
								pxbufs[0] = *std::get<pixelBuffer*>(returned.value());
							}
						}
					}
				}
			} // normal state

			// if (currentState == blah)
			// {
			// 	if (event.type == SDL_MOUSEWHEEL)
			// 	{}
			// 	if (gk[SDL_SCANCODE_A]) { dowhatever(); }
			// 	if (event.type == SDL_KEYDOWN && event.key.repeat == 0)
			// 	{
			// 		if (event.key.keysym.sym == SDLK_s)
			// 		{}
			// 	}
			// }
		} // event loop

		this->renderer->clear({0xFF202020});
		for (auto pb : pxbufs)
		{
			this->renderer->hairline->transformPixelBuffer(pb, dx + (pb.displayDX * scale), dy + (pb.displayDY * scale), scale, scale > 10); //add borders at higher scale
		}
		this->renderer->renderScene();
		frame++;
	}
	return;
}

void Game::quit()
{
	SDL_Quit();
	return;
}

Game::~Game()
{
	//SDL_DestroyTexture(this->screenTexture);
}
