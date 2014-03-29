#include "nbt.h"
#include "nbtnode.h"

#include <fstream>
#include "../tools/typesbysize.h"

using namespace std;


string readString(CompressedNBT& input)
{
	char tmpNameBuffer[256];
	int16 size = input.read<int16>();
	input.read(tmpNameBuffer, size);
	return string(tmpNameBuffer, size);
}



Node* readTag(CompressedNBT& file, TAG_TYPE type)
{
	Node* node = 0;
	switch(type)
	{
	case TAG_BYTE:		node = new NodeValue<int8>(file);	break;
	case TAG_SHORT:		node = new NodeValue<int16>(file);	break;
	case TAG_INT:		node = new NodeValue<int32>(file);	break;
	case TAG_LONG:		node = new NodeValue<int64>(file);	break;
        case TAG_FLOAT:		node = new NodeValue<float32>(file);    break;
        case TAG_DOUBLE:	node = new NodeValue<float64>(file);    break;
	case TAG_BYTE_ARRAY:node = new NodeArray(file);			break;
	case TAG_STRING:	node = new NodeString(file);		break;
        case TAG_LIST:		node = new NodeList(file);		break;
	case TAG_COMPOUND:	node = new NodeCompound(file);		break;
        default: break;
	}
	return node;
}

template<typename T> 
NodeValue<T>::NodeValue(CompressedNBT& input)
{
	value_ = input.read<T>();
}

NodeString::NodeString(CompressedNBT& input) : text_(readString(input)) {}

NodeArray::NodeArray(CompressedNBT& input)
{
	const int32 size = input.read<int32>();
	data_.resize(size);
	input.read(&data_[0], size);
}

NodeList::NodeList(CompressedNBT& input)
{
	TAG_TYPE type = (TAG_TYPE)input.read<int8>();
	int32 size = input.read<int32>();
	entries_.reserve(size);
	
	while (input && size--)
		entries_.push_back(readTag(input, type));
}



NodeList::~NodeList()
{
	for (Node* n : entries_)
		delete n;
}

const Node* NodeList::get(size_t index) const
{ 
	return index < entries_.size() ? entries_[index] : 0;
}



NodeCompound::NodeCompound(CompressedNBT& input)
{
	TAG_TYPE type;
	while ( input && (type = (TAG_TYPE)input.read<int8>()) != TAG_END )
	{
		string name = readString(input);
		elements[name] = readTag(input, type);
	}
}

const Node* NodeCompound::get(const std::string& key) const
{
	ElementTable::const_iterator it = elements.find(key);
	return (it != elements.end()) ? it->second : 0; 
}

NodeCompound::~NodeCompound()
{
	for (ElementTable::value_type p : elements)
		delete p.second;
}
