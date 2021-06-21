#include <stdio.h>
#include <string>
#include <fstream>
#include <sstream>
#include <regex>
#include <unordered_map>

#include <SDL.h>
#include <SDL_image.h>

#include "GraphicsEngine.h"
#include "Tiles.h"

#define MAPR_HEADER_SIZE	8
#define MAPR_NAME_SIZE		31
#define MAPR_DIM_SIZE		4
#define MAPR_DIM_DELIM_SIZE	2
#define MAPR_ZLAYER_SIZE	11
#define MAPR_ZLAYER_POS		7
#define MAPR_PAL_START_SIZE	10
#define MAPR_PAL_INDEX_SIZE	5
#define MAPR_PAL_ENTRY_SIZE	51
#define MAPR_PAL_END_SIZE	2
#define MAPR_TILE_SIZE		5

FrameOrder::FrameOrder(const AFrame& frame, std::string order) :
	frame{ frame },
	order{ order } {}

Tile::Tile(const AFrame& graphic, std::string order, int x, int y, double size) :
	graphic{graphic, order},
	order{ order },
	x{ x },
	y{ y },
	size{ size }
	{ updateInternal(); }

Tile::~Tile() {
	//animator = NULL;
}

void Tile::updatePos(int nx, int ny) {
	x = nx;
	y = ny;
	updateInternal();
}

void Tile::updateScale(double nsize) {
	size = nsize;
	updateInternal();
}

void Tile::updateInternal() {
	this->graphic.setScale(this->size);
	int w{ 0 }, h{ 0 };
	graphic.getScaledWidthHeight(&w, &h);
	this->graphic.setXY(this->x * w, this->y * h);
}



Layer::Layer(AssetManager& assets, std::string mappath, double scale) :
	assets{ assets },
	map{ },
	width{ },
	height{ },
	zlayer{ },
	mapName{ },
	isInit{ false },
	scale{ scale }
{
	if (!loadMap(mappath)) {
		printf("ERROR: Layer::loadMap returned error state.\n");
	}
}

Layer::~Layer() {
	//animator = NULL;
	// TODO: probably destroy all the Tiles
}

bool Layer::loadMap(std::string mappath) {
	
	if (isInit) {
		printf("ERROR: Layer::loadMap tried to load after initialization.\n");
		return false;
	}

	std::ifstream mapFile{ mappath.c_str() };
	if (!mapFile) {
		printf("ERROR: Layer::loadMap could not load map file at %s.\n", mappath.c_str());
		return false;
	}

	char header[MAPR_HEADER_SIZE];
	char mapName[MAPR_NAME_SIZE];
	char width[MAPR_DIM_SIZE];
	char height[MAPR_DIM_SIZE];
	char zlayer[MAPR_ZLAYER_SIZE];
	char paletteStart[MAPR_PAL_START_SIZE];
	char paletteBuf[MAPR_PAL_INDEX_SIZE];
	char entryBuf[MAPR_TILE_SIZE];

	// make sure we're working with a real map file
	mapFile.getline(header, MAPR_HEADER_SIZE);
	if (std::strncmp(header, "MAPFILE", MAPR_HEADER_SIZE) != 0) {
		printf("ERROR: Layer::loadMap loaded invalid map file at %s.\n", mappath.c_str());
		return false;
	}
	// we don't do any more syntax checking -- if it's wrong, it'll blow up (perhaps silently), so be careful
	// Remember that whitespace is important -- no extra newlines or spaces or tabs (unless required)

	mapFile.getline(mapName, MAPR_NAME_SIZE);
	mapFile.getline(width, MAPR_DIM_SIZE, ' ');
	// this read gets clobbered right afterwards (it just flushes the " x ")
	mapFile.getline(height, MAPR_DIM_DELIM_SIZE, ' ');
	mapFile.getline(height, MAPR_DIM_SIZE);
	mapFile.getline(zlayer, MAPR_ZLAYER_SIZE);
	mapFile.getline(paletteStart, MAPR_PAL_START_SIZE);
	
	this->width = std::stoi(width);
	this->height = std::stoi(height);
	std::string zlayerstd(zlayer);
	this->zlayer = std::stoi(zlayerstd.substr(MAPR_ZLAYER_POS));
	this->mapName.assign(mapName);
	
	// read palette entries
	std::unordered_map<int, FrameOrder> palette;
	mapFile.getline(paletteBuf, MAPR_PAL_INDEX_SIZE, '\t');
	while (paletteBuf[0] != '}') {
		char buffer[MAPR_PAL_ENTRY_SIZE];
		mapFile.getline(buffer, MAPR_PAL_ENTRY_SIZE);

		int thisIndex = std::stoi(paletteBuf);
		std::string bufferstd(buffer);
		size_t colonIndex = bufferstd.find(":");
		// i lied we check for this too, though we'd likely hit an out of bounds exception either way
		if (colonIndex == std::string::npos) {
			printf("ERROR: Layer::loadMap hit invalid palette entry %s.\n", buffer);
			// probably memory leaks if this happens but eh whatever, it's broke here anyways
			return false;
		}

		std::string assetName = bufferstd.substr(0, colonIndex);
		// debug
		printf("assetName: %s\n", assetName.c_str());
		
		// +2 gets to the start of the string after the "::"
		// we don't check for out of bounds here ( it should break anyway, but lazy :( )
		std::string orderName = bufferstd.substr(colonIndex + 2);
		// debug
		printf("orderName: %s\n", orderName.c_str());

		FrameOrder animFrame{assets.getAFrame(assetName), orderName};
		palette.emplace(thisIndex, animFrame);

		mapFile.getline(paletteBuf, MAPR_PAL_INDEX_SIZE, '\t');
	}

	map.clear();
	for (int i = 0; i < this->height; ++i) {
		std::vector<Tile> row;
		row.clear();
		// since the last index has a \n delim instead of a \t, we treat it differently
		for (int j = 0; j < this->width - 1; ++j) {
			mapFile.getline(entryBuf, MAPR_TILE_SIZE, '\t');
			FrameOrder thisFrame = palette.at(std::stoi(entryBuf));
			// we now know enough to make our Tile for this position
			//Tile k();
			row.emplace_back(thisFrame.frame, thisFrame.order, j, i, scale);
		}
		// grab our last index and make the Tile
		mapFile.getline(entryBuf, MAPR_TILE_SIZE);
		FrameOrder thisFrame = palette.at(std::stoi(entryBuf));
		Tile k{ thisFrame.frame, thisFrame.order, this->width - 1, i, scale };
		row.push_back(k);
		// this row is complete; push it to the complete map and continue
		map.push_back(row);
	}
	
	setZLayer(this->zlayer);
	
	// if we made it here, we successfully init-ed
	isInit = true;
	return true;
}

void Layer::updateTile(int x, int y, std::string asset, std::string order) {
	Tile t{ assets.getAFrame(asset), order, x, y, map[y][x].getScale() };
	map[y][x] = t;
}

void Layer::setZLayer(int zlayer) {
	this->zlayer = zlayer;
	for (auto& i : map) {
		for (auto& j : i) {
			j.updateZLayer(zlayer);
		}
	}
}
