#include "defines.h"
#include "crafting.h"
#include "block.h"
#include "item.h"

#include <sstream>
#include <algorithm>

#include <tinyxml2.h>

using namespace std;
using namespace tinyxml2;


vector<Blueprint> Blueprint::blueprints;

void Blueprint::init()
{
	XMLDocument xml;
	if (xml.LoadFile("data/xml/blueprints.xml"))
		throw std::runtime_error("Failed to open 'data/xml/blueprints.xml'");

	XMLElement* r = xml.RootElement()->FirstChildElement("blueprint");
	while (r) 
	{
		int w = 0, h = 0;
		int productId = 0;
		int amount = 1;

		r->QueryAttribute("patternWidth", &w);
		r->QueryAttribute("patternHeight", &h);
		if ( r->QueryAttribute("product", &productId) != XML_SUCCESS )
			productId = (short)getItemType( r->Attribute("product") );

		r->QueryAttribute("amount", &amount);

		vector<ItemType> incredients;
		XMLElement* incredient = r->FirstChildElement("incredient");
		while (incredient)
		{
			int type = 0;
			if ( incredient->QueryAttribute("incredient", &type) != XML_SUCCESS)
				type = (short)getItemType( incredient->Attribute("name") );
				
			incredients.push_back( ItemType(type) );
			incredient = incredient->NextSiblingElement("incredient");
		}
		blueprints.push_back( Blueprint( ItemType(productId), amount, w, h, incredients) );

		r = r->NextSiblingElement("blueprint");
	}
}

Item Blueprint::find(const Item* items, size_t craftingArea)
{
	for (const Blueprint& r : blueprints)
	{
		if ( r.match(items, craftingArea) )
			return Item(r.product, (unsigned char)(r.amount) );
	}
	
	return Item(AIR, 0);
}




Blueprint::Blueprint( ItemType product, int amount, int sizeX, int sizeY, const vector<ItemType>& items )
	: sizeX(sizeX), sizeY(sizeY), incredients(items), product(product), amount(amount) {}



bool Blueprint::match(const Item* slots, int areaSize) const
{
	if (sizeX > areaSize && sizeY > areaSize)
		return false;

	const int numSlots = areaSize * areaSize;

	//Skip to first non-empty slot in crafting area
	int slot = 0;
	while (slot < numSlots && slots[slot].id == 0) 
		++slot;

	//Wind back so row is at least as wide as blueprint
	slot -= std::max( 0, (slot % areaSize) - (areaSize - sizeX) );

	for (int y = 0; y < sizeY; ++y)
	{
		//Compare row of blueprint with row of crafting area
		for (int x = 0; x < sizeX; ++x, ++slot)
		{
			if ( slot >= numSlots || slots[slot].id != incredients[y*sizeX + x] )
				return false;
		}

		//Skip slots if blueprint and crafting area differ in size, make sure they are empty
		for (int skip = areaSize - sizeX; skip > 0 && slot < numSlots; --skip, ++slot)
		{
			if (slots[slot].id != 0)
				return false;
		}
	}

	//Remaining slots must be empty
	while (slot < numSlots)
	{
		if (slots[slot++].id != 0)
			return false;
	}
	
	return true;
}


std::map<ItemType, ItemType> Recipe::recipes;

void Recipe::init()
{
	XMLDocument xml;
	if (xml.LoadFile("data/xml/recipes.xml"))
		throw std::runtime_error("Failed to open 'data/xml/recipes.xml'");
	
	XMLElement* r = xml.RootElement()->FirstChildElement("recipe");
	while (r) 
	{
		int in, out;
		if ( r->QueryAttribute("in", &in) != XML_SUCCESS )
		{
			if ( const char* str = r->Attribute("in") )
				in = (short)getItemType(str);
		}
			
		if ( r->QueryAttribute("out", &out) != XML_SUCCESS )
		{
			if ( const char* str = r->Attribute("out") )
				out = (short)getItemType(str);
		}

		if (in && out)
			recipes[ItemType(in)] = ItemType(out);

		r = r->NextSiblingElement("recipe");
	}
}

Item Recipe::find(ItemType in)
{
	map<ItemType, ItemType>::const_iterator it = recipes.find(in);
	return (it != recipes.end()) ? Item(it->second, 1) : Item(AIR, 0);
}
