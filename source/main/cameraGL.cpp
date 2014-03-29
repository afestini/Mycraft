#include "cameraGL.h"

#include <cmath>
#include <memory.h>
#include "OpenGL.h"
#include "Material.h"

CameraGL::CameraGL(float x, float y, float z)
	: xFac(0), yFac(0), xSphereFac(0), ySphereFac(0), 
	  NearP(0), FarP(0), FoV(0), ResX(0), ResY(0)
{
	Transform.SetIdentity();
	Transform.P.x = x; Transform.P.y = y; Transform.P.z = z;

	Right=&Transform.X.x;
	Up=&Transform.Y.x;
	Forward=&Transform.Z.x;
	Position=&Transform.P.x;
	Transform.Z.z = -1;

	FrustumColor[0]=.2f;
	FrustumColor[1]=.4f;
	FrustumColor[2]=1;
}

void CameraGL::SetFrustumColor(float r, float g, float b) 
{
	FrustumColor[0]=r; FrustumColor[1]=g; FrustumColor[2]=b;
};

void CameraGL::setViewDistance(float n, float f) {
	setPerspective(ResX, ResY, FoV, n, f);
}

void CameraGL::setFoV(float fov) {
	setPerspective(ResX, ResY, fov, NearP, FarP);
}

void CameraGL::setPerspective(unsigned rX, unsigned rY, float fov, float nearp, float farp) 
{
	ResX=rX; ResY=rY;
	FoV=fov;
	NearP=nearp; FarP=farp;
	yFac = tanf(FoV * 3.141592f/360.0f);
	xFac = yFac*(float)rX/rY;

	xSphereFac = 1.0f / cosf(atanf(xFac*2.0f));
	ySphereFac = 1.0f / cosf(atanf(yFac*2.0f));
	
	Projection = Matrix44(1.0f/xFac, 0, 0, 0,	
						  0, 1.0f/yFac, 0, 0,
						  0, 0, farp/(farp-nearp), 1.0f,
						  0, 0, -(farp*nearp)/(farp-nearp), 0);
}

void CameraGL::setView() 
{
	Modelview = Matrix44(
		Transform[0], Transform[4], Transform[8], 0,
		Transform[1], Transform[5], Transform[9], 0,
		Transform[2], Transform[6], Transform[10], 0,
		-(Transform[0]*Transform[12] + Transform[1]*Transform[13] + Transform[2]*Transform[14]),
		-(Transform[4]*Transform[12] + Transform[5]*Transform[13] +	Transform[6]*Transform[14]),						  
		-(Transform[8]*Transform[12] + Transform[9]*Transform[13] + Transform[10]*Transform[14]), 1);
}

void CameraGL::move(float x, float y, float z, bool global, float distance) 
{
	if (global) 
	{
		Transform[12] += x * distance;
		Transform[13] += y * distance;
		Transform[14] += z * distance;
	}
	else 
	{
		Transform[12] += distance * (x*Transform[0] + y*Transform[4] + z*Transform[8]);
		Transform[13] += distance * (x*Transform[1] + y*Transform[5] + z*Transform[9]);
		Transform[14] += distance * (x*Transform[2] + y*Transform[6] + z*Transform[10]);
	}
}

void CameraGL::rotate(float deg, float x, float y, float z, bool global)
{
	glPushMatrix();

	glLoadMatrixf(Transform);

	if (global) glRotatef(deg,
		x*Transform[0] + y*Transform[1] + z*Transform[2],
		x*Transform[4] + y*Transform[5] + z*Transform[6],
		x*Transform[8] + y*Transform[9] + z*Transform[10]);
	else 
		glRotatef(deg, x,y,z);

	glGetFloatv(GL_MODELVIEW_MATRIX, Transform);
	glPopMatrix();
}

