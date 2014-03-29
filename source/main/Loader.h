#ifndef LOADER_INCLUDED
#define LOADER_INCLUDED

#include <map>
#include <string>
#include "OpenGL.h"

enum TYPE {LOT_SHADER, LOT_MESH};

struct RessourceObject {unsigned index, type;};

class Loader {
public:
	static GLhandleARB LoadShaderProgram(const char*, const char*, const char* = 0);
	static void Unload(unsigned idx, unsigned type);
	static void UnloadAll();

private:
	static std::map<std::string, RessourceObject> LoadedObjects;
	Loader();
	
};

#endif
