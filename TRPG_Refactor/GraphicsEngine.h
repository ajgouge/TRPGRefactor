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
	void render(SDL_Rect* dst);

private:
	SDL_Texture* texture;
	SDL_Renderer* renderer;

};


class Order {

public:
	Order(int msPerFrame, std::vector<Frame*> frames, std::vector<SDL_Point> offsets, double scale);
	~Order();

private:
	int msPerFrame;
	std::vector<Frame*> frames;
	std::vector<SDL_Point> offsets;
	double scale;

};


class AFrame {

public:
	AFrame();
	~AFrame();
	void update(SDL_Rect camera);
	void addOrder(std::string name, int msPerFrame, std::vector<Frame*> frames, std::vector<SDL_Point> offsets, double scale);

private:
	std::map<std::string, std::vector<Frame*> > orders;

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
	Sprite();
	~Sprite();
	void render(SDL_Rect camera);

private:
	int x;
	int y;
	std::vector<AFrame*> graphics;
	int AIndex;
	int flags;

};
