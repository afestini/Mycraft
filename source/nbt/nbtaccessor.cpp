#include "nbtaccessor.h"
#include "nbtnode.h"


NBTAccessor NBTAccessor::operator[](const std::string& key) const
{
	return NBTAccessor(current ? current->get(key) : 0);
}


NBTAccessor NBTAccessor::operator[](size_t index) const
{
	return NBTAccessor(current ? current->get(index) : 0);
}


size_t NBTAccessor::size() const
{
	return current ? current->size() : 0;
}


const std::vector<uint8>& NBTAccessor::data() const
{
	static const std::vector<uint8> dummy;

	if (const NodeArray* p = dynamic_cast<const NodeArray*>(current))
		return p->data();
	return dummy;
}

std::string NBTAccessor::valueString() const
{
	const NodeString* p = dynamic_cast<const NodeString*>(current);
	return p ? p->text() : "";
}

int8 NBTAccessor::valueByte() const
{
	const NodeValue<int8>* p = dynamic_cast< const NodeValue<int8>* >(current);
	return p ? p->value() : 0;
}

int16 NBTAccessor::valueShort() const
{
	const NodeValue<int16>* p = dynamic_cast< const NodeValue<int16>* >(current);
	return p ? p->value() : 0;
}

int32 NBTAccessor::valueInt() const
{
	const NodeValue<int32>* p = dynamic_cast< const NodeValue<int32>* >(current);
	return p ? p->value() : 0;
}

int64 NBTAccessor::valueLong() const
{
	const NodeValue<int64>* p = dynamic_cast< const NodeValue<int64>* >(current);
	return p ? p->value() : 0;
}

float32 NBTAccessor::valueFloat() const
{
	const NodeValue<float32>* p = dynamic_cast< const NodeValue<float32>* >(current);
	return p ? p->value() : 0;
}

float64 NBTAccessor::valueDouble() const
{
	const NodeValue<float64>* p = dynamic_cast< const NodeValue<float64>* >(current);
	return p ? p->value() : 0;
}