int CameraGL::SphereVisible(const float pos[3], float radius) const
{
	float d[3]={pos[0]-Position[0], pos[1]-Position[1], pos[2]-Position[2]};

	float camZ=Forward[0]*d[0] + Forward[1]*d[1] + Forward[2]*d[2];
	if (camZ+radius < NearP || camZ-radius > FarP) return 0;

	float xLim=camZ*xFac;
	float xRad = radius*xSphereFac;
	float camX=Right[0]*d[0] + Right[1]*d[1] + Right[2]*d[2];
	if ((camX+xRad < -xLim) || (camX-xRad > xLim)) return 0;
	
	float yLim=camZ*yFac;
	float yRad = radius*ySphereFac;
	float camY=Up[0]*d[0] + Up[1]*d[1] + Up[2]*d[2];
	if ((camY+yRad < -yLim) || (camY-yRad > yLim)) return 0;

	if (camZ-radius>NearP && camZ+radius<FarP && camX-xRad>-xLim && camX+xRad<xLim
		&& camY-yRad>-yLim && camY+yRad<yLim) return 1;

	return -1;
}

bool CameraGL::BoxVisible(float bbox[24]) 
{
	float d[3];
	int l=0, r=0, f=0, n=0, t=0, b=0;
	
	for (int i=0; i<8; ++i) 
	{
		bool inX = false;
		bool inY = false;
		bool inZ = false;

		d[0] = bbox[(i*3)]-Position[0];
		d[1] = bbox[(i*3)+1]-Position[1];
		d[2] = bbox[(i*3)+2]-Position[2];

		const float camX = Right[0]*d[0] + Right[1]*d[1] + Right[2]*d[2];
		const float camY = Up[0]*d[0] + Up[1]*d[1] + Up[2]*d[2];
		const float camZ = Forward[0]*d[0] + Forward[1]*d[1] + Forward[2]*d[2];

		if (camX < -xFac*camZ) ++l;
		else if (camX > xFac*camZ) ++r;
		else inX=true;
		
		if (camY < -yFac*camZ) b++;
		else if (camY > yFac*camZ) t++;
		else inY=true;

		if (camZ < NearP) ++n;		
		else if (camZ > FarP) ++f;
		else inZ=true;

		if (inX && inY && inZ) return true;
	}

	if (l==8 || r==8 || f==8 || n==8 || b==8 || t==8) return false;
	return true;
}

bool CameraGL::AABBVisible(const float bbox[6]) const
{
	int l=0, r=0, f=0, n=0, t=0, b=0;
	
	float tmpcorners[2][3];
	tmpcorners[0][0]=bbox[0]-Position[0];
	tmpcorners[0][1]=bbox[1]-Position[1];
	tmpcorners[0][2]=bbox[2]-Position[2];
	tmpcorners[1][0]=bbox[3]-Position[0];
	tmpcorners[1][1]=bbox[4]-Position[1];
	tmpcorners[1][2]=bbox[5]-Position[2];

	for (int i=0; i<8; ++i)
	{
		bool inX = false;
		bool inY = false;
		bool inZ = false;

		float d[3] = {tmpcorners[i&1][0], tmpcorners[(i>>2)&1][1], tmpcorners[(i>>1)&1][2]};

		const float camX = Right[0]*d[0] + Right[1]*d[1] + Right[2]*d[2];
		const float camY = Up[0] * d[0] + Up[1] * d[1] + Up[2] * d[2];
		const float camZ = Forward[0] * d[0] + Forward[1] * d[1] + Forward[2] * d[2];

		if (camX < -xFac*camZ) ++l;
		else if (camX > xFac*camZ) ++r;
		else inX=true;
		
		if (camY < -yFac*camZ) b++;
		else if (camY > yFac*camZ) t++;
		else inY=true;

		if (camZ < NearP) ++n;		
		else if (camZ > FarP) ++f;
		else inZ=true;

		if (inX && inY && inZ) return true;
	}

	if (l==8 || r==8 || f==8 || n==8 || b==8 || t==8) return false;
	return true;
}

