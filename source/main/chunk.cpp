#include "chunk.h"

#include <string>

#include "nbt/nbt.h"
#include "nbt/nbtnode.h"
#include "Console.h"
#include "ProfileCounter.h"

#include "world.h"
#include "block.h"
#include "blockinfo.h"
#include "blockrenderinfo.h"


#include "fluids.h"
#include "tileentity.h"

#include <malloc.h>

using namespace std;

enum CUBE_SIDE {CS_LEFT = 0, CS_RIGHT, CS_FRONT, CS_BACK, CS_TOP, CS_BOTTOM};

Chunk::QueueType lightQueue;
Chunk::DarkQueueType darkQueue;


void Chunk::setLocation(int new_x, int new_z, Chunk* new_north, Chunk* new_east, Chunk* new_south, Chunk* new_west)
{
	if ( new_x != x || new_z != z )
	{
		loaded = false;
		numVertices = 0;
		numFloatVertices = 0;
	}

	x = new_x;
	z = new_z;

	if (north) north->south = 0;
	if (west) west->east = 0;
	if (south) south->north = 0;
	if (east) east->west = 0;

	north = new_north;
	east = new_east;
	south = new_south;
	west = new_west;
	
	if (new_north) new_north->south = this;
	if (new_south) new_south->north = this;
	if (new_east) new_east->west = this;
	if (new_west) new_west->east = this;
}

void Chunk::initBuffers()
{
	vertexBuffer.init();
	vertexTransBuffer.init();
	buffersInitialized = true;
}

void Chunk::readBlocks(NBT& nbt)
{
	int i = 0;
	const int32 size = nbt.readSize();
	uint8 data[0x8000];
	nbt.readArray(&data[0], size);

	for (unsigned char x=0; x<CHUNK_SIZE; ++x)
	for (unsigned char z=0; z<CHUNK_SIZE; ++z)
	for (unsigned char y=0; y<CHUNK_HEIGHT; ++y)
	{
		blocks[x][z][y].type = data[i++];
	}
}

void Chunk::readBlockData(NBT& nbt)
{
	int i = 0;
	const int32 size = nbt.readSize();
	uint8 data[0x4000];
	nbt.readArray(data, size);

	for (unsigned char x=0; x<CHUNK_SIZE; ++x)
	for (unsigned char z=0; z<CHUNK_SIZE; ++z)
	for (unsigned char y=0; y<CHUNK_HEIGHT;)
	{
		blocks[x][z][y++].data = data[i] & 0xf;
		blocks[x][z][y++].data = data[i] >> 4;
		++i;
	}
}

void Chunk::readSkyLight(NBT& nbt)
{
	int i = 0;
	const int32 size = nbt.readSize();
	uint8 data[0x4000];
	nbt.readArray(data, size);

	for (unsigned char x=0; x<CHUNK_SIZE; ++x)
	for (unsigned char z=0; z<CHUNK_SIZE; ++z)
	for (unsigned char y=0; y<CHUNK_HEIGHT;)
	{
		blocks[x][z][y++].light = data[i] & 0xf;
		blocks[x][z][y++].light = data[i] >> 4;
		++i;
	}
}

void Chunk::readBlockLight(NBT& nbt)
{
	int i = 0;
	const int32 size = nbt.readSize();
	uint8 data[0x4000];
	nbt.readArray(data, size);

	for (unsigned char x=0; x<CHUNK_SIZE; ++x)
	for (unsigned char z=0; z<CHUNK_SIZE; ++z)
	for (unsigned char y=0; y<CHUNK_HEIGHT;)
	{
		blocks[x][z][y++].blocklight = data[i] & 0xf;
		blocks[x][z][y++].blocklight = data[i] >> 4;
		++i;
	}
}

void Chunk::readHeightMap(NBT& nbt)
{
	const int32 size = nbt.readSize();
	uint8* data = (uint8*)alloca(size);
	nbt.readArray(data, size);
}

