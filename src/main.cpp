#include <SDL_main.h>
#include "engine/game.h"

int main(int argc, char* argv[]) {
	Game game;
	game.run();
	game.quit();
	return 0;
}
