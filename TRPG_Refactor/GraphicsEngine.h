#ifndef GRAPHICSENGINE_H
#define GRAPHICSENGINE_H

#include <unordered_map>
#include <vector>
#include <list>
#include <string>
#include <map>

#include <SDL.h>

class Sprite;
class AnimationManager;

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
	void queryWidthHeight(int* w, int* h) const;
	// Renders the entire texture to dst
	void render(SDL_Rect* dst) const;

	// Since a Frame is responsible for deleting its texture,
	// we want to give it move semantics
	Frame(Frame&& rhs) noexcept;
	Frame& operator=(Frame&& rhs) noexcept;

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
	void drawFrame(int screenX, int screenY, int frame, double otherScale) const;
	// basic getters
	double getMSPerFrame() const;
	size_t getLength() const;
	void getWidthHeight(int* w, int* h, int frame) const;

private:
	double msPerFrame;
	// these frames should be in order
	// also note we use a vector of Frame* rather than Frame. Why?
	// Well, remember that Frame has no copy constructor or copy
	// assignment; only move semantics. Since AssetManager maintains
	// the actual memory for each Frame, we DO NOT want to copy them
	// here. (That would accidentally delete every Frame we had in
	// AssetManager!) We want a reference instead. Since references don't sit
	// well in vectors (or any container really), we use pointers.
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
	void draw(int screenX, int screenY, std::string order, int frame, double otherScale) const;
	// creates and adds a new order
	void addOrder(std::string name, double msPerFrame, std::vector<Frame*> frames, std::vector<SDL_Point> offsets, double scale);
	// getters
	double getOrderMSPerFrame(std::string order) const;
	size_t getOrderLength(std::string order) const;
	void getWidthHeight(int* w, int* h, std::string order, int frame) const;

private:
	std::map<std::string, Order> orders;

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
	// use this to supply AFrames for your Sprites. AFrames should
	// never be modified outside of AssetManager!!!
	const AFrame& getAFrame(std::string key);

private:
	std::map<std::string, AFrame> assets;
	// this is *essentially* a vector with extra steps. However, it offers something a vector
	// cannot -- insertions don't have the potential to invalidate all references. YAY!
	std::unordered_map<int, Frame> frames;
	int nextKey;
	// used internally to know whether loadAssets or the destructor should do stuff
	bool areTexturesLoaded;

};

// used by Sprites as args for the animation callback. Only spr is used right now as
// a pointer to the Sprite to update.
typedef struct sca_ {
	Sprite* spr;
	SDL_Rect* cam;

	sca_(Sprite* s, SDL_Rect* c) : spr{ s }, cam{ c } {}
	sca_() : sca_(NULL, NULL) {};
} SpriteCallbackArg;

/// <summary>
/// AnimationManager -- creates and manages memory used by Sprites, and bulk-renders
/// all sprites under its control.
/// </summary>
class AnimationManager {

public:
	AnimationManager(Uint32 msPerFrame = 17);
	~AnimationManager();
	// creates a new Sprite and adds it to the Manager
	//Sprite& addSprite(const AFrame& graphics, std::string order);
	// the Sprite copy methods use this one once they're done. TODO: encapsulate;
	// there's no reason for the user to call this
	void addSprite(const Sprite* s);
	// call this once per loop to render all Sprites this Manager manages
	void updateSprites() const;
	void removeSprite(const Sprite* sprite);
	// the memory address of the camera is unchanged after this operation (TODO: probably bad)
	void setCamera(SDL_Rect* camera);
	SDL_Rect* getCamera() const;

	//static Uint32 callback_render(Uint32 interval, void* sp);

private:
	// new design!! yay
	// AnimationManagers now only store addresses of
	// managed Sprite memory, and Sprites will auto
	// add and remove themselves as necessary. should
	// fix everything lol
	std::list<const Sprite*> sprites;
	SDL_Rect* camera;
	//SDL_TimerID callbackID;
	Uint32 msPerUpdate;

};

/// <summary>
/// Sprite -- a container for an AFrame and other rendering data. Automatically
/// updates the animation frame rendered to the screen via an SDL_Timer callback.
/// </summary>
class Sprite {

public:
	// x and y default to 0 in this case
	Sprite(const AFrame& frames, std::string order);
	Sprite(const AFrame& frames, std::string order, int x, int y, int zlayer, double scale);
	~Sprite();

	// Because Sprite has a non-static reference memeber, it doesn't
	// really know how to copy itself. We need to tell it how to.
	// God this was a headache to figure out
	Sprite(const Sprite& rhs);
	Sprite& operator=(Sprite rhs);
	// defining this here b/c idk this is confusing
	// "friend" makes this more portable or smth
	friend void swap(Sprite& a, Sprite& b) {
		using std::swap;
		
		// swap all the elements now
		swap(a.x, b.x);
		swap(a.y, b.y);
		swap(a.zlayer, b.zlayer);
		swap(a.scale, b.scale);
		swap(a.graphics, b.graphics);
		swap(a.order, b.order);
		swap(a.orderPosition, b.orderPosition);
		swap(a.orderLength, b.orderLength);
		swap(a.flags, b.flags);
		// NOPE we don't want to swap those!!!
		//swap(a.callbackID, b.callbackID);
		//swap(a.callbackArg, b.callbackArg);
	}

	// Realize this creates a huge problem: Sprites can be constructed
	// without being registered by the AnimationManager. Oops. But if
	// we make the AnimationManager a static member of the Sprite class,
	// then the Sprites can register themselves. Problem solved!
	static AnimationManager& getAnimator() { return animator; }
	static SDL_Rect* getAnimCamera() { return animator.getCamera(); }
	// Ignore all the extra problems this causes, like how to even
	// instantiate this thing in the first place ;)

	// renders the Sprite using the given camera (TODO: is that right?)
	void render(SDL_Rect* camera) const;
	// called by the SDL_Timer to update the animation frame
	static Uint32 callback_render(Uint32 interval, void* sp);
	// getters and setters
	SpriteCallbackArg* getCallbackArg();
	void setX(int newX);
	void setY(int newY);
	void setXY(int newX, int newY);
	// adds the input value to x and y, rather than updating it to the input
	int moveX(int xOffset);
	int moveY(int yOffset);
	int getX() const;
	int getY() const;
	int getZlayer() const;
	void setZlayer(int z);
	void setScale(double scale);
	double getScale() const;
	void getScaledWidthHeight(int* w, int* h) const;

private:
	static AnimationManager animator;

	int x;
	int y;
	int zlayer;
	double scale;
	// Even though I don't plan on changing
	// it, making this a const & is problematic
	// b/c of copy assignment so we make it a const
	// * instead
	const AFrame* graphics;
	std::string order;
	int orderPosition;
	size_t orderLength;
	// currently unused, but should basically be user data
	int flags;
	SDL_TimerID callbackID;
	SpriteCallbackArg callbackArg;

};

// renders the backbuffer to the window, adjusting scale and preventing stretching
void GE_PushFromBackbuffer(SDL_Renderer* renderer, SDL_Texture* backbuffer, int screenHeight, int screenWidth);


#endif
