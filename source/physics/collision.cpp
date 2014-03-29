#include "collision.h"
#include <algorithm>

using namespace std;

bool pointInQuad(const Vector3& p, const Quad& q)
{
	return abs( distance(p, Plane(q.a, q.normal)) ) <= .00001f
		&& dot((p - q.b), ( cross(q.normal, (q.b - q.a)) )) >= 0
		&& dot((p - q.b), ( cross(q.normal, (q.c - q.b)) )) >= 0
		&& dot((p - q.d), ( cross(q.normal, (q.d - q.c)) )) >= 0
		&& dot((p - q.d), ( cross(q.normal, (q.a - q.d)) )) >= 0;
}

Vector3 closestPointOnLineSegment(const Vector3& p, const Vector3& a, const Vector3& b)
{
	const float lineLen = (b - a).len();
	const Vector3 lineDir = (b - a) * (1.0f/lineLen);

	const Vector3 pa(p-a);
	const float t = dot(lineDir, pa);

	if (t < 0) return a;
	if (t > lineLen) return b;
 
	return a + lineDir * t;
}

float distance(const Vector3& point, const Line& line, Vector3* projectedPoint)
{
	const Vector3 toPoint = point - line.p0;
	const Vector3 lineDir = line.p1 - line.p0;

	const float tToProjection = dot(lineDir, toPoint);
	const Vector3 projection = line.p0 + lineDir * tToProjection;
	
	if (projectedPoint)
		*projectedPoint = projection;

	return (point - projection).len();
}

float distance(const Vector3& point, const Plane& p)
{
	return dot((point - p.point), p.normal);
}


float collide(const Vector3& velocity, const AABB& a, const AABB& b)
{
	//Calculate start and end time of overlap for each axis
	Vector3 tv0(1.0f), tv1(1.0f);

	if ( velocity.x < 0 ) 
	{
		tv0.x = (b.max.x - a.min.x) / velocity.x;
		tv1.x = (b.min.x - a.max.x) / velocity.x;
	}
	else if ( velocity.x > 0 ) 
	{
		tv0.x = (b.min.x - a.max.x) / velocity.x;
		tv1.x = (b.max.x - a.min.x) / velocity.x;
	}

	if ( velocity.y < 0 ) 
	{
		tv0.y = (b.max.y - a.min.y) / velocity.y;
		tv1.y = (b.min.y - a.max.y) / velocity.y;
	}
	else if ( velocity.y > 0 ) 
	{
		tv0.y = (b.min.y - a.max.y) / velocity.y;
		tv1.y = (b.max.y - a.min.y) / velocity.y;
	}

	if ( velocity.z < 0 ) 
	{
		tv0.z = (b.max.z - a.min.z) / velocity.z;
		tv1.z = (b.min.z - a.max.z) / velocity.z;
	}
	else if ( velocity.z > 0 ) 
	{
		tv0.z = (b.min.z - a.max.z) / velocity.z;
		tv1.z = (b.max.z - a.min.z) / velocity.z;
	}

	//Get time where all begin/end to overlap
	const float t0 = (tv0.x > tv0.y) ? ((tv0.x > tv0.z) ? tv0.x : tv0.z) : ((tv0.y > tv0.z) ? tv0.y : tv0.z);
	const float t1 = (tv1.x < tv1.y) ? ((tv1.x < tv1.z) ? tv1.x : tv1.z) : ((tv1.y < tv1.z) ? tv1.y : tv1.z);

	return (t0 <= t1) ? t0 : -1.0f;
}

float collide(const Vector3& velocity, const Sphere& s, const Plane& p)
{
	const float signedDistance = distance(s.center, p);
	const float relativeSpeed = -dot(p.normal, velocity);
	
	float t0 = -1.0f, t1 = -1.0f;
	
	if (relativeSpeed == .0f)
	{
		if (signedDistance < s.radius)
		{
			t0 = 0.0f;
			t1 = 1.0f;
		}
	}
	else if (relativeSpeed > 0)
	{
		t0 = (signedDistance - s.radius) / relativeSpeed;
		t1 = (signedDistance + s.radius) / relativeSpeed;

		if (t0 < 0)
			t0 = .0f;
	}
	return ( t0 <= 1.0f && t1 >= .0f ) ? t0 : -1.0f;
}

float collideUnitSphere(const Vector3& velocity, const Vector3& p)
{
	const float A = dot(velocity, velocity);
	const float B = 2.0f * dot(velocity, -p);
	const float C = p.lensq() - 1.0f;
	const float D = B*B - 4.0f*A*C;

	return (D >= .0f) ? (-B - sqrt(D)) / (2.0f*A) : -1.0f;
}

