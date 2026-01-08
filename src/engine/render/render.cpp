#include <SDL.h>
#include <cstdlib>
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

#include "render.h"
#include "../engconfig.h"
#include "components/CPU3D.h"
#include <vector>
#include "../engTools.h"
#include "components/CPU2D.h"

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

	SDL_SetWindowTitle(window, "Barbershop Engine");
	SDL_ShowCursor(SDL_DISABLE); // Hide cursor
	SDL_SetRelativeMouseMode(SDL_TRUE); // Lock cursor to window
	// Setup screenTexture and other GPU stuff
	this->bufScreen.resize(screenwidth * screenheight, 0xFF000000);
	this->screenTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, screenwidth, screenheight);
	this->cpu3d = new CPU3D(screenTexture, sdlRenderer, screenwidth, screenheight, &this->bufScreen); // Create viewport
	this->cpu2d = new CPU2D(screenTexture, sdlRenderer, screenwidth, screenheight, &this->bufScreen); // Create viewport
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
	this->cpu3d->Clear(0x00000000);
	this->cpu2d->Clear(0x00000000);
	std::vector<TriangleToRender> triangles;
	std::vector<PointToRender> renderPoints;

	if (!scene.currentCam) return;
	Position3d camPos = scene.currentCam->pos;

	for (Object3D& ob : scene.objects)
	{
		if (!(ob.mesh == nullptr))
		{
			if (!ob.mesh->vertices.empty())
			{
				Mesh& mesh = *ob.mesh;
				Position3d pos = mesh.position;
				Rotation3d rot = mesh.rotation;
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
		this->cpu3d->drawTri(tri.v1, tri.v2, tri.v3, tri.material);
		this->cpu2d->drawLine(Point2d(tri.v1), Point2d(tri.v2));
		this->cpu2d->drawLine(Point2d(tri.v2), Point2d(tri.v3));
		this->cpu2d->drawLine(Point2d(tri.v3), Point2d(tri.v1));
	}
	for (PointToRender& pt : renderPoints)
	{
		if (pt.material.pointWidth != 0)
		{
			this->cpu2d->drawPoint(Point2d(pt.pos), pt.material.pointWidth);
		}
	}
	SDL_UpdateTexture(screenTexture, nullptr, bufScreen.data(), cpu2d->width * sizeof(uint32_t));
	SDL_UpdateTexture(screenTexture, nullptr, bufScreen.data(), cpu3d->width * sizeof(uint32_t));
	SDL_RenderCopy(sdlRenderer, screenTexture, nullptr, nullptr);
	SDL_RenderPresent(sdlRenderer);
}
