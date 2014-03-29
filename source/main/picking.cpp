#include "picking.h"
#include "world.h"
#include "cameraGL.h"
#include "chunk.h"
#include "blockrenderinfo.h"

using namespace std;


Block* PickResult::block() 
{
	return chunk ? &chunk->blocks[x][z][y] : 0;
}

float getIntersection(const Vector3& pos, const Vector3& dir, const Face& face)
{
	bool hit = true;
	float sign = 0;

	for (int v = 0; v < 4 && hit; ++v)
	{
		const Vertex& v0 = face.vertex[v];
		const Vertex& v1 = face.vertex[(v+1)%4];
		const Vector3 q = Vector3(v0.x, v0.y, v0.z) - pos;
		const Vector3 r = Vector3(v1.x, v1.y, v1.z) - pos;
		const float s = dot(dir, cross(q,r));
		if ( sign == 0 )
			sign = s;
		else if ( sign*s < 0 )
			hit = false;
	}

	float t = -1.0f;
	if (hit)
	{
		const Vector3 v0(&face.vertex[0].x);
		const Vector3 v1(&face.vertex[1].x);
		const Vector3 v2(&face.vertex[2].x);

		Vector3 n = cross(Vector3(v0-v1), Vector3(v2-v1));
		const float nDotDir = dot(n, dir);
		
		if (nDotDir != 0)
			t = dot(n, (v0 - pos)) / nDotDir;
		
		if (t < 0)
			t = -1.f;
	}

	return t;
}

const Face* checkIntersection(const Vector3& pos, const Vector3& dir, const Block& block)
{
	float minT = 10.0f;
	const Face* hitFace = 0;

	const Vector4 tPos = Vector4(pos.x, pos.y, pos.z, 1);
	const Vector4 tDir = Vector4(dir.x, dir.y, dir.z, 0);

	const BlockRenderInfo& info = BlockRenderInfo::get(block.type);
	vector<Face>::const_iterator face = info.geometryVariants[block.variant].begin();
	vector<Face>::const_iterator end = info.geometryVariants[block.variant].end();
	for ( face; face != end; ++face)
	{
		const float t = getIntersection(Vector3(tPos.x, tPos.y, tPos.z), Vector3(tDir.x, tDir.y, tDir.z), *face);
		if ( t >= 0 && t < minT )
		{
			minT = t;
			hitFace = &*face;
		}
	}
	return hitFace;
}

PickResult getBlockAt(float x, float y, float z)
{
	PickResult desc;
	Chunk* chunk = World::instance().getChunkFromPosition(x, z);
	if ( chunk && chunk->loaded && y < 128 )
	{
		char bx = char(x - chunk->x*CHUNK_SIZE);
		char by = char(y);
		char bz = char(z - chunk->z*CHUNK_SIZE);

		desc = PickResult(chunk, bx, by, bz, DIR_NONE);
	}
	return desc;
}

PickResult getBlockAt(int x, int y, int z)
{
	Chunk* chunk = World::instance().getChunk(x >> 4, z >> 4);
	return (chunk && chunk->loaded && y < 128) ? PickResult(chunk, x&15, (char)y, z&15, DIR_NONE) : PickResult();
}

unsigned char getBlockTypeAt(int x, int y, int z)
{
	const Chunk* chunk = World::instance().getChunk(x >> 4, z >> 4);
	return (chunk && chunk->loaded && y < CHUNK_HEIGHT) ? (chunk->blocks[x & 15][z & 15][y].type) : 0;
}

PickResult pickBlock(const CameraGL& cam, float range)
{
	Vector3 pos(cam.Position[0], cam.Position[1], cam.Position[2]);
	Vector3 dir(cam.Forward[0], cam.Forward[1], cam.Forward[2]);

	if (JSApp::CursorFree)
	{
		int mx, my;
		glfwGetMousePos(&mx, &my);
		cam.Screen2World(mx, my, &dir.x);
		dir.normalize();
	}

	int bx = int(pos.x) - (pos.x<0 ? 1 : 0);
	int by = int(pos.y);
	int bz = int(pos.z) - (pos.z<0 ? 1 : 0);

	int step_x = dir.x < 0 ? -1 : 1;
	int step_y = dir.y < 0 ? -1 : 1;
	int step_z = dir.z < 0 ? -1 : 1;

	DIRECTION x_dir = dir.x < 0 ? DIR_PX : DIR_NX;
	DIRECTION y_dir = dir.y < 0 ? DIR_PY : DIR_NY;
	DIRECTION z_dir = dir.z < 0 ? DIR_PZ : DIR_NZ;
	DIRECTION hit_from = DIR_NONE;

	float t_per_x = 1/fabs(dir.x);
	float t_per_y = 1/fabs(dir.y);
	float t_per_z = 1/fabs(dir.z);

	float t_til_x = t_per_x * (dir.x>=0 ? 1 - (pos.x - bx) : pos.x-bx);
	float t_til_y = t_per_y * (dir.y>=0 ? 1 - (pos.y - by) : pos.y-by);
	float t_til_z = t_per_z * (dir.z>=0 ? 1 - (pos.z - bz) : pos.z-bz);


	float t = 0;

	while (t < range)
	{
		PickResult desc = getBlockAt(bx, by, bz);
		Block* block = desc.block();
		if (block && block->type)
		{
			const Face* face = checkIntersection(pos-Vector3(float(bx), float(by), float(bz)), dir, *block);
			if (face)
			{
				desc.hitDir = hit_from;
				desc.face = face;
				return desc;
			}
		}

		float dt = 0;
		if ( t_til_x < t_til_y && t_til_x < t_til_z )
			dt = t_til_x;
		else 
			dt = ( t_til_y < t_til_z ) ? t_til_y : t_til_z;

		t += dt;

		if ((t_til_x -= dt) <= 0) 
		{
			bx += step_x;
			t_til_x += t_per_x;
			hit_from = x_dir;
		}
		if ((t_til_y -= dt) <= 0)
		{
			by += step_y;
			t_til_y += t_per_y;
			hit_from = y_dir;
		}
		if ((t_til_z -= dt) <= 0)
		{
			bz += step_z;
			t_til_z += t_per_z;
			hit_from = z_dir;
		}
	}

	return PickResult();
}

PickResult getNeighbor(const PickResult& pick, DIRECTION dir)
{
	PickResult p(pick.chunk, pick.x, pick.y, pick.z);
	if (Chunk* chunk = pick.chunk)
	{
		switch (dir)
		{
		case DIR_NZ: if (p.z-- == 0) {p.z = CHUNK_SIZE-1; p.chunk = chunk->north; } break;
		case DIR_PX: if (++p.x == CHUNK_SIZE) {p.x = 0; p.chunk = chunk->east; } break;
		case DIR_PZ: if (++p.z == CHUNK_SIZE) {p.z = 0; p.chunk = chunk->south; } break;
		case DIR_NX: if (p.x-- == 0) {p.x = CHUNK_SIZE-1; p.chunk = chunk->west; } break;
		case DIR_PY: if (++p.y == CHUNK_HEIGHT) p.chunk = 0; break;
		case DIR_NY: if (p.y-- == 0) p.chunk = 0; break;
                default: break;
		}
	}
	return p;
}
