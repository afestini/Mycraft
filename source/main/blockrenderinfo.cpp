#include "blockrenderinfo.h"
#include "block.h"
#include "blockinfo.h"
#include "chunk.h"
#include "picking.h"
#include "MatrixX.h"

#include <tinyxml2.h>

using namespace std;
using namespace tinyxml2;


bool ambientOcclusion = true;

BlockRenderInfo blockRenderInfo[256];
TexCoords blockTexcoords[256];

const float PI = 3.14159265f;

static const Matrix44 rot0Mat = Matrix44(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1);
static const Matrix44 rot90Mat = Matrix44(0,0,1,0, 0,1,0,0, -1,0,0,0, 1,0,0,1);
static const Matrix44 rot180Mat = Matrix44(-1,0,0,0, 0,1,0,0, 0,0,-1,0, 1,0,1,1);
static const Matrix44 rot270Mat = Matrix44(0,0,-1,0, 0,1,0,0, 1,0,0,0, 0,0,1,1);

Vertex operator*(const Matrix44& m, const Vertex& v)
{
	Vector4 pos = m * Vector4(v.x, v.y, v.z, 1);
	Vector4 norm = m * Vector4(v.nx, v.ny, v.nz, 0);
	return Vertex(pos.x, pos.y, pos.z, v.u, v.v, v.tz, norm.x, norm.y, norm.z);
}

struct TextureSet
{
	explicit TextureSet(int texID) 
	{
		for (int i=0; i<6; ++i)
			tex[i] = texID;
	}
	
	int tex[6];
};


void readBlockGeometryInfo(int id, XMLElement* xmlElement)
{
	int texID = 0;

	if ( xmlElement->QueryAttribute("textureID", &texID) == XML_SUCCESS )
		blockTexcoords[id].z = (unsigned char)(texID);

	int collisionType = BlockRenderInfo::SOLID;

	if ( xmlElement->QueryAttribute("collisionType", &collisionType) == XML_SUCCESS )
		blockRenderInfo[id].collisionType = (BlockRenderInfo::CollisionType)collisionType;

	XMLElement* g = xmlElement->FirstChildElement("geometry");
	if (!g)
	{
		for (int v=0; v<24; ++v) 
			cubeGeometry[v].tz = (unsigned char)(texID);

		vector<Face>& geo = blockRenderInfo[id].geometryVariants[0];
		for (int f=0; f<6; ++f)
			geo.push_back( Face(cubeGeometry[f*4], cubeGeometry[f*4+1], cubeGeometry[f*4+2], cubeGeometry[f*4+3], 1<<f) );
	}

	while (g)
	{
		int texture = texID;
		xmlElement->QueryAttribute("texture", &texture);
		TextureSet variantTextures(texture);

		int variant = 0;
		g->QueryAttribute("variant", &variant);
		g->QueryAttribute("textureLeft", &variantTextures.tex[0]);
		g->QueryAttribute("textureRight", &variantTextures.tex[1]);
		g->QueryAttribute("textureFront", &variantTextures.tex[2]);
		g->QueryAttribute("textureBack", &variantTextures.tex[3]);
		g->QueryAttribute("textureTop", &variantTextures.tex[4]);
		g->QueryAttribute("textureBottom", &variantTextures.tex[5]);

		vector<Face>& geo = blockRenderInfo[id].geometryVariants[variant];

		XMLElement* f = g->FirstChildElement("Face");
		if (!f)
		{
			for (int f=0; f<6; ++f)
			{
				for (int v=0; v<4; ++v)
					cubeGeometry[f*4 + v].tz = (unsigned char)(variantTextures.tex[f]);

				geo.push_back( Face(cubeGeometry[f*4], cubeGeometry[f*4+1], cubeGeometry[f*4+2], cubeGeometry[f*4+3], 1<<f) );
			}
		}

		while (f)
		{
			f = f->NextSiblingElement("Face");
		}

		if (blockRenderInfo[id].collisionType != BlockRenderInfo::NONE)
		{
			vector<AABB>& aabb = blockRenderInfo[id].aabb[variant];

			XMLElement* b = g->FirstChildElement("AABB");

			if (!b)
			{
				Vector3 min(1.f), max(.0f);
				for (const Face& face : geo)
				{
					for (int v=0; v<4; ++v)
					{
						min.x = std::min(face.vertex[v].x, min.x);
						min.y = std::min(face.vertex[v].y, min.y);
						min.z = std::min(face.vertex[v].z, min.z);
						max.x = std::max(face.vertex[v].x, max.x);
						max.y = std::max(face.vertex[v].y, max.y);
						max.z = std::max(face.vertex[v].z, max.z);
					}
				}
				aabb.push_back( AABB(min, max) );
			}

			while (b)
			{
				Vector3 min, max;
				g->QueryAttribute("minX", &min.x);
				g->QueryAttribute("minY", &min.y);
				g->QueryAttribute("minZ", &min.z);
				g->QueryAttribute("maxX", &max.x);
				g->QueryAttribute("maxY", &max.y);
				g->QueryAttribute("maxZ", &max.z);

				aabb.push_back( AABB(min, max) );
								
				b = b->NextSiblingElement("AABB");
			}
		}

		g = g->NextSiblingElement("geometry");
	}
}

void readBlockRenderInfo(int id, XMLElement* b)
{
	int lightReduction = 15;
	b->QueryAttribute("lightReduction", &lightReduction);
	
	blockRenderInfo[id].isTransparent = (lightReduction < 15);
	if (blockRenderInfo[id].isTransparent)
		blockRenderInfo[id].visibleDirections = 0xff;

	b->QueryAttribute("specialGeometry", &blockRenderInfo[id].specialGeometry);
	b->QueryAttribute("itemVariant", &blockRenderInfo[id].itemVariant);

	readBlockGeometryInfo(id, b);
}

const BlockRenderInfo& BlockRenderInfo::get(int id) 
{
	return blockRenderInfo[id];
}

const int grassVariantFunc(int, int x, int y, int z) {	return getBlockTypeAt(x, y+1, z) == SNOW; }
const int woodVariantFunc(int data, int, int, int) { return data & 0x3; }
const int leavesVariantFunc(int, int, int, int) { return 0; }
const int torchVariantFunc(int data, int, int, int) { return data - 1; }
const int ladderVariantFunc(int data, int, int, int) { return data - 2; }
const int doorVariantFunc(int data, int, int, int) { return data; }
const int stairsVariantFunc(int data, int, int, int) { return data & 3; }
const int furnaceVariantFunc(int data, int, int, int) { return (data & 7) - 2; }

