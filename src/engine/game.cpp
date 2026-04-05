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
#include <SDL_keycode.h>

#include "import3d.h"
#include "globals.h"
#include "game.h"
#include "general3d.h"
#include "render/render.h"
#include "material.h"
#include "quaternion.h"
#include "stubble.h"

using std::endl, std::cout;
using dtclock = std::chrono::steady_clock;

Game::Game()
{
	this->renderer = new cRenderer();
}

void Game::run()
{
	StubbleParser* sp = new StubbleParser;
	sp->parse("content/stubble/test1.sttbl");




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
	mainScene.objects[0].materials.emplace_back(Colour("black"), 3, false);
	mainScene.objects[0].name = mainScene.getName("Ground plane points");

	const int gridSize = 100;
	for (int x = 0; x < gridSize; x++)
	{
		for (int y = 0; y < gridSize; y++)
		{
			mainScene.objects[0].points.emplace_back(x - gridSize/2, 0, y - gridSize/2);
		}
	}

	int numMonkeys = 150;

	for (int i = 1; i <= numMonkeys; i++)
	{
		mainScene.addObject(importObj("content/obj/ico.obj"));
		if (i % 7 == 0) { mainScene.objects[i].materials[0] = Material(Colour("red")); }
		if (i % 7 == 1) { mainScene.objects[i].materials[0] = Material(Colour("green")); }
		if (i % 7 == 2) { mainScene.objects[i].materials[0] = Material(Colour("yellow")); }
		if (i % 7 == 3) { mainScene.objects[i].materials[0] = Material(Colour("blue")); }
		if (i % 7 == 4) { mainScene.objects[i].materials[0] = Material(Colour("purple")); }
		if (i % 7 == 5) { mainScene.objects[i].materials[0] = Material(Colour("aqua")); }
		if (i % 7 == 6) { mainScene.objects[i].materials[0] = Material(Colour("orange")); }
	}
	// mainScene.addObject(importObj("content/obj/szfl.obj"));
	// mainScene.objects[1].materials[0] = Material(Colour("orange"));

	// mainScene.addObject(importObj("content/obj/skysphere.obj"));
	// mainScene.objectByName("skysphere")->materials[0] = Material(Colour("grey"));

	Quaternion qDelta(pi / 200, { 0,1,0 });
	float freecamspeedbase = 0.01f;
	float freecamspeed = 0.0f;
	currentScene->currentCam->pos = currentScene->currentCam->pos - Position3d(0, 0, 10);

	float fpsRunningTotal = 0;
	int runningAvgPeriodFrames = 30;
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
		float dtMulti = dt / (1.0f / globfpsTarget);
		lastTime = currentTime;
		float fps = 1.0f / dt;
		dtFac = dt * 60.0f;
		fpsRunningTotal += fps;
		if (frame % runningAvgPeriodFrames == 0)
		{
			cout << fpsRunningTotal / (float)runningAvgPeriodFrames << endl;
			fpsRunningTotal = 0;
		}

		// Handle inputs
		mainScene.cams[0].calcCamData();
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
				if (event.key.keysym.sym == SDLK_z) {globDrawPoints = !globDrawPoints;}
				if (event.key.keysym.sym == SDLK_x) {globWireframe = !globWireframe;}
				if (event.key.keysym.sym == SDLK_c) {this->renderer->razor3d->dither = !this->renderer->razor3d->dither;}
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

		if (gk[SDL_SCANCODE_LALT] && gk[SDL_SCANCODE_1])
		{
			this->renderer->resize(globScreenwidth, globScreenheight);
		} 
		if (gk[SDL_SCANCODE_LALT] && gk[SDL_SCANCODE_2])
		{
			this->renderer->resize(globScreenwidth / 2, globScreenheight / 2);
		} 
		if (gk[SDL_SCANCODE_LALT] && gk[SDL_SCANCODE_3])
		{
			this->renderer->resize(globScreenwidth / 3, globScreenheight / 3);
		} 

		// this->renderer->clear(Colour("grey"));
		// this->renderer->clear(Material(Colour("black")));
		// if (wireframe) {this->renderer->clear(Colour("black")); }
		Position3d botVec = mainScene.cams[0].forward;
		botVec.rotateQuat(Quaternion(mainScene.cams[0].fov / 2.0f, mainScene.cams[0].right));
		float botInclination = botVec.dot({ 0, 1, 0 }) / 2 + 0.5f;
		Position3d topVec = mainScene.cams[0].forward;
		topVec.rotateQuat(Quaternion(mainScene.cams[0].fov / -2.0f, mainScene.cams[0].right));
		float topInclination = topVec.dot({ 0, 1, 0 }) / 2 + 0.5f;
		this->renderer->clearGrad(Colour("grey"), Colour("grey4"), this->renderer->razor3d->dither, botInclination, topInclination);
		this->renderer->renderScene(*currentScene);
		frame++;
		
		for (int i = 1; i <= numMonkeys; i++)
		{
			mainScene.objects[i].mesh->rotateAxis(std::sin(gameTime * pi * (0.05f * (float)i)) * 0.05f * dtFac, Position3d(1.0f, 0.0f, 0.0f));
			mainScene.objects[i].mesh->rotateAxis(0.05f * dtFac, Position3d(0.0f, 1.0f, 0.0f));
			mainScene.objects[i].mesh->setPos(Position3d(std::sin((gameTime * pi * 0.22f) - 0.31f * (float)i) * 3, std::sin((gameTime * pi * 0.3f) - 0.41f * (float)i) * 3, std::sin((gameTime * pi * 0.41f) - 0.31f * i)) * 5);
		}

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
