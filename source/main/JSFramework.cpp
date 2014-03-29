#include "JSFramework.h"
#include "OpenGL.h"
#include <GL/glfw.h>
#include "Loader.h"
#include "Console.h"
#include "Globals.h"
#include "ProfileCounter.h"

#include <cstdio>
#include <map>
#include <string>
#include <fstream>

using namespace std;

static const char* CONFIG_FILE = "data/config/config.cfg";

std::vector<JSAction> JSApp::Mappings;
map<string, ActionState> JSApp::Actions;
int JSApp::Mouse_x, JSApp::Mouse_y, JSApp::Mouse_z;
float JSApp::MoveSpeed=100;
bool JSApp::CursorFree=0;
bool JSApp::ConsoleOpen=0;
CameraGL JSApp::Camera;
map<string, int> JSApp::Config;
RDTimer JSApp::Timer;
int JSApp::AxisState[3];	
float JSApp::FrameTime=0;	
char JSApp::KeyState[400];
bool JSApp::Paused=0;
bool JSApp::Quit=0;
int JSApp::numTriangles=0;

void JSApp::OnMouseBtn(int btn, int state) 
{
	KeyState[350+btn] = char(state);
}
void JSApp::OnMouseMove(int x, int y) 
{
	AxisState[0]=x-Mouse_x;
	AxisState[1]=y-Mouse_y;
	Mouse_x=x; Mouse_y=y;
}
void JSApp::OnMouseWheel(int pos) 
{
	AxisState[2]=pos-Mouse_z;
	Mouse_z=pos;
}
void JSApp::ResizeCallback(int, int) 
{
	glfwSetWindowSize(Config["ScreenX"], Config["ScreenY"]);
}

void JSApp::OnKey(int key, int state)
{
	if (!state) 
		KeyState[key]=0;
	else if ( !ConsoleOpen || !Console::ReceiveKey(key) )
		KeyState[key]=1;
}

void JSApp::Init()
{
	memset(KeyState, 0, sizeof(KeyState));
	if (ReadConfig()) 
	{
		printf("Could not read configuration file, run config.exe!\n");
		return;
	}

	if (!glfwInit()) 
	{
		printf("GLFW failed to initialize!\n");
		return;
	}
	else printf("GLFW initialized\n");

	if(!glfwOpenWindow(Config["ScreenX"],Config["ScreenY"],
		Config["ColorBits"]/4, Config["ColorBits"]/4, Config["ColorBits"]/4, Config["ColorBits"]/4,
		Config["DepthBits"],0, (Config["Fullscreen"]) ? GLFW_FULLSCREEN : GLFW_WINDOW)) 
	{
		printf("Could not set Video Mode\n");
		glfwTerminate(); 
		return;
	}
	glfwSetWindowTitle( "MC world renderer" );
	glfwSetWindowSizeCallback(ResizeCallback);

	int ActualColor = glfwGetWindowParam(GLFW_RED_BITS) +
					  glfwGetWindowParam(GLFW_GREEN_BITS) +
					  glfwGetWindowParam(GLFW_BLUE_BITS) +
					  glfwGetWindowParam(GLFW_ALPHA_BITS);	
	int ActualDepth = glfwGetWindowParam(GLFW_DEPTH_BITS);

	Console::Output("OpenGL initialized\n");
	char out[50];
	_snprintf(out, 50, "Video Mode set to %dbit Color and %dbit Depth\n", ActualColor, ActualDepth);
	Console::Output(out);

	if (glewInit()) Console::Output("ERROR: OpenGL extenstions failed to initialize!\n");
	else Console::Output("OpenGL extensions initialized\n");
	
	MaterialManager::Init();
	Console::Output("Material Manager initialized\n");
	
	Console::LoadBackground("console.dds");
	glViewport(0,0,Config["ScreenX"],Config["ScreenY"]);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glClearColor(.3f,.5f,.8f,1);
	glDepthFunc(GL_LESS);

	Font::Init("data/font/verdana.glf");
	Console::Output("Font initialized\n");

	RDTimer::Init();
	Timer.Start();
	Console::Output("Timer initialized, Measured CPU speed: ");
	_snprintf(out, 50, "%.2fMhz\n", (0.000001f/RDTimer::cpuSpeed));
	Console::Output(out);

	Camera.setPerspective(Config["ScreenX"], Config["ScreenY"], 45, .5, 4000);
	Console::Output("Camera set to 45 degree fov and 5000 units view distance\n");

	glfwGetMousePos(&Mouse_x, &Mouse_y);
	Mouse_z=glfwGetMouseWheel();
	glfwSetKeyCallback(OnKey);
	glfwSetMouseButtonCallback(OnMouseBtn);
	glfwSetMousePosCallback(OnMouseMove);
	glfwSetMouseWheelCallback(OnMouseWheel);
	glfwEnable(GLFW_KEY_REPEAT);
	glfwDisable(GLFW_MOUSE_CURSOR);
	
	Console::Output("Mouse and Keyboard Callbacks registered\n");

	Console::LoadBackground("data/textures/console.dds");
	Console::ExecuteScript("data/config/autoexec.csf");
	Console::ListCommands();
}

