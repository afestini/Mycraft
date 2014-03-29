#ifndef MATERIAL_INCLUDED
#define MATERIAL_INCLUDED

#include <map>
#include <string>
#include "OpenGL.h"
#include "Globals.h"

class MaterialManager 
{
	static std::map<std::string, unsigned> Textures;
	static std::map<std::string, unsigned> Shaders;

	static float CurrentColor[4];
	static unsigned CurrentBlendMode[2];
	static int CurrentTextureUnit;
	static int CurrentTexture[16];
	static unsigned numUnits;
	static bool ShaderEnabled, BlendingEnabled;

	MaterialManager();
	static int LoadTGA(FILE* file);
	static int LoadDDS(FILE* file);

public:
	static void Init();
	static void Cleanup();
	static int LoadTexture(const char*, bool mips=1, int Wrapmode=GL_CLAMP_TO_EDGE);
	static int LoadShader(const char* vrt, const char* frg);
	static void BindTexture(unsigned unit, unsigned tex);
	
	static void ReleaseTexture(unsigned tex) 
	{
		std::map<std::string, unsigned>::iterator it=Textures.begin();
		while (it!=Textures.end() && it->second!=tex) ++it;
		if (it==Textures.end()) return;
		glDeleteTextures(1, &tex);
		Textures.erase(it);
	}

	static void BlendMode(unsigned src, unsigned dst) 
	{
		if (!BlendingEnabled) {glEnable(GL_BLEND); BlendingEnabled=1;}
		if (src==CurrentBlendMode[0] && dst==CurrentBlendMode[1]) return;
		glBlendFunc(CurrentBlendMode[0]=src, CurrentBlendMode[1]=dst);
	}
	static void SetAlpha(float a) 
	{
		if (CurrentColor[3]==a) return;
		CurrentColor[3]=a;
		glColor4fv(CurrentColor);
	}	
	static void SetColor(float r, float g, float b, float a=-1) 
	{
		bool Changed=0;
		if (CurrentColor[0]!=r) {CurrentColor[0]=r; Changed=1;}
		if (CurrentColor[1]!=g) {CurrentColor[1]=g; Changed=1;}
		if (CurrentColor[2]!=b) {CurrentColor[2]=b; Changed=1;}
		if (a>=0 && CurrentColor[3]!=a) {CurrentColor[3]=a; Changed=1;}
		if (Changed) glColor4fv(CurrentColor);
	}

	static void BindShader(GLhandleARB shader_program) 
	{
		glUseProgramObjectARB(shader_program);
	};
	
	static void DisableBlending() 
	{
		if (BlendingEnabled) {
			glDisable(GL_BLEND);
			BlendingEnabled=0;
		}
	}
	static void DisableTextures() 
	{
			glDisable(GL_TEXTURE_2D);
	};
	static void DisableShader() 
	{
		glUseProgramObjectARB(0);
	}
};

#endif
