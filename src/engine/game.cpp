#include <iostream>
#include <vector>
#include <cstdlib>
#include <chrono>
#include <cmath>

#include <SDL_events.h>
#include <SDL_keyboard.h>
#include <SDL_scancode.h>
#include <SDL_stdinc.h>
#include <SDL_timer.h>

#include "import3d.h"
#include "engconfig.h"
#include "game.h"
#include "engTools.h"
#include "render/render.h"

using std::endl, std::cout;
using dtclock = std::chrono::steady_clock;

Game::Game()
{
	this->renderer = new cRenderer();
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
	float gameTime = 0.0f; //Time since game start, use for framerate independent motion eg trig anim
	float dtFac = 1.0f; // dt as ratio; shouldn't change anything if you multiply with it and you're running at 60fps

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

	mainScene.addObject(importObj("content/obj/sz2.obj"));

	mainScene.objects[1].materials.emplace_back(1.0f, 0.1f, 1.0f);

	Quaternion qDelta(pi / 200, { 0,1,0 });
	float freecamspeedbase = 0.01f;
	float freecamspeed = 0.0f;
	currentScene->currentCam->pos = currentScene->currentCam->pos - Position3d(0, 0, 10);

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
		float dtMulti = dt / (1.0f / fpsTarget);
		lastTime = currentTime;
		float fps = 1.0f / dt;
		dtFac = dt * 60.0f;
		if (frame % 10 == 0)
		{
			cout << fps << endl;
		}

		// Handle inputs
		mainScene.cams[0].calcBaseVecs();
		gk = SDL_GetKeyboardState(NULL); 
		while (SDL_PollEvent(&event)){
			if (event.type == SDL_QUIT || gk[SDL_SCANCODE_ESCAPE]) {
				break;  
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
			if (event.type == SDL_KEYDOWN && event.key.repeat == 0)
			{
				// Keypress events here
				if (event.key.keysym.sym == SDLK_f) {drawPoints = !drawPoints;}
				if (event.key.keysym.sym == SDLK_v) {wireframe = !wireframe;}
			}
		}

		// Key hold events here
		if (gk[SDL_SCANCODE_LSHIFT]) { freecamspeed = freecamspeedbase * 5 * dtFac; }
		else { freecamspeed = freecamspeedbase * dtFac; }
		if (gk[SDL_SCANCODE_W]) { mainScene.cams[0].pos += mainScene.cams[0].forward * freecamspeed; }
		if (gk[SDL_SCANCODE_S]) { mainScene.cams[0].pos -= mainScene.cams[0].forward * freecamspeed; }
		if (gk[SDL_SCANCODE_D]) { mainScene.cams[0].pos += mainScene.cams[0].right * freecamspeed; }
		if (gk[SDL_SCANCODE_A]) { mainScene.cams[0].pos -= mainScene.cams[0].right * freecamspeed; }
		if (gk[SDL_SCANCODE_E]) { mainScene.cams[0].pos += mainScene.cams[0].up * freecamspeed; }
		if (gk[SDL_SCANCODE_Q]) { mainScene.cams[0].pos -= mainScene.cams[0].up * freecamspeed; }

		//if (gk[SDL_SCANCODE_J]) mainScene.cams[0].rot.yaw += 0.05f; // yaw left
		//if (gk[SDL_SCANCODE_L]) mainScene.cams[0].rot.yaw -= 0.05f; // yaw right
		//if (gk[SDL_SCANCODE_I] && camRot.pitch <  pi - 0.05f) mainScene.cams[0].rot.pitch += 0.05f; // pitch up
		//if (gk[SDL_SCANCODE_K] && camRot.pitch > 0.05f) mainScene.cams[0].rot.pitch -= 0.05f; // pitch down

		this->renderer->renderScene(*currentScene);
		frame++;
		//mainScene.meshes[0].rotateQuat(qDelta);
		// mainScene.objectByName("cube")->mesh->setPos({0.0f, std::sin(gameTime * 5), 0.0f});
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
