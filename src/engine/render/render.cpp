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
#include <process.h>
#include <algorithm>
#include <vector>
#include <cstdint>

#include "render.h"
#include "components/CPU3D.h"
#include "components/CPU2D.h"

#include "../globals.h"
#include "../general3d.h"
#include "../general2d.h"
#include "../quaternion.h"

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

	SDL_SetWindowTitle(window, "Barbershop Engine");
	SDL_ShowCursor(SDL_DISABLE); // Hide cursor
	SDL_SetRelativeMouseMode(SDL_TRUE); // Lock cursor to window
	// Setup screenTexture and other GPU stuff
	this->bufScreen.resize(renderwidth * renderheight, 0xFF000000);
	this->screenTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, renderwidth, renderheight);
	this->razor3d = new Razor3D(renderwidth, renderheight, &this->bufScreen, true); // Create viewport
	this->hairline = new Hairline(renderwidth, renderheight, &this->bufScreen); // Create viewport
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
	this->razor3d->width = newWidth;
	this->razor3d->height = newHeight;
	this->razor3d->bufMain = &bufScreen;
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

void cRenderer::renderScene(Scene& scene) const
{
	std::vector<TriangleToRender> triangles;
	std::vector<PointToRender> renderPoints;

	if (!scene.currentCam) return;
	Position3d camPos = scene.currentCam->pos;
	Quaternion inverseRotaton = scene.currentCam->quatIdentity.conjugate();

	for (Object3D& ob : scene.objects)
	{
		if (!(ob.mesh == nullptr))
		{
			if (!ob.mesh->vertices.empty())
			{
				Mesh& mesh = *ob.mesh;
				Position3d pos = mesh.position;
				//Rotation3d rot = mesh.rotation;
				for (size_t i = 0; i + 2 < mesh.indices.size(); i += 3)
				{
					Vertex3d& v1 = mesh.vertices[mesh.indices[i]];
					Vertex3d& v2 = mesh.vertices[mesh.indices[i + 1]];
					Vertex3d& v3 = mesh.vertices[mesh.indices[i + 2]];
					//Material mat = mesh.materials[mesh.matIndices[i / 3]];
					Material mat(1.0f, 0.0f, 1.0f); // magenta
					if (ob.materials.size() != 0)
					{
						mat = ob.materials[0];
					}
					triangles.emplace_back(v1, v2, v3, camPos, &mat);
				}
			}
		}
		if (!ob.points.empty() && globDrawPoints)
		{
			for (Position3d point : ob.points)
			{
				Material mat(1.0f, 0.0f, 1.0f); // magenta
				if (ob.materials.size() != 0)
				{
					mat = ob.materials[0];
				}
				renderPoints.emplace_back(point, camPos, &mat);
			}
		}
	}

	std::sort(triangles.begin(), triangles.end(),
		[](const TriangleToRender& a, const TriangleToRender& b) {
			return a.distanceToCamera > b.distanceToCamera;
		});

	// Draw
	Quaternion cameraRotationInverse = currentScene->currentCam->quatIdentity.conjugate();
	for (TriangleToRender& tri : triangles)
	{
		if (globWireframe)
		{
			Point2d p1 = tri.v1.position.project(scene.currentCam, this);
			Point2d p2 = tri.v2.position.project(scene.currentCam, this);
			Point2d p3 = tri.v3.position.project(scene.currentCam, this);

			if ((tri.v1.position.cameraspace().z > 0 && tri.v2.position.cameraspace().z > 0 && tri.v3.position.cameraspace().z > 0) &&
				(isTriangleOnScreen(p1, p2, p3, globScreenwidth, globScreenheight)) &&
				(p1.x != -99999 && p2.x != -99999 && p3.x != -99999)
				)
			{
				this->hairline->drawLine(p1, p2, tri.material.colour.raw());
				this->hairline->drawLine(p2, p3, tri.material.colour.raw());
				this->hairline->drawLine(p3, p1, tri.material.colour.raw());
			}
		}
		else
		{
			this->razor3d->drawTri(tri.v1, tri.v2, tri.v3, tri.material, scene.currentCam, this);
		}
	}
	for (PointToRender& pt : renderPoints)
	{
		if (pt.material.pointWidth != 0)
		{
			this->hairline->drawPoint(pt.pos.project(scene.currentCam, this), pt.material.pointWidth);
		}
	}
	SDL_UpdateTexture(screenTexture, nullptr, bufScreen.data(), hairline->width * sizeof(uint32_t));
	SDL_UpdateTexture(screenTexture, nullptr, bufScreen.data(), razor3d->width * sizeof(uint32_t));
	SDL_RenderCopy(sdlRenderer, screenTexture, nullptr, nullptr);
	SDL_RenderPresent(sdlRenderer);
}

void cRenderer::clear(Colour col)
{
	this->razor3d->clear(col.raw());
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

void cRenderer::clearGrad(Colour colBot, Colour colTop, float ditherValBot, float ditherValTop)
{
	if (ditherValBot == -1.0f)
	{
		float r = colTop.red;
		float rStep = (colBot.red - colTop.red) / (float)this->height;
		float g = colTop.green;
		float gStep = (colBot.green - colTop.green) / (float)this->height;
		float b = colTop.blue;
		float bStep = (colBot.blue - colTop.blue) / (float)this->height;

		for (int y = 0; y < this->height; y++)
		{
			for (int x = 0; x < this->width; x++)
			{
				this->bufScreen[(y*this->width) + x] = Colour(r, g, b).raw();	
			}
			r += rStep;
			g += gStep;
			b += bStep;
		}
		return;
	}
	else
	{
		float val = ditherValTop * 256.0f;
		float valStep = ((ditherValBot * 256.0f) - val) / (float)this->height;
		uint32_t colBotRaw = colBot.raw();
		uint32_t colTopRaw = colTop.raw();
		for (int y = 0; y < height; ++y)
		{
			const uint8_t* b = razor3d->bayer8x8[y & 7];
			uint32_t* row = bufScreen.data() + y * width;

			int x = 0;
			for (; x + 7 < width; x += 8)
			{
				row[x + 0] = val < b[0] ? colBotRaw : colTopRaw;
				row[x + 1] = val < b[1] ? colBotRaw : colTopRaw;
				row[x + 2] = val < b[2] ? colBotRaw : colTopRaw;
				row[x + 3] = val < b[3] ? colBotRaw : colTopRaw;
				row[x + 4] = val < b[4] ? colBotRaw : colTopRaw;
				row[x + 5] = val < b[5] ? colBotRaw : colTopRaw;
				row[x + 6] = val < b[6] ? colBotRaw : colTopRaw;
				row[x + 7] = val < b[7] ? colBotRaw : colTopRaw;
			}

			for (; x < width; ++x)
			{
				row[x] = val < b[x & 7] ? colBotRaw : colTopRaw;
			}
			val += valStep;
		}	
		return;
	}
}