#include <stdio.h>
#include <regex>
#include <string>
#include <functional>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>

#include "GraphicsEngine.h"
#include "Tiles.h"

// this should be a good internal target (for now)
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
const int TILE_SIZE = 64;

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
			SDL_Delay(1000);

			char* c_basePath = SDL_GetBasePath();
			std::string basePath(c_basePath);
			printf("Project directory: %s\n", basePath.c_str());

			AssetManager assets;
			int res = assets.loadAssets(renderer, basePath + "assets\\");
			if (res != 0) {
				printf("ERROR: loadAssets returned an error state %d...\n", res);
				return -1;
			}

			std::vector<Sprite> sprites;

			printf("Assets loaded. Preparing to create Sprites...\n");
			
			//Sprite test{ assets.getAFrame("anti_air"), "idle" };

			//printf("Sprite created with scale %lf.\n", test.getScale());
			//printf("If you see this, the error is very strange...\n");
			
			sprites.emplace_back(assets.getAFrame("anti_air"), "idle");
			sprites.emplace_back(assets.getAFrame("apc"),"idle");
			sprites.emplace_back(assets.getAFrame("artillery"),"idle");
			sprites.emplace_back(assets.getAFrame("battle_copter"),"idle");
			sprites.emplace_back(assets.getAFrame("battleship"),"idle");
			sprites.emplace_back(assets.getAFrame("bomber"),"idle");
			sprites.emplace_back(assets.getAFrame("carrier"),"idle");
			sprites.emplace_back(assets.getAFrame("cruiser"),"idle");
			sprites.emplace_back(assets.getAFrame("fighter"),"idle");
			sprites.emplace_back(assets.getAFrame("heavy_tank"),"idle");
			sprites.emplace_back(assets.getAFrame("hidden_stealth_fighter"),"idle");
			sprites.emplace_back(assets.getAFrame("infantry"),"idle");
			sprites.emplace_back(assets.getAFrame("lander"),"idle");
			sprites.emplace_back(assets.getAFrame("light_tank"),"idle");
			sprites.emplace_back(assets.getAFrame("mech"),"idle");
			sprites.emplace_back(assets.getAFrame("medium_tank"),"idle");
			sprites.emplace_back(assets.getAFrame("missile"),"idle");
			sprites.emplace_back(assets.getAFrame("recon"),"idle");
			sprites.emplace_back(assets.getAFrame("rocket"),"idle");
			sprites.emplace_back(assets.getAFrame("stealth_fighter"),"idle");
			sprites.emplace_back(assets.getAFrame("submarine"),"idle");
			sprites.emplace_back(assets.getAFrame("submerged_submarine"),"idle");
			sprites.emplace_back(assets.getAFrame("transport_copter"),"idle");

			sprites[1].setX(64);
			sprites[2].setX(128);
			sprites[3].setX(192);
			sprites[4].setX(256);
			sprites[5].setX(320);
			sprites[6].setX(384);
			sprites[8].setX(64);
			sprites[9].setX(128);
			sprites[10].setX(192);
			sprites[11].setX(256);
			sprites[12].setX(320);
			sprites[13].setX(384);
			sprites[15].setX(64);
			sprites[16].setX(128);
			sprites[17].setX(192);
			sprites[18].setX(256);
			sprites[19].setX(320);
			sprites[20].setX(384);
			sprites[22].setX(64);

			sprites[7].setY(64);
			sprites[8].setY(64);
			sprites[9].setY(64);
			sprites[10].setY(64);
			sprites[11].setY(64);
			sprites[12].setY(64);
			sprites[13].setY(64);
			sprites[14].setY(128);
			sprites[15].setY(128);
			sprites[16].setY(128);
			sprites[17].setY(128);
			sprites[18].setY(128);
			sprites[19].setY(128);
			sprites[20].setY(128);
			sprites[21].setY(192);
			sprites[22].setY(192);

			printf("All sprites set. Preparing Layer test...\n");

			// Layer test
			Layer testLayer(assets, basePath + "assets\\testmap1.txt");

			SDL_Rect* camera = Sprite::getAnimCamera();
			camera->x = 0;
			camera->y = 0;
			camera->w = SCREEN_WIDTH;
			camera->h = SCREEN_HEIGHT;
			//sprites[0]->render(camera);

			int displayHeight = SCREEN_HEIGHT, displayWidth = SCREEN_WIDTH;

			SDL_SetRenderTarget(renderer, resBuffer);

			// TODO: the goal is to make this class invisible to
			// the caller. That will come with removing rendering
			// from the game loop and putting it somewhere else!
			AnimationManager& animator{ Sprite::getAnimator() };

			while (true) {
				// poll event
				SDL_Event event;
				bool isQuit = false;
				while (SDL_PollEvent(&event)) {
					if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
						isQuit = true;
					if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
						displayHeight = event.window.data2;
						displayWidth = event.window.data1;
					}
				}

				SDL_SetRenderDrawColor(renderer, 50, 20, 20, 255);
				SDL_RenderClear(renderer);

				printf("Preparing to update Sprites...\n");

				// TODO: Ideally rendering should happen independently of game logic
				// at some point. One step (though only *one* step!) is putting this
				// into an SDL_Timer callback.
				animator.updateSprites();
				//test.render(camera);

				printf("Done. Rendering backbuffer...\n");

				GE_PushFromBackbuffer(renderer, resBuffer, displayHeight, displayWidth);

				if (isQuit) break;
			}

			SDL_DestroyTexture(resBuffer);
			SDL_DestroyRenderer(renderer);
			SDL_DestroyWindow(window);

			SDL_Quit();

		}

	}

	return 0;

}
