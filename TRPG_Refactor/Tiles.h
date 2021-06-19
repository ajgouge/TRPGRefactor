#ifndef TILES_H
#define TILES_H

#include <stdio.h>

#include <SDL.h>

#include "GraphicsEngine.h"

/// <summary>
/// This is purely a graphics concept for tile based rendering. This
/// is combined into Layers.
/// </summary>
class Tile {
	
public:
	Tile(AnimationManager& animator, const AFrame& graphic, std::string order, int x, int y, double size);
	~Tile();
	void updatePos(int nx, int ny);
	void updateScale(double nsize);

private:
	AnimationManager& animator;
	Sprite& graphic;
	std::string order;
	int x;
	int y;
	double size;

};

/// <summary>
/// Baby lil tuple class because VS is pissed at me
/// </summary>
class FrameOrder {
public:
	FrameOrder(const AFrame& frame, std::string order);

	const AFrame& frame;
	std::string order;
};

/// <summary>
/// This is essentially a grid of Tiles. Generally, this works best
/// when initialized early and modified infrequently. If you want to move
/// stuff around, a plain Sprite is better for the job. Layers also
/// feature a fixed zlayer for all Tiles in the Layer.
/// 
/// Note also that the default (soon to be hardcoded) behavior for the map
/// is for each member Tile to be square, ie height and width are the same.
/// TODO: (maybe) add support for rectangles other than squares (though
/// each tile having the same dimensions will likely remain a requirement)
/// 
/// TODO: (maybe) right now everything is loaded as a Sprite, which might be a little intensive.
/// These objects only move relative to the camera so it's a good idea to separate their
/// rendering from the AnimationManager (might save time in the rendering process). Perhaps
/// inserting the whole layer as one "Sprite" with one zlayer could speed things up a bit.
/// 
/// TODO: (one more ok?) the constructor requires a mappath right now and then loads a
/// map file every time. you can't make a Layer without a map file, so you can't really
/// make a custom Layer at runtime. Maybe add support for empty Layers that can be
/// populated on the fly? Not sure if that's useful yet
/// </summary>
class Layer {

public:
	Layer(AnimationManager& animator, AssetManager& assets, std::string mappath, double scale = 1);
	~Layer();
	//void setVisible(bool isVisible);
	//void updateTile(int x, int y, std::string asset, std::string order);
	//void setZLayer(int zlayer);

private:
	bool loadMap(std::string mappath);
	AnimationManager& animator;
	AssetManager& assets;
	std::vector< std::vector<Tile> > map;
	int width, height; // in Tiles, not pixels
	int zlayer;
	std::string mapName;
	bool isInit;
	double scale; // how much to resize each Tile AFrame by. 1 is default

};

#endif
