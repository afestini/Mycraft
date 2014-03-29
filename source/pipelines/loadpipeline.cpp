#include "loadpipeline.h"

#include <fstream>
#include <tbb/pipeline.h>
#include <tuple>

#include "main/world.h"
#include "main/chunkcreator.h"
#include "nbt/nbt.h"

using namespace std;
using namespace tbb;

typedef map< std::tuple<int, int>, ifstream* > RegionFileTable;
RegionFileTable regionFileTable;

struct LoadChunkData
{
	LoadChunkData() : chunk(0) {}
	Chunk* chunk; NBT nbt; 
};

struct LoadNBTFile : public filter
{
	LoadNBTFile(int startX, int endX, int startZ, int endZ)
		: filter(serial_in_order),
		startX(startX), endX(endX), startZ(startZ), endZ(endZ),
		x(startX), z(startZ), loaded(0) {}

	int startX, endX, startZ, endZ;
	int x, z;
	unsigned loaded;

	inline void next()
	{
		if (++z >= endZ)
		{
			++x;
			z = startZ;
		}
	}

	void* operator()(void*)
	{
		World& world = World::instance();
		LoadChunkData* res = 0;

		while (!res && x < endX)
		{
			Chunk* chunk = world.getChunk(x, z);
			if (chunk)
			{
				chunk->setLocation(x, z, world.getChunk(x, z - 1), world.getChunk(x + 1, z), world.getChunk(x, z + 1), world.getChunk(x - 1, z));

				if (!chunk->loaded)
				{
					const int fileX = (x >= 0) ? x / 32 : -(31 - x) / 32;
					const int fileZ = (z >= 0) ? z / 32 : -(31 - z) / 32;

					ifstream*& streamPtr = regionFileTable[std::make_tuple(fileX, fileZ)];
					if (!streamPtr)
						streamPtr = new ifstream;

					ifstream& stream = *streamPtr;
					if (!stream.is_open())
					{
						const string filename = world.folderPath + "/region/r." + to_string(fileX) + "." + to_string(fileZ) + ".mcr";
						stream.open(filename.c_str(), ios::binary);
					}

					bool chunkFound = false;

					if (stream)
					{
						unsigned char buffer[5] = { 0 };

						stream.seekg(4 * ((x & 31) + (z & 31) * 32));
						stream.read((char*)buffer, 4);
						//3 bytes offset, 1 byte size in # sectors
						const unsigned offset = (buffer[0] << 16 | buffer[1] << 8 | buffer[2]) * 4096;
						if (offset > 0)
						{
							stream.seekg(offset);
							stream.read((char*)buffer, 4).read((char*)buffer + 4, 1);

							//Subtract 1 byte version
							int size = (buffer[0] << 24 | buffer[1] << 16 | buffer[2] << 8 | buffer[3]) - 1;

							res = new LoadChunkData;
							res->chunk = chunk;
							res->nbt.readFile(stream, size);

							chunkFound = true;
							++loaded;
						}
					}
#if 0					
					if (!chunkFound)
					{
						createChunk(*chunk, x, z);
						++loaded;
					}
#endif
				}
			}
			next();
		}
		return res;
	}
};

struct ReadNBTFile : public filter
{
	ReadNBTFile() : filter(parallel) {}

	void* operator()(void* data)
	{
		LoadChunkData* loadData = static_cast<LoadChunkData*>(data);
		loadData->chunk->fromNBT(loadData->nbt);
		return 0;
	}
};


unsigned executeLoadPipeline(World& world, int startX, int startZ, int endX, int endZ)
{
	LoadNBTFile loader(startX, endX, startZ, endZ);
	ReadNBTFile reader;
	
	tbb::pipeline pipe;
	pipe.add_filter(loader);
	pipe.add_filter(reader);
	pipe.run(Globals::NThreads*4);
	pipe.clear();

	if (loader.loaded > 0)
		world.hasDirtyChunks = true;

	return loader.loaded;
}
