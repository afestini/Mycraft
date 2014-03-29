#ifndef MATRIX_INCLUDED
#define MATRIX_INCLUDED

#include "VectorX.h"
#include <memory.h>
#include <utility>

class Matrix44;
static unsigned MatrixTransformationLimit=50;




class Matrix44 {
	unsigned numTrans;
public:
	Vector4 X;
	Vector4 Y;
	Vector4 Z;
	Vector4 P;

	Matrix44() :numTrans(0) {};
	Matrix44(float v0, float v1, float v2, float v3, float v4, float v5, float v6, float v7,
		     float v8, float v9, float v10, float v11, float v12, float v13, float v14, float v15)
	: numTrans(0), X(v0,v1,v2,v3), Y(v4,v5,v6,v7), Z(v8,v9,v10,v11), P(v12,v13,v14,v15) {}

	explicit Matrix44(const float m[16])
		: numTrans(0), X(m[0],m[1],m[2],m[3]), Y(m[4],m[5],m[6],m[7]), Z(m[8],m[9],m[10],m[11]), P(m[12],m[13],m[14],m[15]) {}

	inline void SetIdentity() {
		memset(&X.x, 0, 16*sizeof(float));
		X.x=Y.y=Z.z=P.w=1;
	}

	inline void Random() {
		float* t=&X.x;
		for (int i=0; i<16; ++i) *(t++)=(float)(rand()%10000)/5000 - 1.0f;
	}

	inline void ReOrtho() {
		X.normalize();
		Y.normalize();
		Z.normalize();
		numTrans=0;
	}

	inline operator float*() {return &X.x;}
	inline operator const float*() const {return &X.x;}

	inline Matrix44& RotateX(float rad) {
		float cr=cos(rad), sr=sin(rad);
		Vector4 tmp=Y;
		Y.x=Y.x*cr + Z.x*sr;  Y.y=Y.y*cr + Z.y*sr;  Y.z=Y.z*cr + Z.z*sr;
		Z.x=tmp.x*-sr + Z.x*cr;  Z.y=tmp.y*-sr + Z.y*cr;  Z.z=tmp.z*-sr + Z.z*cr;
		if (++numTrans>MatrixTransformationLimit) ReOrtho();
		return *this;
	}

	inline Matrix44& RotateY(float rad) {
		float cr=cos(rad), sr=sin(rad);
		Vector4 tmp=X;
		X.x=X.x*cr + Z.x*sr;  X.y=X.y*cr + Z.y*sr;  X.z=X.z*cr + Z.z*sr;
		Z.x=tmp.x*-sr + Z.x*cr;  Z.y=tmp.y*-sr + Z.y*cr;  Z.z=tmp.z*-sr + Z.z*cr;
		if (++numTrans>MatrixTransformationLimit) ReOrtho();
		return *this;
	}

	inline Matrix44& RotateZ(float rad) {
		float cr=cos(rad), sr=sin(rad);
		Vector4 tmp=X;
		X.x=X.x*cr + Y.x*sr;  X.y=X.y*cr + Y.y*sr;  X.z=X.z*cr + Y.z*sr;
		Y.x=tmp.x*-sr + Y.x*cr;  Y.y=tmp.y*-sr + Y.y*cr;  Y.z=tmp.z*-sr + Y.z*cr;
		if (++numTrans>MatrixTransformationLimit) ReOrtho();
		return *this;
	}

	inline Matrix44& TransposeIn() {
		std::swap(X.y,Y.x);
		std::swap(X.z,Z.x);
		std::swap(X.w,P.x);		
		std::swap(Y.z,Z.y);
		std::swap(Y.w,P.y);
		std::swap(Z.w,P.z);
		return *this;
	}

	inline const Matrix44 Transpose() const
	{
		return Matrix44(X.x,Y.x,Z.x,P.x, X.y,Y.y,Z.y,P.y, X.z,Y.z,Z.z,P.z, X.w,Y.w,Z.w,P.w);
	}

