#ifndef ITEM_INCLUDED
#define ITEM_INCLUDED

#include "defines.h"

class NBT;
class NBTAccessor;

class ItemInfo
{
public:
	ItemInfo() 
		: texture(255)
		, durability(0)
		, burnValue(0)
		, maxStackSize(64)
		, category(ITEMCAT_NONE)
		, harvestLevel(0)
		, damageBlock(1)
		, damageEntity(0)
{
}

	ItemInfo(int texture, int durability = 0) 
		: texture(texture)
		, durability(durability)
		, burnValue(0)
		, maxStackSize(durability ? 1 : 64)
		, category(ITEMCAT_NONE)
		, harvestLevel(0)
		, damageBlock(1)
		, damageEntity(0)
{
}

	int texture;
	int durability;
	short burnValue;
	char maxStackSize;
	ItemCategory category;
	int harvestLevel;
	float damageBlock, damageEntity;

	static void init();
	static const ItemInfo& get(ItemType);
};

class Item
{
public:
	explicit Item(ItemType id, char count = 1) : id(id), damage(0), count(count) {}

	Item();

	ItemType id;
	short damage;
	char count;

	char add(char amount);
	char remove(char amount);

	void fillFromNode(const NBTAccessor& nbt);
};

#endif
