#include <cmath>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <cstdlib>
#include <chrono>
#include <cstring>

#include <SDL_events.h>
#include <SDL_keyboard.h>
#include <SDL_scancode.h>
#include <SDL_stdinc.h>
#include <SDL_timer.h>
#include <SDL_keycode.h>
#include <optional>
#include <string>

#include "../../external/tinyfiledialogs/tinyfiledialogs.h"

#include "game.h"
#include "SDL.h"
#include "general2d.h"
#include "general3d.h"
#include "globals.h"
#include "render/components/bmpfont.h"
#include "render/render.h"
#include "material.h"
#include "stubble/stubble.h"
#include "helpers/stringy.h"
#include "buffer.h"
#include "stubble/types.h"
#include "ui-impls.h"

using dtclock = std::chrono::steady_clock;

Game::Game()
{
	if (!globDeferGFXcreation)
	{
		this->renderer = new cRenderer();
	}
	this->stubbleparser = new StubbleParser();

	frame = 0;
	lastTime = dtclock::now();
	fpsLimit = 0;
	fpsLimTick = 1.0f / fpsLimit;
	gameTime = 0.0f; //Time since game start, use for framerate independent motion eg trig anim
	dtFac = 1.0f; // dt as ratio; shouldn't change anything if you multiply with it and you're running at 60fps

	previewRuleHeight = 0;

	std::cout << "Would you like to {C}reate a new font, or {O}pen one that already exists?\n";
	char response;
	std::cin >> response;

	if (response == 'C' || response == 'c')
	{
		std::string nameEntered;
		std::cout << "Enter default width for new font: ";
		std::cin >> defaultWidth;
		std::cout << "\nEnter default height for new font: ";
		std::cin >> defaultHeight;
		std::cout << "\nEnter name for new font: ";
		std::cin >> nameEntered;
		std::cout << std::endl;

		workingFont = new bmpFont(defaultWidth, defaultHeight);
		workingFont->name = nameEntered;
		workingFont->sizepx = defaultHeight;
	}
	else
	{
		auto imported = stubbleparser->importGUI(TypesEnum::_bmpFont);
		if (imported.has_value())
		{
			workingFont = std::get<bmpFont*>(imported.value());
		}
		else
		{
			std::cout << "quitting";
			this->quit();
			return;
		}
		if (workingFont == nullptr)
		{
			this->quit();
			return;
		}
	}

	testString =
	"Lorem ipsum dolor sit amet, consectetur adipiscing elit.\n"
	"Sed do eiusmod tempor incididunt ut labore et dolore magna.\n"
	"Ut enim ad minim veniam, quis nostrud exercitation ullamco.\n"
	"Laboris nisi ut aliquip ex ea commodo consequat.\n"
	"Duis aute irure dolor in reprehenderit in voluptate velit.\n"
	"Esse cillum dolore eu fugiat nulla pariatur.\n"
	"Excepteur sint occaecat cupidatat non proident.\n"
	"Sunt in culpa qui officia deserunt mollit anim id est.\n"
	"Curabitur pretium tincidunt lacus, vitae suscipit nulla.\n"
	"Praesent blandit, risus eget feugiat fermentum, nunc.";

	// auto imported = this->stubbleparser->import("/home/felix/Downloads/Tx.stbbl", TypesEnum::_bmpFont);
	// if (imported.has_value()) { this->stopgapFont = std::get<bmpFont*>(imported.value()); }

	this->dealFontBuffers(workingFont);

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
	rmbdown = false;

	currentState = state::roughing_normal;
	currentTool = tool::pen;
	unsavedWork = false;
	fpsRunningTotal = 0;
	runningAvgPeriodFrames = 30;

	if (globAppdatalocation == "")
	{
		std::cout << "Error: could not assign appdata location. Proceed with caution; the program may be unstable." << std::endl;
	}
	this->stubbleparser->stbExport(this->workingFont, globAppdatalocation / "font-ed" / "autosav.stbbl");
}

