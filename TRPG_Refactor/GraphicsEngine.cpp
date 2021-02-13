#include <stdio.h>
#include <string>
#include <fstream>
#include <sstream>
#include <regex>
#include <queue>

#include <SDL.h>
#include <SDL_image.h>

#include "GraphicsEngine.h"

//Frame::Frame() {
//	renderer = NULL;
//	texture = NULL;
//}

/// <summary>
/// The basic Frame constructor you should always use.
/// </summary>
/// <param name="renderer">The current renderer</param>
/// <param name="graphic">A texture to wrap with that renderer</param>
Frame::Frame(SDL_Renderer* renderer, SDL_Texture* graphic) : renderer(renderer), texture(graphic) {}

/// <summary>
/// Destructor for Frame
/// </summary>
Frame::~Frame() {
	// We don't want to destroy the renderer (since everyone uses that), but the texture should
	// only be ours. We can destroy that.
	renderer = NULL;
	SDL_DestroyTexture(texture);
	texture = NULL;
}

/// <summary>
/// Lets you grab the width and height of the internal texture, just in case
/// </summary>
/// <param name="w">int pointer to place the width</param>
/// <param name="h">int pointer to place the height</param>
void Frame::queryWidthHeight(int* w, int* h) {
	if (SDL_QueryTexture(texture, NULL, NULL, w, h) < 0) {
		printf("Frame::Frame: Couldn't query texture. SDL_Error: %s\n", SDL_GetError());
	}
}

/// <summary>
/// Call this to render the internal texture to the active render target
/// </summary>
/// <param name="dst">destination SDL_Rect</param>
void Frame::render(SDL_Rect* dst) {
	//printf("Frame:: preparing to render...\n");
	if (SDL_RenderCopy(renderer, texture, NULL, dst) < 0) {
		printf("Frame::render(): Failed to render. SDL_Error: %s\n", SDL_GetError());
	}
	//printf("Done. Frame succesfully rendered...\n");
}


/// <summary>
/// AssetManager constructor; doesn't actually do much. Call loadAssets before using.
/// </summary>
AssetManager::AssetManager() { areTexturesLoaded = false; }

/// <summary>
/// AssetManager destructor. DELETES all internal frames! You don't have to do that yourself!
/// </summary>
AssetManager::~AssetManager() {
	if (areTexturesLoaded) {
		for (Frame* e : frames) {
			delete e;
		}
	}
}