void BlockRenderInfo::init()
{
	blockRenderInfo[GRASS].variantFunction = grassVariantFunc;
	blockRenderInfo[WOOD].variantFunction = woodVariantFunc;
	blockRenderInfo[LEAVES].variantFunction = leavesVariantFunc;
	blockRenderInfo[TORCH].variantFunction = torchVariantFunc;	
	blockRenderInfo[LADDER].variantFunction = ladderVariantFunc;
	blockRenderInfo[WOODEN_DOOR_BLOCK].variantFunction = doorVariantFunc;
	blockRenderInfo[IRON_DOOR_BLOCK].variantFunction = doorVariantFunc;
	blockRenderInfo[WOODEN_STAIRS].variantFunction = stairsVariantFunc;
	blockRenderInfo[COBBLESTONE_STAIRS].variantFunction = stairsVariantFunc;
	//blockRenderInfo[CHEST].variantFunction = furnaceVariantFunc;
	blockRenderInfo[FURNACE_ON].variantFunction = furnaceVariantFunc;
	blockRenderInfo[FURNACE_OFF].variantFunction = furnaceVariantFunc;
	blockRenderInfo[DISPENSER].variantFunction = furnaceVariantFunc;

	unsigned plants[] = {6, 37, 38, 39, 40};
	for (int i=0; i<3; ++i)
	{
		blockRenderInfo[ plants[i] ].spriteIcon = true;
		const TexCoords& tc = blockTexcoords[plants[i]];
		vector<Face>& geo = blockRenderInfo[plants[i]].geometryVariants[0];

		geo.clear();

		geo.push_back( Face( Vertex(0,1,1, tc.u0,tc.v0, tc.z, .577f,0,-.577f),
							 Vertex(0,0,1, tc.u0,tc.v1, tc.z, .577f,0,-.577f),
							 Vertex(1,0,0, tc.u1,tc.v1, tc.z, .577f,0,-.577f),
							 Vertex(1,1,0, tc.u1,tc.v0, tc.z, .577f,0,-.577f), 0xff ) );

		geo.push_back( Face( Vertex(0,1,1, tc.u0,tc.v0, tc.z, -.577f,0,.577f),
							 Vertex(1,1,0, tc.u1,tc.v0, tc.z, -.577f,0,.577f),
							 Vertex(1,0,0, tc.u1,tc.v1, tc.z, -.577f,0,.577f),
							 Vertex(0,0,1, tc.u0,tc.v1, tc.z, -.577f,0,.577f), 0xff ) );

		geo.push_back( Face( Vertex(0,1,0, tc.u0,tc.v0, tc.z, -.577f,0,-.577f),
							 Vertex(0,0,0, tc.u0,tc.v1, tc.z, -.577f,0,-.577f),
							 Vertex(1,0,1, tc.u1,tc.v1, tc.z, -.577f,0,-.577f),
							 Vertex(1,1,1, tc.u1,tc.v0, tc.z, -.577f,0,-.577f), 0xff ) );

		geo.push_back( Face( Vertex(0,1,0, tc.u0,tc.v0, tc.z, .577f,0,.577f),
							 Vertex(1,1,1, tc.u1,tc.v0, tc.z, .577f,0,.577f),
							 Vertex(1,0,1, tc.u1,tc.v1, tc.z, .577f,0,.577f),
							 Vertex(0,0,0, tc.u0,tc.v1, tc.z, .577f,0,.577f), 0xff ) );

		vector<Face>& geo2 = blockRenderInfo[plants[i]].geometryVariants[1];
		geo2.clear();

		geo2.push_back( Face( Vertex(0,1,1, tc.u0,tc.v0, tc.z, 0,0,1),
							  Vertex(1,1,0, tc.u1,tc.v0, tc.z, 0,0,1),
							  Vertex(1,0,0, tc.u1,tc.v1, tc.z, 0,0,1),
							  Vertex(0,0,1, tc.u0,tc.v1, tc.z, 0,0,1), 0xff ) );
	}

	{ //Leaves
		const TexCoords& tc = blockTexcoords[LEAVES];
		vector<Face>& geo = blockRenderInfo[LEAVES].geometryVariants[0];

		geo.push_back( Face( Vertex(0,0,1, 0,1,tc.z, 1, 0, 0),
							 Vertex(0,0,0, 1,1,tc.z, 1, 0, 0),
							 Vertex(0,1,0, 1,0,tc.z, 1, 0, 0),
							 Vertex(0,1,1, 0,0,tc.z, 1, 0, 0), 0x01 ) );

		geo.push_back( Face( Vertex(1,0,1, 1,1,tc.z, -1, 0, 0),
							 Vertex(1,1,1, 1,0,tc.z, -1, 0, 0),
							 Vertex(1,1,0, 0,0,tc.z, -1, 0, 0),
							 Vertex(1,0,0, 0,1,tc.z, -1, 0, 0), 0x02 ) );
	
		geo.push_back( Face( Vertex(1,0,1, 0,1,tc.z, 0, 0, -1),
							 Vertex(0,0,1, 1,1,tc.z, 0, 0, -1),
							 Vertex(0,1,1, 1,0,tc.z, 0, 0, -1),
							 Vertex(1,1,1, 0,0,tc.z, 0, 0, -1), 0x04 ) );
	
		geo.push_back( Face( Vertex(1,0,0, 1,1,tc.z, 0, 0,1),
							 Vertex(1,1,0, 1,0,tc.z, 0, 0,1),
							 Vertex(0,1,0, 0,0,tc.z, 0, 0,1),
							 Vertex(0,0,0, 0,1,tc.z, 0, 0,1), 0x08 ) );
	
		geo.push_back( Face( Vertex(0,1,1, 1,0,tc.z, 0, -1, 0),
							 Vertex(0,1,0, 1,1,tc.z, 0, -1, 0),
							 Vertex(1,1,0, 0,1,tc.z, 0, -1, 0),	
							 Vertex(1,1,1, 0,0,tc.z, 0, -1, 0), 0x10 ) );

		geo.push_back( Face( Vertex(1,0,0, 0,1,tc.z, 0,1, 0),
							 Vertex(0,0,0, 1,1,tc.z, 0,1, 0),
							 Vertex(0,0,1, 1,0,tc.z, 0,1, 0),
							 Vertex(1,0,1, 0,0,tc.z, 0,1, 0), 0x20 ) );
	}

	{ //Glass
		const TexCoords& tc = blockTexcoords[GLASS];
		vector<Face>& geo = blockRenderInfo[GLASS].geometryVariants[0];

		geo.push_back( Face( Vertex(0,0,1, 0,1,tc.z, 1, 0, 0),
							 Vertex(0,0,0, 1,1,tc.z, 1, 0, 0),
							 Vertex(0,1,0, 1,0,tc.z, 1, 0, 0),
							 Vertex(0,1,1, 0,0,tc.z, 1, 0, 0), 0x01 ) );

		geo.push_back( Face( Vertex(1,0,1, 1,1,tc.z, -1, 0, 0),
							 Vertex(1,1,1, 1,0,tc.z, -1, 0, 0),
							 Vertex(1,1,0, 0,0,tc.z, -1, 0, 0),
							 Vertex(1,0,0, 0,1,tc.z, -1, 0, 0), 0x02 ) );
	
		geo.push_back( Face( Vertex(1,0,1, 0,1,tc.z, 0, 0, -1),
							 Vertex(0,0,1, 1,1,tc.z, 0, 0, -1),
							 Vertex(0,1,1, 1,0,tc.z, 0, 0, -1),
							 Vertex(1,1,1, 0,0,tc.z, 0, 0, -1), 0x04 ) );
	
		geo.push_back( Face( Vertex(1,0,0, 1,1,tc.z, 0, 0,1),
							 Vertex(1,1,0, 1,0,tc.z, 0, 0,1),
							 Vertex(0,1,0, 0,0,tc.z, 0, 0,1),
							 Vertex(0,0,0, 0,1,tc.z, 0, 0,1), 0x08 ) );
	
		geo.push_back( Face( Vertex(0,1,1, 1,0,tc.z, 0, -1, 0),
							 Vertex(0,1,0, 1,1,tc.z, 0, -1, 0),
							 Vertex(1,1,0, 0,1,tc.z, 0, -1, 0),	
							 Vertex(1,1,1, 0,0,tc.z, 0, -1, 0), 0x10 ) );

		geo.push_back( Face( Vertex(1,0,0, 0,1,tc.z, 0,1, 0),
							 Vertex(0,0,0, 1,1,tc.z, 0,1, 0),
							 Vertex(0,0,1, 1,0,tc.z, 0,1, 0),
							 Vertex(1,0,1, 0,0,tc.z, 0,1, 0), 0x20 ) );
	}

	{ //Torch
		blockRenderInfo[TORCH].spriteIcon = true;
		TexCoords& tc = blockTexcoords[TORCH];
		vector<Face>& geo = blockRenderInfo[TORCH].geometryVariants[4];
		
		float x0 = .4375f, x1 = .5625f, y0 = 0, y1 = .625f;
		tc.u0 = .44f; tc.u1 = .56f;
		tc.v0 = .38f;

		geo.push_back( Face( Vertex(x0,y0,x0, tc.u0,tc.v1, tc.z, -1,0,0),
							 Vertex(x0,y0,x1, tc.u1,tc.v1, tc.z, -1,0,0),
							 Vertex(x0,y1,x1, tc.u1,tc.v0, tc.z, -1,0,0),
							 Vertex(x0,y1,x0, tc.u0,tc.v0, tc.z, -1,0,0), 0xff ) );

		geo.push_back( Face( Vertex(x1,y0,x0, tc.u1,tc.v1, tc.z,  1,0,0),
							 Vertex(x1,y1,x0, tc.u1,tc.v0, tc.z,  1,0,0),
							 Vertex(x1,y1,x1, tc.u0,tc.v0, tc.z,  1,0,0),
							 Vertex(x1,y0,x1, tc.u0,tc.v1, tc.z,  1,0,0), 0xff ) );
	
		geo.push_back( Face( Vertex(x0,y0,x1, tc.u0,tc.v1, tc.z,  0,0,1),
							 Vertex(x1,y0,x1, tc.u1,tc.v1, tc.z,  0,0,1),
							 Vertex(x1,y1,x1, tc.u1,tc.v0, tc.z,  0,0,1),
							 Vertex(x0,y1,x1, tc.u0,tc.v0, tc.z,  0,0,1), 0xff ) );

		geo.push_back( Face( Vertex(x0,y0,x0, tc.u0,tc.v1, tc.z,  0,0,-1),
							 Vertex(x0,y1,x0, tc.u0,tc.v0, tc.z,  0,0,-1),
							 Vertex(x1,y1,x0, tc.u1,tc.v0, tc.z,  0,0,-1),
							 Vertex(x1,y0,x0, tc.u1,tc.v1, tc.z,  0,0,-1), 0xff ) );

		geo.push_back( Face( Vertex(x0,y1,x0, .44f,.38f, tc.z,  0, 1, 0),
							 Vertex(x0,y1,x1, .44f,.50f, tc.z,  0, 1, 0),
							 Vertex(x1,y1,x1, .56f,.50f, tc.z,  0, 1, 0),
							 Vertex(x1,y1,x0, .56f,.38f, tc.z,  0, 1, 0),	0xff ) );

		for (int var=0; var<4; ++var)
		{
			Matrix44 transMat;
			switch(var)
			{
			case 0:
				transMat = Matrix44::Translation(.3f,.70f,.5f).RotateZ(-.2f * PI); 
				break;
			case 1:
				transMat = Matrix44::Translation(.7f,.70f,.5f).RotateZ( .2f * PI);
				break;
			case 2:
				transMat = Matrix44::Translation(.5f,.70f,.3f).RotateX( .2f * PI);
				break;
			case 3:
				transMat = Matrix44::Translation(.5f,.70f,.7f).RotateX(-.2f * PI);
				break;
			}
			transMat *= Matrix44::Translation(-.5f,-.5f,-.5f);

			blockRenderInfo[TORCH].geometryVariants[var] = blockRenderInfo[TORCH].geometryVariants[4];
			for (size_t f=0; f<geo.size(); ++f)
				for (int v=0; v<4; ++v)
					blockRenderInfo[TORCH].geometryVariants[var][f].vertex[v] = transMat * blockRenderInfo[TORCH].geometryVariants[var][f].vertex[v];			
		}

		x0 = .40625f; x1 = .59375f; y0 = 0; y1 = .9375f;
		blockRenderInfo[TORCH].geometryVariants[5].push_back( Face( Vertex(x0,y1,1, tc.u0,tc.v0, tc.z, 0,0,1),
																	Vertex(x1,y1,0, tc.u1,tc.v0, tc.z, 0,0,1),
																	Vertex(x1,y0,0, tc.u1,tc.v1, tc.z, 0,0,1),
																	Vertex(x0,y0,1, tc.u0,tc.v1, tc.z, 0,0,1), 0xff ) );
	}

	{ //Crops
		const TexCoords& tc = blockTexcoords[59];
		vector<Face>& geo = blockRenderInfo[59].geometryVariants[0];
		
		geo.clear();

		geo.push_back( Face( Vertex(.3f,0,0, tc.u0,tc.v1, tc.z, -1,0,0),
							 Vertex(.3f,0,1, tc.u1,tc.v1, tc.z, -1,0,0),
							 Vertex(.3f,1,1, tc.u1,tc.v0, tc.z, -1,0,0),
							 Vertex(.3f,1,0, tc.u0,tc.v0, tc.z, -1,0,0), 0xff ) );

		geo.push_back( Face( Vertex(.7f,0,0, tc.u1,tc.v1, tc.z,  1,0,0),
							 Vertex(.7f,1,0, tc.u1,tc.v0, tc.z,  1,0,0),
							 Vertex(.7f,1,1, tc.u0,tc.v0, tc.z,  1,0,0),
							 Vertex(.7f,0,1, tc.u0,tc.v1, tc.z,  1,0,0), 0xff ) );
	
		geo.push_back( Face( Vertex(0,0,.7f, tc.u0,tc.v1, tc.z,  0,0,1),
							 Vertex(1,0,.7f, tc.u1,tc.v1, tc.z,  0,0,1),
							 Vertex(1,1,.7f, tc.u1,tc.v0, tc.z,  0,0,1),
							 Vertex(0,1,.7f, tc.u0,tc.v0, tc.z,  0,0,1), 0xff ) );

		geo.push_back( Face( Vertex(0,0,.3f, tc.u0,tc.v1, tc.z,  0,0,-1),
							 Vertex(0,1,.3f, tc.u0,tc.v0, tc.z,  0,0,-1),
							 Vertex(1,1,.3f, tc.u1,tc.v0, tc.z,  0,0,-1),
							 Vertex(1,0,.3f, tc.u1,tc.v1, tc.z,  0,0,-1), 0xff ) );
	}

	unsigned doors[] = {WOODEN_DOOR_BLOCK, IRON_DOOR_BLOCK};
	for (int i=0; i<2; ++i)
	{
		const TexCoords& tc = blockTexcoords[doors[i]];
		vector<Face>& geo = blockRenderInfo[doors[i]].geometryVariants[3];

		geo.push_back( Face( Vertex(0,0,.85f, tc.u0,tc.v1, tc.z, -1,0,0),
							 Vertex(0,0,1, tc.u1,tc.v1, tc.z, -1,0,0),
							 Vertex(0,1,1, tc.u1,tc.v0, tc.z,-1,0,0),
							 Vertex(0,1,.85f, tc.u0,tc.v0, tc.z, -1,0,0), 0xff )  );

		geo.push_back( Face( Vertex(1,0,.85f, tc.u1,tc.v1, tc.z,  1,0,0),
							 Vertex(1,1,.85f, tc.u1,tc.v0, tc.z,  1,0,0),
							 Vertex(1,1,1, tc.u0,tc.v0, tc.z,  1,0,0),
							 Vertex(1,0,1, tc.u0,tc.v1, tc.z,  1,0,0), 0xff ) );
	
		geo.push_back( Face( Vertex(0,0,1, tc.u0,tc.v1, tc.z,  0,0,1),
							 Vertex(1,0,1, tc.u1,tc.v1, tc.z,  0,0,1),
							 Vertex(1,1,1, tc.u1,tc.v0, tc.z,  0,0,1),
							 Vertex(0,1,1, tc.u0,tc.v0, tc.z,  0,0,1), 0xff ) );

		geo.push_back( Face( Vertex(0,0,.85f, tc.u0,tc.v1, tc.z,  0,0,-1),
							 Vertex(0,1,.85f, tc.u0,tc.v0, tc.z,  0,0,-1),
							 Vertex(1,1,.85f, tc.u1,tc.v0, tc.z,  0,0,-1),
							 Vertex(1,0,.85f, tc.u1,tc.v1, tc.z,  0,0,-1), 0xff ) );

		geo.push_back( Face( Vertex(0,1,.85f, tc.u0,tc.v0, tc.z,  0, 1, 0),
							 Vertex(0,1,1, tc.u0,tc.v1, tc.z,  0, 1, 0),
							 Vertex(1,1,1, tc.u1,tc.v1, tc.z,  0, 1, 0),
							 Vertex(1,1,.85f, tc.u1,tc.v0, tc.z,  0, 1, 0), 0xff ) );

		geo.push_back( Face( Vertex(0,0,.85f, tc.u0,tc.v0, tc.z,  0,-1, 0),
							 Vertex(1,0,.85f, tc.u1,tc.v0, tc.z,  0,-1, 0),
							 Vertex(1,0,1, tc.u1,tc.v1, tc.z,  0,-1, 0),
							 Vertex(0,0,1, tc.u0,tc.v1, tc.z,  0,-1, 0), 0xff ) );

		blockRenderInfo[doors[i]].geometryVariants[0] = blockRenderInfo[doors[i]].geometryVariants[3];			
		for (size_t f=0; f<geo.size(); ++f)
			for (int v=0; v<4; ++v)
				blockRenderInfo[doors[i]].geometryVariants[0][f].vertex[v] = rot90Mat * geo[f].vertex[v];

		blockRenderInfo[doors[i]].geometryVariants[1] = blockRenderInfo[doors[i]].geometryVariants[3];		
		for (size_t f=0; f<geo.size(); ++f)
			for (int v=0; v<4; ++v)
				blockRenderInfo[doors[i]].geometryVariants[1][f].vertex[v] = rot180Mat * geo[f].vertex[v];

		blockRenderInfo[doors[i]].geometryVariants[2] = blockRenderInfo[doors[i]].geometryVariants[3];			
		for (size_t f=0; f<geo.size(); ++f)
			for (int v=0; v<4; ++v)
				blockRenderInfo[doors[i]].geometryVariants[2][f].vertex[v] = rot270Mat * geo[f].vertex[v];


		Matrix44 transMat = Matrix44::Translation(0,0,.85f);
		transMat.RotateY(-.5f*PI);
		transMat *= Matrix44::Translation(-.15f,0,-.85f);

		blockRenderInfo[doors[i]].geometryVariants[4] = blockRenderInfo[doors[i]].geometryVariants[3];
		for (size_t f=0; f<geo.size(); ++f)
			for (int v=0; v<4; ++v)
				blockRenderInfo[doors[i]].geometryVariants[4][f].vertex[v] = rot90Mat * transMat * geo[f].vertex[v];

		blockRenderInfo[doors[i]].geometryVariants[5] = blockRenderInfo[doors[i]].geometryVariants[3];
		for (size_t f=0; f<geo.size(); ++f)
			for (int v=0; v<4; ++v)
				blockRenderInfo[doors[i]].geometryVariants[5][f].vertex[v] = rot180Mat * transMat * geo[f].vertex[v];

		blockRenderInfo[doors[i]].geometryVariants[6] = blockRenderInfo[doors[i]].geometryVariants[3];
		for (size_t f=0; f<geo.size(); ++f)
			for (int v=0; v<4; ++v)
				blockRenderInfo[doors[i]].geometryVariants[6][f].vertex[v] = rot270Mat * transMat * geo[f].vertex[v];

		blockRenderInfo[doors[i]].geometryVariants[7] = blockRenderInfo[doors[i]].geometryVariants[3];
		for (size_t f=0; f<geo.size(); ++f)
			for (int v=0; v<4; ++v)
				blockRenderInfo[doors[i]].geometryVariants[7][f].vertex[v] = transMat * geo[f].vertex[v];

		
		for (int var=8; var<16; ++var)
		{
			vector<Face>& geo = blockRenderInfo[doors[i]].geometryVariants[var];
			geo = blockRenderInfo[doors[i]].geometryVariants[var-8];
			for (size_t f=0; f<geo.size(); ++f)
				for (int v=0; v<4; ++v)
					geo[f].vertex[v].tz -= 16;
		}
	}

	{ //Ladder
		const TexCoords& tc = blockTexcoords[LADDER];		
		
		blockRenderInfo[LADDER].hideSameType = false;
		blockRenderInfo[LADDER].geometryVariants[0].clear();

		blockRenderInfo[LADDER].geometryVariants[0].push_back( Face( Vertex(0,0,.99f, tc.u0,tc.v1, tc.z,  0,0,-1),
																	Vertex(0,1,.99f, tc.u0,tc.v0, tc.z,  0,0,-1),
																	Vertex(1,1,.99f, tc.u1,tc.v0, tc.z,  0,0,-1),
																	Vertex(1,0,.99f, tc.u1,tc.v1, tc.z,  0,0,-1), 0xff ) );

		blockRenderInfo[LADDER].geometryVariants[1].push_back( Face( Vertex(0,0,.01f, tc.u1,tc.v1, tc.z,  0,0,1),
																	Vertex(1,0,.01f, tc.u0,tc.v1, tc.z,  0,0,1),
																	Vertex(1,1,.01f, tc.u0,tc.v0, tc.z,  0,0,1),
																	Vertex(0,1,.01f, tc.u1,tc.v0, tc.z,  0,0,1), 0xff ) );

		blockRenderInfo[LADDER].geometryVariants[2].push_back( Face( Vertex(.99f,0,0, tc.u0,tc.v1, tc.z,  -1,0,0),
																	Vertex(.99f,0,1, tc.u1,tc.v1, tc.z,  -1,0,0),
																	Vertex(.99f,1,1, tc.u1,tc.v0, tc.z,  -1,0,0),
																	Vertex(.99f,1,0, tc.u0,tc.v0, tc.z,  -1,0,0), 0xff ) );

		blockRenderInfo[LADDER].geometryVariants[3].push_back( Face( Vertex(.01f,0,0, tc.u1,tc.v1, tc.z,  1,0,0),
																	Vertex(.01f,1,0, tc.u1,tc.v0, tc.z,  1,0,0),
																	Vertex(.01f,1,1, tc.u0,tc.v0, tc.z,  1,0,0),
																	Vertex(.01f,0,1, tc.u0,tc.v1, tc.z,  1,0,0), 0xff ) );
	}

	unsigned stairs[] = {53, 67};
	for (int i=0; i<2; ++i)
	{
		const TexCoords& tc = blockTexcoords[stairs[i]];
		vector<Face>& geo = blockRenderInfo[stairs[i]].geometryVariants[3];

		geo.push_back( Face( Vertex(0,0,0, tc.u0,tc.v1/2, tc.z, -1,0,0),
							 Vertex(0,0,1, tc.u1,tc.v1/2, tc.z, -1,0,0),
							 Vertex(0,.5f,1, tc.u1,tc.v0, tc.z, -1,0,0),
							 Vertex(0,.5f,0, tc.u0,tc.v0, tc.z, -1,0,0), 0xff ) );

		geo.push_back( Face( Vertex(0,.5f,0, tc.u0,tc.v1, tc.z, -1,0,0),
							 Vertex(0,.5f,.5f, tc.u1/2,tc.v1, tc.z, -1,0,0),
							 Vertex(0,1,.5f, tc.u1,tc.v1/2, tc.z, -1,0,0),
							 Vertex(0,1,0, tc.u0/2,tc.v1/2, tc.z, -1,0,0), 0xff ) );

		geo.push_back( Face( Vertex(1,0,0, tc.u1,tc.v1/2, tc.z,  1,0,0),
							 Vertex(1,.5f,0, tc.u1,tc.v0, tc.z,  1,0,0),
							 Vertex(1,.5f,1, tc.u0,tc.v0, tc.z,  1,0,0),
							 Vertex(1,0,1, tc.u0,tc.v1/2, tc.z,  1,0,0), 0xff ) );

		geo.push_back( Face( Vertex(1,.5f,0, tc.u1,tc.v1, tc.z,  1,0,0),
							 Vertex(1,1,0, tc.u1,tc.v1/2, tc.z,  1,0,0),
							 Vertex(1,1,.5f, tc.u1/2,tc.v1/2, tc.z,  1,0,0),
							 Vertex(1,.5f,.5f, tc.u1/2,tc.v1, tc.z,  1,0,0), 0xff ) );

		geo.push_back( Face( Vertex(0,0,1, tc.u0,tc.v1/2, tc.z,  0,0,1),
							 Vertex(1,0,1, tc.u1,tc.v1/2, tc.z,  0,0,1),
							 Vertex(1,.5f,1, tc.u1,tc.v0, tc.z,  0,0,1),
							 Vertex(0,.5f,1, tc.u0,tc.v0, tc.z,  0,0,1), 0xff ) );

		geo.push_back( Face( Vertex(0,.5f,.5f, tc.u0,tc.v1/2, tc.z,  0,0,1),
							 Vertex(1,.5f,.5f, tc.u1,tc.v1/2, tc.z,  0,0,1),
							 Vertex(1,1,.5f, tc.u1,tc.v0, tc.z,  0,0,1),
							 Vertex(0,1,.5f, tc.u0,tc.v0, tc.z,  0,0,1), 0xff ) );

		geo.push_back( Face( Vertex(0,0,0, tc.u0,tc.v1, tc.z,  0,0,-1),
							 Vertex(0,1,0, tc.u0,tc.v0, tc.z,  0,0,-1),
							 Vertex(1,1,0, tc.u1,tc.v0, tc.z,  0,0,-1),
							 Vertex(1,0,0, tc.u1,tc.v1, tc.z,  0,0,-1), 0xff ) );

		geo.push_back( Face( Vertex(0,1,0, tc.u0,tc.v1/2, tc.z,  0, 1, 0),
							 Vertex(0,1,.5f, tc.u0,tc.v1, tc.z,  0, 1, 0),
							 Vertex(1,1,.5f, tc.u1,tc.v1, tc.z,  0, 1, 0),
							 Vertex(1,1,0, tc.u1,tc.v1/2, tc.z,  0, 1, 0), 0xff ) );

		geo.push_back( Face( Vertex(0,.5f,.5f, tc.u0,tc.v1/2, tc.z,  0, 1, 0),
							 Vertex(0,.5f,1, tc.u0,tc.v1, tc.z,  0, 1, 0),
							 Vertex(1,.5f,1, tc.u1,tc.v1, tc.z,  0, 1, 0),
							 Vertex(1,.5f,.5f, tc.u1,tc.v1/2, tc.z,  0, 1, 0), 0xff ) );

		geo.push_back( Face( Vertex(0,0,0, tc.u0,tc.v0, tc.z,  0,-1, 0),
							 Vertex(1,0,0, tc.u1,tc.v0, tc.z,  0,-1, 0),
							 Vertex(1,0,1, tc.u1,tc.v1, tc.z,  0,-1, 0),
							 Vertex(0,0,1, tc.u0,tc.v1, tc.z,  0,-1, 0), 0xff ) );

		blockRenderInfo[stairs[i]].geometryVariants[0] = blockRenderInfo[stairs[i]].geometryVariants[3];			
		for (size_t f=0; f<geo.size(); ++f)
			for (int v=0; v<4; ++v)
				blockRenderInfo[stairs[i]].geometryVariants[0][f].vertex[v] = rot90Mat * geo[f].vertex[v];

		blockRenderInfo[stairs[i]].geometryVariants[1] = blockRenderInfo[stairs[i]].geometryVariants[3];			
		for (size_t f=0; f<geo.size(); ++f)
			for (int v=0; v<4; ++v)
				blockRenderInfo[stairs[i]].geometryVariants[1][f].vertex[v] = rot270Mat * geo[f].vertex[v];

		blockRenderInfo[stairs[i]].geometryVariants[2] = blockRenderInfo[stairs[i]].geometryVariants[3];			
		for (size_t f=0; f<geo.size(); ++f)
			for (int v=0; v<4; ++v)
				blockRenderInfo[stairs[i]].geometryVariants[2][f].vertex[v] = rot180Mat * geo[f].vertex[v];
	}

	unsigned fluid[] = {WATER, STILL_WATER, LAVA, STILL_LAVA};
	for (int i=0; i<4; ++i)
	{
		const TexCoords& tc = blockTexcoords[fluid[i]];
		vector<Face>& geo = blockRenderInfo[fluid[i]].geometryVariants[0];
		blockRenderInfo[fluid[i]].visibleDirections = 0xff;

		geo.clear();

		geo.push_back( Face( Vertex(0,0,0, tc.u0,tc.v1, tc.z, -1,0,0),
							 Vertex(0,0,1, tc.u1,tc.v1, tc.z, -1,0,0),
							 Vertex(0,.85f,1, tc.u1,tc.v0, tc.z, -1,0,0),
							 Vertex(0,.85f,0, tc.u0,tc.v0, tc.z, -1,0,0), 0x1 ) );

		geo.push_back( Face( Vertex(1,0,0, tc.u1,tc.v1, tc.z,  1,0,0),
							 Vertex(1,.85f,0, tc.u1,tc.v0, tc.z,  1,0,0),
							 Vertex(1,.85f,1, tc.u0,tc.v0, tc.z,  1,0,0),
							 Vertex(1,0,1, tc.u0,tc.v1, tc.z,  1,0,0), 0x2 ) );
	
		geo.push_back( Face( Vertex(0,0,1, tc.u0,tc.v1, tc.z,  0,0,1),
							 Vertex(1,0,1, tc.u1,tc.v1, tc.z,  0,0,1),
							 Vertex(1,.85f,1, tc.u1,tc.v0, tc.z,  0,0,1),
							 Vertex(0,.85f,1, tc.u0,tc.v0, tc.z,  0,0,1), 0x4 ) );

		geo.push_back( Face( Vertex(0,0,0, tc.u0,tc.v1, tc.z,  0,0,-1),
							 Vertex(0,.85f,0, tc.u0,tc.v0, tc.z,  0,0,-1),
							 Vertex(1,.85f,0, tc.u1,tc.v0, tc.z,  0,0,-1),
							 Vertex(1,0,0, tc.u1,tc.v1, tc.z,  0,0,-1), 0x8 ) );

		geo.push_back( Face( Vertex(0,.85f,0, tc.u0,tc.v0, tc.z,  0, 1, 0),
							 Vertex(0,.85f,1, tc.u0,tc.v1, tc.z,  0, 1, 0),
							 Vertex(1,.85f,1, tc.u1,tc.v1, tc.z,  0, 1, 0),
							 Vertex(1,.85f,0, tc.u1,tc.v0, tc.z,  0, 1, 0), 0x10 ) );

		geo.push_back( Face( Vertex(0,0,0, tc.u0,tc.v0, tc.z,  0,-1, 0),
							 Vertex(1,0,0, tc.u1,tc.v0, tc.z,  0,-1, 0),
							 Vertex(1,0,1, tc.u1,tc.v1, tc.z,  0,-1, 0),
							 Vertex(0,0,1, tc.u0,tc.v1, tc.z,  0,-1, 0), 0x20 ) );
	}

	{	//Grass
		for (int v=0; v<4; ++v)
		{
			blockRenderInfo[2].geometryVariants[0][4].vertex[v].tz = TEX_GRASS;
			blockRenderInfo[2].geometryVariants[0][5].vertex[v].tz = TEX_DIRT;
		}

		blockRenderInfo[2].geometryVariants[1] = blockRenderInfo[2].geometryVariants[0];
		for (int f=0; f<4; ++f)
			for (int v=0; v<4; ++v)
				blockRenderInfo[2].geometryVariants[1][f].vertex[v].tz = TEX_SNOW_SIDE;
	}

	{	//Snow
		const TexCoords& tc = blockTexcoords[80];
		vector<Face>& geo = blockRenderInfo[78].geometryVariants[0];

		geo.clear();

		geo.push_back( Face( Vertex(0,0,0, tc.u0,tc.v1, tc.z, -1,0,0),
							 Vertex(0,0,1, tc.u1,tc.v1, tc.z, -1,0,0),
							 Vertex(0,.20f,1, tc.u1,tc.v0, tc.z, -1,0,0),
							 Vertex(0,.20f,0, tc.u0,tc.v0, tc.z, -1,0,0), 0x1 ) );

		geo.push_back( Face( Vertex(1,0,0, tc.u1,tc.v1, tc.z,  1,0,0),
							 Vertex(1,.20f,0, tc.u1,tc.v0, tc.z,  1,0,0),
							 Vertex(1,.20f,1, tc.u0,tc.v0, tc.z,  1,0,0),
							 Vertex(1,0,1, tc.u0,tc.v1, tc.z,  1,0,0), 0x2 ) );
	
		geo.push_back( Face( Vertex(0,0,1, tc.u0,tc.v1, tc.z,  0,0,1),
							 Vertex(1,0,1, tc.u1,tc.v1, tc.z,  0,0,1),
							 Vertex(1,.20f,1, tc.u1,tc.v0, tc.z,  0,0,1),
							 Vertex(0,.20f,1, tc.u0,tc.v0, tc.z,  0,0,1), 0x4 ) );

		geo.push_back( Face( Vertex(0,0,0, tc.u0,tc.v1, tc.z,  0,0,-1),
							 Vertex(0,.20f,0, tc.u0,tc.v0, tc.z,  0,0,-1),
							 Vertex(1,.20f,0, tc.u1,tc.v0, tc.z,  0,0,-1),
							 Vertex(1,0,0, tc.u1,tc.v1, tc.z,  0,0,-1), 0x8 ) );

		geo.push_back( Face( Vertex(0,.20f,0, tc.u0,tc.v0, tc.z,  0, 1, 0),
							 Vertex(0,.20f,1, tc.u0,tc.v1, tc.z,  0, 1, 0),
							 Vertex(1,.20f,1, tc.u1,tc.v1, tc.z,  0, 1, 0),
							 Vertex(1,.20f,0, tc.u1,tc.v0, tc.z,  0, 1, 0), 0xff ) );
	}

	{	//Stone Slab
		const TexCoords& tc = blockTexcoords[SLAB];
		vector<Face>& geo = blockRenderInfo[SLAB].geometryVariants[0];

		geo.clear();

		geo.push_back( Face( Vertex(0,0,0, tc.u0,tc.v1, tc.z, -1,0,0),
							 Vertex(0,0,1, tc.u1,tc.v1, tc.z, -1,0,0),
							 Vertex(0,.5f,1, tc.u1,.5f, tc.z, -1,0,0),
							 Vertex(0,.5f,0, tc.u0,.5f, tc.z, -1,0,0), 0x1 ) );

		geo.push_back( Face( Vertex(1,0,0, tc.u1,tc.v1, tc.z,  1,0,0),
							 Vertex(1,.5f,0, tc.u1,.5f, tc.z,  1,0,0),
							 Vertex(1,.5f,1, tc.u0,.5f, tc.z,  1,0,0),
							 Vertex(1,0,1, tc.u0,tc.v1, tc.z,  1,0,0), 0x2 ) );
	
		geo.push_back( Face( Vertex(0,0,1, tc.u0,tc.v1, tc.z,  0,0,1),
							 Vertex(1,0,1, tc.u1,tc.v1, tc.z,  0,0,1),
							 Vertex(1,.5f,1, tc.u1,.5f, tc.z,  0,0,1),
							 Vertex(0,.5f,1, tc.u0,.5f, tc.z,  0,0,1), 0x4 ) );

		geo.push_back( Face( Vertex(0,0,0, tc.u0,tc.v1, tc.z,  0,0,-1),
							 Vertex(0,.5f,0, tc.u0,.5f, tc.z,  0,0,-1),
							 Vertex(1,.5f,0, tc.u1,.5f, tc.z,  0,0,-1),
							 Vertex(1,0,0, tc.u1,tc.v1, tc.z,  0,0,-1), 0x8 ) );

		geo.push_back( Face( Vertex(0,.5f,0, tc.u0,tc.v0, tc.z,  0, 1, 0),
							 Vertex(0,.5f,1, tc.u0,tc.v1, tc.z,  0, 1, 0),
							 Vertex(1,.5f,1, tc.u1,tc.v1, tc.z,  0, 1, 0),
							 Vertex(1,.5f,0, tc.u1,tc.v0, tc.z,  0, 1, 0), 0xff ) );

		geo.push_back( Face( Vertex(0,0,0, tc.u0,tc.v0, tc.z,  0,-1, 0),
							 Vertex(1,0,0, tc.u1,tc.v0, tc.z,  0,-1, 0),
							 Vertex(1,0,1, tc.u1,tc.v1, tc.z,  0,-1, 0),
							 Vertex(0,0,1, tc.u0,tc.v1, tc.z,  0,-1, 0), 0x20 ) );
	}

	{ //Fence
		TexCoords& tc = blockTexcoords[FENCE];
		vector<Face>& geo = blockRenderInfo[FENCE].geometryVariants[0];

		float x0 = .375f, x1 = .625f;
		tc.u0 = .375f; tc.u1 = .625f;
		
		geo.clear();

		geo.push_back( Face( Vertex(x0,0,x0, tc.u0,tc.v1, tc.z, -1,0,0),
							 Vertex(x0,0,x1, tc.u1,tc.v1, tc.z, -1,0,0),
							 Vertex(x0,1,x1, tc.u1,tc.v0, tc.z, -1,0,0),
							 Vertex(x0,1,x0, tc.u0,tc.v0, tc.z, -1,0,0), 0xff ) );

		geo.push_back( Face( Vertex(x1,0,x0, tc.u1,tc.v1, tc.z,  1,0,0),
							 Vertex(x1,1,x0, tc.u1,tc.v0, tc.z,  1,0,0),
							 Vertex(x1,1,x1, tc.u0,tc.v0, tc.z,  1,0,0),
							 Vertex(x1,0,x1, tc.u0,tc.v1, tc.z,  1,0,0), 0xff ) );
	
		geo.push_back( Face( Vertex(x0,0,x1, tc.u0,tc.v1, tc.z,  0,0,1),
							 Vertex(x1,0,x1, tc.u1,tc.v1, tc.z,  0,0,1),
							 Vertex(x1,1,x1, tc.u1,tc.v0, tc.z,  0,0,1),
							 Vertex(x0,1,x1, tc.u0,tc.v0, tc.z,  0,0,1), 0xff ) );

		geo.push_back( Face( Vertex(x0,0,x0, tc.u0,tc.v1, tc.z,  0,0,-1),
							 Vertex(x0,1,x0, tc.u0,tc.v0, tc.z,  0,0,-1),
							 Vertex(x1,1,x0, tc.u1,tc.v0, tc.z,  0,0,-1),
							 Vertex(x1,0,x0, tc.u1,tc.v1, tc.z,  0,0,-1), 0xff ) );

		geo.push_back( Face( Vertex(x0,1,x0, tc.u0,x0, tc.z,  0, 1, 0),
							 Vertex(x0,1,x1, tc.u0,x1, tc.z,  0, 1, 0),
							 Vertex(x1,1,x1, tc.u1,x1, tc.z,  0, 1, 0),
							 Vertex(x1,1,x0, tc.u1,x0, tc.z,  0, 1, 0), 0xff ) );

		geo.push_back( Face( Vertex(x0,0,x0, tc.u0,x0, tc.z,  0,-1, 0),
							 Vertex(x1,0,x0, tc.u1,x0, tc.z,  0,-1, 0),
							 Vertex(x1,0,x1, tc.u1,x1, tc.z,  0,-1, 0),
							 Vertex(x0,0,x1, tc.u0,x1, tc.z,  0,-1, 0), 0xff ) );

		vector<Face> bars;
		
		for (int bar=0; bar<2; ++bar)
		{
			const float top = bar ? .5625f : .9375f;
			const float bottom = bar ? .4375f : .8125f;
			const float rear = x0 + .0625f;
			const float front = x1 - .0625f;

			tc.u0 = 0;
			tc.u1 = x0;
			tc.v0 = top;
			tc.v1 = bottom;
			
			bars.push_back(Face( Vertex( 0,bottom,front,	  0,tc.v1, tc.z,  0,0,1),
								 Vertex(x0,bottom,front,  tc.u1,tc.v1, tc.z,  0,0,1),
								 Vertex(x0,top,front,	  tc.u1,tc.v0, tc.z,  0,0,1),
								 Vertex( 0,top,front,		  0,tc.v0, tc.z,  0,0,1), 0xff ) );

			bars.push_back(Face( Vertex( 0,bottom,rear,	    0,tc.v1, tc.z,  0,0,-1),
								 Vertex( 0,top,rear,	    0,tc.v0, tc.z,  0,0,-1),
								 Vertex(x0,top,rear,    tc.u1,tc.v0, tc.z,  0,0,-1),
								 Vertex(x0,bottom,rear, tc.u1,tc.v1, tc.z,  0,0,-1), 0xff ) );

			bars.push_back(Face( Vertex( 0,top,rear,	  0,x0, tc.z,  0, 1, 0),
								 Vertex( 0,top,front,     0,x1, tc.z,  0, 1, 0),
								 Vertex(x0,top,front, tc.u1,x1, tc.z,  0, 1, 0),
								 Vertex(x0,top,rear,  tc.u1,x0, tc.z,  0, 1, 0), 0xff ) );

			bars.push_back(Face( Vertex( 0,bottom,rear,	     0,x0, tc.z,  0,-1, 0),
								 Vertex(x0,bottom,rear,  tc.u1,x0, tc.z,  0,-1, 0),
								 Vertex(x0,bottom,front, tc.u1,x1, tc.z,  0,-1, 0),
								 Vertex( 0,bottom,front,	 0,x1, tc.z,  0,-1, 0), 0xff ) );
		}

		for (int var=1; var<16; ++var)
		{
			vector<Face>& geo = blockRenderInfo[FENCE].geometryVariants[var];
			geo = blockRenderInfo[FENCE].geometryVariants[0];

			if (var & 1) //left
			{
				geo.insert(geo.end(), bars.begin(), bars.end());
			}
			if (var & 2) //right
			{
				geo.insert(geo.end(), bars.begin(), bars.end());
				for (size_t f=geo.size()-8; f<geo.size(); ++f)
					for (int v=0; v<4; ++v)
					{
						geo[f].vertex[v].x += x1;
						geo[f].vertex[v].u += x1;
					}
			}
			if (var & 4) //rear
			{
				geo.insert(geo.end(), bars.begin(), bars.end());
				for (size_t f=geo.size()-8; f<geo.size(); ++f)
					for (int v=0; v<4; ++v)
						geo[f].vertex[v] = rot90Mat * geo[f].vertex[v];
			}
			if (var & 8) //front
			{
				geo.insert(geo.end(), bars.begin(), bars.end());
				for (size_t f=geo.size()-8; f<geo.size(); ++f)
					for (int v=0; v<4; ++v)
					{
						geo[f].vertex[v] = rot90Mat * geo[f].vertex[v];
						geo[f].vertex[v].z += x1;
						geo[f].vertex[v].u += x1;
					}
			}
		}
	}
}

