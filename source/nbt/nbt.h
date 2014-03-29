#ifndef NBT_INCLUDED
#define NBT_INCLUDED

#include "compressed.h"
#include "nbtaccessor.h"

enum TAG_TYPE { TAG_END = 0, TAG_BYTE, TAG_SHORT, TAG_INT, TAG_LONG, TAG_FLOAT, TAG_DOUBLE, TAG_BYTE_ARRAY, TAG_STRING, TAG_LIST, TAG_COMPOUND, NUM_TAGS };
const char* typeStr(TAG_TYPE);

class Node;

class NBT
{
public:
	NBT() : root_(0) {}
	~NBT() { release(); }

	CompressedNBT compressedData;

	void load(const std::string& filename);
	void readFile(const std::string& filename);
	void readFile(std::ifstream& file, int size);
	void parse();
	void release();
	NBTAccessor operator[](const std::string& key) const;
	NBTAccessor operator[](size_t index) const;

	std::string readString();
	
	TAG_TYPE readTag() { return (TAG_TYPE)compressedData.read<int8>(); }
	int32 readSize() { return compressedData.read<int32>(); }

	float32 readFloat() { return compressedData.read<float32>(); }
	float64 readDouble() { return compressedData.read<float64>(); }
	int64 readLong() { return compressedData.read<int64>(); }
	int32 readInt() { return compressedData.read<int32>(); }
	int16 readShort() { return compressedData.read<int16>(); }
	int8 readByte() { return compressedData.read<int8>(); }
	void readArray(uint8* buffer, size_t size);
	void skipTag(TAG_TYPE tag);

private:
	Node* root_;
};

#endif