#ifndef NBT_ACCESSOR_INCLUDED
#define NBT_ACCESSOR_INCLUDED

#include <string>
#include <vector>
#include "../tools/typesbysize.h"

class Node;

class NBTAccessor
{
public:
	explicit NBTAccessor(const Node* node) : current(node) {}

	NBTAccessor operator[](const std::string& key) const;
	NBTAccessor operator[](size_t index) const;
	size_t size() const;
	const std::vector<uint8>& data() const;
	
	std::string valueString() const;
	int8 valueByte() const;
	int16 valueShort() const;
	int32 valueInt() const;
	int64 valueLong() const;
	float32 valueFloat() const;
	float64 valueDouble() const;

private:
	const Node* current;
};

#endif
