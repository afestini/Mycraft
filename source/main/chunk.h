#ifndef CHUNK_INCLUDED
#define CHUNK_INCLUDED

#define CHUNK_SIZE 16
#define CHUNK_SIZE 16
#define CHUNK_HEIGHT 128

#include <vector>
#include <queue>
#include <set>
#include <memory>

#include "block.h"
#include "vertex.h"
#include "VBuffer.h"
#include "nbt/nbt.h"
#include "item.h"
#include "tileentity.h"
#include "picking.h"


class Entity;
class TileEntity;

struct BlockPos
{
	unsigned char xz, y, shade, texType;
	BlockPos(unsigned char x, unsigned char y, unsigned char z, unsigned char light, unsigned char texType) 
		: xz((x << 4) | z), y(y), shade(17+light), texType(texType) {}

	BlockPos() : xz(0), y(0), shade(255), texType(0) {}
};

struct SearchEntry
{
	PickResult p;
	int originalLight;
	SearchEntry( const PickResult& p, int light ) : p(p), originalLight(light) {}
};

class Chunk
{
public:
	Chunk() 
		: x(0), z(0), dirty(false), loaded(false), buffersInitialized(false),
		  north(0), east(0), south(0), west(0), 
		  numVertices(0), numFloatVertices(0), lowestVisBlock(0), highestVisBlock(0) 
	{
		memset(blocks, 0, sizeof(blocks));
	}

	Chunk(const Chunk&) = delete;
	Chunk(Chunk&&) = default;

	void setLocation(int x, int z, Chunk* north, Chunk* east, Chunk* south, Chunk* west);
	void setDirty() { dirty = true; }
	void setAdjacentChunksDirty(int x, int z);

	void initBuffers();
	void updateGeometry(std::vector<CompVertex>& tmpVBuffer, std::vector<FloatVertex>& tmpTransBuffer);
	bool removeBlock(char x, char y, char z);
	bool addBlock(char x, char y, char z, ItemType type);

	const Block* getBlock(int x, int y, int z) const;

	void fromNBT(NBT& nbt);
	void readBlockData(NBT& nbt);
	void readBlockLight(NBT& nbt);
	void readBlocks(NBT& nbt);
	void readSkyLight(NBT& nbt);
	void readHeightMap(NBT& nbt);

	void update(float dt);

	TileEntity* getTileEntity(int x, int y, int z);

	typedef std::deque<PickResult> QueueType;
	typedef std::deque<SearchEntry> DarkQueueType;

	void increaseSunLight(QueueType& q);
	void increaseBlockLight(QueueType& q);
	void decreaseSunLight(DarkQueueType& q);
	void decreaseBlockLight(DarkQueueType& q);

	Buffer vertexBuffer;
	Buffer vertexTransBuffer;

	NBT nbt;

	int x, z;
	bool dirty;
	bool loaded;
	bool buffersInitialized;

	Chunk *north, *east, *south, *west;

	unsigned numVertices, numFloatVertices;
	Block blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_HEIGHT];
	std::vector<Item> items;

	std::vector<std::unique_ptr<TileEntity>> tileEntities;

	unsigned char lowestVisBlock;
	unsigned char highestVisBlock;
};

extern Chunk::QueueType lightQueue;
extern Chunk::DarkQueueType darkQueue;

#endif