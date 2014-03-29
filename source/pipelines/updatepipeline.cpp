#include "updatepipeline.h"

#include <thread>
#include <functional>
#include <tbb/pipeline.h>
#include <tbb/tick_count.h>

#include "main/vertex.h"
#include "main/world.h"

using namespace std;
using namespace tbb;

struct UpdateData
{
	explicit UpdateData(Chunk& chunk) : chunk(chunk) {}

	UpdateData& operator=(const UpdateData&);

	Chunk& chunk;
	vector<CompVertex> vertexBuffer;
	vector<FloatVertex> transBuffer;
};

struct GetUpdatableChunks : public filter
{
	explicit GetUpdatableChunks(World& world) 
		: filter(serial_in_order), world(world), updated(0) 
	{
		vector<Chunk>::iterator it = world.chunks.begin();
		for (it; it != world.chunks.end(); ++it)
		{
			if (it->dirty && it->loaded)
				sortedChunks.push_back(&*it);
		}

		sort( sortedChunks.begin(), sortedChunks.end(), DistToPlayer(0, 0) );
	}

	void* operator()(void*)
	{
		UpdateData* data = 0;

		if (!sortedChunks.empty() )
		{
			data = new UpdateData( *sortedChunks.back() );
			sortedChunks.pop_back();
			++updated;
		}
		return data;
	}

	struct DistToPlayer
	{ 
		DistToPlayer(float px, float pz) : playerX(px), playerZ(pz) {}

		bool operator()(const Chunk* a, const Chunk* b) 
		{
			const Vector2 v1(a->x - playerX, a->z - playerZ);
			const Vector2 v2(b->x - playerX, b->z - playerZ);
			return v1.lensq() > v2.lensq();
		}

		float playerX, playerZ; 
	};

	World& world;
	vector<Chunk*> sortedChunks;
	unsigned updated;
};

struct UpdateChunk : public filter
{
	UpdateChunk() : filter(parallel) {}

	void* operator()(void* data)
	{
		UpdateData* updateData = static_cast<UpdateData*>(data);

		updateData->vertexBuffer.reserve(5000);
		updateData->transBuffer.reserve(1500);

		updateData->chunk.updateGeometry(updateData->vertexBuffer, updateData->transBuffer);

		return updateData;
	}
};


struct UploadChunkBuffer : public thread_bound_filter
{
	unsigned blockSize, transBlockSize;

	UploadChunkBuffer() : thread_bound_filter(serial_out_of_order), blockSize(0), transBlockSize(0) {}

	void* operator()(void* data)
	{
		UpdateData* updateData = static_cast<UpdateData*>(data);

		if (!updateData->chunk.buffersInitialized)
			updateData->chunk.initBuffers();

		if ( !updateData->vertexBuffer.empty() )
			updateData->chunk.vertexBuffer.setData(&updateData->vertexBuffer[0], updateData->vertexBuffer.size() * sizeof(updateData->vertexBuffer[0]));

		if ( !updateData->transBuffer.empty() )
			updateData->chunk.vertexTransBuffer.setData(&updateData->transBuffer[0], updateData->transBuffer.size() * sizeof(updateData->transBuffer[0]));

		updateData->chunk.numVertices = updateData->vertexBuffer.size();
		updateData->chunk.numFloatVertices = updateData->transBuffer.size();

		blockSize += updateData->chunk.numVertices * sizeof(CompVertex);
		transBlockSize += updateData->chunk.numFloatVertices * sizeof(FloatVertex);

		delete updateData;	

		return 0;
	}
};

struct UpdatePipeline
{
	explicit UpdatePipeline(World& world) : getChunks(world), updated(0)
	{
		pipe.add_filter(getChunks);
		pipe.add_filter(updateChunk);
		pipe.add_filter(uploadChunk);
	}

	GetUpdatableChunks getChunks;
	UpdateChunk updateChunk;
	UploadChunkBuffer uploadChunk;
	pipeline pipe;

	std::thread updateThread;
	int updated;

	void execute()
	{
		std::thread newThread( std::bind((void(pipeline::*)(size_t))&pipeline::run, &pipe, Globals::NThreads*4) );
		updateThread.swap(newThread);

		while(uploadChunk.process_item() != thread_bound_filter::end_of_stream) ;
		updateThread.join();

		updated = getChunks.updated;
	}

	void executeAsync()
	{
		std::thread newThread( std::bind((void(pipeline::*)(size_t))&pipeline::run, &pipe, max(1, Globals::NThreads - 1)) );
		updateThread.swap(newThread);
	}

	bool updateAsync(int maxTimeForUpdate)
	{
		thread_bound_filter::result_type result = thread_bound_filter::success;

		const tick_count startTicks = tick_count::now();
		while ( result == thread_bound_filter::success )
		{
			result = uploadChunk.process_item();
			++updated;

			const double timePassed = (tick_count::now() - startTicks).seconds() * 1000;
			if ( timePassed >= maxTimeForUpdate )
				break;
		}

		const bool finished = (result == thread_bound_filter::end_of_stream);
		if ( finished )
			updateThread.join();
		
		return finished;
	}
};

static UpdatePipeline *g_pipe = 0;

void executeUpdatePipeline(World& world)
{
	const tick_count startTicks = tick_count::now();

	UpdatePipeline pipe(world);
	pipe.execute();

	const double time = (tick_count::now() - startTicks).seconds() * 1000;

	Console::Output( string("Updated ")+to_string(pipe.updated)+" chunks in " + to_string(time) + "ms\n" );
	Console::Output( string("  Buffer size: ") + to_string(pipe.uploadChunk.blockSize/1024) + "/" + to_string(pipe.uploadChunk.transBlockSize/1024) + "kb\n" );
	Console::Output( string("  Chunk data size: ") + to_string((pipe.updated * sizeof(Chunk)) / 1024) + "kb\n");
}

bool executeUpdatePipelineAsync(World& world)
{
	const bool canStartUpdate = !g_pipe;
	if (canStartUpdate)
	{
		g_pipe = new UpdatePipeline(world);
		g_pipe->executeAsync();
	}
	return canStartUpdate;
}

void updateUpdatePipelineAsync(int maxTimeForUpdate)
{
	if (g_pipe && g_pipe->updateAsync(maxTimeForUpdate))
	{
		delete g_pipe;
		g_pipe = 0;
	}
}
