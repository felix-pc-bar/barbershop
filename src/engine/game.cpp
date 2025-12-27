#include <iostream>
#include <sdl.h>
#include <vector>
#include <cstdlib>
#include <chrono>

#include <SDL_error.h>
#include <SDL_events.h>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>
#include <SDL_pixels.h>
#include <SDL_render.h>
#include <SDL_scancode.h>
#include <SDL_stdinc.h>
#include <SDL_timer.h>
#include <SDL_video.h>

#include "render/cpurenderer.h"
#include "import3d.h"
#include "engconfig.h"
#include "game.h"
#include "engTools.h"

using std::endl, std::cout;
using dtclock = std::chrono::steady_clock;

Game::Game()
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
	this->screenTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, screenwidth, screenheight);
	this->currentRenderer= new CPURenderer(screenTexture, sdlRenderer, screenwidth, screenheight); // Create viewport
}

void Game::run()
{
	const Uint8* gk; // Used to read off inputs
	SDL_Event event; // SDL event buffer


	// ====
	// TIME
	// ====

	int frame = 0;
	auto lastTime = dtclock::now();
	int fpsLimit = 0;
	float fpsLimTick = 1.0f / fpsLimit;


	// ===========
	// SCENE SETUP
	// ===========

	Scene mainScene; //Create main scene
	currentScene = &mainScene; // Set the current scene to mainScene
	
	mainScene.cams.emplace_back(); // Add a camera to mainScene
	mainScene.currentCam = &mainScene.cams[0]; // Set mainScene's current camera to the camera we just created

	// Add the ground points object
	mainScene.objects.emplace_back();
	mainScene.objects[0].materials.emplace_back(0.5f, 0.5f, 0.5f, 3, false);
	mainScene.objects[0].name = mainScene.getName("Ground plane points");

	const int gridSize = 100;
	for (int x = 0; x < gridSize; x++)
	{
		for (int y = 0; y < gridSize; y++)
		{
			mainScene.objects[0].points.emplace_back(x - gridSize/2, 0, y - gridSize/2);
		}
	}

	mainScene.addObject(importObj("content/obj/obj.obj"));

	mainScene.objects[1].materials.emplace_back(0.8f, 0.8f, 0.8f);

	Quaternion qDelta(pi / 200, { 0,1,0 });
	float freecamspeedbase = 0.01f;
	float freecamspeed = 0.0f;
	currentScene->currentCam->pos = currentScene->currentCam->pos - Position3d(0, 0, 10);

	while (true)
	{
		auto currentTime = dtclock::now();
		std::chrono::duration<float> elapsed = currentTime - lastTime;
		float deltaTime = elapsed.count(); // Raw frametime (s)
		if (fpsLimit != 0 && deltaTime < fpsLimTick)
		{
			SDL_Delay((fpsLimTick - deltaTime) * 1000);
			deltaTime += fpsLimTick - deltaTime;
			currentTime = dtclock::now();
		}
		float dtMulti = deltaTime / (1.0f / fpsTarget);
		lastTime = currentTime;
		float fps = 1.0f / deltaTime;
		cout << fps << endl;

		// Handle inputs
		mainScene.cams[0].calcBaseVecs();
		gk = SDL_GetKeyboardState(NULL); 
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || gk[SDL_SCANCODE_ESCAPE]) {
				return;  
			}
			if (event.type == SDL_MOUSEMOTION)
			{
				mainScene.cams[0].rotateCam((float)event.motion.xrel / 1000.0f, { 0,1,0 });
				mainScene.cams[0].rotateCam((float)event.motion.yrel / 1000.0f, mainScene.cams[0].right);
			}
			if (event.type == SDL_MOUSEWHEEL)
			{
				if (event.wheel.y > 0) { freecamspeedbase += 0.01f; }
				if (event.wheel.y < 0 && freecamspeedbase > 0.01f) { freecamspeedbase -= 0.01f; }
			}
		}

		if (gk[SDL_SCANCODE_LSHIFT]) { freecamspeed = freecamspeedbase * 5; }
		else { freecamspeed = freecamspeedbase; }
		if (gk[SDL_SCANCODE_W]) { mainScene.cams[0].pos += mainScene.cams[0].forward * freecamspeed; }
		if (gk[SDL_SCANCODE_S]) { mainScene.cams[0].pos -= mainScene.cams[0].forward * freecamspeed; }
		if (gk[SDL_SCANCODE_D]) { mainScene.cams[0].pos += mainScene.cams[0].right * freecamspeed; }
		if (gk[SDL_SCANCODE_A]) { mainScene.cams[0].pos -= mainScene.cams[0].right * freecamspeed; }
		if (gk[SDL_SCANCODE_E]) { mainScene.cams[0].pos += mainScene.cams[0].up * freecamspeed; }
		if (gk[SDL_SCANCODE_Q]) { mainScene.cams[0].pos -= mainScene.cams[0].up * freecamspeed; }
		if (gk[SDL_SCANCODE_F]) {
			if (drawPoints) { drawPoints = false; }
			else { drawPoints = true; }
		}
		//if (gk[SDL_SCANCODE_J]) mainScene.cams[0].rot.yaw += 0.05f; // yaw left
		//if (gk[SDL_SCANCODE_L]) mainScene.cams[0].rot.yaw -= 0.05f; // yaw right
		//if (gk[SDL_SCANCODE_I] && camRot.pitch <  pi - 0.05f) mainScene.cams[0].rot.pitch += 0.05f; // pitch up
		//if (gk[SDL_SCANCODE_K] && camRot.pitch > 0.05f) mainScene.cams[0].rot.pitch -= 0.05f; // pitch down


		//cout << mainScene.meshes[0].position.cameraspace() << endl;
		this->currentRenderer->Clear(0xFF000000);
		this->currentRenderer->drawScene(*currentScene);
		this->currentRenderer->Present();
		frame++;
		//mainScene.meshes[0].rotateQuat(qDelta);
		//mainScene.meshes[0].setPos({ 0.0f, 0.0f, sin((float) frame / 10) });
	}

	system("pause");

	SDL_DestroyWindow(window);

	SDL_Quit();

	return;

}

Game::~Game()
{
	SDL_DestroyTexture(this->screenTexture);
}
