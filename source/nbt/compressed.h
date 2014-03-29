#ifndef NBT_COMPRESSED_INCLUDED
#define NBT_COMPRESSED_INCLUDED

#define ZLIB_WINAPI 
#include "../zlib/zlib.h"

#include <vector>
#include <fstream>
#include <tools/typesbysize.h>

template<typename T>
T reverseByteOrder(T t)
{
	uint8 *a = (uint8*)&t, *b = ((uint8*)&t) + sizeof(t) - 1;
        while (a<b) std::swap(*a++, *b--);
	return t;
}

struct CompressedNBT
{
	CompressedNBT() : needCleanup(false) { memset(&strm, 0, sizeof(strm)); }
	~CompressedNBT() { endDecompression(); }

	void readFile(const std::string& filename);
	void readFile(std::ifstream& file, int size);
	void initDecompression();
	void endDecompression();

	std::vector<unsigned char> buffer;
	z_stream strm;
	bool needCleanup;

	template<typename T>
	T read()
	{
		T value = T();

		strm.avail_out = sizeof(value);
		strm.next_out = (Bytef*)&value;
		inflate(&strm, Z_NO_FLUSH);
		return reverseByteOrder(value);
	}

	void read(void* buffer, size_t size)
	{
		strm.avail_out = size;
		strm.next_out = static_cast<Bytef*>(buffer);
		inflate(&strm, Z_NO_FLUSH);
	}

	operator void*() const { return buffer.empty() ? 0 : (void*)1; }
};

#endif
