#ifndef VERTEX_INCLUDED
#define VERTEX_INCLUDED

struct Vertex
{
	float x, y, z;
	float u, v;
	unsigned char tz;
	float nx, ny, nz;

	Vertex(float x, float y, float z) 
		: x(x), y(y), z(z), u(0), v(0), tz(0), nx(0), ny(0), nz(0) {}

	Vertex(float x, float y, float z, float u, float v, unsigned char tz) 
		: x(x), y(y), z(z), u(u), v(v), tz(tz), nx(0), ny(0), nz(0) {}

	Vertex(float x, float y, float z, float u, float v, float nx, float ny, float nz) 
		: x(x), y(y), z(z), u(u), v(v), tz(0), nx(nx), ny(ny), nz(nz) {}

	Vertex(float x, float y, float z, float u, float v, unsigned char tz, float nx, float ny, float nz) 
		: x(x), y(y), z(z), u(u), v(v), tz(tz), nx(nx), ny(ny), nz(nz) {}
	
	Vertex() : x(0), y(0), z(0), u(0), v(0), tz(0), nx(0), ny(0), nz(0) {}
};

struct FloatVertex
{
	short x,y,z;
	unsigned char u, v, texture;
	unsigned char lights;

	FloatVertex() : x(0), y(0), z(0), u(0), v(0), texture(0), lights(0) {}

	template<typename PosType, typename TexType, typename TexLayerType, typename LightType>
	FloatVertex( PosType x, PosType y, PosType z, TexType u, TexType v, TexLayerType t, LightType blocklight, LightType light) 
                : x((short)(x*255)), y((short)(y*255)), z((short)(z*255)),
                  u((unsigned char)(u*255)), v((unsigned char)(v*255)), texture((unsigned char)(t)),
                  lights((unsigned char)(blocklight)<<4 | (unsigned char)(light)) {}
};


struct CompVertex
{
	unsigned char xz, y, f, t;
	unsigned char light[4];

	CompVertex(int x, int y, int z, size_t face, unsigned char t)
                : xz((unsigned char)(x << 4) | (unsigned char)(z)), y((unsigned char)(y)), f((unsigned char)(face)), t((unsigned char)(t)) {}
};

#endif