	inline const Matrix44 QuickInverse() const
	{
		return Matrix44(
		X.x, Y.x, Z.x, 0,
		X.y, Y.y, Z.y, 0,
		X.z, Y.z, Z.z, 0,
		-(X.x*P.x + X.y*P.y + X.z*P.z),
		-(Y.x*P.x + Y.y*P.y + Y.z*P.z),						  
		-(Z.x*P.x + Z.y*P.y + Z.z*P.z), 1);
	}

	inline Vector4 operator* (const Vector4& v) const
	{
		return Vector4(
			X.x*v.x + Y.x*v.y + Z.x*v.z + P.x*v.w,
			X.y*v.x + Y.y*v.y + Z.y*v.z + P.y*v.w,
			X.z*v.x + Y.z*v.y + Z.z*v.z + P.z*v.w,
			X.w*v.x + Y.w*v.y + Z.w*v.z + P.w*v.w);
	}

	inline Matrix44 operator* (const Matrix44& b) const
	{
		Matrix44 r;
		r.X.x=X.x*b.X.x + Y.x*b.X.y + Z.x*b.X.z + P.x*b.X.w;
		r.Y.x=X.x*b.Y.x + Y.x*b.Y.y + Z.x*b.Y.z + P.x*b.Y.w;
		r.Z.x=X.x*b.Z.x + Y.x*b.Z.y + Z.x*b.Z.z + P.x*b.Z.w;
		r.P.x=X.x*b.P.x + Y.x*b.P.y + Z.x*b.P.z + P.x*b.P.w;

		r.X.y=X.y*b.X.x + Y.y*b.X.y + Z.y*b.X.z + P.y*b.X.w;
		r.Y.y=X.y*b.Y.x + Y.y*b.Y.y + Z.y*b.Y.z + P.y*b.Y.w;
		r.Z.y=X.y*b.Z.x + Y.y*b.Z.y + Z.y*b.Z.z + P.y*b.Z.w;
		r.P.y=X.y*b.P.x + Y.y*b.P.y + Z.y*b.P.z + P.y*b.P.w;

		r.X.z=X.z*b.X.x + Y.z*b.X.y + Z.z*b.X.z + P.z*b.X.w;
		r.Y.z=X.z*b.Y.x + Y.z*b.Y.y + Z.z*b.Y.z + P.z*b.Y.w;
		r.Z.z=X.z*b.Z.x + Y.z*b.Z.y + Z.z*b.Z.z + P.z*b.Z.w;
		r.P.z=X.z*b.P.x + Y.z*b.P.y + Z.z*b.P.z + P.z*b.P.w;

		r.X.w=X.w*b.X.x + Y.w*b.X.y + Z.w*b.X.z + P.w*b.X.w;
		r.Y.w=X.w*b.Y.x + Y.w*b.Y.y + Z.w*b.Y.z + P.w*b.Y.w;
		r.Z.w=X.w*b.Z.x + Y.w*b.Z.y + Z.w*b.Z.z + P.w*b.Z.w;
		r.P.w=X.w*b.P.x + Y.w*b.P.y + Z.w*b.P.z + P.w*b.P.w;
		return r;
	}

