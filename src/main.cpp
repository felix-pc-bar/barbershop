#include <iostream>
#include <SDL.h>
#include <cstdint>
#include <vector>
#include <cmath>
#include <filesystem>
#include "engine/render/CPURenderer.h"
#include "engine/import3d.h"
#include "engine/engconfig.h"
#include <chrono>

using std::endl, std::cout;
using dtclock = std::chrono::steady_clock;

int main(int argc, char** args) {
	// =========
	// SDL SETUP
	// =========

	// Pointers to our window and surface
	SDL_Surface* winSurface = NULL;
	SDL_Window* window = NULL;
	SDL_Renderer* mainRenderer = NULL;
	int result;
	result = SDL_Init(SDL_INIT_EVERYTHING);
	if (result < 0) 
	{
		cout << "Error initializing SDL: " << SDL_GetError() << endl;
		system("pause");
		return 1;
	}
	result = SDL_CreateWindowAndRenderer(screenwidth, screenheight, SDL_WINDOW_FULLSCREEN_DESKTOP, &window, &mainRenderer);
	if (result < 0)
	{
		cout << "Error creating window and renderer: " << SDL_GetError() << endl;
		return 1;
	}
	SDL_SetWindowTitle(window, "Provision");
	SDL_ShowCursor(SDL_DISABLE); // Hide cursor
	SDL_SetRelativeMouseMode(SDL_TRUE); // Lock cursor to window
	const Uint8* gk; // Used to read off inputs
	SDL_Event event; // SDL event buffer
	CPURenderer vp(mainRenderer, screenwidth, screenheight); // Create viewport
	vp.Clear(0xFF5792FF); // sky blue
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

	mainScene.addMesh(importObj("content/obj/suzanne.obj"));
	mainScene.meshes[0].materials.emplace_back(0.8f, 0.8f, 0.8f);


	Quaternion qDelta(pi / 200, { 0,1,0 });
	mainScene.meshes[0].rotateQuat({ -pi / 4, -1, 0, 0 });
	float freecamspeed = 0.05f;

	while (true)
	{
		Rotation3d& camRot = mainScene.cams[0].rot;
		// Handle inputs
		mainScene.cams[0].calcBaseVecs();
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				return 0;  
			}
			if (event.type == SDL_MOUSEMOTION)
			{
				//mainScene.cams[0].rot.yaw -= (float) event.motion.xrel / 1000.0f;
				mainScene.cams[0].rotateCam((float)event.motion.xrel / 1000.0f, { 0,1,0 });
				mainScene.cams[0].rotateCam((float)event.motion.yrel / 1000.0f, {1,0,0});
				//pitchDelta = std::min(pitchDelta, (pi / 2.0f) - camRot.pitch);
				//pitchDelta = std::max(pitchDelta, (-pi / 2.0f) - camRot.pitch);
			}
		}

		gk = SDL_GetKeyboardState(NULL); 
		if (gk[SDL_SCANCODE_LSHIFT]) { freecamspeed = 0.05f; }
		else { freecamspeed = 0.01f; }
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


		//cout << mainScene.meshes[0].position.cameraspace() << endl;
		vp.drawScene(*currentScene);
		vp.Present();
		frame++;
		vp.Clear(0xFF000000);
		//mainScene.meshes[0].rotateQuat(qDelta);
		//mainScene.meshes[0].setPos({ 0.0f, 0.0f, sin((float) frame / 10) });
	}

	system("pause");

	SDL_DestroyWindow(window);

	SDL_Quit();

	return 0;
}