void JSApp::Shutdown() {
	Console::Output("Shutting down...\n");
	Font::Cleanup();
	glfwTerminate();
}

int JSApp::ReadConfig() 
{
	Config.clear();
	ifstream file(CONFIG_FILE);

	if (!file) return -1;
	
	int value;
	string option;
	string key;

	file>>option;
	while (!file.eof()) {
		if (option == "[MAPPING]") {
			while (!file.eof() && '['!= (file>>option, option[0])) 
			{	
				file >> key;
				JSAction action;
				action.Name=option;
				action.Key=InputToInt(key);
				Mappings.push_back(action);
			}
		}
		else
		{
			while (!file.eof() && '['!= (file>>option, option[0])) 
			{
				file >> value;
				Config[option]=value;
			}
		}
	}
	file.close();
	return 0;
}

void JSApp::FreeCursor(bool freeCursor)
{
	CursorFree = freeCursor;
	if (CursorFree) glfwEnable(GLFW_MOUSE_CURSOR);
	else glfwDisable(GLFW_MOUSE_CURSOR);
}

void JSApp::HandleActions() 
{
	float CamMove[3]={0};
	float CamRotate[3]={0};

	for (vector<JSAction>::iterator it = Mappings.begin(); it!=Mappings.end(); ++it) 
	{
		if (it->Name=="Quit" && KeyState[it->Key]) {Quit=1; KeyState[it->Key]=0; return;}
		if (it->Name=="Console" && KeyState[it->Key]) 
		{
			ConsoleOpen=!ConsoleOpen;
			KeyState[it->Key]=0;
			continue;
		}
		if (it->Name=="FreeCursor" && KeyState[it->Key]) 
		{
			FreeCursor(!CursorFree);			
			KeyState[it->Key]=0;
			continue;
		}
		if (it->Name=="Pause" && KeyState[it->Key]) {
			Paused=!Paused;
			KeyState[it->Key]=0;
			continue;
		}

		if (it->Name=="CamMoveRight" && KeyState[it->Key]) {CamMove[0]+=1; continue;}
		if (it->Name=="CamMoveLeft" && KeyState[it->Key]) {CamMove[0]+=-1; continue;}
		if (it->Name=="CamMoveForward" && KeyState[it->Key]) {CamMove[2]+=1; continue;}
		if (it->Name=="CamMoveBackward" && KeyState[it->Key]) {CamMove[2]+=-1; continue;}
		if (it->Name=="CamMoveUp" && KeyState[it->Key]) {CamMove[1]+=1; continue;}
		if (it->Name=="CamMoveDown" && KeyState[it->Key]) {CamMove[1]+=-1; continue;}
		
		if (!CursorFree) {
			if (it->Name=="CamPanLR" && AxisState[it->Key]) {
				CamRotate[0] += 0.1f * AxisState[it->Key]; continue;
			}
			if (it->Name=="CamPanUD" && AxisState[it->Key]) {
				CamRotate[1] += 0.1f * AxisState[it->Key]; continue;
			}
			if (it->Name=="CamRoll" && AxisState[it->Key]) {
				CamRotate[2] += 0.1f * AxisState[it->Key]; continue;
			}
		}

		if ( !KeyState[it->Key] )
			Actions[it->Name].handled = false;
		Actions[it->Name].active = KeyState[it->Key] && !Actions[it->Name].handled;
	}
	if (CamRotate[0]) 
		Camera.rotate(CamRotate[0], 0,1,0, 1);
	if (CamRotate[1]) 
		Camera.rotate(CamRotate[1], 1,0,0);
	if (CamRotate[2]) 
		Camera.rotate(CamRotate[2], 0,0,1);

	Camera.move(CamMove[0]*FrameTime*MoveSpeed, CamMove[1]*FrameTime*MoveSpeed, CamMove[2]*FrameTime*MoveSpeed);
	memset(AxisState, 0, sizeof(AxisState));
}


