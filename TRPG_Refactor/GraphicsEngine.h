#pragma once

#include <vector>
#include <SDL.h>
#include <string>
#include <map>

class Sprite;

/**
	Frame -- a wrapper class for SDL_Textures.
	These should be unique -- each SDL_Texture loaded gets one Frame
	to wrap it in. The AssetManager class ensures this.
 */
class Frame {

public:
	//Frame();
	Frame(SDL_Renderer* renderer, SDL_Texture* graphic);
	// Frees the SDL_Texture ONLY, not the renderer
	~Frame();
	void queryWidthHeight(int* w, int* h);
	// Renders the entire texture to dst
	void render(SDL_Rect* dst);

private:
	// The texture this Frame draws with.
	SDL_Texture* texture;
	// The renderer this Frame uses to draw itself.
	SDL_Renderer* renderer;

};


/**
	Order -- an ordered list of Frames and offsets.
	The order of the Frames determines the animation order.
	The animation rate and texture scaling are also specified here.
	Primarily created by AssetManagers.
*/
class Order {

public:
	Order(double msPerFrame, std::vector<Frame*> frames, std::vector<SDL_Point> offsets, double scale);
	~Order();
	// calls the appropriate Frame::render() function of this order
	void drawFrame(int screenX, int screenY, int frame);
	double getMSPerFrame();
	size_t getLength();

private:
	double msPerFrame;
	std::vector<Frame*> frames;
	std::vector<SDL_Point> offsets;
	double scale;

};


class AFrame {

public:
	AFrame();
	~AFrame();
	void draw(int screenX, int screenY, std::string order, int frame);
	void addOrder(std::string name, double msPerFrame, std::vector<Frame*> frames, std::vector<SDL_Point> offsets, double scale);
	double getOrderMSPerFrame(std::string order);
	size_t getOrderLength(std::string order);

private:
	std::map<std::string, Order > orders;

};


class AssetManager {

public:
	AssetManager();
	~AssetManager();
	int loadAssets(SDL_Renderer* renderer, std::string assetDir);
	AFrame* getAFrame(std::string key);

private:
	std::map< std::string, AFrame > assets;
	std::vector< Frame* > frames;
	bool areTexturesLoaded;

};


typedef struct {
	Sprite* spr;
	SDL_Rect* cam;
} SpriteCallbackArg;


class Sprite {

public:
	Sprite(AFrame* frames, std::string order, SDL_Rect* camera);
	Sprite(AFrame* frames, std::string order, SDL_Rect* camera, int x, int y);
	~Sprite();
	void render(SDL_Rect* camera);
	static Uint32 callback_render(Uint32 interval, void* camera);
	SpriteCallbackArg* getCallbackArg();
	void setX(int newX);
	void setY(int newY);
	void setXY(int newX, int newY);
	int moveX(int xOffset);
	int moveY(int yOffset);

private:
	int x;
	int y;
	AFrame* graphics;
	std::string order;
	int orderPosition;
	size_t orderLength;
	int flags;
	SDL_TimerID callbackID;
	SpriteCallbackArg callbackArg;

};

class AnimationManager {

public:
	AnimationManager();
	~AnimationManager();
	Sprite* addSprite(AFrame* graphics, std::string order);
	void updateSprites();
	void removeSprite(Sprite* sprite);
	void setCamera(SDL_Rect* camera);
	SDL_Rect* getCamera();

private:
	std::vector<Sprite*> sprites;
	SDL_Rect* camera;

};


void GE_PushFromBackbuffer(SDL_Renderer* renderer, SDL_Texture* backbuffer, int screenHeight, int screenWidth);
