#include <stdio.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <string>
#include "GraphicsEngine.h"

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

int main(int argc, char* args[]) {
	
	SDL_Window* window = NULL;
	SDL_Renderer* renderer = NULL;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL could not initialize. SDL_Error: %s\n", SDL_GetError());
	}
	else if (IMG_Init(IMG_INIT_PNG) < 0) {
		printf("SDL_Image could not be initialized. SDL_Error: %s\n", IMG_GetError());
	}
	else {

		SDL_CreateWindow("Window Title", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN, &window, &renderer) < 0) {
			printf("Window and renderer could not be created. SDL_Error: %s\n", SDL_GetError());
		}
		else {

			if (renderer == NULL) {
				printf("Renderer is null. SDL_Error: %s\n", SDL_GetError());
			}

			if (SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND) < 0) {
				printf("Renderer blend mode could not be set. SDL_Error: %s\n", SDL_GetError());
			}

			SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
			SDL_RenderClear(renderer);
			SDL_RenderPresent(renderer);
			SDL_Delay(2000);

			char* c_basePath = SDL_GetBasePath();
			std::string basePath(c_basePath);
			printf("Project directory: %s\n", basePath.c_str());

			SDL_RWops* fReader = SDL_RWFromFile(basePath.append("assets\\objects.txt").c_str(),"r");

			if (fReader == NULL) {
				printf("file reader is null. SDL_Error: %s\n", SDL_GetError());
			}

			SDL_Rect offscreenRect;
			offscreenRect.x = -20;
			offscreenRect.y = -20;
			offscreenRect.w = 40;
			offscreenRect.h = 40;

			SDL_RenderClear(renderer);
			SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
			SDL_RenderFillRect(renderer,&offscreenRect);

			SDL_RenderPresent(renderer);

			SDL_Delay(2000);

			SDL_DestroyRenderer(renderer);
			SDL_DestroyWindow(window);

			SDL_Quit();

		}

	}

	return 0;

}