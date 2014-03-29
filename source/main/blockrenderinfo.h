#ifndef BLOCK_RENDER_INFO_INCLUDED
#define BLOCK_RENDER_INFO_INCLUDED

#include <vector>

#include "defines.h"
#include "vertex.h"
#include "../physics/collision.h"

struct Block;
class Chunk;

namespace tinyxml2 {
	class XMLElement;
}

struct TexCoords
{
	float u0, u1, v0, v1;
	unsigned char z;
	TexCoords() : u0(0), u1(1), v0(0), v1(1), z(0) {}
};

extern TexCoords blockTexcoords[256];

extern bool ambientOcclusion;

static Vertex cubeGeometry[] = 
{
	//Left - north
	Vertex(0,1,0, 0,0, -1, 0, 0), Vertex(0,0,0, 0,1, -1, 0, 0), Vertex(0,0,1, 1,1, -1, 0, 0), Vertex(0,1,1, 1,0, -1, 0, 0),
	//Right - south
	Vertex(1,1,1, 0,0,  1, 0, 0), Vertex(1,0,1, 0,1,  1, 0, 0), Vertex(1,0,0, 1,1,  1, 0, 0), Vertex(1,1,0, 1,0,  1, 0, 0),
	//Front - west
	Vertex(0,1,1, 0,0,  0, 0, 1), Vertex(0,0,1, 0,1,  0, 0, 1), Vertex(1,0,1, 1,1,  0, 0, 1), Vertex(1,1,1, 1,0,  0, 0, 1),
	//Back - east
	Vertex(1,1,0, 0,0,  0, 0,-1), Vertex(1,0,0, 0,1,  0, 0,-1), Vertex(0,0,0, 1,1,  0, 0,-1), Vertex(0,1,0, 1,0,  0, 0,-1), 
	//Top
	Vertex(0,1,0, 0,0,  0, 1, 0), Vertex(0,1,1, 0,1,  0, 1, 0), Vertex(1,1,1, 1,1,  0, 1, 0), Vertex(1,1,0, 1,0,  0, 1, 0),
	//Bottom
	Vertex(1,0,0, 0,0,  0,-1, 0), Vertex(1,0,1, 0,1,  0,-1, 0), Vertex(0,0,1, 1,1,  0,-1, 0), Vertex(0,0,0, 1,0,  0,-1, 0)
};

struct Face
{
	Face(const Vertex& v0, const Vertex& v1, const Vertex& v2, const Vertex& v3, unsigned visibleFrom)
		: visibleFromDirection(visibleFrom)
	{
		vertex[0] = v0;
		vertex[1] = v1;
		vertex[2] = v2;
		vertex[3] = v3;
	}
	Vertex vertex[4];
	unsigned visibleFromDirection;
};

class BlockRenderInfo
{
public:
	enum CollisionType { NONE, SOLID, FLUID };

	typedef const int (*VariantFunc)(int data, int x, int y, int z);

	BlockRenderInfo() : isTransparent(false), hideSameType(true),
					    isAnimated(false), specialGeometry(false),
					    spriteIcon(false), visibleDirections(0),
					    variantFunction(0), itemVariant(0),
						collisionType(SOLID) {}

	static void init();
	static const BlockRenderInfo& get(int);

	bool isTransparent;
	bool hideSameType;
	bool isAnimated;
	bool specialGeometry;
	bool spriteIcon;
	unsigned visibleDirections;

	VariantFunc variantFunction;
	int itemVariant;

	CollisionType collisionType;

	std::vector<Face> geometryVariants[16];
	std::vector<AABB> aabb[16];

	void appendGeometry(const Block& block, std::vector<FloatVertex>& buffer, 
								int x, int y, int z, unsigned visibleDirections, 
								int cx, int cz) const;

	void appendGeometry(const Block& block, std::vector<CompVertex>& buffer, 
								int x, int y, int z, unsigned visibleDirections, 
								int cx, int cz) const;
		
	void appendGeometryAmbient(const Block& block, std::vector<CompVertex>& buffer, 
						int x, int y, int z, unsigned visibleDirections, 
						const Chunk& chunk) const;
};

void readBlockRenderInfo(int, tinyxml2::XMLElement*);

#endif