void BlockRenderInfo::appendGeometryAmbient(const Block& block, vector<CompVertex>& buffer, 
								int x, int y, int z, unsigned visibleDirections, 
								const Chunk& chunk) const
{
	struct Coords { int x,y,z; };
	static const Coords sampleCoords[] = 
	{
		{-1, 0, 0}, {-1, 1, 0}, {-1, 0,-1}, {-1, 1,-1},
		{-1, 0, 0}, {-1,-1, 0}, {-1, 0,-1}, {-1,-1,-1},
		{-1, 0, 0}, {-1,-1, 0}, {-1, 0, 1}, {-1,-1, 1},
		{-1, 0, 0}, {-1, 1, 0}, {-1, 0, 1}, {-1, 1, 1},

		{ 1, 0, 0}, { 1, 1, 0}, { 1, 0, 1}, { 1, 1, 1},
		{ 1, 0, 0}, { 1,-1, 0}, { 1, 0, 1}, { 1,-1, 1},
		{ 1, 0, 0}, { 1,-1, 0}, { 1, 0,-1}, { 1,-1,-1},
		{ 1, 0, 0}, { 1, 1, 0}, { 1, 0,-1}, { 1, 1,-1},

		{ 0, 0, 1}, {-1, 0, 1}, { 0, 1, 1}, {-1, 1, 1},
		{ 0, 0, 1}, {-1, 0, 1}, { 0,-1, 1}, {-1,-1, 1},
		{ 0, 0, 1}, { 1, 0, 1}, { 0,-1, 1}, { 1,-1, 1},
		{ 0, 0, 1}, { 1, 0, 1}, { 0, 1, 1}, { 1, 1, 1},

		{ 0, 0,-1}, { 1, 0,-1}, { 0, 1,-1}, { 1, 1,-1},
		{ 0, 0,-1}, { 1, 0,-1}, { 0,-1,-1}, { 1,-1,-1},
		{ 0, 0,-1}, {-1, 0,-1}, { 0,-1,-1}, {-1,-1,-1},
		{ 0, 0,-1}, {-1, 0,-1}, { 0, 1,-1}, {-1, 1,-1},

		{ 0, 1, 0}, { 0, 1,-1}, {-1, 1, 0}, {-1, 1,-1},
		{ 0, 1, 0}, { 0, 1, 1}, {-1, 1, 0}, {-1, 1, 1},
		{ 0, 1, 0}, { 0, 1, 1}, { 1, 1, 0}, { 1, 1, 1},
		{ 0, 1, 0}, { 0, 1,-1}, { 1, 1, 0}, { 1, 1,-1},

		{ 0,-1, 0}, { 0,-1,-1}, { 1,-1, 0}, { 1,-1,-1},
		{ 0,-1, 0}, { 0,-1, 1}, { 1,-1, 0}, { 1,-1, 1},
		{ 0,-1, 0}, { 0,-1, 1}, {-1,-1, 0}, {-1,-1, 1},
		{ 0,-1, 0}, { 0,-1,-1}, {-1,-1, 0}, {-1,-1,-1},
	};
	
	const vector<Face>& faces = geometryVariants[block.variant];
	const size_t end = (faces.size() < 6) ? 0 : 6;

	for ( size_t f = 0; f < end; ++f )
	{
		if ( faces[f].visibleFromDirection & visibleDirections )
		{
			CompVertex faceData( x, y, z, f, faces[f].vertex[0].tz );

			for (int i=0; i<4; ++i)
			{
				char occlusion = 0;
				char light = 0;
				char blocklight = 0;

				//If the two direct neighbors are occluders, the diagonal can't contribute light, 
				//ergo stop at 2 occluders (1st neighbor is adjacent block and can't occlude, 4th neighbor is diagnoal)
				for (int s=0; s<4 && occlusion < 2; ++s)
				{
					const Coords& sample = sampleCoords[f*16 + i*4 + s];
					const Block* n = chunk.getBlock( x + sample.x, y + sample.y, z + sample.z );

					if (n)
					{
						if (!blockRenderInfo[n->type].isTransparent && !blockRenderInfo[n->type].specialGeometry)
							++occlusion;
						else
						{
							if (n->blocklight > blocklight)
								blocklight = n->blocklight;

							if (n->light > light)
								light = n->light;
						}
					}
					else
						light = 15;
				}

				blocklight -= occlusion;
				light -= occlusion;
				
				if (blocklight < block.blocklight)
					blocklight = block.blocklight;

				if (light < 0)
					light = 0;

                                faceData.light[i] = (unsigned char)((blocklight << 4) | light);
			}
			buffer.push_back(faceData);
		}
	}
}


