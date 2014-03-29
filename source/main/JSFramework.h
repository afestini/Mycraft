#ifndef SDLFRAME_INCLUDED
#define SDLFRAME_INCLUDED

#include "OpenGL.h"
#include <map>
#include <string>
#include <vector>
#include "cameraGL.h"
#include "glfont.h"
#include "Timer.h"

struct JSAction {
	std::string Name;
	unsigned Key;
};

struct ActionState
{
	bool active;
	bool handled;
};

class JSApp {
	static std::vector<JSAction> Mappings;
	static int Mouse_x, Mouse_y, Mouse_z;
	static bool ConsoleOpen;
	
	static int ReadConfig();
	static void HandleActions();
	static void ResizeCallback(int widht, int height);

public:
	static float MoveSpeed;

	static void OnKey(int key, int state);
	static void OnMouseBtn(int btn, int state);
	static void OnMouseMove(int x, int y);
	static void OnMouseWheel(int pos);
	
	static CameraGL Camera;
	static std::map<std::string, int> Config;
	static std::map<std::string, ActionState> Actions;
	static RDTimer Timer;
	static int AxisState[3];	
	static float FrameTime;	
	static char KeyState[400];
	static bool Paused;
	static bool Quit;
	static bool CursorFree;
	static int numTriangles;

	static void Init();
	static void Shutdown();

	static void FreeCursor(bool freeCursor);

	static void Tick(bool newFrame=1);
	static unsigned InputToInt(std::string key);
	static void Enter2D();
	static void EnterHUDMode();
	static void LeaveHUDMode();
	static void ShowStats();
	static void SetMoveSpeed(float s, std::string unit);
};

#endif
