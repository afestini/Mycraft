#include "chunkcreator.h"
#include "chunk.h"
#include "block.h"
#include "defines.h"

using namespace std;


const int perm[512] = 
{
	151,160,137,91,90,15,
	131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
	190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
	88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
	77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
	102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
	135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
	5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
	223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
	129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
	251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
	49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
	138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
	151,160,137,91,90,15,
	131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
	190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
	88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
	77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
	102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
	135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
	5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
	223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
	129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
	251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
	49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
	138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
};

const int grad3[12][3] = 
{
	{1,1,0},{-1,1,0},{1,-1,0},{-1,-1,0},
	{1,0,1},{-1,0,1},{1,0,-1},{-1,0,-1},
	{0,1,1},{0,-1,1},{0,1,-1},{0,-1,-1}
};

static int fastfloor(float x) {	return x>0 ? (int)x : (int)x-1; }

static float dot(const int g[], float x, float y) { return g[0]*x + g[1]*y; }

static float dot(const int g[], float x, float y, float z) { return g[0]*x + g[1]*y + g[2]*z; }

static const float F2 = .5f * (sqrt(3.f) - 1.f);
static const float G2 = (3.f - sqrt(3.f)) / 6.f;

static const float F3 = 1.0f/3.0f;
static const float G3 = 1.0f/6.0f;

float noise2D(float xin, float yin) 
{
	float n0, n1, n2;
	float s = (xin+yin)*F2;
	int i = fastfloor(xin+s);
	int j = fastfloor(yin+s);
	
	float t = (i+j)*G2;
	float X0 = i-t;
	float Y0 = j-t;
	float x0 = xin-X0;
	float y0 = yin-Y0;

	int i1, j1;
	if (x0 > y0)
	{
		i1 = 1;
		j1 = 0;
	}
	else 
	{
		i1 = 0;
		j1 = 1;
	}

	float x1 = x0 - i1 + G2;
	float y1 = y0 - j1 + G2;
	float x2 = x0 - 1.0f + 2.0f * G2;
	float y2 = y0 - 1.0f + 2.0f * G2;

	int ii = i & 255;
	int jj = j & 255;
	int gi0 = perm[ii+perm[jj]] % 12;
	int gi1 = perm[ii+i1+perm[jj+j1]] % 12;
	int gi2 = perm[ii+1+perm[jj+1]] % 12;

	float t0 = 0.5f - x0*x0 - y0*y0;
	if (t0 < 0) 
		n0 = 0.0;
	else
	{
		t0 *= t0;
		n0 = t0 * t0 * dot(grad3[gi0], x0, y0);
	}

	float t1 = 0.5f - x1*x1 - y1*y1;
	if (t1 < 0)
		n1 = 0.0;
	else
	{
		t1 *= t1;
		n1 = t1 * t1 * dot(grad3[gi1], x1, y1);
	}

	float t2 = 0.5f - x2*x2 - y2*y2;
	if(t2 < 0) 
		n2 = 0.0;
	else
	{
		t2 *= t2;
		n2 = t2 * t2 * dot(grad3[gi2], x2, y2);
	}

	return 70.0f * (n0 + n1 + n2);
}


static float noise3D(float x, float y, float z) 
{
	float n0, n1, n2, n3;
	float s = (x+y+z)*F3; // Very nice and simple skew factor for 3D
	int i = fastfloor(x+s);
	int j = fastfloor(y+s);
	int k = fastfloor(z+s);

	float t = (i+j+k)*G3;
	float X0 = i-t, Y0 = j-t, Z0 = k-t;
	float x0 = x-X0, y0 = y-Y0, z0 = z-Z0;

	int i1, j1, k1;
	int i2, j2, k2;

	if(x0>=y0) 
	{
		if(y0>=z0)
		{
			i1=1; j1=0; k1=0; i2=1; j2=1; k2=0; 
		}
		else if (x0>=z0)
		{
			i1=1; j1=0; k1=0; i2=1; j2=0; k2=1; 
		}
		else 
		{ 
			i1=0; j1=0; k1=1; i2=1; j2=0; k2=1; 
		}
	}
	else 
	{
		if(y0<z0) 
		{ 
			i1=0; j1=0; k1=1; i2=0; j2=1; k2=1; 
		}
		else if (x0<z0) 
		{ 
			i1=0; j1=1; k1=0; i2=0; j2=1; k2=1; 
		}
		else 
		{
			i1=0; j1=1; k1=0; i2=1; j2=1; k2=0; 
		}
	}

	float x1 = x0 - i1 + G3;
	float y1 = y0 - j1 + G3;
	float z1 = z0 - k1 + G3;
	float x2 = x0 - i2 + 2.0f*G3;
	float y2 = y0 - j2 + 2.0f*G3;
	float z2 = z0 - k2 + 2.0f*G3;
	float x3 = x0 - 1.0f + 3.0f*G3;
	float y3 = y0 - 1.0f + 3.0f*G3;
	float z3 = z0 - 1.0f + 3.0f*G3;

	int ii = i & 255;
	int jj = j & 255;
	int kk = k & 255;
	int gi0 = perm[ii+perm[jj+perm[kk]]] % 12;
	int gi1 = perm[ii+i1+perm[jj+j1+perm[kk+k1]]] % 12;
	int gi2 = perm[ii+i2+perm[jj+j2+perm[kk+k2]]] % 12;
	int gi3 = perm[ii+1+perm[jj+1+perm[kk+1]]] % 12;

	float t0 = 0.6f - x0*x0 - y0*y0 - z0*z0;
	float t1 = 0.6f - x1*x1 - y1*y1 - z1*z1;
	float t2 = 0.6f - x2*x2 - y2*y2 - z2*z2;
	float t3 = 0.6f - x3*x3 - y3*y3 - z3*z3;

	if (t0<0) 
		n0 = 0.0;
	else 
	{
		t0 *= t0;
		n0 = t0 * t0 * dot(grad3[gi0], x0, y0, z0);
	}

	if (t1<0) 
		n1 = 0.0;
	else 
	{
		t1 *= t1;
		n1 = t1 * t1 * dot(grad3[gi1], x1, y1, z1);
	}

	if (t2<0) 
		n2 = 0.0;
	else 
	{
		t2 *= t2;
		n2 = t2 * t2 * dot(grad3[gi2], x2, y2, z2);
	}
	
	if (t3<0) 
		n3 = 0.0;
	else 
	{
		t3 *= t3;
		n3 = t3 * t3 * dot(grad3[gi3], x3, y3, z3);
	}
	
	return 32.0f*(n0 + n1 + n2 + n3);
}