void BlockRenderInfo::appendGeometry(const Block& block, std::vector<CompVertex>& buffer, 
								int x, int y, int z, unsigned visibleDirections, 
								int cx, int cz) const
{
	const Vector4 blockpos(float(cx+x), float(y), float(cz+z), 0);

	const vector<Face>& faces = geometryVariants[block.variant];
	const size_t end = (faces.size() < 6) ? 0 : 6;

	for ( size_t f = 0; f < end; ++f )
	{
		if ( faces[f].visibleFromDirection & visibleDirections )
		{
			const Vector4 normal = Vector4(faces[f].vertex[0].nx, faces[f].vertex[0].ny, faces[f].vertex[0].nz, 0);
			const Vector4 neighborPos = blockpos + normal;
			const Block* neighbor = getBlockAt(int(neighborPos.x), int(neighborPos.y), int(neighborPos.z)).block();

			CompVertex faceData( x, y, z, f, faces[f].vertex[0].tz );

			for (int i=0; i<4; ++i)
			{
				const Vertex v = faces[f].vertex[i];
				const Vector4 pos = Vector4(v.x + x, v.y + y, v.z + z, 1);				

				unsigned char blocklight = block.blocklight;
				unsigned char light = block.light;

				if (neighbor)
				{
					if (neighbor->blocklight > blocklight)
						blocklight = neighbor->blocklight;

					if (neighbor->light > light)
						light = neighbor->light;
				}
				else
					light = 15;

                                faceData.light[i] = (unsigned char)((blocklight << 4) | light);
			}

			buffer.push_back(faceData);
		}
	}
}