void Chunk::fromNBT(NBT& nbt)
{
	nbt.compressedData.initDecompression();

	TAG_TYPE tag;
	string name;

	while ( (tag = nbt.readTag()) != TAG_END )
	{
		name = nbt.readString();

		if ( name == "" || name == "Level" )
			;

		else if (name == "Data")
			readBlockData(nbt);

		else if (name == "SkyLight")
			readSkyLight(nbt);

		else if (name == "BlockLight")
			readBlockLight(nbt);

		else if (name == "Blocks")
			readBlocks(nbt);

		else if (name == "HeightMap")
			readHeightMap(nbt);

		else if (name == "xPos")
			x = nbt.readInt();

		else if (name == "zPos")
			z = nbt.readInt();

		else if (name == "TerrainPopulated")
			nbt.readByte();

		else if (name == "LastUpdate")
			nbt.readLong();

		else if (name == "Entities")
		{
			nbt.readTag();
			int32 num = nbt.readSize();
			while (num--)
				NodeCompound(nbt.compressedData);
		}

		else if (name == "TileEntities")
		{
			nbt.readTag();
			int32 num = nbt.readSize();

			while (num--)
			{
				if ( TileEntity* te = TileEntity::readTileEntity(nbt) )
					tileEntities.emplace_back(te);
			}
		}

		else
			nbt.skipTag(tag);
	}

	nbt.compressedData.endDecompression();

	if ( north ) north->setDirty();
	if ( east ) east->setDirty();
	if ( south ) south->setDirty();
	if ( west ) west->setDirty();

	dirty = true;
	loaded = true;
}


void Chunk::setAdjacentChunksDirty(int x, int z)
{
	if (east && x == CHUNK_SIZE-1)
		east->setDirty();
	if (west && x == 0)
		west->setDirty();
	if (south && z == CHUNK_SIZE-1)
		south->setDirty();
	if (north && z == 0)
		north->setDirty();
}

void Chunk::updateGeometry(vector<CompVertex>& tmpVBuffer, vector<FloatVertex>& tmpVFloatBuffer)
{
	static const Block stoneDummy = { 1, 0, 0 };
	static const Block airDummy = { 0, 15, 0 };

	lowestVisBlock = CHUNK_HEIGHT;
	highestVisBlock = 0;

	tmpVBuffer.clear();
	tmpVFloatBuffer.clear();

	for (unsigned char x=0; x<CHUNK_SIZE; ++x)
	for (unsigned char z=0; z<CHUNK_SIZE; ++z)
	for (unsigned char y=0; y<CHUNK_HEIGHT; ++y)
	{
		Block& block = blocks[x][z][y];

		if ( block.type )
		{
			//We cheat and pretend level is surrounded by stone, except for the top
			const Block& left = (x > 0) ? blocks[x-1][z][y] : (west && west->loaded ? west->blocks[CHUNK_SIZE-1][z][y] : stoneDummy);
			const Block& right = (x < CHUNK_SIZE-1) ? blocks[x+1][z][y] : (east && east->loaded ? east->blocks[0][z][y] : stoneDummy);

			const Block& front = (z < CHUNK_SIZE-1) ? blocks[x][z+1][y] : (south && south->loaded ? south->blocks[x][0][y] : stoneDummy);
			const Block& back =(z > 0) ? blocks[x][z-1][y] : (north && north->loaded ? north->blocks[x][CHUNK_SIZE-1][y] : stoneDummy);

			const Block& bottom = (y > 0) ? blocks[x][z][y-1] : stoneDummy; 
			const Block& top = (y < CHUNK_HEIGHT-1) ? blocks[x][z][y+1] : airDummy;

			unsigned visibleDirections =(BlockRenderInfo::get(left.type).visibleDirections & 1) |
										(BlockRenderInfo::get(right.type).visibleDirections & 2) |
										(BlockRenderInfo::get(front.type).visibleDirections & 4) |
										(BlockRenderInfo::get(back.type).visibleDirections & 8) |
										(BlockRenderInfo::get(top.type).visibleDirections & 16) |
										(BlockRenderInfo::get(bottom.type).visibleDirections & 32);

			if ( block.type == SNOW || block.type == STILL_LAVA || block.type == STILL_WATER )
					visibleDirections |= 16;

			if (BlockRenderInfo::get(block.type).hideSameType)
				visibleDirections &= ~(((block.type == left.type) << 0) | ((block.type == right.type) << 1) | ((block.type == front.type ) << 2) |
										((block.type == back.type) << 3) | ((block.type == top.type) << 4) | ((block.type == bottom.type) << 5));

			if (visibleDirections)
			{
				if ( block.type == FENCE )
					block.variant = (left.type == FENCE)<<0 | (right.type == FENCE)<<1 | (back.type == FENCE)<<2 | (front.type == FENCE)<<3;
			
				else if ( BlockRenderInfo::VariantFunc getVariant = BlockRenderInfo::get(block.type).variantFunction )
					block.variant = getVariant(block.data, this->x*CHUNK_SIZE + x, y, this->z*CHUNK_SIZE + z);

				else block.variant = 0;

				if ((block.type == STILL_WATER || block.type == STILL_LAVA) && ((block.data&7) || top.data == block.data))
				{
					appendFluidGeometry(x, y, z, tmpVFloatBuffer, visibleDirections, *this);
				}
				else if (BlockRenderInfo::get(block.type).specialGeometry)
				{
					BlockRenderInfo::get(block.type).appendGeometry(block, tmpVFloatBuffer,x,y,z, visibleDirections, this->x*CHUNK_SIZE, this->z*CHUNK_SIZE);
				}
				else
				{
					if (ambientOcclusion)
						BlockRenderInfo::get(block.type).appendGeometryAmbient(block, tmpVBuffer,x,y,z, visibleDirections, *this);
					else
						BlockRenderInfo::get(block.type).appendGeometry(block, tmpVBuffer,x,y,z, visibleDirections, this->x*CHUNK_SIZE, this->z*CHUNK_SIZE);
				}						

				lowestVisBlock = min<unsigned char>(lowestVisBlock, y);
				highestVisBlock = max<unsigned char>(highestVisBlock, y);
			}
		}
	}

	++highestVisBlock; //Top of block
	dirty = false;
}


