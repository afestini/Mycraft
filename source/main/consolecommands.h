#define BEGINCOMMAND(x) class _Cmd##x :public Console::Command {public: \
						_Cmd##x () {Name=#x; Console::AddCommand(this);} \
						virtual void Execute(string*) {

#define ENDCOMMAND(x) }} Cmd##x;


void Switch(bool& val, string name) {
	val =! val;
	Console::Output(name+' ');
	Console::Output(val ? "enabled\n" : "disabled\n");
}

#include "JSFramework.h"

BEGINCOMMAND(externalcam)
Switch(Globals::ExtCam, "External Camera");
ENDCOMMAND(externalcam)

BEGINCOMMAND(frustumculling)
Switch(Globals::FrustumCulling, "Frustum Culling");
ENDCOMMAND(frustumculling)

BEGINCOMMAND(drawcounters)
Switch(Globals::DrawCounters, "Draw Counters");
ENDCOMMAND(drawcounter)

BEGINCOMMAND(autotype)
Switch(Console::AutoType, "Autotype");
ENDCOMMAND(autotype)

BEGINCOMMAND(drawfps)
Switch(Globals::DrawFps, "Fps");
ENDCOMMAND(drawfps)

BEGINCOMMAND(noclip)
Switch(Globals::NoClip, "No clip");
ENDCOMMAND(noclip)

BEGINCOMMAND(listcommands)
Console::ListCommands();
ENDCOMMAND(listcommands)

class _CmdCamSetViewDistance: public Console::Command {
public:
	_CmdCamSetViewDistance() {
		Name="camsetviewdistance";
		Params.push_back(Console::Param("fNear", Console::FLOAT));
		Params.push_back(Console::Param("fFar", Console::FLOAT));
		Console::AddCommand(this);
	}
	void Execute(string* str) {
		JSApp::Camera.setViewDistance((float)atof(str[0].c_str()), (float)atof(str[1].c_str()));
	}
} CmdSetViewDistance;

class _CmdCamSetFoV: public Console::Command {
public:
	_CmdCamSetFoV() {
		Name="camsetfov";
		Params.push_back(Console::Param("fFoV", Console::FLOAT));
		Console::AddCommand(this);
	}
	void Execute(string* str) {
		JSApp::Camera.setFoV((float)atof(str[0].c_str()));
	}
} CmdCamSetFoV;

class _CmdCamRotate: public Console::Command {
public:
	_CmdCamRotate() {
		Name="camrotate";
		Params.push_back(Console::Param("fDegree", Console::FLOAT));
		Params.push_back(Console::Param("fAxisX", Console::FLOAT));
		Params.push_back(Console::Param("fAxisY", Console::FLOAT));
		Params.push_back(Console::Param("fAxisZ", Console::FLOAT));
		Console::AddCommand(this);
	}
	void Execute(string* str) {
		JSApp::Camera.rotate(	(float)atof(str[0].c_str()), (float)atof(str[1].c_str()), 
								(float)atof(str[2].c_str()), (float)atof(str[3].c_str()));
	}
} CmdCamRotate;

class _CmdCamSetPosition: public Console::Command {
public:
	_CmdCamSetPosition() {
		Name="camsetposition";
		Params.push_back(Console::Param("fX", Console::FLOAT));
		Params.push_back(Console::Param("fY", Console::FLOAT));
		Params.push_back(Console::Param("fZ", Console::FLOAT));
		Console::AddCommand(this);
	}
	void Execute(string* str) {
		JSApp::Camera.Position[0] = (float)atof(str[0].c_str());
		JSApp::Camera.Position[1] = (float)atof(str[1].c_str());
		JSApp::Camera.Position[2] = (float)atof(str[2].c_str());
	}
} CmdCamSetPosition;

class _CmdScript: public Console::Command {
public:
	_CmdScript() {
		Name="script";
		Params.push_back(Console::Param("sScript", Console::STRING));
		Console::AddCommand(this);
	}
	void Execute(string* str) {
		Console::ExecuteScript((str[0]+".csf").c_str());
	}
} CmdScript;

class _CmdSetMoveSpeed: public Console::Command {
public:
	_CmdSetMoveSpeed() {
		Name="setmovementspeed";
		Params.push_back(Console::Param("fSpeed", Console::FLOAT));
		Params.push_back(Console::Param("sUnit", Console::STRING));
		Console::AddCommand(this);
	}
	void Execute(string* str) {
		JSApp::SetMoveSpeed( (float)atof(str[0].c_str()), str[1]);
	}
} CmdSetMoveSpeed;