void BlockRenderInfo::appendGeometry(const Block& block, std::vector<FloatVertex>& buffer, 
								int x, int y, int z, unsigned visibleDirections, 
								int cx, int cz) const
{
	const Vector4 blockpos(float(cx+x), float(y), float(cz+z), 0);

	const vector<Face>& faces = geometryVariants[block.variant];

	for ( size_t f = 0; f < faces.size(); ++f )
	{
		if ( faces[f].visibleFromDirection & visibleDirections )
		{
			const Vector4 normal = Vector4(faces[f].vertex[0].nx, faces[f].vertex[0].ny, faces[f].vertex[0].nz, 0);
			const Vector4 neighborPos = blockpos + normal;
			const Block* neighbor = getBlockAt(int(neighborPos.x), int(neighborPos.y), int(neighborPos.z)).block();
				
			for (int i=0; i<4; ++i)
			{
				const Vertex v = faces[f].vertex[i];
				const Vector4 pos = Vector4(v.x + x, v.y + y, v.z + z, 1);				

				unsigned char blocklight = block.blocklight;
				unsigned char light = block.light;

				if (neighbor)
				{
					if (neighbor->blocklight > blocklight)
						blocklight = neighbor->blocklight;

					if (neighbor->light > light)
						light = neighbor->light;
				}
				else
					light = 15;

				buffer.push_back( FloatVertex(pos.x, pos.y, pos.z, v.u, v.v, v.tz, blocklight, light) );
			}
		}
	}
}