const float amp = 1.0f;
const int octaves = 8;

void createHeightmap(Chunk& chunk, int cx, int cz)
{
	const float freq = .002f;
	const float offX = float(.5f + cx * 16), offZ = float(.5f + cz * 16);

	for (unsigned char z = 0; z < CHUNK_SIZE; ++z)
	{
		const float pz = freq * (offZ + z);

		for (unsigned char x = 0; x < CHUNK_SIZE; ++x)
		{
			const float px = freq * (offX + x);

			float noise = .0f, fac = 1.f;
			for (int i = 0; i < octaves; ++i, fac*=2)
			{
				noise += (amp/fac) * noise2D(px * fac, pz * fac);
			}

			//(elevation + (roughness*detail))*64+64.
			//(scaled 8x along the horizontals, 4x along the vertical)
			const int height = max(min(int(64 + 32.f * noise), CHUNK_HEIGHT - 1), 0);

			unsigned char y = 0;
			while (y < height)
				chunk.blocks[x][z][y++].type = 1;

			while (y <= 64)
			{
				chunk.blocks[x][z][y].light = max(0, 15 - 3*(64-y));
				chunk.blocks[x][z][y++].type = STILL_WATER;
			}

			while (y < CHUNK_HEIGHT)
				chunk.blocks[x][z][y++].light = 15;
		}
	}
}

void createCaves(Chunk& chunk, int cx, int cz)
{
	const float freq = .04f;
	const float offX = float(.5f + cx * 16), offZ = float(.5f + cz * 16);

	for (unsigned char z = 0; z < CHUNK_SIZE; ++z)
	{
		const float pz = freq * (offZ + z);

		for (unsigned char x = 0; x < CHUNK_SIZE; ++x)
		{
			const float px = freq * (offX + x);
			
			for (unsigned char y = 1; y < CHUNK_HEIGHT; ++y)
			{
				const float py = freq * y;
				float noise = .0f, fac = 1.f;

				for (int i = 0; i < 3; ++i, fac *= 2.f)
				{
					noise += (amp / fac) * noise3D(px * fac, py * fac, pz * fac);
				}

				if (noise >.75f && chunk.blocks[x][z][y].type == 1)
				{
					chunk.blocks[x][z][y].type = 0;
					chunk.blocks[x][z][y].light = 15;
				}
			}
		}
	}
}


void createChunk(Chunk& chunk, int cx, int cz)
{

	memset(chunk.blocks, 0, sizeof(chunk.blocks));

	createHeightmap(chunk, cx, cz);
	createCaves(chunk, cx, cz);

/*
	if (chunk.north && chunk.north->loaded)
	{
		for (unsigned char y = 0; y < CHUNK_HEIGHT; ++y)
		for (unsigned char x = 0; x < CHUNK_SIZE; ++x)
		{
			PickResult pr(chunk.north, x, y, CHUNK_SIZE - 1);
			if (pr.block()->light > 0 || pr.block()->blocklight > 0)
				lightQueue.push_back(pr);
		}
	}

	if (chunk.south && chunk.south->loaded)
	{
		for (unsigned char y = 0; y < CHUNK_HEIGHT; ++y)
		for (unsigned char x = 0; x < CHUNK_SIZE; ++x)
		{
			PickResult pr(chunk.south, x, y, 0);
			if (pr.block()->light > 0 || pr.block()->blocklight > 0)
				lightQueue.push_back(pr);
		}
	}

	if (chunk.west && chunk.west->loaded)
	{
		for (unsigned char y = 0; y < CHUNK_HEIGHT; ++y)
		for (unsigned char z = 0; z < CHUNK_SIZE; ++z)
		{
			PickResult pr(chunk.west, CHUNK_SIZE-1, y, z);
			if (pr.block()->light > 0 || pr.block()->blocklight > 0)
				lightQueue.push_back(pr);
		}
	}

	if (chunk.east && chunk.east->loaded)
	{
		for (unsigned char y = 0; y < CHUNK_HEIGHT; ++y)
		for (unsigned char z = 0; z < CHUNK_SIZE; ++z)
		{
			PickResult pr(chunk.east, 0, y, z);
			if (pr.block()->light > 0 || pr.block()->blocklight > 0)
				lightQueue.push_back(pr);
		}
	}

	chunk.increaseSunLight(lightQueue);
/**/
	if ( chunk.north ) chunk.north->setDirty();
	if ( chunk.east ) chunk.east->setDirty();
	if ( chunk.south ) chunk.south->setDirty();
	if ( chunk.west ) chunk.west->setDirty();

	chunk.dirty = true;
	chunk.loaded = true;
}