#ifndef GLOBALS_INCLUDED
#define GLOBALS_INCLUDED

#include <stdio.h>

#define SDELETE(x) {delete x; x=0;}
#define SDELETEARY(x) {delete[] x; x=0;}

class Globals {
public:
	static bool Wireframe;
	static bool DrawCounters;
	static bool DrawFps;
	static bool NoClip;
	static bool ShowSpheres;
	static bool ShowBoxes;
	static bool ExtCam;
	static bool FrustumCulling;
	static bool Particles;
	static int  NThreads;
};

#endif