void getLightFromNeighbors(PickResult& pr)
{
	Block* block = pr.block();
	const BlockInfo& info = BlockInfo::get(block->type);

	const Block* top = getNeighbor(pr, DIR_PY).block();
	if (!top || top->light == 15)
		block->light = 15 - info.lightReduction;

	for (int d = 0; d < 6; ++d)
	{
		if ( const Block* b = getNeighbor( pr, DIRECTION(d) ).block() )
		{
			block->light = max<int>(block->light, b->light - (info.lightReduction + 1));
			block->blocklight = max<int>(block->blocklight, b->blocklight - (info.lightReduction + 1));
		}
	}
}


bool Chunk::removeBlock(char x, char y, char z)
{
	Block& b = blocks[x][z][y];

	bool modified = (b.type != 0);
	if (modified)
	{
		const BlockInfo& info = BlockInfo::get(b.type);
		b.type = 0;

		if (info.blockLightBrightness > 0)
		{
			darkQueue.push_back( SearchEntry( PickResult(this, x, y, z), b.blocklight) );
			b.blocklight = 0;
			decreaseBlockLight(darkQueue);
		}

		if (info.lightReduction > 0)
		{
			PickResult pr(this, x, y, z);
			getLightFromNeighbors(pr);
	
			lightQueue.push_back(pr);
			increaseSunLight(lightQueue);

			lightQueue.push_back(pr);
			increaseBlockLight(lightQueue);
		}

		dirty = true;
		setAdjacentChunksDirty(x,z);
		World::instance().hasDirtyChunks = true;
	}	
	return modified;
}

bool Chunk::addBlock(char x, char y, char z, ItemType type)
{
	Block& b = blocks[x][z][y];
	bool modified = (b.type == 0 && type < 256);
	if (modified)
	{
                b.type = (unsigned char)(type);
		const BlockInfo& info = BlockInfo::get(b.type);

		if (info.lightReduction > 0)
		{
			PickResult pr(this, x, y, z);
			darkQueue.push_back( SearchEntry(pr, b.light) );
			b.light = max(0, b.light - info.lightReduction);
			decreaseSunLight(darkQueue);

			if (b.blocklight > info.blockLightBrightness)
			{
				darkQueue.push_back( SearchEntry(pr, b.blocklight) );
				b.blocklight = info.blockLightBrightness;
				decreaseBlockLight(darkQueue);
			}
		}

		if (info.blockLightBrightness > 0)
		{
			PickResult pr(this, x, y, z);
			getLightFromNeighbors(pr);
	
			b.blocklight = info.blockLightBrightness;

			lightQueue.push_back(pr);
			increaseBlockLight(lightQueue);
		}

		dirty = true;
		setAdjacentChunksDirty(x,z);
		World::instance().hasDirtyChunks = true;
	}	
	return modified;
}

const Block* Chunk::getBlock(int x, int y, int z) const
{
	const Chunk* c = this;
	if (x < 0) { c = c->west; x = CHUNK_SIZE-1; }
	else if (x >= CHUNK_SIZE) { c = c->east; x = 0; }

	if (c)
	{
		if (z < 0) { c = c->north; z = CHUNK_SIZE-1; }
		else if (z >= CHUNK_SIZE) { c = c->south; z = 0; }

		if ( y < 0 || y >= CHUNK_HEIGHT ) c = 0;
	}

	return c ? &c->blocks[x][z][y] : 0;
}


