#include "nbt.h"

#include "nbtnode.h"

using namespace std;

static const char* typeStrings[NUM_TAGS] = {"End", "Byte", "Short", "Int", "Long", "Float", "Double", "Array", "String", "List", "Compound" };

const char* typeStr(TAG_TYPE tag) { return typeStrings[tag]; }

void NBT::load(const std::string& filename)
{
	readFile(filename);
	parse();
}

void NBT::readFile(const std::string& filename)
{
	compressedData.readFile(filename);
}

void NBT::readFile(ifstream& file, int size)
{
	compressedData.readFile(file, size);
}

void NBT::release() 
{
	delete root_;
	root_ = 0; 
}

void NBT::parse()
{
	delete root_;
	compressedData.initDecompression();
	root_ = new NodeCompound(compressedData);
	compressedData.endDecompression();
}

NBTAccessor NBT::operator[](const std::string& key) const
{ 
	return NBTAccessor(root_->get(""))[key]; 
}

NBTAccessor NBT::operator[](size_t index) const
{ 
	return NBTAccessor(root_->get(""))[index];
}

string NBT::readString()
{
	char tmpNameBuffer[128];
	int16 size = compressedData.read<int16>();
	compressedData.read(tmpNameBuffer, size);
	return string(tmpNameBuffer, size);
}

void NBT::readArray(uint8* buffer, size_t size)
{
	compressedData.read(buffer, size);
}

void NBT::skipTag(TAG_TYPE tag)
{
	int32 length = 0;
	TAG_TYPE type;

	switch (tag)
	{
	case TAG_BYTE: readByte(); break;
	case TAG_SHORT: readShort(); break;
	case TAG_INT: readInt(); break;
	case TAG_LONG: readLong(); break;
	case TAG_FLOAT: readFloat(); break;
	case TAG_DOUBLE: readDouble(); break;
	case TAG_STRING: readString(); break;

	case TAG_BYTE_ARRAY:
		length = readSize();
		uint8 data[0x8000];
		readArray(&data[0], length);
		break;

	case TAG_LIST:
		type = readTag();
		length = readSize();
		while (length--) skipTag(type);		
		break;

	case TAG_COMPOUND:
		while ( (type = readTag()) != TAG_END)
		{
			readString();
			skipTag(type);
		}
		break;
        default: break;
	}
}