float collideUnitSphere(const Vector3& velocity, const Line& seg, Vector3* collisionPoint)
{
	const Vector3 edge(seg.p1 - seg.p0);

	const float edgeSq = edge.lensq();
	const float edgeDotP0 = dot(edge, seg.p0);
	const float edgeDotVel = dot(edge, velocity);

	const float A = edgeSq * -velocity.lensq() + edgeDotVel*edgeDotVel;
	const float B = edgeSq * 2.0f * dot(velocity, seg.p0) - 2.0f*edgeDotVel*edgeDotP0;
	const float C = edgeSq * (1.0f - seg.p0.lensq()) + edgeDotP0*edgeDotP0;
	const float D = B*B - 4.0f*A*C;  
	
	float t = -1.0f;
	
	if (D >= .0f)
	{
		const float x = min ((-B - sqrt(D)) / (2.0f*A), (-B + sqrt(D)) / (2.0f*A) );
		
		t = (edgeDotVel*x - edgeDotP0) / edgeSq;

		if (collisionPoint)
			*collisionPoint = seg.p0 + (seg.p1 - seg.p0) * t;

		if (t >= .0f && t <= 1.0f)
			t = x;
	}
	return (t >= .0f && t <= 1.0f) ? t : -1.0f;
}

float collideUnitSphere(const Vector3& velocity, const Plane& p)
{
	const float signedDistance = -dot(p.point, p.normal);
	const float relativeSpeed = -dot(p.normal, velocity);
	
	float t0 = -1.0f, t1 = -1.0f;

	if (relativeSpeed == .0f)
	{
		if (signedDistance < 1.0f && signedDistance > -1.0f)
		{
			t0 = 0.0f;
			t1 = 1.0f;
		}
	}
	else if (relativeSpeed > 0)
	{
		t0 = (signedDistance - 1.0f) / relativeSpeed;
		t1 = (signedDistance + 1.0f) / relativeSpeed;

		if (t0 < 0)
			t0 = .0f;
	}
	return ( t0 <= 1.0f && t1 >= .0f ) ? t0 : -1.0f;
}

float collideUnitSphere(const Vector3& velocity, const Quad& q, Vector3* collisionPoint)
{
	float t = collideUnitSphere(velocity, Plane(q.a, q.normal));
	if ( t >= .0f )
	{
		Vector3 collisionAt(velocity * t - q.normal);
		if ( !pointInQuad(collisionAt, q) )
		{
			t = 1.0f;
			float tmpT = collideUnitSphere(velocity, q.a);
			if ( tmpT >= .0f && tmpT <= t )
			{
				t = tmpT;
				collisionAt = q.a;
			}
			
			tmpT = collideUnitSphere(velocity, q.b);
			if ( tmpT >= .0f && tmpT <= t )
			{
				t = tmpT;
				collisionAt = q.b;
			}

			tmpT = collideUnitSphere(velocity, q.c);
			if ( tmpT >= .0f && tmpT <= t )
			{
				t = tmpT;
				collisionAt = q.c;
			}

			tmpT = collideUnitSphere(velocity, q.d);
			if ( tmpT >= .0f && tmpT <= t )
			{
				t = tmpT;
				collisionAt = q.d;
			}

			Vector3 edgeIntersection;
			tmpT = collideUnitSphere(velocity, Line(q.a, q.b), &edgeIntersection);
			if ( tmpT >= .0f && tmpT < t )
			{
				t = tmpT;
				collisionAt = edgeIntersection;
			}

			tmpT = collideUnitSphere(velocity, Line(q.b, q.c), &edgeIntersection);
			if ( tmpT >= .0f && tmpT < t )
			{
				t = tmpT;
				collisionAt = edgeIntersection;
			}

			tmpT = collideUnitSphere(velocity, Line(q.c, q.d), &edgeIntersection);
			if ( tmpT >= .0f && tmpT < t )
			{
				t = tmpT;
				collisionAt = edgeIntersection;
			}

			tmpT = collideUnitSphere(velocity, Line(q.d, q.a), &edgeIntersection);
			if ( tmpT >= .0f && tmpT < t )
			{
				t = tmpT;
				collisionAt = edgeIntersection;
			}
		}
		if (collisionPoint)
			*collisionPoint = collisionAt;
	}
	return t;
}

float collide(const Vector3& velocity, const Ellipsoid& e, const Quad& q, Vector3* newVelocity)
{
	Quad qTrans( (q.a - e.center) / e.radii, 
				 (q.b - e.center) / e.radii,
				 (q.c - e.center) / e.radii,
				 (q.d - e.center) / e.radii, 
				 (q.normal / e.radii).normalize() );

	//Don't test if moving parallel/away
	if (dot(velocity, q.normal) >= .0f)
		return -1.0f;

	Vector3 collisionPoint;
	const float t = collideUnitSphere( velocity / e.radii, qTrans, &collisionPoint );

	if (t >= .0f && t < 1.0f && newVelocity)
	{
		const Vector3 newPosition(velocity * t);
		const Vector3 normal = (newPosition - collisionPoint).normalize();

		const Vector3 restVelocity = (velocity - newPosition);

		const float dist = dot(restVelocity, normal);
		
		*newVelocity = restVelocity + (normal * -dist);
		*newVelocity *= e.radii;
	}
	return t;
}