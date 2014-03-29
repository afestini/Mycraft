#ifndef COLLISION_INCLUDED
#define COLLISION_INCLUDED

#include "../main/VectorX.h"

struct AACylinder
{
	Vector3 position;
	float height;
	float radius;
};

struct AABB
{
	Vector3 min;
	Vector3 max;

	AABB() {}
	AABB(const Vector3& min, const Vector3& max) : min(min), max(max) {}
};

struct Line
{
	Vector3 p0, p1;
	Line(const Vector3& p0, const Vector3& p1) : p0(p0), p1(p1) {}
};

struct Ray
{
	Vector3 origin;
	Vector3 direction;

	Ray(const Vector3& p, const Vector3& d) : origin(p), direction(d) {}
};

struct Sphere
{
	Vector3 center;
	float radius;

	Sphere(const Vector3& c, float r) : center(c), radius(r) {}
};

struct Ellipsoid
{
	Vector3 center;
	Vector3 radii;

	Ellipsoid(const Vector3& c, const Vector3& r) : center(c), radii(r) {}		
};

struct Plane
{
	Vector3 point;
	Vector3 normal;

	Plane() {}
	Plane(const Vector3& p, const Vector3& n) : point(p), normal(n) {}
};

struct Quad
{
	Vector3 a, b, c, d;
	Vector3 normal;

	Quad(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& d) 
		: a(a), b(b), c(c), d(d), normal(cross((a-b), (c-b)))
	{
	}
	
	Quad(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& d, const Vector3& n)	
		: a(a), b(b), c(c), d(d), normal(n)	
	{
	}
};

bool pointInQuad(const Vector3& p, const Quad& q);

float distance(const Vector3& point, const Plane& p);

float collide(const Vector3& velocity, const Ellipsoid& e, const Quad& q, Vector3* newVelocity = nullptr);
float collide(const Vector3& velocity, const Sphere& s, const Plane& p);
float collide(const Vector3& velocity, const Sphere& s, const Quad& q, Vector3* collisionPoint = nullptr);
float collide(const Vector3& velocity, const AABB& a, const AABB& b);

float collideUnitSphere(const Vector3& velocity, const Line& seg, Vector3* collisionPoint = nullptr);
float collideUnitSphere(const Vector3& velocity, const Plane& p);
float collideUnitSphere(const Vector3& velocity, const Quad& q, Vector3* collisionPoint = nullptr);


#endif