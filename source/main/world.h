#ifndef WORLD_LOADER_INCLUDED
#define WORLD_LOADER_INCLUDED

#include <string>
#include <map>
#include <vector>

#include "VBuffer.h"
#include "chunk.h"
#include "picking.h"

extern int MAP_SIZE;

const float PLAYER_LIGHT = .97f;

class World
{
public:
	World() 
		: mapOffsetX(0), mapOffsetZ(0), hasPosition(false), hasDirtyChunks(false), 
		  playerLight(0), globalLight(0),
		  worldTime(0), timeFactor(0), frameTimeSum(0) {}

	static World& instance() { static World world; return world; }

	void init();
	void load(const std::string& path);
	void update( float dtime );
	void rebuildChunks();

	unsigned daytime() const;
	float darkness() const;

	Chunk* getChunk(int x, int z);
	Chunk* getChunkFromPosition(float x, float z);
	void setSize(unsigned chunks);
	void setPosition(int x, int z);

	std::vector<Chunk> chunks;
	Buffer vrtBuffer;

	int mapOffsetX, mapOffsetZ;
	bool hasPosition;
	bool hasDirtyChunks;
	float playerLight, globalLight;
	unsigned worldTime;
	float timeFactor;
	float frameTimeSum;
	PickResult selection;

	std::string folderPath;
};

#endif