void Chunk::increaseSunLight(QueueType& q)
{
	double time = glfwGetTime();

	while (!q.empty())
	{
		PickResult& p = q.front();
		const int light = p.block()->light;

		for (int d = 0; d < 6; ++d)
		{
			PickResult n = getNeighbor( p, DIRECTION(d) );
			if ( Block* b = n.block() )
			{
				const int newLight = (light == 15 && d == DIR_NY) ? 
									  light - BlockInfo::get(b->type).lightReduction : 
									  light - max(BlockInfo::get(b->type).lightReduction, 1);

				if (newLight > b->light)
				{
					b->light = newLight;
					q.push_back(n);
				}
			}
		}
		p.chunk->setAdjacentChunksDirty(p.x,p.z);

		q.pop_front();
	}

	time = (glfwGetTime() - time)*1000;
	Console::Output(string("Sunlight increase updated in ") + to_string(time) + "ms\n");
}


void Chunk::decreaseSunLight(DarkQueueType& q)
{
	double time = glfwGetTime();

	while (!q.empty())
	{
		PickResult& p = q.front().p;
		const BlockInfo& info = BlockInfo::get(p.block()->type);
		const int light = q.front().originalLight;

		for (int d = 0; d < 6; ++d)
		{
			PickResult n = getNeighbor( p, DIRECTION(d) );
			if ( Block* b = n.block() )
			{
				const bool lightsUs = (b->light == 15 && d != DIR_NY ) || 
									  (b->light < 15 && b->light >= light + info.lightReduction);

				if (lightsUs)
					lightQueue.push_back(n);
				
				else if ( (b->light == 15 && d == DIR_NY) || 
						  (b->light == light - BlockInfo::get(b->type).lightReduction - 1) )
				{
					q.push_back( SearchEntry(n, b->light) );
					b->light = max(0, p.block()->light - BlockInfo::get(b->type).lightReduction - 1);
				}
			}
		}
		p.chunk->setAdjacentChunksDirty(p.x,p.z);

		q.pop_front();
	}

	time = (glfwGetTime() - time)*1000;
	Console::Output(string("Sunlight decrease updated in ") + to_string(time) + "ms\n");

	increaseSunLight(lightQueue);
}


void Chunk::increaseBlockLight(QueueType& q)
{
	double time = glfwGetTime();

	while (!q.empty())
	{
		PickResult& p = q.front();
		const int blocklight = p.block()->blocklight;

		for (int d = 0; d < 6; ++d)
		{
			PickResult n = getNeighbor( p, DIRECTION(d) );
			if ( Block* b = n.block() )
			{
				const int oldLight = b->blocklight;
				const int newLight = blocklight - max(BlockInfo::get(b->type).lightReduction, 1);
				b->blocklight = max(oldLight, newLight);
				if (b->blocklight > oldLight)
					q.push_back(n);
			}
		}
		p.chunk->setAdjacentChunksDirty(p.x,p.z);

		q.pop_front();
	}

	time = (glfwGetTime() - time)*1000;
	Console::Output(string("Blocklight increase updated in ") + to_string(time) + "ms\n");
}

void Chunk::decreaseBlockLight(DarkQueueType& q)
{
	double time = glfwGetTime();

	while (!q.empty())
	{
		PickResult& p = q.front().p;
		const int blocklight = q.front().originalLight;
		p.block()->blocklight = BlockInfo::get(p.block()->type).blockLightBrightness;

		for (int d = 0; d < 6; ++d)
		{
			PickResult n = getNeighbor( p, DIRECTION(d) );
			if ( Block* b = n.block() )
			{
				const bool lightsUs = b->blocklight >= blocklight + BlockInfo::get(p.block()->type).lightReduction;

				if (lightsUs)
					lightQueue.push_back(n);
				
				else if ( b->blocklight == blocklight - BlockInfo::get(b->type).lightReduction - 1 )
				{
					q.push_back( SearchEntry(n, b->blocklight) );
					b->blocklight = max(BlockInfo::get(b->type).blockLightBrightness, p.block()->blocklight - BlockInfo::get(b->type).lightReduction - 1);
				}
			}
		}
		p.chunk->setAdjacentChunksDirty(p.x,p.z);

		q.pop_front();
	}

	time = (glfwGetTime() - time)*1000;
	Console::Output(string("Blocklight decrease updated in ") + to_string(time) + "ms\n");

	increaseBlockLight(lightQueue);
}

void Chunk::update(float dt)
{
	for (auto& te : tileEntities)
		te->update(dt);
}

TileEntity* Chunk::getTileEntity(int x, int y, int z)
{
	x += this->x * CHUNK_SIZE;
	z += this->z * CHUNK_SIZE;

	for (auto& t : tileEntities)
	{
		if (t->x == x && t->y == y && t->z == z)
			return t.get();
	}
	return 0;
}
