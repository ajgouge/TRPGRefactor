#ifndef GRAPHICSENGINE_H
#define GRAPHICSENGINE_H

#include <vector>
#include <string>
#include <map>

#include <SDL.h>

class Sprite;

/// <summary>
/// Frame -- a wrapper class for SDL_Textures.
/// These should be unique -- each SDL_Texture loaded gets one Frame
/// to wrap it in. The AssetManager class ensures this.
/// </summary>
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


/// <summary>
///	Order -- an ordered list of Frames and offsets.
///	The order of the Frames determines the animation order.
///	The animation rate and texture scaling are also specified here.
///	Primarily created by AssetManagers.
/// </summary>
class Order {

public:
	Order(double msPerFrame, std::vector<Frame*> frames, std::vector<SDL_Point> offsets, double scale);
	~Order();
	// calls the appropriate Frame::render() function of this order
	void drawFrame(int screenX, int screenY, int frame);
	// basic getters
	double getMSPerFrame();
	size_t getLength();

private:
	double msPerFrame;
	// these frames should be in order
	std::vector<Frame*> frames;
	std::vector<SDL_Point> offsets;
	double scale;

};

/// <summary>
/// AFrame -- holds a collection of Orders to draw. Used primarily
/// by Sprites.
/// </summary>
class AFrame {

public:
	AFrame();
	~AFrame();
	// calls the Order::draw function on the given order
	void draw(int screenX, int screenY, std::string order, int frame);
	// creates and adds a new order
	void addOrder(std::string name, double msPerFrame, std::vector<Frame*> frames, std::vector<SDL_Point> offsets, double scale);
	// getters
	double getOrderMSPerFrame(std::string order);
	size_t getOrderLength(std::string order);

private:
	std::map<std::string, Order > orders;

};

/// <summary>
/// AssetManager -- loads and manages asset memory (textures stored in Frames and their parents Order and AFrame). This
/// is done once after initialization via the loadAssets function.
/// </summary>
class AssetManager {

public:
	AssetManager();
	~AssetManager();
	// call this before using the AssetManager for anything
	int loadAssets(SDL_Renderer* renderer, std::string assetDir);
	// use this to supply AFrames for your Sprites
	AFrame* getAFrame(std::string key);

private:
	std::map< std::string, AFrame > assets;
	std::vector< Frame* > frames;
	// used internally to know whether loadAssets or the destructor should do stuff
	bool areTexturesLoaded;

};

// used by Sprites as args for the animation callback. Only spr is used right now as
// a pointer to the Sprite to update.
typedef struct {
	Sprite* spr;
	SDL_Rect* cam;
} SpriteCallbackArg;

/// <summary>
/// Sprite -- a container for an AFrame and other rendering data. Automatically
/// updates the animation frame rendered to the screen via an SDL_Timer callback.
/// </summary>
class Sprite {

public:
	// x and y default to 0 in this case
	Sprite(AFrame* frames, std::string order, SDL_Rect* camera);
	Sprite(AFrame* frames, std::string order, SDL_Rect* camera, int x, int y);
	~Sprite();
	// renders the Sprite using the given camera (TODO: is that right?)
	void render(SDL_Rect* camera);
	// called by the SDL_Timer to update the animation frame
	static Uint32 callback_render(Uint32 interval, void* camera);
	// getters and setters
	SpriteCallbackArg* getCallbackArg();
	void setX(int newX);
	void setY(int newY);
	void setXY(int newX, int newY);
	// adds the input value to x and y, rather than updating it to the input
	int moveX(int xOffset);
	int moveY(int yOffset);

private:
	int x;
	int y;
	AFrame* graphics;
	std::string order;
	int orderPosition;
	size_t orderLength;
	// currently unused, but should basically be user data
	int flags;
	SDL_TimerID callbackID;
	SpriteCallbackArg callbackArg;

};

/// <summary>
/// AnimationManager -- creates and manages memory used by Sprites, and bulk-renders
/// all sprites under its control.
/// </summary>
class AnimationManager {

public:
	AnimationManager();
	~AnimationManager();
	// creates a new Sprite and adds it to the Manager
	Sprite* addSprite(AFrame* graphics, std::string order);
	// call this once per loop to render all Sprites this Manager manages
	void updateSprites();
	void removeSprite(Sprite* sprite);
	// the memory address of the camera is unchanged after this operation (TODO: probably bad)
	void setCamera(SDL_Rect* camera);
	SDL_Rect* getCamera();

private:
	std::vector<Sprite*> sprites;
	SDL_Rect* camera;

};

// renders the backbuffer to the window, adjusting scale and preventing stretching
void GE_PushFromBackbuffer(SDL_Renderer* renderer, SDL_Texture* backbuffer, int screenHeight, int screenWidth);


#endif