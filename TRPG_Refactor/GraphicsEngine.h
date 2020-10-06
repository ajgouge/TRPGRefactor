#pragma once

#include <vector>
#include <SDL.h>
#include <string>
#include <map>

class Frame {

public:
	Frame();
	Frame(SDL_Renderer* renderer, SDL_Texture* graphic);
	~Frame();
	void queryWidthHeight(int* w, int* h);
	void render(SDL_Rect* dst);

private:
	SDL_Texture* texture;
	SDL_Renderer* renderer;

};


class Order {

public:
	Order(double msPerFrame, std::vector<Frame*> frames, std::vector<SDL_Point> offsets, double scale);
	~Order();
	void drawFrame(int screenX, int screenY, int frame);

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
	std::map< std::string, AFrame* > assets;
	std::vector< Frame* > frames;
	bool areTexturesLoaded;

};


class Sprite {

public:
	Sprite(AFrame* frames, std::string order);
	Sprite(AFrame* frames, std::string order, int x, int y);
	~Sprite();
	void render(SDL_Rect camera);

private:
	int x;
	int y;
	AFrame* graphics;
	std::string order;
	int flags;

};

class AnimationManager {

public:
	AnimationManager();
	~AnimationManager();
	Sprite* addSprite(AFrame* graphics, std::string order);
	void removeSprite(Sprite* sprite);
	void updateSprites(SDL_Rect camera);

private:
	std::vector<Sprite*> sprites;

};