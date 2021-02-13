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
	Tile(AFrame* graphic, int x, int y, int size);
	~Tile();
	void updatePos(int nx, int ny);
	void updateScale(int nsize);

private:
	Sprite* graphic;
	int x;
	int y;
	int size;

};

/// <summary>
/// This is essentially a grid of Tiles. Generally, this works best
/// when initialized early and modified infrequently. If you want to move
/// stuff around, a plain Sprite is better for the job. Layers also
/// feature a fixed zlayer for all Tiles in the Layer.
/// </summary>
class Layer {

public:
	Layer(int width, int height);
	~Layer();
	

private:
	std::vector< std::vector<Tile> > map;
	int width, height;
	int zlayer;

};

#endif
