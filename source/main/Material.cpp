#include "Material.h"
#include "Console.h"
#include <SOIL.h>

using namespace std;

map<string, unsigned> MaterialManager::Textures;
map<string, unsigned> MaterialManager::Shaders;
bool MaterialManager::ShaderEnabled=0;
bool MaterialManager::BlendingEnabled=0;
float MaterialManager::CurrentColor[4]={1,1,1,1};
int MaterialManager::CurrentTexture[16] = {0};
int MaterialManager::CurrentTextureUnit = 0;
unsigned MaterialManager::CurrentBlendMode[2]={0};
unsigned MaterialManager::numUnits=0;

void MaterialManager::Init() 
{
	int num;
	glGetIntegerv(GL_MAX_TEXTURE_UNITS, &num);
	numUnits=num;
	DisableTextures();
	BindShader(0);
	DisableShader();
	SetColor(1,1,1,1);
	BlendMode(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	DisableBlending();
}

void MaterialManager::Cleanup() 
{
	map<string, unsigned>::iterator it;
	for (it=Textures.begin(); it!=Textures.end(); ++it) glDeleteTextures(1, &it->second);
	for (it=Shaders.begin(); it!=Shaders.end(); ++it) glDeleteProgramsARB(1, &it->second);
	Textures.clear();
	Shaders.clear();
	numUnits=0;
}

void MaterialManager::BindTexture(unsigned unit, unsigned tex) 
{
	if (CurrentTexture[unit] != (int)tex)
	{
		if (CurrentTextureUnit != (int)unit)
			glActiveTexture(GL_TEXTURE0+unit);

		glBindTexture(GL_TEXTURE_2D, tex);
		
		if (unit != 0)
			glActiveTexture(GL_TEXTURE0);
	}
};

int MaterialManager::LoadTexture(const char* filename, bool mips, int WrapMode)
{
	map<string, unsigned>::iterator it=Textures.find(filename);
	if (it!=Textures.end()) return it->second;

	unsigned flags = mips ? SOIL_FLAG_MIPMAPS : 0;
	int ID = SOIL_load_OGL_texture( filename, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, flags | SOIL_FLAG_MULTIPLY_ALPHA );

	if (ID)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, WrapMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, WrapMode);
		if (mips) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		else glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
		Textures[filename]=ID;
	}
	else 
		Console::Output((string)"ERROR: Loading Texture "+filename+" failed\n");
	
	return ID;
}