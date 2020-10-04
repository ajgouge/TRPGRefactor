#include "GraphicsEngine.h"
#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <string>
#include <fstream>
#include <sstream>
#include <regex>

Frame::Frame() {
	renderer = NULL;
	texture = NULL;
}

Frame::Frame(SDL_Renderer* renderer, SDL_Texture* graphic) : renderer(renderer), texture(graphic) {
	// This is something AFrames has to do now, so I'll leave it here until I do that
	//if (SDL_QueryTexture(graphic, NULL, NULL, &(this->rect.w), &(this->rect.h)) < 0) {
	//	printf("Frame::Frame: Couldn't query texture. SDL_Error: %s\n", SDL_GetError());
	//}
}

Frame::~Frame() {
	// We don't want to destroy the renderer (since everyone uses that), but the texture should
	// only be ours. We can destroy that.
	renderer = NULL;
	SDL_DestroyTexture(texture);
	texture = NULL;
}

void Frame::render(SDL_Rect* dst) {
	if (SDL_RenderCopy(renderer, texture, NULL, dst) < 0) {
		printf("Frame::render(): Failed to render. SDL_Error: %s\n", SDL_GetError());
	}
}



AssetManager::AssetManager() { areTexturesLoaded = false; }

AssetManager::~AssetManager() {
	if (areTexturesLoaded) {
		for (Frame* e : frames) {
			delete e;
		}
	}
}

