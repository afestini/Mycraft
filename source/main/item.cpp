#include "item.h"
#include "block.h"

#include <algorithm>
#include <map>
#include "nbt/nbtaccessor.h"
#include "defines.h"
#include "tools/templatetools.h"

#include <tinyxml2.h>

using namespace std;
using namespace tinyxml2;

map<ItemType, ItemInfo> itemInfo;

map<std::string, ItemType> itemNames = { ITEM_LIST };

map<std::string, ItemCategory> itemCategories = { { "No Category", ITEMCAT_NONE } };

const ItemInfo& ItemInfo::get(ItemType id)
{
	return itemInfo[id];
}

void ItemInfo::init()
{
	XMLDocument xml;
	if (xml.LoadFile("data/xml/items.xml"))
		throw std::runtime_error("Failed to open 'data/xml/items.xml'");

	XMLElement* c = xml.RootElement()->FirstChildElement("itemcategory");
	while (c) 
	{
		int id = 0;
		if ( c->QueryAttribute("id", &id) == XML_SUCCESS)
		{
			string name = c->Attribute("name");
			itemCategories[name] = ItemCategory(id);
		}
		c = c->NextSiblingElement("itemcategory");
	}

	XMLElement* b = xml.RootElement()->FirstChildElement("item");
	while (b) 
	{
		int id = 0;

		if ( b->QueryAttribute("id", &id) == XML_SUCCESS && id >= 256 && id < 32768)
		{
			const ItemType type = ItemType(id);

			string name = b->Attribute("name");
			itemNames[name] = type;

			ItemInfo& info = itemInfo[type];

			const char* category = b->Attribute("category");
			if (category)
				info.category = getItemCategory(category);

			b->QueryAttribute("textureID", &info.texture);
			b->QueryAttribute("durability", &info.durability);
			b->QueryAttribute("harvestLevel", &info.harvestLevel);
			b->QueryAttribute("dmgBlock", &info.damageBlock);
			b->QueryAttribute("dmgEntity", &info.damageEntity);

			info.maxStackSize = info.durability ? 1 : 64;
		}
		b = b->NextSiblingElement("item");
	}
}


Item::Item() 
	: id(AIR), damage(0), count(0) {}


char Item::add(char amount)
{
	char maxStack = (int)ItemInfo::get(this->id).maxStackSize;
	
	amount = clamp<char>( amount, 0, maxStack - count );
	count += amount;
	return amount;
}

char Item::remove(char amount)
{
	amount = min(count, amount);
	count -= amount;

	if (count <= 0)
		*this = Item();

	return amount;
}

void Item::fillFromNode(const NBTAccessor& nbt)
{
	id = (ItemType)nbt["id"].valueShort();
	damage = nbt["Damage"].valueShort();
	count = nbt["Count"].valueByte();
}
