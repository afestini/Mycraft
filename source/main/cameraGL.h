#ifndef CAMERAGL_INCLUDED
#define CAMERAGL_INCLUDED

#include "matrixx.h"

class CameraGL {
public:
	float const *Right, *Up, *Forward;
	float *Position;
	float xFac, yFac;
	float xSphereFac, ySphereFac;
	float NearP, FarP;
	float FoV;

//private:
	float FrustumColor[3];
	Matrix44 Transform;
	Matrix44 Modelview;
	Matrix44 Projection;
	unsigned ResX, ResY;

public:
	CameraGL(float x=0.0f, float y=0.0f, float z=0.0f);

	const float* modelview() const { return &Modelview.X.x; }
	const float* projection() const { return &Projection.X.x; }
	const Matrix44 modelviewprojection() const { return Projection * Modelview; }

	void setView();
	void move(float x, float y, float z, bool global=false, float distance=1);	
	void rotate(float deg, float x, float y, float z, bool global=false);

	void setPerspective(unsigned resX, unsigned resY, float fov, float nearp, float farp);
	int SphereVisible(const float pos[3], float radius) const;
	int AltSphereVisible(float pos[3], float radius);
	bool BoxVisible(float bbox[24]);
	bool AABBVisible(const float bbox[6]) const;
	void SetFrustumColor(float r, float g, float b);
	void DrawFrustum(float dist=0);
	//Please note this returns the direction vector (for intersections tests)
	void Screen2World(int x, int y, float vec[3]) const;
	//Returns screen coordinates relative to the viewport, fancy viewports need to add their position
	//Also: Just added that without any testing, so might return complete BS
	void World2Screen(const float vec[3], int result[2]) const;
	void setViewDistance(float n, float f);
	void setFoV(float fov);
	void reset();
};

#endif

