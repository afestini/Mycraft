#ifndef VECTORX_INCLUDED
#define VECTORX_INCLUDED

#include <cmath>
#include <cstdlib>

class Vector2;
class Vector3;
class Vector4;

const float epsilon = .000001f;

class Vector2 {
public:
	Vector2() : x(0), y(0) {};
	explicit Vector2(float s) : x(s), y(s) {};
	Vector2(float ix, float iy) : x(ix), y(iy) {};

	float x, y;

	operator const float*() const { return &x; }

	Vector2  operator-() const { return Vector2(-x, -y); }
	Vector2  operator*(float s) const { return Vector2(s*x, s*y); }
	Vector2  operator/(float s) const { s = 1 / s; return Vector2(x*s, y*s); }
	Vector2& operator=(float s) { x = y = s; return *this; }
	Vector2& operator*=(float s) { x *= s; y *= s; return *this; }
	Vector2& operator/=(float s) { s = 1 / s; x *= s; y *= s; return *this; }

	Vector2  operator+(const Vector2& b) const { return Vector2(x + b.x, y + b.y); }
	Vector2  operator-(const Vector2& b) const { return Vector2(x - b.x, y - b.y); }
	Vector2  operator*(const Vector2& b) const { return Vector2(x * b.x, y * b.y); }
	Vector2  operator/(const Vector2& b) const { return Vector2(x / b.x, y / b.y); }
	Vector2& operator+=(const Vector2& b) { x += b.x; y += b.y; return *this; }
	Vector2& operator-=(const Vector2& b) { x -= b.x; y -= b.y; return *this; }
	Vector2& operator*=(const Vector2& b) { x *= b.x; y *= b.y; return *this; }
	Vector2& operator/=(const Vector2& b) { x /= b.x; y /= b.y; return *this; }

	float lensq() const { return x*x + y*y; }
	float len() const { return sqrt(lensq()); }
	Vector2& normalize() { return *this /= len(); }

	void random(float xlim, float ylim) {
		x = xlim*(1 - (float)(rand() % 2000) / 1000);
		y = ylim*(1 - (float)(rand() % 2000) / 1000);
	}
};

inline float dot(const Vector2& a, const Vector2& b)
{
	return a.x*b.x + a.y*b.y;
}


class Vector3 {
public:
	Vector3() : x(0), y(0), z(0) {};
	explicit Vector3(float s) : x(s), y(s), z(s) {};
	explicit Vector3(const float* v) : x(v[0]), y(v[1]), z(v[2]) {};
	Vector3(float ix, float iy, float iz) : x(ix), y(iy), z(iz) {};

	float x, y, z;

	operator const float*() const { return &x; }
	operator Vector2*() const { return (Vector2*)&x; }

	Vector3  operator-() const { return Vector3(-x, -y, -z); }
	Vector3  operator*(float s) const { return Vector3(s * x, s * y, s * z); }
	Vector3  operator/(float s) const { s = 1 / s; return Vector3(x * s, y * s, z * s); }
	Vector3& operator=(float s) { x = y = z = s; return *this; }
	Vector3& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }
	Vector3& operator/=(float s) { s = 1 / s; x *= s; y *= s; z *= s; return *this; }

	Vector3  operator+(const Vector3& b) const { return Vector3(x + b.x, y + b.y, z + b.z); }
	Vector3  operator-(const Vector3& b) const { return Vector3(x - b.x, y - b.y, z - b.z); }
	Vector3  operator*(const Vector3& b) const { return Vector3(x * b.x, y * b.y, z * b.z); }
	Vector3  operator/(const Vector3& b) const { return Vector3(x / b.x, y / b.y, z / b.z); }

	Vector3& operator+=(const Vector3& b) { x += b.x; y += b.y; z += b.z; return *this; }
	Vector3& operator-=(const Vector3& b) { x -= b.x; y -= b.y; z -= b.z; return *this; }
	Vector3& operator*=(const Vector3& b) { x *= b.x; y *= b.y; z *= b.z; return *this; }
	Vector3& operator/=(const Vector3& b) { x /= b.x; y /= b.y; z /= b.z; return *this; }

	bool operator==(const Vector3& b) const
	{
		return	fabs(x - b.x) < epsilon
			&& 	fabs(y - b.y) < epsilon
			&&  fabs(z - b.z) < epsilon;
	}
	bool operator!=(const Vector3& b) const { return !(*this == b); }

	float lensq() const { return x*x + y*y + z*z; }
	float len() const { return sqrt(lensq()); }
	Vector3& normalize() { return *this /= len(); }

	void random(float xlim, float ylim, float zlim) {
		x = xlim*(1 - (float)(rand() % 2000) / 1000);
		y = ylim*(1 - (float)(rand() % 2000) / 1000);
		z = zlim*(1 - (float)(rand() % 2000) / 1000);
	}
};

