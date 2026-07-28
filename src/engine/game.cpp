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
#include "render/components/bmpfont.h"
#include "render/render.h"
#include "material.h"
#include "stubble/stubble.h"
#include "helpers/stringy.h"

#include "buffer.h"

using dtclock = std::chrono::steady_clock;

Game::Game()
{
	if (!globDeferGFXcreation)
	{
		this->renderer = new cRenderer();
	}
	this->stubbleparser = new StubbleParser();
	// this->renderer = nullptr;

	frame = 0;
	lastTime = dtclock::now();
	fpsLimit = 0;
	fpsLimTick = 1.0f / fpsLimit;
	gameTime = 0.0f; //Time since game start, use for framerate independent motion eg trig anim
	dtFac = 1.0f; // dt as ratio; shouldn't change anything if you multiply with it and you're running at 60fps

	previewRuleHeight = 0;

	testFont = new bmpFont();
	testFont->name = "Test font";

	std::cout << "Enter default width for new font: ";
	std::cin >> defaultWidth;
	std::cout << "\nEnter default height for new font: ";
	std::cin >> defaultHeight;
	std::cout << std::endl;

	testFont->sizepx = defaultHeight;

	for (int i = 0; i < testFont->glyphs.size(); i++)
	{
		testFont->glyphs[i] = new bmpGlyph();
		bmpGlyph* currentGlyph = testFont->glyphs[i];
		currentGlyph->bitmap = new pixelBuffer(defaultWidth, defaultHeight);
		currentGlyph->bitmap->displayDX = i * (defaultWidth + 5);
		pxbufs.emplace_back(currentGlyph->bitmap);
	}

	// these are the "camera" offset
	dx = globScreenwidth / 2;
	dy = globScreenheight / 2;

	// zoom
	scale = 1;

	// in screen space,y-up
	mousex = 0;
	mousey = 0;

	// 0,0 when origin is at centre of screen; origin @ bottom left of screen -> (-ve,-ve)
	originDX = 0;
	originDY = 0;

	bufrels.emplace_back();

	lmbdown = false;
	mmbdown = false;

	currentState = state::roughing_normal;
	currentTool = tool::pen;
	unsavedWork = false;
	fpsRunningTotal = 0;
	runningAvgPeriodFrames = 30;
}

void Game::run()
{
	if (this->renderer == nullptr && !globDeferGFXcreation)
	{
		std::cout << "Error: renderer not initialised. Exiting..." << std::endl;
		return;
	}
	else
	{
		this->renderer = new cRenderer(globScreenwidth, globScreenheight);
	}

	if (this->renderer == nullptr)
	{
		std::cout << "Error: renderer not initialised. Exiting..." << std::endl;
		return;
	}

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
			if (gk[SDL_SCANCODE_G])
			{
				mmbdown = true;
			} else { mmbdown = false; }
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
					originDX = dx - (globScreenwidth / 2);
					originDY = dy - (globScreenheight / 2);
				}

				mousex = event.motion.x;
				mousey = globScreenheight - event.motion.y;
			}

			if (currentState == state::roughing_normal)
			{
				if (currentTool == tool::pen)
				{
					if (lmbdown)
					{
						for (auto& pb : pxbufs)
						{
							int mouserelx = mousex - (dx + (pb->displayDX * scale));
							int mouserely = mousey - (dy + (pb->displayDY * scale));
							pb->set( mouserelx / scale, mouserely / scale, true);
						}
					}
				}
				if (currentTool == tool::placing_ruler)
				{
					previewRuleHeight = std::round(((mousey - dy) / scale) + 0.5f);
					if (lmbdown)
					{
						rulers.emplace_back(previewRuleHeight);
						currentTool = tool::pen;
					}
				}
				if (event.type == SDL_MOUSEWHEEL)
				{
					// wheel
					int mouseDX = dx - mousex;
					int mouseDY = dy - mousey;
					int deltaScale = 0;
					if (event.wheel.y > 0 && scale < 64)
					{
						deltaScale = std::round(scale * 1.75 - scale);
					}
					if (event.wheel.y < 0 && scale > 1)
					{
						deltaScale = std::round(scale / 1.75 - scale);
					}
					float scaleMultiplicand = (static_cast<float>(scale + deltaScale) / static_cast<float>(scale)) - 1.0f;
					scale += deltaScale;
					dx += mouseDX * scaleMultiplicand;
					dy += mouseDY * scaleMultiplicand;
				}

				// Key hold events here
				// if (gk[SDL_SCANCODE_LEFT]) { dx -= 20 * dtFac; }

				if (event.type == SDL_KEYDOWN && event.key.repeat == 0)
				{
					if (event.key.keysym.sym == SDLK_HOME)
					{
						dx = globScreenwidth / 2;
						dy = globScreenheight / 2;
					}
					if (event.key.keysym.sym == SDLK_r)
				{
						if (currentTool != tool::placing_ruler) { currentTool = tool::placing_ruler; }
						else { currentTool = tool::pen; }
					}
					if (event.key.keysym.sym == SDLK_c)
					{
						rulers.clear();
					}
					if (event.key.keysym.sym == SDLK_s)
					{
						const char* fp = tinyfd_saveFileDialog(
							"Save font",
							"font.stbbl",
							0,
							NULL,
							NULL
						);
						// this->stubbleparser->stbExport(pxbufs[0], fp);
						this->stubbleparser->stbExport(testFont, fp);
					}
					if (event.key.keysym.sym == SDLK_l)
					{
						bool pass = true;
						if (unsavedWork)
						{
							std::cout << "You currently have unsaved work. Really load a new font? You will lose work done on current font since last save (y/N)\n";
							std::string response;
							std::cin >> response;
							if (helpers::toLower(response) != "y") { pass = true; }
						}
						if (pass)
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
								auto returned = this->stubbleparser->import(file);
								if (returned.has_value())
								{
									testFont = std::get<bmpFont*>(returned.value());
									pxbufs.clear();
									for (int i = 0; i < testFont->glyphs.size(); i++)
									{
										testFont->glyphs[i]->bitmap->displayDX = i * (testFont->sizepx + 5);
										pxbufs.emplace_back(testFont->glyphs[i]->bitmap);
									}
								}
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
			this->renderer->hairline->transformPixelBuffer(pb, dx + (pb->displayDX * scale), dy + (pb->displayDY * scale), scale, scale > 10); //add borders at higher scale
		}
		for (int height : rulers)
		{
			this->renderer->hairline->drawLine({0,(height * scale) + dy}, {this->renderer->width,(height * scale) + dy}, 0xFFFFFFFF, 1);
		}
		if (currentTool == tool::placing_ruler)
		{
			this->renderer->hairline->drawLine({0,(previewRuleHeight * scale) + dy}, {this->renderer->width,(previewRuleHeight * scale) + dy}, 0xFFFFa000, 1);
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
