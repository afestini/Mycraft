#include "glfont.h"
#include "OpenGL.h"
#include "Loader.h"
#include "globals.h"
#include "Material.h"

bool Font::Initialized=0;
unsigned Font::Tex=0, Font::TexWidth=0, Font::TexHeight=0, Font::TexStart=0, Font::TexEnd=0;
Font::CHAR* Font::Char=0;

void Font::Init(const char* filename) {
	SDELETEARY(Char);

	FILE *Input;
	Input = fopen(filename, "rb");
	if (!Input) return;

	fread(&Tex, sizeof(int), 1, Input);
	fread(&TexWidth, sizeof(int), 1, Input);
	fread(&TexHeight, sizeof(int), 1, Input);
	fread(&TexStart, sizeof(int), 1, Input);
	fread(&TexEnd, sizeof(int), 1, Input);
	fread(&Char, sizeof(CHAR*), 1, Input);

	int Num = TexEnd - TexStart + 1;
	Char = new CHAR[Num];
	fread(Char, sizeof(CHAR), Num, Input);

	Num = TexWidth * TexHeight * 2;
	char *TexBytes = new char[Num];
	fread(TexBytes, sizeof(char), Num, Input);

	unsigned ID;
	glGenTextures(1, &ID);
	Tex=ID;
	MaterialManager::BindTexture(0, Tex);
	glTexImage2D(GL_TEXTURE_2D, 0, 2, TexWidth, TexHeight, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, (void *)TexBytes);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_CLAMP);
	
	delete []TexBytes;
	fclose(Input);

	Initialized=1;
}

void Font::Cleanup() {
	SDELETEARY(Char);
	glDeleteTextures(1, &Tex);
}

unsigned char ColStr2Int(char col[2]) {
	unsigned char d1 = 16 * ((col[0]>47 && col[0]<58) ? (col[0]-48) : (col[0]-87));
	return d1 + ((col[1]>47 && col[1]<58) ? (col[1]-48) : (col[1]-87));
}

float Font::GetWidth(std::string String, float sz)
{
	const int Length = String.length();
	const float wFac = sz/Char->height;
	float linelength=0;

	for (int i = 0; i < Length; i++) 
		linelength += Char[(int)String[i] - TexStart].width * wFac;
	return linelength;
}

void Font::Print(std::string String, float x, float y, float sz, bool right) {
	if (!Initialized) return;
	float wFac=sz/Char->height;
	float Tab=sz*2;
	int Length = String.length();
	if (right) {
		float linelength=0;
		for (int i = 0; i < Length; i++) 
			linelength+=Char[(int)String[i] - TexStart].width*wFac;
		x-=linelength;
	}
	float X=x;

	MaterialManager::BindTexture(0, Tex);
	MaterialManager::BlendMode(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBegin(GL_QUADS);
		float DX, DY=sz;
		for (int i = 0; i < Length; i++) {
			if (String[i]=='\n') {x=X; y+=DY; continue;}
			if (String[i]=='\t') {
				x=Tab+(int)(x/Tab)*Tab; continue;}
			if (String[i]=='^') {
				char val[2];
				val[0]=String[++i];
				val[1]=String[++i];
				unsigned char r=ColStr2Int(val);
				val[0]=String[++i];
				val[1]=String[++i];
				unsigned char g=ColStr2Int(val);
				val[0]=String[++i];
				val[1]=String[++i];
				unsigned char b=ColStr2Int(val);
				glColor3ub(r,g,b);					
				continue;
			}

			CHAR *Ch = &Char[(int)String[i] - TexStart];
			DX=Ch->width*wFac;

			glTexCoord2f(Ch->tx1, Ch->ty1);
			glVertex2f(x, y);
			glTexCoord2f(Ch->tx1, Ch->ty2);
			glVertex2f(x, y + DY);
			glTexCoord2f(Ch->tx2, Ch->ty2);
			glVertex2f(x + DX, y + DY);
			glTexCoord2f(Ch->tx2, Ch->ty1);
			glVertex2f(x + DX, y);
			x += DX;
		}
	glEnd();
}

void Font::PrintArea(std::string String, float x, float y, float w, float h, float sz) {
	if (!Initialized) return;
	float X=x;
	float wFac=sz/Char->height;
	int Length = String.length();
	MaterialManager::BindTexture(0, Tex);

	int lastword=0, i=0;
	float width=0;
	while (i < Length) {
		if (String[i]=='\n') {width=0; lastword=i;}
		else {
			if (String[i]==' ') lastword=i;
			CHAR *Ch = &Char[(int)String[i] - TexStart];
			width+=Ch->width*wFac;
			if (width > w && lastword && String[lastword]!='\n') {
				i=lastword;
				String[i]='\n';
				width=0;
			}
		}
		++i;
	}

	float height=sz;
	glBegin(GL_QUADS);
		float DX, DY=sz;
		for (int j = 0; j < Length; j++) {
			if (String[j]=='\n') {x=X; y+=DY; height+=sz; continue;}
			if (height > h) break;
			CHAR *Ch = &Char[(int)String[j] - TexStart];

			DX=Ch->width*wFac;
			glTexCoord2f(Ch->tx1, Ch->ty1);
			glVertex3f(x, y, 0);
			glTexCoord2f(Ch->tx1, Ch->ty2);
			glVertex3f(x, y + DY, 0);
			glTexCoord2f(Ch->tx2, Ch->ty2);
			glVertex3f(x + DX, y + DY, 0);
			glTexCoord2f(Ch->tx2, Ch->ty1);
			glVertex3f(x + DX, y, 0);
			x += DX;
		}
	glEnd();
}
