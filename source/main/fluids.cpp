#include "fluids.h"
#include "world.h"
#include "picking.h"
#include "block.h"
#include "blockrenderinfo.h"

using namespace std;

void appendFluidGeometry(int x, int y, int z, vector<FloatVertex>& vertexBuffer, unsigned visibleDirections, const Chunk& chunk)
{
	const Block* block = chunk.getBlock(x, y, z);
	const Block* top = chunk.getBlock(x, y+1, z);

	struct Coords { int x,y,z; };
	const Coords neighborCoords[8] = 
	{
		{x-1,y,z-1}, {x,y,z-1}, {x+1,y,z-1},
		{x-1,y,z  },			{x+1,y,z  },
		{x-1,y,z+1}, {x,y,z+1}, {x+1,y,z+1}
	};

	const float step = .12f;
	const float height = ((top && top->type == block->type) || (block->data & 8)) ? 1 : .85f - (block->data & 7) * step;
	float neighborHeight[8] = {0};

	for (int i=0; i<8; ++i)
	{
		const Block* neighbor = chunk.getBlock(neighborCoords[i].x, neighborCoords[i].y, neighborCoords[i].z);
		const Block* nTop = chunk.getBlock(neighborCoords[i].x, neighborCoords[i].y + 1, neighborCoords[i].z);

		if (neighbor && neighbor->type == block->type)
			neighborHeight[i] = ((nTop && nTop->type == block->type) || (neighbor->data & 8)) ? 1 : .85f - (neighbor->data & 7) * step;
	}

	float cornerHeights[4] = {height, height, height, height};
	
	cornerHeights[0] = max(cornerHeights[0], neighborHeight[0]);
	cornerHeights[0] = max(cornerHeights[0], neighborHeight[1]);
	cornerHeights[0] = max(cornerHeights[0], neighborHeight[3]);
	cornerHeights[1] = max(cornerHeights[1], neighborHeight[1]);
	cornerHeights[1] = max(cornerHeights[1], neighborHeight[2]);
	cornerHeights[1] = max(cornerHeights[1], neighborHeight[4]);
	cornerHeights[2] = max(cornerHeights[2], neighborHeight[3]);
	cornerHeights[2] = max(cornerHeights[2], neighborHeight[5]);
	cornerHeights[2] = max(cornerHeights[2], neighborHeight[6]);
	cornerHeights[3] = max(cornerHeights[3], neighborHeight[4]);
	cornerHeights[3] = max(cornerHeights[3], neighborHeight[6]);
	cornerHeights[3] = max(cornerHeights[3], neighborHeight[7]);

	const float x0 = float(x), x1 = x0+1;
	const float z0 = float(z), z1 = z0+1;
	const float y0 = float(y);
	const float y10 = y0 + cornerHeights[0];
	const float y11 = y0 + cornerHeights[1];
	const float y12 = y0 + cornerHeights[2];
	const float y13 = y0 + cornerHeights[3];
	const unsigned char tz = blockTexcoords[block->type].z;

	unsigned char blocklight = block->blocklight;
	unsigned char light = min<unsigned char>(block->light+3, 15);
	//Left
	if (visibleDirections & 1)
	{
		vertexBuffer.push_back( FloatVertex(x0,y0,z0, 0,1,tz,  blocklight, light) );
		vertexBuffer.push_back( FloatVertex(x0,y0,z1, 1,1,tz,  blocklight, light) );
		vertexBuffer.push_back( FloatVertex(x0,y12,z1, 1,0,tz, blocklight, light) );
		vertexBuffer.push_back( FloatVertex(x0,y10,z0, 0,0,tz, blocklight, light) );
	}
	//Right
	if (visibleDirections & 2)
	{
		vertexBuffer.push_back( FloatVertex(x1,y0,z0, 1,1,tz,  blocklight, light) );
		vertexBuffer.push_back( FloatVertex(x1,y11,z0, 1,0,tz, blocklight, light) );
		vertexBuffer.push_back( FloatVertex(x1,y13,z1, 0,0,tz, blocklight, light) ); 
		vertexBuffer.push_back( FloatVertex(x1,y0,z1, 0,1,tz,  blocklight, light) );
	}
		//Front
	if (visibleDirections & 4)
	{
		vertexBuffer.push_back( FloatVertex(x0,y0,z1, 0,1,tz,  blocklight, light) );
		vertexBuffer.push_back( FloatVertex(x1,y0,z1, 1,1,tz,  blocklight, light) );
		vertexBuffer.push_back( FloatVertex(x1,y13,z1, 1,0,tz, blocklight, light) );
		vertexBuffer.push_back( FloatVertex(x0,y12,z1, 0,0,tz, blocklight, light) );
	}
		//Back
	if (visibleDirections & 8)
	{
		vertexBuffer.push_back( FloatVertex(x0,y0,z0, 1,1,tz,  blocklight, light) );
		vertexBuffer.push_back( FloatVertex(x0,y10,z0, 1,0,tz, blocklight, light) );
		vertexBuffer.push_back( FloatVertex(x1,y11,z0, 0,0,tz, blocklight, light) );
		vertexBuffer.push_back( FloatVertex(x1,y0,z0, 0,1,tz,  blocklight, light) );
	}
		//Top
	if (visibleDirections & 16)
	{
		vertexBuffer.push_back( FloatVertex(x0,y10,z0, 0,0,tz, blocklight, light) );
		vertexBuffer.push_back( FloatVertex(x0,y12,z1, 0,1,tz, blocklight, light) );
		vertexBuffer.push_back( FloatVertex(x1,y13,z1, 1,1,tz, blocklight, light) );
		vertexBuffer.push_back( FloatVertex(x1,y11,z0, 1,0,tz, blocklight, light) );
	}
		//Bottom
	if (visibleDirections & 32)
	{	
		vertexBuffer.push_back( FloatVertex(x0,y0,z0, 0,0,tz, blocklight, light) );
		vertexBuffer.push_back( FloatVertex(x1,y0,z0, 1,0,tz, blocklight, light) );
		vertexBuffer.push_back( FloatVertex(x1,y0,z1, 1,1,tz, blocklight, light) );
		vertexBuffer.push_back( FloatVertex(x0,y0,z1, 0,1,tz, blocklight, light) );
	}
}