inline float dot(const Vector3& a, const Vector3& b)
{
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

inline Vector3 cross(const Vector3& a, const Vector3& b)
{
	return Vector3(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x);
}


class Vector4 {
public:
	Vector4() : x(0), y(0), z(0), w(0) {};
	explicit Vector4(float s) : x(s), y(s), z(s), w(s) {};
	Vector4(float ix, float iy, float iz, float iw) : x(ix), y(iy), z(iz), w(iw) {};

	float x, y, z, w;

	const Vector4  operator-() const { return Vector4(-x, -y, -z, -w); }
	const Vector4  operator*(float s) const { return Vector4(s*x, s*y, s*z, s*w); }
	const Vector4  operator/(float s) const { s = 1 / s; return Vector4(x*s, y*s, z*s, w*s); }
	Vector4& operator=(float s) { x = y = z = w = s; return *this; }
	Vector4& operator*=(float s) { x *= s; y *= s; z *= s; w *= s; return *this; }
	Vector4& operator/=(float s) { s = 1 / s; x *= s; y *= s; z *= s; w *= s; return *this; }

	const Vector4 operator+(const Vector4& b) const { return Vector4(x + b.x, y + b.y, z + b.z, w + b.w); }
	const Vector4 operator-(const Vector4& b) const { return Vector4(x - b.x, y - b.y, z - b.z, w - b.w); }
	const Vector4 operator*(const Vector4& b) const { return Vector4(x * b.x, y * b.y, z * b.z, w * b.w); }
	const Vector4 operator/(const Vector4& b) const { return Vector4(x / b.x, y / b.y, z / b.z, w / b.w); }
	Vector4& operator+=(const Vector4& b) { x += b.x; y += b.y; z += b.z; w += b.w; return *this; }
	Vector4& operator-=(const Vector4& b) { x -= b.x; y -= b.y; z -= b.z; w -= b.w; return *this; }
	Vector4& operator*=(const Vector4& b) { x *= b.x; y *= b.y; z *= b.z; w *= b.w; return *this; }
	Vector4& operator/=(const Vector4& b) { x /= b.x; y /= b.y; z /= b.z; w /= b.w; return *this; }

	bool operator==(const Vector4& b) const
	{
		return	fabs(x - b.x) < epsilon
			&& 	fabs(y - b.y) < epsilon
			&&  fabs(z - b.z) < epsilon
			&&  fabs(w - b.w) < epsilon;
	}

	float lensq() const { return x*x + y*y + z*z + w*w; }
	float len() const { return sqrt(lensq()); }
	Vector4& normalize() { return *this /= len(); }

	void random(float xlim, float ylim, float zlim, float wlim) {
		x = xlim*(1 - (float)(rand() % 2000) / 1000);
		y = ylim*(1 - (float)(rand() % 2000) / 1000);
		z = zlim*(1 - (float)(rand() % 2000) / 1000);
		w = wlim*(1 - (float)(rand() % 2000) / 1000);
	}
};

inline float dot(const Vector4& a, const Vector4& b)
{
	return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
}


#endif
