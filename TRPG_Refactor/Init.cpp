#include <stdio.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <string>
#include "GraphicsEngine.h"
#include <regex>

// this should be a good internal target
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

int main(int argc, char* args[]) {
	
	SDL_Window* window = NULL;
	SDL_Renderer* renderer = NULL;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
		printf("SDL could not initialize. SDL_Error: %s\n", SDL_GetError());
	}
	else if (IMG_Init(IMG_INIT_PNG) < 0) {
		printf("SDL_Image could not be initialized. SDL_Error: %s\n", IMG_GetError());
	}
	else {

		if ((window = SDL_CreateWindow("Witty title", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE)) == NULL) {
			printf("Window could not be created. SDL_Error: %s\n", SDL_GetError());
		}
		else if ((renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC)) == NULL) {
			printf("Renderer could not be created. SDL_Error: %s\n", SDL_GetError());
		}
		else {

			if (renderer == NULL) {
				printf("Renderer is null. SDL_Error: %s\n", SDL_GetError());
			}

			if (SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND) < 0) {
				printf("Renderer blend mode could not be set. SDL_Error: %s\n", SDL_GetError());
			}

			SDL_Texture* resBuffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);
			if (resBuffer == NULL) {
				printf("backbuffer is null. SDL_Error: %s\n", SDL_GetError());
			}

			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			SDL_RenderClear(renderer);
			SDL_RenderPresent(renderer);
			SDL_Delay(2000);

			char* c_basePath = SDL_GetBasePath();
			std::string basePath(c_basePath);
			printf("Project directory: %s\n", basePath.c_str());

			AssetManager assets;
			int res = assets.loadAssets(renderer, basePath + "assets\\");
			if (res != 0) {
				printf("ERROR: loadAssets returned an error state %d...\n", res);
				return -1;
			}

			AnimationManager animator;
			std::vector<Sprite*> sprites;
			
			sprites.push_back(animator.addSprite(assets.getAFrame("anti_air"),"idle"));
			sprites.push_back(animator.addSprite(assets.getAFrame("apc"),"idle"));
			sprites.push_back(animator.addSprite(assets.getAFrame("artillery"),"idle"));
			sprites.push_back(animator.addSprite(assets.getAFrame("battle_copter"),"idle"));
			sprites.push_back(animator.addSprite(assets.getAFrame("battleship"),"idle"));
			sprites.push_back(animator.addSprite(assets.getAFrame("bomber"),"idle"));
			sprites.push_back(animator.addSprite(assets.getAFrame("carrier"),"idle"));
			sprites.push_back(animator.addSprite(assets.getAFrame("cruiser"),"idle"));
			sprites.push_back(animator.addSprite(assets.getAFrame("fighter"),"idle"));
			sprites.push_back(animator.addSprite(assets.getAFrame("heavy_tank"),"idle"));
			sprites.push_back(animator.addSprite(assets.getAFrame("hidden_stealth_fighter"),"idle"));
			sprites.push_back(animator.addSprite(assets.getAFrame("infantry"),"idle"));
			sprites.push_back(animator.addSprite(assets.getAFrame("lander"),"idle"));
			sprites.push_back(animator.addSprite(assets.getAFrame("light_tank"),"idle"));
			sprites.push_back(animator.addSprite(assets.getAFrame("mech"),"idle"));
			sprites.push_back(animator.addSprite(assets.getAFrame("medium_tank"),"idle"));
			sprites.push_back(animator.addSprite(assets.getAFrame("missile"),"idle"));
			sprites.push_back(animator.addSprite(assets.getAFrame("recon"),"idle"));
			sprites.push_back(animator.addSprite(assets.getAFrame("rocket"),"idle"));
			sprites.push_back(animator.addSprite(assets.getAFrame("stealth_fighter"),"idle"));
			sprites.push_back(animator.addSprite(assets.getAFrame("submarine"),"idle"));
			sprites.push_back(animator.addSprite(assets.getAFrame("submerged_submarine"),"idle"));
			sprites.push_back(animator.addSprite(assets.getAFrame("transport_copter"),"idle"));

			sprites[1]->setX(100);
			sprites[2]->setX(200);
			sprites[3]->setX(300);
			sprites[4]->setX(400);
			sprites[5]->setX(500);
			sprites[6]->setX(600);
			sprites[8]->setX(100);
			sprites[9]->setX(200);
			sprites[10]->setX(300);
			sprites[11]->setX(400);
			sprites[12]->setX(500);
			sprites[13]->setX(600);
			sprites[15]->setX(100);
			sprites[16]->setX(200);
			sprites[17]->setX(300);
			sprites[18]->setX(400);
			sprites[19]->setX(500);
			sprites[20]->setX(600);
			sprites[22]->setX(100);

			sprites[7]->setY(100);
			sprites[8]->setY(100);
			sprites[9]->setY(100);
			sprites[10]->setY(100);
			sprites[11]->setY(100);
			sprites[12]->setY(100);
			sprites[13]->setY(100);
			sprites[14]->setY(200);
			sprites[15]->setY(200);
			sprites[16]->setY(200);
			sprites[17]->setY(200);
			sprites[18]->setY(200);
			sprites[19]->setY(200);
			sprites[20]->setY(200);
			sprites[21]->setY(300);
			sprites[22]->setY(300);


			SDL_Rect* camera = animator.getCamera();
			camera->x = 0;
			camera->y = 0;
			camera->w = SCREEN_WIDTH;
			camera->h = SCREEN_HEIGHT;
			//sprites[0]->render(camera);

			int displayHeight = SCREEN_HEIGHT, displayWidth = SCREEN_WIDTH;

			SDL_SetRenderTarget(renderer, resBuffer);

			while (true) {
				// poll event
				SDL_Event event;
				bool isQuit = false;
				while (SDL_PollEvent(&event)) {
					if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
						isQuit = true;
					if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
						displayHeight = event.window.data2;
						displayWidth = event.window.data1;
					}
				}

				SDL_SetRenderDrawColor(renderer, 50, 20, 20, 255);
				SDL_RenderClear(renderer);

				animator.updateSprites();

				GE_PushFromBackbuffer(renderer, resBuffer, displayHeight, displayWidth);

				if (isQuit) break;
			}

			SDL_DestroyRenderer(renderer);
			SDL_DestroyWindow(window);

			SDL_Quit();

		}

	}

	return 0;

}
