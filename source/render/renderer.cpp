#include "main/OpenGL.h"
#include <SOIL.h>
#include <vector>

using namespace std;

int loadTextureArray(const char* filename)
{
	GLuint texid = 0;
	int width, height, channels;
	unsigned char* const image = SOIL_load_image(filename, &width, &height, &channels, 0);

	if (image)
	{
		const int newWidth = width / 16;
		const int newHeight = height * 16;
		const int texHeight = height / 16;

		vector<unsigned char> buffer(newWidth*newHeight*channels);

		unsigned char* dst = &buffer[0];
		const unsigned char* src = image;

		for (int texRow = 0; texRow < 16; ++texRow)
		{
			for (int line = 0; line < texHeight; ++line)
			{
				for (int texCol = 0; texCol < 16; ++texCol)
				{
					memcpy(dst + (texCol * newWidth * texHeight * channels), src, newWidth*channels);
					src += newWidth*channels;
				}			
				dst += newWidth * channels;
			}
			dst += (width - newWidth) * texHeight * channels;
		}

		SOIL_free_image_data(image);

		glGenTextures(1, &texid);
		glBindTexture( GL_TEXTURE_2D_ARRAY, texid);

		glTexParameteri( GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri( GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
		glTexParameteri( GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri( GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri( GL_TEXTURE_2D_ARRAY, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, newWidth, texHeight, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, &buffer[0]);
	}

	return texid;
}
