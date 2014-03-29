#ifndef TILE_ENTITY_INCLUDED
#define TILE_ENTITY_INCLUDED

#include <vector>
#include <string>

#include "item.h"
#include "nbt/nbt.h"

class Item;
class NBT;
class NBTAccessor;

class TileEntity
{
public:
	static TileEntity* readTileEntity(NBT& nbt);

	virtual void fillFromNode(const NBTAccessor& nbt);
	virtual void update(float) {}

	int x, y, z;
};


class Furnace : public TileEntity
{
public:
        Furnace() : burnTime(0), fullBurnTime(300), cookTime(0), fullCookTime(200), items(3) {}

	virtual void fillFromNode(const NBTAccessor& nbt);
	virtual void update(float dt);

	short burnTime, fullBurnTime, cookTime, fullCookTime;
	std::vector<Item> items;
};

class Workbench : public TileEntity
{
public:
	Workbench() : items(10) {}
	virtual void fillFromNode(const NBTAccessor& nbt);
	virtual void update(float dt);

	std::vector<Item> items;
};

class Sign : public TileEntity
{
public:
	virtual void fillFromNode(const NBTAccessor& nbt);

	std::string line1, line2, line3, line4;
};

class MonsterSpawner : public TileEntity
{
public:
	virtual void fillFromNode(const NBTAccessor& nbt);
	virtual void update(float dt);

	std::string entity;
	short delay;
};

class Chest : public TileEntity
{
public:
	Chest() : items(27) {}
	virtual void fillFromNode(const NBTAccessor& nbt);

	std::vector<Item> items;
};

class Music : public TileEntity
{
public:
	virtual void fillFromNode(const NBTAccessor& nbt);

	unsigned char note;
};

class Dispenser : public TileEntity
{
public:
	virtual void fillFromNode(const NBTAccessor& nbt);

	Item items[9];
};


#endif
