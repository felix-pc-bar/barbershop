class Game
{
public:
	Game();
	~Game();
	void run();
private:
	// Pointers to our window and surface
	SDL_Surface* winSurface;
	SDL_Window* window;
	SDL_Renderer* mainRenderer;

};