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

#include "../engconfig.h"
#include "../engTools.h"
#include "../logic2d.h"

using std::cout, std::endl;

cRenderer::cRenderer()
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

	result = SDL_CreateWindowAndRenderer(screenwidth, screenheight, SDL_WINDOW_FULLSCREEN_DESKTOP, &window, &sdlRenderer);
	if (result < 0)
	{
		cout << "Error creating window and renderer: " << SDL_GetError() << endl;
	}

	this->width = screenwidth; // from global config, can be changed later
	this->height = screenheight;

	SDL_SetWindowTitle(window, "Barbershop Engine");
	SDL_ShowCursor(SDL_DISABLE); // Hide cursor
	SDL_SetRelativeMouseMode(SDL_TRUE); // Lock cursor to window
	// Setup screenTexture and other GPU stuff
	this->bufScreen.resize(screenwidth * screenheight, 0xFF000000);
	this->screenTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, screenwidth, screenheight);
	this->razor3d = new Razor3D(screenTexture, sdlRenderer, screenwidth, screenheight, &this->bufScreen, true); // Create viewport
	this->hairline = new Hairline(screenTexture, sdlRenderer, screenwidth, screenheight, &this->bufScreen); // Create viewport
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
		if (!ob.points.empty() && drawPoints)
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
	for (TriangleToRender& tri : triangles)
	{
		Point2d p1(tri.v1);
		Point2d p2(tri.v2);
		Point2d p3(tri.v3);

		if ((tri.v1.position.cameraspace().z > 0 && tri.v2.position.cameraspace().z > 0 && tri.v3.position.cameraspace().z > 0) && 
			(isTriangleOnScreen(p1, p2, p3, screenwidth, screenheight)) &&
			(p1.x != -99999 && p2.x != -99999 && p3.x != -99999)
		)
		{
			if (wireframe)
			{
				this->hairline->drawLine(Point2d(tri.v1), Point2d(tri.v2), tri.material.colour.raw());
				this->hairline->drawLine(Point2d(tri.v2), Point2d(tri.v3), tri.material.colour.raw());
				this->hairline->drawLine(Point2d(tri.v3), Point2d(tri.v1), tri.material.colour.raw());
			}
			else
			{
				this->razor3d->drawTri(tri.v1, tri.v2, tri.v3, tri.material);
			}
		}
	}
	for (PointToRender& pt : renderPoints)
	{
		if (pt.material.pointWidth != 0)
		{
			this->hairline->drawPoint(Point2d(pt.pos), pt.material.pointWidth);
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

void cRenderer::clearGrad(Colour colBot, Colour colTop, bool dither)
{
	if (!dither)
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
		float val = 0;
		float valStep = 256 / (float)this->height;
		uint32_t colBotRaw = colBot.raw();
		uint32_t colTopRaw = colTop.raw();
		for (int y = 0; y < this->height; y++)
		{
			for (int x = 0; x < this->width; x++)
			{
				this->bufScreen[(y*this->width) + x] = val > this->razor3d->bayer8x8[x % 8][y % 8] ? colBotRaw : colTopRaw;
			}
			val += valStep;
		}
		return;
	}
}