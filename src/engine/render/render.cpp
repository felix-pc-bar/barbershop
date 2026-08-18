#include <SDL.h>
#include <iostream>
#include <ostream>
#include <begin_code.h>
#include <SDL_error.h>
#include <SDL_events.h>
#include <SDL_mouse.h>
#include <SDL_pixels.h>
#include <SDL_render.h>
#include <SDL_stdinc.h>
#include <SDL_video.h>
// #include <process.h> // Linux doesn't like this?
#include <algorithm>
#include <vector>
#include <cstdint>

#include "render.h"
#include "components/CPU2D.h"
#include "../ui.h"


using std::cout, std::endl;

cRenderer::cRenderer(int renderwidth, int renderheight)
{
	// =========
	// SDL SETUP
	// =========

	// Pointers to our window and surface
	this->winSurface = NULL;
	this->window = NULL;
	this->sdlRenderer = NULL;

	int result;
	result = SDL_Init(SDL_INIT_EVERYTHING);
	if (result < 0)
	{
		cout << "Error initializing SDL: " << SDL_GetError() << endl;
		system("pause");
	}

	result = SDL_CreateWindowAndRenderer(renderwidth, renderheight, SDL_WINDOW_FULLSCREEN_DESKTOP, &window, &sdlRenderer);
	if (result < 0)
	{
		cout << "Error creating window and renderer: " << SDL_GetError() << endl;
	}

	this->width = renderwidth;
	this->height = renderheight;

	SDL_SetWindowTitle(window, "font-ed");
	// SDL_ShowCursor(SDL_DISABLE); // Hide cursor
	// SDL_SetRelativeMouseMode(SDL_TRUE); // Lock cursor to window
	// Setup screenTexture and other GPU stuff
	this->bufScreen.resize(renderwidth * renderheight, 0xFF000000);
	this->screenTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, renderwidth, renderheight);
	this->hairline = new Hairline(renderwidth, renderheight, &this->bufScreen); // Create viewport
	this->UI = new LayoutElement();
}

void cRenderer::resize(int newWidth, int newHeight) // TODO crashes upon second resize?
{
	// if (newWidth == width && newHeight == height) { return; }

	if (screenTexture)
	{
		SDL_DestroyTexture(screenTexture);
		screenTexture = nullptr;
	}

	this->width = newWidth;
	this->height = newHeight;

	this->bufScreen.resize(newWidth * newHeight, 0xFF000000);
	this->screenTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, newWidth, newHeight);
	this->hairline->width = newWidth;
	this->hairline->height = newHeight;
	this->hairline->bufMain = &bufScreen;
	return;
}

cRenderer::~cRenderer() {
	if (screenTexture) {
		SDL_DestroyTexture(screenTexture);
		screenTexture = nullptr;
	}
	if (sdlRenderer) {
		SDL_DestroyRenderer(sdlRenderer);
		sdlRenderer = nullptr;
	}
	if (window) {
		SDL_DestroyWindow(window);
		window = nullptr;
	}
}

void cRenderer::renderScene()
{
	// do whatever in hairline here
	// this->hairline->transformPixelBuffer(pbuf, xoffset, yoffset, scaling);
	// this->hairline->SetPixel(5, 5, Colour(255, 0, 255).raw());

	UI->draw(nullptr, this);

	SDL_UpdateTexture(screenTexture, nullptr, bufScreen.data(), hairline->width * sizeof(uint32_t));
	SDL_RenderCopy(sdlRenderer, screenTexture, nullptr, nullptr);
	SDL_RenderPresent(sdlRenderer);
}

void cRenderer::clear(Colour col)
{
	std::fill(bufScreen.begin(), bufScreen.end(), col.raw());
	return;
}

// void cRenderer::clear(Material mat) 
// {
// 	for (int y = 0; y < this->height; y++)
// 	{
// 		for (int x = 0; x < this->width; x++)
// 		{
// 			if (mat.ditherValue < this->razor3d->bayer8x8[x % 8][y % 8])
// 			{ bufScreen[(y * this->height) + x] = Colour("black").raw(); }
// 			else {  bufScreen[(y * this->height) + x] = mat.colour.raw(); }
// 		}
// 	}
// 	return;
// }
