#include "world.h"

#include "block.h"
#include "blockinfo.h"
#include "blockrenderinfo.h"

#include "pipelines/loadpipeline.h"
#include "pipelines/updatepipeline.h"

#include "player.h"
#include "crafting.h"

using namespace std;

extern Vertex cubeGeometry[];

#ifdef _DEBUG
	int MAP_SIZE = 9;
#else
	int MAP_SIZE = 32;
#endif


void World::init()
{
	ItemInfo::init();
	BlockInfo::init();
	BlockRenderInfo::init();
	Blueprint::init();
	Recipe::init();

	vrtBuffer.init();
	vrtBuffer.setData(cubeGeometry, sizeof(cubeGeometry));
}

void World::load(const std::string& path)
{
	folderPath = path;

	NBT nbt;
	nbt.load(folderPath + "/level.dat");
	worldTime = (unsigned)nbt["Data"]["Time"].valueLong();

	int size = JSApp::Config["WorldSize"];
	if (size > 0)
		MAP_SIZE = size;

	setSize(MAP_SIZE);

	size_t numItems = nbt["Data"]["Player"]["Inventory"].size();
	for (size_t i = 0; i < numItems; ++i)
	{
		NBTAccessor node = nbt["Data"]["Player"]["Inventory"][i];
		unsigned char slot = node["Slot"].valueByte();
		player.inventory[slot].id = ItemType(node["id"].valueShort());
		player.inventory[slot].damage = node["Damage"].valueShort();
		player.inventory[slot].count = node["Count"].valueByte();
	}
}


Chunk* World::getChunk(int x, int z)
{
	Chunk* chunk = 0;
	if ( x >= mapOffsetX && x - mapOffsetX < MAP_SIZE && z >= mapOffsetZ && z - mapOffsetZ < MAP_SIZE )
	{
		int ix = (x % MAP_SIZE); ix += (ix<0 ? MAP_SIZE : 0);
		int iz = (z % MAP_SIZE); iz += (iz<0 ? MAP_SIZE : 0);
		chunk = &chunks[ix*MAP_SIZE + iz];
	}
	return chunk;
}

Chunk* World::getChunkFromPosition(float x, float z)
{
	int cx = int(x / CHUNK_SIZE) - ((fmod(x, 16.f) < .0f) ? 1 : 0);
	int cz = int(z / CHUNK_SIZE) - ((fmod(z, 16.f) < .0f) ? 1 : 0);
	return getChunk(cx, cz);
}

void World::setSize(unsigned numChunks)
{
	MAP_SIZE = numChunks;
	chunks.resize(numChunks*numChunks);

	hasPosition = false;
	setPosition(mapOffsetX-MAP_SIZE/2, mapOffsetZ-MAP_SIZE/2);
}

void World::setPosition(int x, int z)
{
	if ( x != mapOffsetX || z != mapOffsetZ )
	{
		unsigned loaded = 0;
		double time = glfwGetTime();

		const int colShift = hasPosition ? x - mapOffsetX : MAP_SIZE;
		const int rowShift = hasPosition ? z - mapOffsetZ : MAP_SIZE;

		mapOffsetX = x;
		mapOffsetZ = z;
		hasPosition = true;

		const int fromX = (colShift < 0) ? x : x + MAP_SIZE - colShift;
		const int toX = fromX + abs(colShift);

		const int fromX2 = (colShift < 0) ? toX : x;
		const int toX2 = fromX2 + MAP_SIZE - abs(colShift);

		const int fromZ = (rowShift < 0) ? z : z + MAP_SIZE - rowShift;
		const int toZ = fromZ + abs(rowShift);

		loaded += executeLoadPipeline(*this, fromX, z, toX, z+MAP_SIZE);
		loaded += executeLoadPipeline(*this, fromX2, fromZ, toX2, toZ);

		time = (glfwGetTime() - time)*1000;
	
		Console::Output(string("Loaded ")+to_string(loaded)+" chunks in " + to_string(time) + "ms\n");
	}
}

static const float tickDuration = .05f;

void World::update( float dtime )
{
	worldTime += unsigned(dtime * timeFactor);

	frameTimeSum += dtime;

	const int maxTimeForUpdate = JSApp::Config["MaxUpdateTimePerFrame"];

	if ( hasDirtyChunks )
	{
		if ( maxTimeForUpdate > 0 )
		{
			if ( executeUpdatePipelineAsync(*this) )
				hasDirtyChunks = false;
		}
		else
		{
			executeUpdatePipeline(*this);
			hasDirtyChunks = false;
		}
	}

	updateUpdatePipelineAsync(maxTimeForUpdate);

	while (frameTimeSum >= tickDuration)
	{
		for (Chunk& chunk : chunks)
			chunk.update(tickDuration);

		frameTimeSum -= tickDuration;
	}

	selection = pickBlock(JSApp::Camera, 15.0f);
}

void World::rebuildChunks()
{
	hasDirtyChunks = true;
	for (Chunk& chunk : chunks)
		chunk.setDirty();
}

unsigned World::daytime() const
{
	return (worldTime+6000)%24000;
}

float World::darkness() const
{
	const unsigned startDawn = 5000, endDawn = 7000;
	const unsigned startDusk = 20000, endDusk = 22000;

	//Dusk 90min: 1500
	unsigned timeOfDay = daytime();
	if (timeOfDay > startDawn && timeOfDay < endDawn)
		return 11.f * (endDawn-timeOfDay)/(endDawn-startDawn);
	else if (timeOfDay >= endDawn && timeOfDay <= startDusk)
		return 0;
	else if (timeOfDay > startDusk && timeOfDay < endDusk)
		return 11.f * (timeOfDay-startDusk)/(endDusk-startDusk);
	else
		return 11.f;
}