void CameraGL::DrawFrustum(float dist) 
{
	if (!dist) dist=FarP;
	float front[3]={Forward[0]*dist - Position[0], Forward[1]*dist - Position[1], Forward[2]*dist - Position[2]};
	float up[3]={Up[0]*yFac*dist, Up[1]*yFac*dist, Up[2]*yFac*dist};
	float right[3]={Right[0]*xFac*dist, Right[1]*xFac*dist, Right[2]*xFac*dist};

	MaterialManager::DisableBlending();
	MaterialManager::SetColor(FrustumColor[0]*FrustumColor[0], 
							  FrustumColor[1]*FrustumColor[1],
							  FrustumColor[2]*FrustumColor[2], 1);

	glBegin(GL_LINE_LOOP);
		glVertex3f(Position[0], Position[1], Position[2]);
		glVertex3f(front[0]+up[0]+right[0], front[1]+up[1]+right[1], front[2]+up[2]+right[2]);
		glVertex3f(front[0]+up[0]-right[0], front[1]+up[1]-right[1], front[2]+up[2]-right[2]);
		glVertex3f(Position[0], Position[1], Position[2]);
		glVertex3f(front[0]-up[0]+right[0], front[1]-up[1]+right[1], front[2]-up[2]+right[2]);
		glVertex3f(front[0]-up[0]-right[0], front[1]-up[1]-right[1], front[2]-up[2]-right[2]);
	glEnd();

	glBegin(GL_LINES);
		glVertex3f(front[0]+up[0]+right[0], front[1]+up[1]+right[1], front[2]+up[2]+right[2]);
		glVertex3f(front[0]-up[0]+right[0], front[1]-up[1]+right[1], front[2]-up[2]+right[2]);
		glVertex3f(front[0]+up[0]-right[0], front[1]+up[1]-right[1], front[2]+up[2]-right[2]);
		glVertex3f(front[0]-up[0]-right[0], front[1]-up[1]-right[1], front[2]-up[2]-right[2]);
	glEnd();

	MaterialManager::DisableTextures();
	MaterialManager::SetColor(FrustumColor[0]*2, FrustumColor[1]*2, FrustumColor[2]*2, .5f);
	MaterialManager::BlendMode(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.0, 1.0);

	glBegin(GL_TRIANGLE_FAN);
	glVertex3fv(Position);
	glVertex3f(front[0]+up[0]+right[0], front[1]+up[1]+right[1], front[2]+up[2]+right[2]);
	glVertex3f(front[0]+up[0]-right[0], front[1]+up[1]-right[1], front[2]+up[2]-right[2]);
	glVertex3f(front[0]-up[0]-right[0], front[1]-up[1]-right[1], front[2]-up[2]-right[2]);
	glVertex3f(front[0]-up[0]+right[0], front[1]-up[1]+right[1], front[2]-up[2]+right[2]);
	glVertex3f(front[0]+up[0]+right[0], front[1]+up[1]+right[1], front[2]+up[2]+right[2]);
	glEnd();

	glBegin(GL_QUADS);
	glVertex3f(front[0]-up[0]+right[0], front[1]-up[1]+right[1], front[2]-up[2]+right[2]);
	glVertex3f(front[0]-up[0]-right[0], front[1]-up[1]-right[1], front[2]-up[2]-right[2]);
	glVertex3f(front[0]+up[0]-right[0], front[1]+up[1]-right[1], front[2]+up[2]-right[2]);
	glVertex3f(front[0]+up[0]+right[0], front[1]+up[1]+right[1], front[2]+up[2]+right[2]);
	glEnd();

	glDisable(GL_POLYGON_OFFSET_FILL);

	MaterialManager::SetColor(1,1,1,1);
}

void CameraGL::Screen2World(int x, int y, float vec[3]) const
{
	float vx = xFac * (2.0f*x/ResX - 1.0f);
	float vy = yFac * (2.0f*y/ResY - 1.0f);
	vec[0]= (Forward[0] + vx*Right[0] - vy*Up[0]);
	vec[1]= (Forward[1] + vx*Right[1] - vy*Up[1]);
	vec[2]= (Forward[2] + vx*Right[2] - vy*Up[2]);
}

void CameraGL::World2Screen(const float vec[3], int result[2]) const 
{
	float d[3] = { vec[0]-Position[0], vec[1]-Position[1], vec[2]-Position[2] };
	
	float x = Right[0]*d[0] + Right[1]*d[1] + Right[2]*d[2];
	float y = Up[0]*d[0] + Up[1]*d[1] + Up[2]*d[2];
	float z = Forward[0]*d[0] + Forward[1]*d[1] + Forward[2]*d[2];

	result[0] = int( ((ResX*x) / (z*xFac) + ResX) * .5f );
	result[1] = int( ((ResY*y) / (z*yFac) + ResY) * .5f );
}

void CameraGL::reset() 
{
	Transform.SetIdentity();
	Transform.Z.z = -1.0f;
}
