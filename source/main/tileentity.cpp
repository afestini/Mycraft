#include "tileentity.h"

#include "defines.h"
#include "nbt/nbt.h"
#include "nbt/nbtnode.h"
#include "item.h"
#include "crafting.h"

#include <algorithm>

using namespace std;

TileEntity* TileEntity::readTileEntity(NBT& nbt)
{
	NodeCompound node(nbt.compressedData);

	const string type = NBTAccessor(&node)["id"].valueString();

	TileEntity* e = 0;

	if ( type == "Furnace" )
		e = new Furnace;

	else if ( type == "Chest" )
		e = new Chest;

	else if ( type == "Sign" )
		e = new Sign;

	else if ( type == "MobSpawner" )
		e = new MonsterSpawner;

	else if ( type == "Music" )
		e = new Music;

	else if ( type == "Trap" )
		e = new Dispenser;

	if (e) e->fillFromNode(NBTAccessor(&node));

	return e;
}


void TileEntity::fillFromNode(const NBTAccessor& nbt)
{
	x = nbt["x"].valueInt();
	y = nbt["y"].valueInt();
	z = nbt["z"].valueInt();
}

	
void Furnace::fillFromNode(const NBTAccessor& nbt)
{
	TileEntity::fillFromNode(nbt);

	burnTime = nbt["BurnTime"].valueShort();
	cookTime = nbt["CookTime"].valueShort();
	
        int numItems = min( 3u, nbt["Items"].size() );
	for (int i = 0; i < numItems; ++i)
	{
		const int8 slot = nbt["Items"][i]["Slot"].valueByte();
		items[slot].fillFromNode(nbt["Items"][i]);
	}
}

void Furnace::update(float dt)
{
	const int fullPart = short(dt * 20);

	short totalBurnTime = 0;

	const Item out = Recipe::find(items[0].id);

	bool canCook = (out.id != AIR && ( items[2].id == AIR || items[2].id == out.id) );

	const bool burnNext = (items[0].count > 0 && items[1].count > 0 && 
						   /*ItemInfo::get(items[1].id).burnValue > 0 &&*/
						   canCook);

	while ( totalBurnTime < fullPart && (burnTime > 0 || burnNext) )
	{
		const short iterTime = short( min<int>(burnTime, fullPart) );
		burnTime -= iterTime;
		totalBurnTime += iterTime;
		
		if (burnTime <= 0 && burnNext)
		{
			burnTime += 300;//ItemInfo::get(items[1].id).burnValue;
			items[1].remove(1);
		}
	}

	cookTime -= totalBurnTime;

	while (canCook && cookTime <= 0)
	{
		if ( items[2].id == AIR )
			items[2] = Item(out.id, 0);

		if	(items[2].id == out.id)
			items[0].remove( items[2].add(1) );

		canCook = (items[0].count > 0);		
		cookTime += 200;
	}

	if (!canCook)
		cookTime = 200;
}


void Workbench::fillFromNode(const NBTAccessor& nbt)
{
	TileEntity::fillFromNode(nbt);

        int numItems = min( 10u, nbt["Items"].size() );
	for (int i = 0; i < numItems; ++i)
	{
		const int8 slot = nbt["Items"][i]["Slot"].valueByte();
		items[slot].fillFromNode(nbt["Items"][i]);
	}
}

void Workbench::update(float)
{
	Item product = Blueprint::find(&items[0], 3);
	items[9] = product;
}


void Chest::fillFromNode(const NBTAccessor& nbt)
{
	TileEntity::fillFromNode(nbt);

        int numItems = min( 27u, nbt["Items"].size() );
	for (int i = 0; i < numItems; ++i)
	{
		const int8 slot = nbt["Items"][i]["Slot"].valueByte();
		items[slot].fillFromNode(nbt["Items"][i]);
	}
}

void Sign::fillFromNode(const NBTAccessor& nbt)
{
	TileEntity::fillFromNode(nbt);
}

void MonsterSpawner::fillFromNode(const NBTAccessor& nbt)
{
	TileEntity::fillFromNode(nbt);
}

void MonsterSpawner::update(float dt)
{
	delay -= short(dt * 20);
}

void Music::fillFromNode(const NBTAccessor& nbt)
{
	TileEntity::fillFromNode(nbt);
}

void Dispenser::fillFromNode(const NBTAccessor& nbt)
{
	TileEntity::fillFromNode(nbt);

	int numItems = nbt["Items"].size();
	for (int i = 0; i < numItems; ++i)
	{
		const int8 slot = nbt["Items"][i]["Slot"].valueByte();
		items[slot].fillFromNode(nbt["Items"][i]);
	}
}