int AssetManager::loadAssets(SDL_Renderer* renderer, std::string assetDir) {

	if (areTexturesLoaded) {
		printf("AssetManager::loadAssets: Assets have already been loaded!\n");
		return -1;
	}

	// open the object file using std::ifstream
	std::ifstream oFile{ assetDir.append("objects.txt").c_str() };
	if (!oFile) {
		printf("AssetManager::loadAssets: Couldn't load asset format file.\n");
		return -1;
	}

	// now put all of that file into a stringstream so we can regex it
	std::stringstream buf;
	buf << oFile.rdbuf();
	// close the file (we won't need it anymore)
	oFile.close();

	// this matches a complete asset entry with capture 1 as the name, capture 2 as the default scale (if specified), and capture 3 as the order entries
	std::regex assetEntry("([A-Za-z0-9]+)\\s*(?:\\(\\s*([0-9]+(?:\\.[0-9]+)?)\\s*\\))?\\s*:\\s*{\\s*([^}]*)\\s*");
	// then this matches an order entry inside of the former's capture 2, with name of the order in capture 1, the msperframe in capture 2, and the CSV in capture 3
	std::regex orderEntry("\\s*([A-Za-z0-9]+)\\s*(?:\\(\\s*([0-9]+(?:\\.[0-9]+)?)\\s*\\))\\s*=\\s*\\[(\\s*(?:[0-9]+\\s*(?:\\(\\s*-?[0-9]+\\s*,\\s*-?[0-9]+\\s*\\))?\\s*,\\s*)*[0-9]+\\s*(?:\\(\\s*-?[0-9]+\\s*,\\s*-?[0-9]+\\s*\\)\\s*)?)\\]\\s*");
	// and this matches each number in the CSV in capture 1, with the optional offset pair in capture 2
	std::regex orderValue("\\s*([0-9]+)(\\s*\\(\\s*-?[0-9]+\\s*,\\s*-?[0-9]+\\s*\\))*\\s*");
	// this matches the ordered x,y pair attached to each element of the CSV (capture 1 is the x offset, capture 2 is the y offset)
	std::regex orderedPair("\\s*(-?[0-9]+)\\s*,\\s*(-?[0-9]+)\\s*");

	// make a new iterator over the matches
	std::sregex_iterator assetIterator(buf.str().begin(), buf.str().end(), assetEntry);
	
	// iterate thru all of them
	while (assetIterator != std::sregex_iterator()) {

		// the first capture is the name
		std::string assetName = assetIterator->str(1);
		// second capture is scale. If not specified, we default to 1.
		double assetScale = (assetIterator->str(2).empty()) ? 1 : std::stod(assetIterator->str(2));

		// now we load all the textures from the asset directory for this asset into a buffer
		SDL_Surface* img = NULL;
		int i = 0;
		std::vector<Frame*> textureBuffer{};
		while (true) {

			// we assume all the assets are stored as assets\name\name_[i].png, where i starts at 0 and increments each time
			// these i values are used later in the order lists to specify which frames occur in each order
			img = IMG_Load(assetDir.append(assetName).append("\\").append(assetName).append("_").append(std::to_string(i)).append(".png").c_str());
			// if the load didn't work, then there shouldn't be any more assets
			if (img == NULL) {
				// DEBUG:: print statement
				printf("Loaded %d images from directory %s...\n", i, assetDir.append(assetName).c_str());
				break;
			}

			// put the new texture into a Frame wrapper and push it to the buffer, then free the surface
			textureBuffer.push_back(new Frame(renderer, SDL_CreateTextureFromSurface(renderer, img)));
			SDL_FreeSurface(img);
			
		}

		// now put these frames onto the AssetManager's BEEG BUFFER
		frames.insert(frames.end(), textureBuffer.begin(), textureBuffer.end());

		// the map needs an AFrame, which collects all the order information for us.
		AFrame* assetAFrame{};

		// more regex yay
		std::sregex_iterator orders(assetIterator->str(2).begin(),assetIterator->str(2).end(), orderEntry);
		while (orders != std::sregex_iterator()) {

			// the order name is the first capture
			std::string orderName = orders->str(1);
			// the msperframe is the second capture (not optional)
			double orderMSPerFrame = std::stod(orders->str(2));

			// and this buffer will hold pointers to Frames, in the specified order.
			std::vector<Frame*> orderFrames{};
			// this will hold the offsets, many of which are likely to be defaulted (0,0)
			std::vector<SDL_Point> orderOffsets{};

			// now regex the CSV
			std::sregex_iterator values(orders->str(2).begin(), orders->str(2).end(), orderValue);
			while (values != std::sregex_iterator()) {
				// we push each pointer in the original buffer to this new, order-specific buffer
				orderFrames.push_back(textureBuffer.at(std::stoi(values->str(1))));

				SDL_Point offset;
				if (!(values->str(2).empty())) {
					std::sregex_iterator offsetPairs(values->str(2).begin(), values->str(2).end(), orderedPair);
					offset.x = stoi(offsetPairs->str(1));
					offset.y = stoi(offsetPairs->str(2));
				} else {
					offset.x = 0;
					offset.y = 0;
				}
				orderOffsets.push_back(offset);

				++values;
			}

			// and we add this order to the AFrame
			assetAFrame->addOrder(orderName, orderMSPerFrame, orderFrames, orderOffsets, assetScale);

			++orders;
		}

		// now that the AFrame is complete, we add it to the AssetManager's global map of AFrames
		assets.emplace(assetName, assetAFrame);

		++assetIterator;
	}

	areTexturesLoaded = true;

}

AFrame* AssetManager::getAFrame(std::string key) {
	return assets.at(key);
}



Order::Order(double msPerFrame, std::vector<Frame*> frames, std::vector<SDL_Point> offsets, double scale) : msPerFrame(msPerFrame), frames(frames), offsets(offsets), scale(scale) {}

Order::~Order() {}



AFrame::AFrame() {}

AFrame::~AFrame() {}

void AFrame::update(SDL_Rect camera) {

}

void AFrame::addOrder(std::string name, double msPerFrame, std::vector<Frame*> frames, std::vector<SDL_Point> offsets, double scale) {
	orders.emplace(name, Order(msPerFrame, frames, offsets, scale));
}



Sprite::Sprite() {

}

Sprite::~Sprite() {

}

void Sprite::render(SDL_Rect camera) {

}