	inline Matrix44& operator*=(const Matrix44& b) 
	{
		float tx=X.x, ty=Y.x, tz=Z.x;
		X.x=tx*b.X.x + ty*b.X.y + tz*b.X.z + P.x*b.X.w;
		Y.x=tx*b.Y.x + ty*b.Y.y + tz*b.Y.z + P.x*b.Y.w;
		Z.x=tx*b.Z.x + ty*b.Z.y + tz*b.Z.z + P.x*b.Z.w;
		P.x=tx*b.P.x + ty*b.P.y + tz*b.P.z + P.x*b.P.w;

		tx=X.y; ty=Y.y; tz=Z.y;
		X.y=tx*b.X.x + ty*b.X.y + tz*b.X.z + P.y*b.X.w;
		Y.y=tx*b.Y.x + ty*b.Y.y + tz*b.Y.z + P.y*b.Y.w;
		Z.y=tx*b.Z.x + ty*b.Z.y + tz*b.Z.z + P.y*b.Z.w;
		P.y=tx*b.P.x + ty*b.P.y + tz*b.P.z + P.y*b.P.w;

		tx=X.z; ty=Y.z; tz=Z.z;
		X.z=tx*b.X.x + ty*b.X.y + tz*b.X.z + P.z*b.X.w;
		Y.z=tx*b.Y.x + ty*b.Y.y + tz*b.Y.z + P.z*b.Y.w;
		Z.z=tx*b.Z.x + ty*b.Z.y + tz*b.Z.z + P.z*b.Z.w;
		P.z=tx*b.P.x + ty*b.P.y + tz*b.P.z + P.z*b.P.w;

		tx=X.w; ty=Y.w; tz=Z.w;
		X.w=tx*b.X.x + ty*b.X.y + tz*b.X.z + P.w*b.X.w;
		Y.w=tx*b.Y.x + ty*b.Y.y + tz*b.Y.z + P.w*b.Y.w;
		Z.w=tx*b.Z.x + ty*b.Z.y + tz*b.Z.z + P.w*b.Z.w;
		P.w=tx*b.P.x + ty*b.P.y + tz*b.P.z + P.w*b.P.w;
		if (++numTrans>MatrixTransformationLimit) ReOrtho();
		return *this;
	}

	inline static Matrix44 Identity()
	{
		return Matrix44(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1);
	}

	inline static Matrix44 Translation(float x, float y, float z)
	{
		return Matrix44(1,0,0,0, 0,1,0,0, 0,0,1,0, x,y,z,1);
	}

	inline static Matrix44 Scale(float x, float y, float z)
	{
		return Matrix44(x,0,0,0, 0,y,0,0, 0,0,z,0, 0,0,0,1);
	}

	inline static Matrix44 RotationX(float rad) {
		const float cr=cos(rad), sr=sin(rad);
		return Matrix44(1,0,0,0,
			            0,cr,sr,0,
						0,-sr,cr,0,
						0,0,0,1);
	}
	inline static Matrix44 RotationY(float rad) {
		const float cr=cos(rad), sr=sin(rad);
		return Matrix44(cr,0,sr,0,
			            0,1,0,0,
						-sr,0,cr,0,
						0,0,0,1);
	}
	inline static Matrix44 RotationZ(float rad) {
		const float cr=cos(rad), sr=sin(rad);
		return Matrix44(cr,sr,0,0,
			            -sr,cr,0,0,
						0,0,1,0,
						0,0,0,1);
	}

	static inline const Matrix44 Rotation(float rad, float x, float y, float z) 
	{
		const float q0 = cos(rad/2),  q1 = sin(rad/2)*x,  q2 = sin(rad/2)*y,  q3 = sin(rad/2)*z;

		return Matrix44(
		 q0*q0 + q1*q1 - q2*q2 - q3*q3,		2*(q1*q2 - q0*q3),	-2*(q1*q3 + q0*q2),	0,
		 2*(q2*q1 + q0*q3),		q0*q0 - q1*q1 + q2*q2 - q3*q3,	-2*(q2*q3 - q0*q1),	0,
		 -2*(q3*q1 - q0*q2),	-2*(q3*q2 + q0*q1),	q0*q0 - q1*q1 - q2*q2 + q3*q3,	0,
		 0,0,0,1);
	}
};

inline Matrix44 mirrorMatrix(float nx, float ny, float nz, float d) 
{
	return Matrix44(1.0f-2.0f*nx*nx, -2.0f*ny*nx, -2.0f*nz*nx, 0,
					-2.0f*nx*ny, 1.0f-2.0f*ny*ny, -2.0f*nz*ny, 0,
					-2.0f*nx*nz, -2.0f*ny*nz, 1.0f-2.0f*nz*nz, 0,
					-2.0f*nx*d, -2.0f*ny*d, -2.0f*nz*d, 1.0f);
}

#endif
