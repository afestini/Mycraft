#include "compressed.h"
#include <fstream>

using namespace std;

void CompressedNBT::readFile(const string& filename)
{
	ifstream file(filename.c_str(), ios::binary);
	if (file)
	{
		file.seekg(0, ios::end);
		size_t size = (size_t)file.tellg();
		file.seekg(0);
		buffer.resize(size);
		file.read((char*)&buffer[0], size);
	}
}

void CompressedNBT::readFile(ifstream& file, int size)
{
	buffer.resize(size);
	file.read((char*)&buffer[0], size);
}

void CompressedNBT::initDecompression()
{
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = buffer.size()-1;
	strm.next_in = (Bytef*)&buffer[0];
	if (buffer[0] == 0x78 && buffer[1] == 0x9C)
		inflateInit(&strm);
	else
		inflateInit2(&strm, 16+MAX_WBITS);
	
	needCleanup = true;
}

void CompressedNBT::endDecompression()
{
        buffer.clear();

	if (needCleanup)
		inflateEnd(&strm);
}
