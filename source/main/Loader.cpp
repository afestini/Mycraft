#include "OpenGL.h"
#include "Loader.h"
#include "Console.h"

using namespace std;

map<string, RessourceObject> Loader::LoadedObjects;

void printLog(GLhandleARB id) 
{
	int infologLength = 0;
        int charsWritten  = 0;
	
	glGetObjectParameterivARB(id, GL_OBJECT_INFO_LOG_LENGTH_ARB, &infologLength);
	if (infologLength) {
                vector<char> infoLog(infologLength + 1);
                glGetInfoLogARB(id, infologLength, &charsWritten, &infoLog[0]);
                printf("%s\n\n", &infoLog[0]);
	}
	else printf("Compile successful\n\n");
}

GLhandleARB loadShader(const char* filename, int type) 
{
	static std::map<std::string, GLhandleARB> loadedShaders;
	if (!filename) return 0;

	if (loadedShaders.find(filename)!=loadedShaders.end()) return loadedShaders[filename];

	FILE* file=fopen(filename, "r");
	if (!file) return loadedShaders[filename]=0;

	fseek(file, 0, SEEK_END);
	GLint size = ftell(file);
	fseek(file, 0, SEEK_SET);

	unsigned ID = glCreateShaderObjectARB(type);
	char* source = new char[size+1];
	size = (GLint)fread(source, 1, size, file);
	fclose(file);

	const GLcharARB* src = source;
	glShaderSourceARB(ID, 1 , &src, &size);
	glCompileShaderARB(ID);
	delete[] source;

	return loadedShaders[filename]=ID;
}

struct tuple {GLhandleARB a,b,c;};
bool operator<(const ::tuple& a, const ::tuple& b) {return (a.a!=b.a) ? (a.a < b.a) : (a.b < b.b);}

GLhandleARB Loader::LoadShaderProgram(const char* vrt, const char* frg, const char* geo) {
	static std::map<::tuple, GLhandleARB> programs;
	
	GLhandleARB vrtID = loadShader(vrt, GL_VERTEX_SHADER);
	GLhandleARB geoID = loadShader(geo, GL_GEOMETRY_SHADER);
	GLhandleARB frgID = loadShader(frg, GL_FRAGMENT_SHADER);

	::tuple pair={vrtID, frgID, geoID};
	std::map<::tuple, GLhandleARB>::iterator it=programs.find(pair);
	if (it!=programs.end()) return it->second;

	GLhandleARB prgID = glCreateProgramObjectARB();

	if (vrtID) glAttachObjectARB(prgID, vrtID);
	if (geoID) glAttachObjectARB(prgID, geoID);
	if (frgID) glAttachObjectARB(prgID, frgID);

	glLinkProgramARB(prgID);
	printf("Log for program\n");
	printLog(prgID);

	if (!vrtID && !frgID) return programs[pair]=0;

	return programs[pair]=prgID;
}

void Loader::UnloadAll() 
{
	map<string, RessourceObject>::iterator it;
	for (it=LoadedObjects.begin(); it!=LoadedObjects.end(); ++it) 
	{
		switch (it->second.type) 
		{
		case LOT_SHADER: glDeleteProgramsARB(1, &it->second.index); continue;
		case LOT_MESH: continue;
		}
	}
}
