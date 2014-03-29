#ifndef PICKING_INCLUDED
#define PICKING_INCLUDED

class World;
class CameraGL;
class Chunk;
struct Block;
struct Face;

//Negative/Positive-axis
enum DIRECTION { DIR_NZ, DIR_PX, DIR_PZ, DIR_NX, DIR_PY, DIR_NY, DIR_NONE };

struct PickResult 
{
	Chunk* chunk;
	const Face* face;
	DIRECTION hitDir;
	char x, y, z;

	PickResult() : chunk(0), face(0), hitDir(DIR_NONE), x(0), y(0), z(0) {}

	PickResult(Chunk* chunk, char x, char y, char z, DIRECTION dir = DIR_NONE)
		: chunk(chunk), face(0), hitDir(dir), x(x), y(y), z(z) {}

	bool operator==(const PickResult& p) const { return chunk == p.chunk && x == p.x && y == p.y && z == p.z && face == p.face; }
	Block* block();
};

PickResult pickBlock(const CameraGL& cam, float range);
PickResult getBlockAt(int x, int y, int z);
PickResult getBlockAt(float x, float y, float z);
unsigned char getBlockTypeAt(int x, int y, int z);
PickResult getNeighbor(const PickResult& pick, DIRECTION dir);

#endif