/// <summary>
/// This is what actually inits the AssetManager, now with a useful(ish) return!
/// Automagically loads all correctly formatted assets in the given directory.
/// </summary>
/// <param name="renderer">The active renderer</param>
/// <param name="assetDir">An absolute path to the assets, with a trailing \ character.
///  Use SDL_GetBasePath() and then append the relative path.</param>
/// <returns>0 if everything went smoothly; -1 or something else if not. Check stdout for more details.</returns>
int AssetManager::loadAssets(SDL_Renderer* renderer, std::string assetDir) {

	if (areTexturesLoaded) {
		printf("AssetManager::loadAssets: Assets have already been loaded!\n");
		return -1;
	}

	// open the object file using std::ifstream
	std::ifstream oFile{ (assetDir + "objects.txt").c_str() };
	if (!oFile) {
		printf("AssetManager::loadAssets: Couldn't load asset format file.\n");
		return -1;
	}

	// now put all of that file into a stringstream so we can regex it
	std::stringstream buf;
	buf << oFile.rdbuf();
	// close the file (we won't need it anymore)
	oFile.close();

	//printf("Getting ready to make some regex thingamabobs...\n");

	// this matches a complete asset entry with capture 1 as the name, capture 2 as the default scale (if specified), and capture 3 as the order entries
	std::regex assetEntry("([A-Za-z0-9_]+)\\s*(?:\\(\\s*([0-9]+(?:\\.[0-9]+)?)\\s*\\))?\\s*:\\s*\\{\\s*([^}]*)\\s*");
	// then this matches an order entry inside of the former's capture 3, with name of the order in capture 1, the msperframe in capture 2, and the CSV in capture 3
	std::regex orderEntry("\\s*([A-Za-z0-9_]+)\\s*(?:\\(\\s*([0-9]+(?:\\.[0-9]+)?)\\s*\\))\\s*=\\s*\\[(\\s*(?:[0-9]+\\s*(?:\\(\\s*-?[0-9]+\\s*,\\s*-?[0-9]+\\s*\\))?\\s*,\\s*)*[0-9]+\\s*(?:\\(\\s*-?[0-9]+\\s*,\\s*-?[0-9]+\\s*\\)\\s*)?)\\]\\s*");
	// and this matches each number in the CSV in capture 1, with the optional offset pair in capture 2
	std::regex orderValue("\\s*([0-9]+)(\\s*\\(\\s*-?[0-9]+\\s*,\\s*-?[0-9]+\\s*\\))*\\s*");
	// this matches the ordered x,y pair attached to each element of the CSV (capture 1 is the x offset, capture 2 is the y offset)
	std::regex orderedPair("\\s*(-?[0-9]+)\\s*,\\s*(-?[0-9]+)\\s*");

	//printf("Done. Now making the first iterator...\n");

	// make a new iterator over the matches
	std::string s(std::istreambuf_iterator<char>(buf), {});

	std::sregex_iterator assetIterator(s.begin(), s.end(), assetEntry);

	printf("\nPreparing to load all assets...\n");
	
	// iterate thru all of them
	while (assetIterator != std::sregex_iterator()) {

		// the first capture is the name
		std::string assetName = assetIterator->str(1);
		// second capture is scale. If not specified, we default to 1.
		double assetScale = (assetIterator->str(2).empty()) ? 1 : std::stod(assetIterator->str(2));

		//printf("loading assets from folder %s with scale %.1f...\n", assetName.c_str(), assetScale);

		// now we load all the textures from the asset directory for this asset into a buffer
		SDL_Surface* img = NULL;
		int i = 0;
		std::vector<Frame*> textureBuffer{};
		//printf("beginning to load assets...\n");
		while (true) {

			// we assume all the assets are stored as assets\name\name_i.png, where i starts at 0 and increments each time
			// these i values are used later in the order lists to specify which frames occur in each order
			img = IMG_Load((assetDir + assetName + "\\" + assetName + "_" + std::to_string(i) + ".png").c_str());
			//printf("%d: Attempted loading asset of name %s, result was %hi\n", i, (assetDir + assetName + "\\" + assetName + "_" + std::to_string(i) + ".png").c_str(), img);
			// if the load didn't work, then there shouldn't be any more assets
			if (img == NULL) {
				// DEBUG:: print statement
				//printf("Loaded %d images from directory %s...\n", i, (assetDir + assetName).c_str());
				break;
			}

			// put the new texture into a Frame wrapper and push it to the buffer, then free the surface
			textureBuffer.push_back(new Frame(renderer, SDL_CreateTextureFromSurface(renderer, img)));
			SDL_FreeSurface(img);

			++i;
			
		}

		//printf("Finished loading assets. Inserting new frames into frame buffer...\n");

		// now put these frames onto the AssetManager's BEEG BUFFER
		frames.insert(frames.end(), textureBuffer.begin(), textureBuffer.end());

		//printf("Done. Preparing a new regex iterator to look at the orders...\n");

		// the map needs an AFrame, which collects all the order information for us.
		AFrame assetAFrame{};

		// more regex yay
		std::string orderS = assetIterator->str(3);
		std::sregex_iterator orders(orderS.begin(), orderS.end(), orderEntry);
		//printf("match: %s\n", orderS.c_str());
		while (orders != std::sregex_iterator()) {

			// the order name is the first capture
			std::string orderName = orders->str(1);
			// the msperframe is the second capture (not optional)
			double orderMSPerFrame = std::stod(orders->str(2));

			//printf("preparing order %s with msPerFrame = %.1f...\n", orderName.c_str(), orderMSPerFrame);

			// and this buffer will hold pointers to Frames, in the specified order.
			std::vector<Frame*> orderFrames{};
			// this will hold the offsets, many of which are likely to be defaulted (0,0)
			std::vector<SDL_Point> orderOffsets{};

			// now regex the CSV
			std::string valueS = orders->str(3);
			std::sregex_iterator values(valueS.begin(), valueS.end(), orderValue);
			//printf("Loading from CSV %s:\n", valueS.c_str());
			while (values != std::sregex_iterator()) {
				// we push each pointer in the original buffer to this new, order-specific buffer
				orderFrames.push_back(textureBuffer.at(std::stoi(values->str(1))));

				SDL_Point offset;
				if (!(values->str(2).empty())) {
					std::string pairS = values->str(2);
					std::sregex_iterator offsetPairs(pairS.begin(), pairS.end(), orderedPair);
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
			//printf("Preparing to emplace Order into AFrame map...\n");
			assetAFrame.addOrder(orderName, orderMSPerFrame, orderFrames, orderOffsets, assetScale);

			++orders;
		}

		// now that the AFrame is complete, we add it to the AssetManager's global map of AFrames
		//printf("Preparing to emplace AFrame into asset map...\n");
		assets.emplace(assetName, assetAFrame);

		++assetIterator;

		//printf("Done; moving to next asset...\n\n\n");
	}

	printf("Done. All assets loaded.\n\n");

	areTexturesLoaded = true;

	return 0;

}

/// <summary>
/// Once loaded, gets the AFrame with the input key
/// </summary>
/// <param name="key">The name of the AFrame to get. This should've been the name of each
///  file in the asset directory, if you remove the number afterwards.</param>
/// <returns>A pointer to the requested AFrame, usually to give your Sprites.</returns>
AFrame* AssetManager::getAFrame(std::string key) {
	return &(assets.at(key));
}


/// <summary>
/// The Order constructor. Pretty basic.
/// </summary>
/// <param name="msPerFrame">After this much time, the Order will advance to the next Frame.</param>
/// <param name="frames">This should be all the frames, IN ORDER.</param>
/// <param name="offsets">And these are the offsets, in the same order as frames.</param>
/// <param name="scale">Each frame will be scaled up or down by this amount. Leave 1.0 for no scaling.</param>
Order::Order(double msPerFrame, std::vector<Frame*> frames, std::vector<SDL_Point> offsets, double scale) : msPerFrame(msPerFrame), frames(frames), offsets(offsets), scale(scale) {}

Order::~Order() {}

/// <summary>
/// Renders the requested frame in the order at the given coordinates.
/// </summary>
/// <param name="screenX"></param>
/// <param name="screenY"></param>
/// <param name="frame">An index of the frames Vector telling which Frame to render.</param>
void Order::drawFrame(int screenX, int screenY, int frame) {
	//printf("Order:: Preparing dst rectangle...\n");
	SDL_Rect dst;
	dst.x = screenX + offsets.at(frame).x;
	dst.y = screenY + offsets.at(frame).y;
	frames.at(frame)->queryWidthHeight(&(dst.w), &(dst.h));
	dst.w = (int)(dst.w * scale);
	dst.h = (int)(dst.h * scale);
	//printf("Done. dst = [x(%d),y(%d),w(%d),h(%d)]. Drawing frame %d to this rectangle...\n", dst.x, dst.y, dst.w, dst.h, frame);
	frames.at(frame)->render(&dst);
}

double Order::getMSPerFrame() {
	return msPerFrame;
}

size_t Order::getLength() {
	return frames.size();
}



AFrame::AFrame() {}

AFrame::~AFrame() {}

/// <summary>
/// Renders the requested Frame of the given Order at the given coordinates.
/// </summary>
/// <param name="screenX"></param>
/// <param name="screenY"></param>
/// <param name="order">This should be the name of an Order for this AFrame, otherwise things won't work (TODO: Prevent this or make this an error)</param>
/// <param name="frame">See Order::drawFrame</param>
void AFrame::draw(int screenX, int screenY, std::string order, int frame) {
	//printf("AFrame:: Drawing frame %d of order %s to position %d,%d...\n", frame, order.c_str(), screenX, screenY);
	orders.at(order).drawFrame(screenX, screenY, frame);
}

/// <summary>
/// Adds a new Order to this AFrame with the given attributes and name.
/// </summary>
/// <param name="name">This should be unique to any other name already in this AFrame!</param>
/// <param name="msPerFrame"></param>
/// <param name="frames"></param>
/// <param name="offsets"></param>
/// <param name="scale"></param>
void AFrame::addOrder(std::string name, double msPerFrame, std::vector<Frame*> frames, std::vector<SDL_Point> offsets, double scale) {
	orders.emplace(name, Order(msPerFrame, frames, offsets, scale));
}

double AFrame::getOrderMSPerFrame(std::string order) {
	return orders.at(order).getMSPerFrame();
}

size_t AFrame::getOrderLength(std::string order) {
	return orders.at(order).getLength();
}


/// <summary>
/// Constructor for Sprite; defaults x and y to 0. z layer is also 0.
/// </summary>
/// <param name="frames"></param>
/// <param name="order"></param>
/// <param name="camera"></param>
Sprite::Sprite(AFrame* frames, std::string order, SDL_Rect* camera) : graphics(frames), x(0), y(0), order(order), orderPosition(0), flags(0), zlayer(0) {
	callbackArg.spr = this;
	callbackArg.cam = camera;
	callbackID = SDL_AddTimer((Uint32)graphics->getOrderMSPerFrame(order), Sprite::callback_render, &callbackArg);
	orderLength = frames->getOrderLength(order);
}
/// <summary>
/// Constructor for Sprite. 
/// </summary>
/// <param name="frames">A pointer to the AFrame to get animations from.</param>
/// <param name="order">The Order of the given AFrame to start with. It will start at the first entry for the Order</param>
/// <param name="camera">A pointer to the SDL_Rect being used as the camera. You either maintain this yourself, or use the
///  one supplied by your AnimationManager (recommended).</param>
/// <param name="x"></param>
/// <param name="y"></param>
Sprite::Sprite(AFrame* frames, std::string order, SDL_Rect* camera, int x, int y, int zlayer) : graphics(frames), x(x), y(y), order(order), orderPosition(0), flags(0), zlayer(zlayer) {
	callbackArg.spr = this;
	callbackArg.cam = camera;
	callbackID = SDL_AddTimer((Uint32)graphics->getOrderMSPerFrame(order), Sprite::callback_render, &callbackArg);
	orderLength = frames->getOrderLength(order);
}

/// <summary>
/// Sprite destructor. Simply cleans up the animation callback.
/// </summary>
Sprite::~Sprite() {
	SDL_RemoveTimer(callbackID);
}

/// <summary>
/// Draws the Sprite using the given camera (TODO: don't we already have the camera?). Note: The actual frame rendered is handled by 
///  a timer that cycles through the active Order of the AFrame. You have to actually render it for the graphic to update.
///  Ideally, you should just sync to VSync and render everything once per main loop, and then if the graphic updated, it'll
///  update. OR you could just put everything in an AnimationManager, and it'll do that for you. Just sayin.
/// </summary>
/// <param name="camera">The camera to use (?) for rendering.</param>
void Sprite::render(SDL_Rect* camera) {
	// DEBUG:: only draws the first frame for now, we need logic to handle this later
	//printf("Drawing sprite order %s at %d,%d...\n", order.c_str(), x, y);
	graphics->draw(x - camera->x, y - camera->y, order, orderPosition);
}

/// <summary>
/// Updates the current Frame at the correct interval for this Order. This all happens under the hood.
/// Note: Deceiving name! This doesn't actually render anything, it just sets up the next render!
/// </summary>
/// <param name="interval"></param>
/// <param name="sp"></param>
/// <returns></returns>
Uint32 Sprite::callback_render(Uint32 interval, void* sp) {
	
	SpriteCallbackArg* args = (SpriteCallbackArg*)sp;

	(args->spr->orderPosition)++;
	if (args->spr->orderPosition >= args->spr->orderLength) args->spr->orderPosition = 0;
	//printf("Callback from sprite %hi updated to order position %i of %i.\n", args->spr, args->spr->orderPosition, args->spr->orderLength - 1);
	//args->spr->render(args->cam);
	
	return interval;
}

/// <summary>
/// For getting the args passed to the callback, if you want that for some reason. I guess you could
/// update those args that way, but there aren't any you'd want to update yet.
/// </summary>
/// <returns></returns>
SpriteCallbackArg* Sprite::getCallbackArg() {
	return &callbackArg;
}

void Sprite::setX(int newX) { x = newX; }
void Sprite::setY(int newY) { y = newY; }
void Sprite::setXY(int newX, int newY) { x = newX; y = newY; }
int Sprite::moveX(int xOffset) { x += xOffset; return x; }
int Sprite::moveY(int yOffset) { y += yOffset; return y; }
int Sprite::getZlayer() { return zlayer; }
void Sprite::setZlayer(int z) { zlayer = z; }



/// <summary>
/// Inits a new AnimationManager with a default camera, being all 0's.
/// </summary>
AnimationManager::AnimationManager() {
	camera = new SDL_Rect;
	camera->x = 0;
	camera->y = 0;
	camera->h = 0;
	camera->w = 0;
}

/// <summary>
/// AnimationManager destructor. Deletes all Sprites it's responsible for as well as the camera.
/// </summary>
AnimationManager::~AnimationManager() {
	for (Sprite* e : sprites) {
		delete e;
	}
	delete camera;
}

/// <summary>
/// Adds a new Sprite. Once added, the AnimationManager assumes ownership of the Sprite and will delete it when it's destroyed.
/// DON'T delete it yourself. You still get a pointer to it for updating its values, but do not delete it.
/// </summary>
/// <param name="graphics"></param>
/// <param name="order"></param>
/// <returns></returns>
Sprite* AnimationManager::addSprite(AFrame* graphics, std::string order) {
	Sprite* sprite = new Sprite(graphics, order, camera);
	sprites.push_back(sprite);
	sprite->render(camera);
	return sprite;
}

/// <summary>
/// Renders all the Sprites managed by this AnimationManager. Call this once in your main loop.
/// Renders everything in z stages. Lower z values render first.
/// </summary>
void AnimationManager::updateSprites() {

	// make a compare lambda for the PQ
	auto compare = [](Sprite* left, Sprite* right) {
		// false if left >= right, otherwise true
		// (that has to be not >=, since comp(a,a) must be false)
		return !(left->getZlayer() >= right->getZlayer());
	};
	std::priority_queue<Sprite*, std::vector<Sprite*>, decltype(compare)> heap(compare);

	// push everything by zorder
	for (Sprite* e : sprites) {
		heap.push(e);
	}

	// then render in zorder
	while (!heap.empty()) {
		(heap.top())->render(camera);
		heap.pop();
	}

}

/// <summary>
/// Removes the input sprite from the Manager. This will delete it. It will NOT delete its
/// AFrame or any associated graphics; those remain for later use by other Sprites.
/// </summary>
/// <param name="sprite"></param>
void AnimationManager::removeSprite(Sprite* sprite) {
	int i = 0;
	for (Sprite* e : sprites) {
		if (e == sprite) {
			sprites.erase(sprites.begin() + i);
			break;
		}
		++i;
	}
	delete sprite;
}

/// <summary>
/// Updates the camera to a new set of values. It still maintains the old address for the camera, so
/// past references are valid while this one passed in isn't. That's probably bad design (TODO: fix that)
/// </summary>
/// <param name="camera"></param>
void AnimationManager::setCamera(SDL_Rect* camera) { 
	// we do a deep copy so that the other addresses remain valid
	this->camera->x = camera->x; 
	this->camera->y = camera->y;
	this->camera->w = camera->w;
	this->camera->h = camera->h;
}
SDL_Rect* AnimationManager::getCamera() { return camera; }

/// <summary>
/// Helper function. Call this in the main loop instead of SDL_RenderPresent.
/// 
/// To make resizing easier, we maintain a backbuffer texture and render all updates to that. It has a constant size. Then,
///  we render this to the screen, scaling and adjusting without stretching. If the aspect ratios don't match, this adds black
///  bars to the screen to prevent stretching. Note that you don't have to worry about switching the render targets. Just make sure the
///  backbuffer is still there.
/// </summary>
/// <param name="renderer">The active renderer.</param>
/// <param name="backbuffer">The backbuffer texture to render to the screen.</param>
/// <param name="screenHeight">The ACTUAL height of the window.</param>
/// <param name="screenWidth">The ACTUAL width of the window.</param>
void GE_PushFromBackbuffer(SDL_Renderer* renderer, SDL_Texture* backbuffer, int screenHeight, int screenWidth) {
	
	SDL_SetRenderTarget(renderer, NULL);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	
	// do some math to make the texture fit and scale
	int virtualHeight, virtualWidth;
	SDL_QueryTexture(backbuffer, NULL, NULL, &virtualWidth, &virtualHeight);
	double hScale = (double)screenHeight / (double)virtualHeight, wScale = (double)screenWidth / (double)virtualWidth;
	double scale = (hScale < wScale) ? hScale : wScale;

	SDL_Rect dest;
	dest.w = (int)(virtualWidth * scale);
	dest.h = (int)(virtualHeight * scale);

	if (dest.w != screenWidth) {
		dest.y = 0;
		dest.x = (screenWidth - dest.w) / 2;
	}
	else if (dest.h != screenHeight) {
		dest.x = 0;
		dest.y = (screenHeight - dest.h) / 2;
	}
	else {
		dest.x = 0;
		dest.y = 0;
	}

	SDL_RenderCopy(renderer, backbuffer, NULL, &dest);

	SDL_RenderPresent(renderer);

	SDL_SetRenderTarget(renderer, backbuffer);
}
