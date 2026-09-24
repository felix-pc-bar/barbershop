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
#include "components/CPU3D.h"


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
	this->razor = new Razor3D(renderwidth, renderheight, &this->bufScreen);
	this->setScreenDimensions();
	this->UI = new LayoutElement("UI", {0.5f, 0.5f}, {0.5f, 0.5f}, frac2d{1.0f, 1.0f});
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

void cRenderer::renderScene(Scene& scene)
{
	// do whatever in hairline here
	// this->hairline->transformPixelBuffer(pbuf, xoffset, yoffset, scaling);
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
			this->razor->drawTri(tri.v1, tri.v2, tri.v3, tri.material, scene.currentCam, this);
		}
	}
	for (PointToRender& pt : renderPoints)
	{
		if (pt.material.pointWidth != 0)
		{
			this->hairline->drawPoint(pt.pos.project(scene.currentCam, this), pt.material.pointWidth);
		}
	}
	// SDL_UpdateTexture(screenTexture, nullptr, bufScreen.data(), hairline->width * sizeof(uint32_t));
	// SDL_UpdateTexture(screenTexture, nullptr, bufScreen.data(), razor3d->width * sizeof(uint32_t));

	UI->updateLiteralValues(nullptr);
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

// todo: dynamic resolutions for window mode, resizing?
void cRenderer::setScreenDimensions(int scaling)
{
	int display = SDL_GetWindowDisplayIndex(this->window);
	SDL_DisplayMode mode;
	SDL_GetCurrentDisplayMode(display, &mode);
	globScreenheight = mode.h / scaling;
	globScreenwidth = mode.w / scaling;
	globIntScaling = scaling;
	this->resize(globScreenwidth, globScreenheight);
	return;
}
