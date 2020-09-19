#include <stdio.h>
#include <SDL.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

int main(int argc, char* args[]) {
	
	SDL_Window* window = NULL;
	SDL_Surface* surface = NULL;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
	}
	else {

		window = SDL_CreateWindow("Window Title", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (window == NULL) {
			printf("Window could not be created. SDL_Error: %s\n", SDL_GetError());
		}
		else {

			surface = SDL_GetWindowSurface(window);

			SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 255, 255, 255));

			SDL_UpdateWindowSurface(window);

			SDL_Delay(2000);

			SDL_DestroyWindow(window);

			SDL_Quit();

		}

	}

	return 0;

}