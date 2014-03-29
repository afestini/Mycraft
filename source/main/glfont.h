#ifndef GRFONT_INCLUDED
#define GRFONT_INCLUDED

#include <string>

class Font {
public:
	static bool Initialized;
        static void Init(const char* filename);
	static void Cleanup();
	static float GetWidth(std::string String, float sz);
	static void Print(std::string String, float x, float y, float sz, bool right=0);
	static void PrintArea(std::string String, float x, float y, float w, float h, float sz);
	
private:
	Font() {};
	struct CHAR	{
		float width, height;
		float tx1, ty1, tx2, ty2;
	};

	static unsigned Tex, TexWidth, TexHeight, TexStart, TexEnd;
	static CHAR* Char;
};

#endif