void JSApp::SetMoveSpeed(float s, string unit) 
{
	if (unit=="kmh") s*=0.277777f;
	else if (unit=="mph") s*=0.44704f;
	else if (unit!="mps") Console::Output("Unknown unit, defaulting to mps (use 'kmh', 'mph' or 'mps')!\n");
	MoveSpeed=s;
	char out[100];
	_snprintf(out, 100, "Movement speed set to %.2fmps = %.2fkmh = %.2fmph\n", s, s*3.6, s*2.2369363f);
	Console::Output(out);
}

void JSApp::Tick(bool newFrame) 
{
	if (!glfwGetWindowParam(GLFW_OPENED)) 
	{
		Quit=1;
		return;
	}
	FrameTime=Timer.Reset();
	HandleActions();
	if (!newFrame) return;

	numTriangles=0;
	static ProfileCounter presentCounter("Swap buffer", .2f, 5);
	presentCounter.Start();
	glfwSwapBuffers();
	presentCounter.Stop();

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(Camera.Projection);
	glMatrixMode(GL_MODELVIEW);

	if (Globals::ExtCam) 
	{
		CameraGL tmp(0,500,0);
		tmp.rotate(0, 0,1,0);
		tmp.rotate(90, 1,0,0);
		tmp.setView();
		glLoadMatrixf(tmp.Modelview);
	}
	else
	{
		Camera.setView();
		glLoadMatrixf(Camera.Modelview);
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void JSApp::Enter2D() 
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	float PMtr[16]={2.f/JSApp::Config["ScreenX"],0,0,0,	 0,-2.f/JSApp::Config["ScreenY"],0,0,  0,0,-1,0,  -1,1,0,1};
	glLoadMatrixf(PMtr);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	MaterialManager::BlendMode(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
}

void JSApp::EnterHUDMode()
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	float PMtr[16]={.02f,0,0,0,	 0,-.02f,0,0,  0,0,-1,0,  -1,1,0,1};
	glLoadMatrixf(PMtr);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	MaterialManager::BlendMode(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
}

void JSApp::LeaveHUDMode() 
{
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
	MaterialManager::DisableBlending();
}

void JSApp::ShowStats() 
{
	static RDTimer FpsTimer;
	static float frames=0;
	static float fps=0;

	++frames;
	if (FpsTimer.Peek() > 1) 
	{
		fps=frames/FpsTimer.Reset();
		frames=0;
	}

	if (Globals::ExtCam) 
		Camera.DrawFrustum();
	
	EnterHUDMode();

	if (Globals::DrawCounters)
		ProfileCounter::DrawCounters();

	if (Globals::DrawFps) 
	{
		static char strg[80]="Fps: measuring";
	
		_snprintf(strg, 80, "Fps: %.2f   Tris: %d  (%.2f MTri/s)", fps, numTriangles, (fps*numTriangles)/1000000);
		Font::Print(strg, 0.5f, .5f, 2);

		_snprintf(strg, 80, "Pos: %.2f/%.2f/%.2f", Camera.Position[0], Camera.Position[1], Camera.Position[2]);	
		Font::Print(strg, 65.f, .5f, 2);
	}

	if (ConsoleOpen) 
		Console::Render();

	glEnable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	LeaveHUDMode();
}