void Game::dealFontBuffers(bmpFont* font)
{
	this->pxbufs.clear();
	for (int i = 0; i < font->glyphs.size(); i++)
	{
		font->glyphs[i]->bitmap->displayDX = i * (font->sizepx + 5);
		pxbufs.emplace_back(font->glyphs[i]->bitmap);
	}
	return;
}

void Game::createUndoState()
{
	try
	{
		std::filesystem::rename(globAppdatalocation / "font-ed" / "autosav.stbbl", globAppdatalocation / "font-ed" / "undostate.stbbl");
		this->stubbleparser->stbExport(this->workingFont, globAppdatalocation / "font-ed" / "autosav.stbbl");
	}
	catch (std::filesystem::filesystem_error)
	{ std::cout << "Error whilst creating autosave...\n"; }
	return;
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

	std::cout << this->renderer->width  << 'x' << this->renderer->height << '\n';

	int focusedGlyphIndex = 0;

	// Console console(this->renderer);

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
		if (frame % runningAvgPeriodFrames == 0 && globPrintFPS)
		{
			std::cout << fpsRunningTotal / (float)runningAvgPeriodFrames << std::endl;
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
			if (event.key.keysym.sym == SDLK_g && event.type == SDL_KEYDOWN) { mmbdown = true; }
			if (event.key.keysym.sym == SDLK_g && event.type == SDL_KEYUP) { mmbdown = false; }

			if (event.button.button == SDL_BUTTON_LEFT && event.type == SDL_MOUSEBUTTONDOWN) { lmbdown = true; }
			if (event.button.button == SDL_BUTTON_RIGHT && event.type == SDL_MOUSEBUTTONDOWN) { rmbdown = true; }
			if (event.button.button == SDL_BUTTON_LEFT && event.type == SDL_MOUSEBUTTONUP) { lmbdown = false; this->createUndoState(); }
			if (event.button.button == SDL_BUTTON_RIGHT && event.type == SDL_MOUSEBUTTONUP) { rmbdown = false; this->createUndoState(); }

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
					if (lmbdown || rmbdown)
					{
						for (auto& pb : pxbufs)
						{
							int mouserelx = mousex - (dx + (pb->displayDX * scale));
							int mouserely = mousey - (dy + (pb->displayDY * scale));
							// if (((mouserelx / scale) < pb->width) && (mouserelx > 0) && (((mouserely / scale) < pb->height()) && (mouserely > 0)))
							pb->set( mouserelx / scale, mouserely / scale, lmbdown);
						}
					}
				}
				if (currentTool == tool::placing_ruler)
				{
					previewRuleHeight = std::round(((mousey - dy) / static_cast<float>(scale)) + 0.5f);
					if (lmbdown)
					{
						rulers.emplace_back(previewRuleHeight);
						currentTool = tool::pen;
					}
				}
				mods = SDL_GetModState();
				if (event.type == SDL_MOUSEWHEEL)
				{
					if (mods & KMOD_ALT)
					{
						if (event.wheel.y < 0 && focusedGlyphIndex < 127) { focusedGlyphIndex++; }
						if (event.wheel.y > 0 && focusedGlyphIndex > 0) { focusedGlyphIndex--; }
					}
					else
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
				}

				// Key hold events here
				// if (gk[SDL_SCANCODE_LEFT]) { dx -= 20 * dtFac; }

				if (event.type == SDL_KEYDOWN && event.key.repeat == 0)
				{
					if ((mods & KMOD_CTRL) && event.key.keysym.sym == SDLK_z)
					{
						auto import = this->stubbleparser->import(globAppdatalocation / "font-ed" / "undostate.stbbl", TypesEnum::_bmpFont);
						if (import.has_value())
						{
							this->workingFont = std::get<bmpFont*>(import.value());
							this->dealFontBuffers(workingFont);
						}
						this->createUndoState();
					}
					if ((mods & KMOD_CTRL) && event.key.keysym.sym == SDLK_s)
					{
						const char* fp = tinyfd_saveFileDialog(
							"Save font",
							"font.stbbl",
							0,
							NULL,
							NULL
						);
						// this->stubbleparser->stbExport(pxbufs[0], fp);
						this->stubbleparser->stbExport(workingFont, fp);
					}
					if ((mods & KMOD_CTRL) && event.key.keysym.sym == SDLK_o)
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
							auto returned = this->stubbleparser->importGUI(TypesEnum::_bmpFont);
							if (returned.has_value())
							{
								workingFont = std::get<bmpFont*>(returned.value());
								this->renderer->hairline->backupFont = workingFont;
								this->dealFontBuffers(workingFont);
							}
						}
					}
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
					if (event.key.keysym.sym == SDLK_q)
					{
						std::cin >> focusedGlyphIndex;
					}
					if (event.key.keysym.sym == SDLK_p)
					{
						workingFont->glyphs[focusedGlyphIndex]->isPrintable = !workingFont->glyphs[focusedGlyphIndex]->isPrintable;
					}
					if (event.key.keysym.sym == SDLK_MINUS) { focusedGlyphIndex--; }
					if (event.key.keysym.sym == SDLK_EQUALS) { focusedGlyphIndex++; }
					if (event.key.keysym.sym == SDLK_COMMA) { workingFont->defaultKerning--; }
					if (event.key.keysym.sym == SDLK_PERIOD) { workingFont->defaultKerning++; }
					if ((mods & KMOD_ALT))
					{
						if (event.key.keysym.sym == SDLK_w) { workingFont->glyphs[focusedGlyphIndex]->placementY++; }
						if (event.key.keysym.sym == SDLK_a) { workingFont->glyphs[focusedGlyphIndex]->placementX--; }
						if (event.key.keysym.sym == SDLK_s) { workingFont->glyphs[focusedGlyphIndex]->placementY--; }
						if (event.key.keysym.sym == SDLK_d) { workingFont->glyphs[focusedGlyphIndex]->placementX++; }
						if (event.key.keysym.sym == SDLK_t)
						{
							std::string line;
							testString.clear();
							std::cout << "Enter new sample text:\n";
							while (std::getline(std::cin, line))
							{
								if (line == ".") { break; }
								testString += line + '\n';
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

		for (int i = 0; i < 128; i++)
		{
			auto pb = pxbufs[i];
			uint32_t outlinecol = i == focusedGlyphIndex ? 0xFFFFFF80 : (workingFont->glyphs[i]->isPrintable ? 0xFF00A000 : 0x00000000);
			this->renderer->hairline->transformPixelBuffer(pb, dx + (pb->displayDX * scale), dy + (pb->displayDY * scale), scale, scale > 10, outlinecol, 0xFFFFFFFF, 0xFF000000); //add borders at higher scale
			if (scale > 3)
			{
				this->renderer->hairline->drawText(Point2d{dx + (pb->displayDX * scale), dy + (pb->displayDY * scale) - 8}, std::to_string(i));
			}
		}
		for (int height : rulers)
		{
			this->renderer->hairline->drawLine({0,(height * scale) + dy}, {this->renderer->width,(height * scale) + dy}, 0xFFFFFFFF, 1);
		}
		if (currentTool == tool::placing_ruler)
		{
			this->renderer->hairline->drawLine({0,(previewRuleHeight * scale) + dy}, {this->renderer->width,(previewRuleHeight * scale) + dy}, 0xFFFFa000, 1);
		}

		this->renderer->hairline->drawText({16, 70}, testString, workingFont, 1, 0xFFFFFFFF, 1);
		this->renderer->hairline->drawText({16, 50}, " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~", workingFont, 3);
		this->renderer->hairline->drawText({16, 30}, " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~", workingFont, 2);
		this->renderer->hairline->drawText({16, 20}, " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~", workingFont);

		this->renderer->hairline->drawText({16, 1050}, "Editing " + workingFont->name, nullptr, 2);
		this->renderer->hairline->drawText({16, 1030}, "Glyph " + std::to_string(focusedGlyphIndex), nullptr, 2);
		this->renderer->hairline->drawText({16, 1010}, this->asciiDescriptions[focusedGlyphIndex], nullptr, 2);
		this->renderer->hairline->drawText({16, 990}, workingFont->glyphs[focusedGlyphIndex]->isPrintable ? "Printable" : "Not printable", nullptr, 2);

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
