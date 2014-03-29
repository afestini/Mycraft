#include "blockinfo.h"
#include "blockrenderinfo.h"
#include "picking.h"
#include "block.h"
#include "chunk.h"
#include "world.h"

#include <tinyxml2.h>

using namespace tinyxml2;


BlockInfo blockInfo[256];

void BlockInfo::addDrop(ItemType type, int prob)
{
	dropTable.push_back( DropInfo(type, prob) );
}

ItemType BlockInfo::drop() const
{
	int num = rand()%100;

	for (const DropInfo& di : dropTable)
	{
		if (num < di.prob)
			return di.type;

		num -= di.prob;
	}
	return AIR;
}

const BlockInfo& BlockInfo::get(int id) 
{
	return blockInfo[id];
}

void BlockInfo::init()
{
	blockInfo[WOODEN_DOOR_BLOCK].onActivate = activateDoor;
	blockInfo[IRON_DOOR_BLOCK].onActivate = activateDoor;
	blockInfo[WORKBENCH].onActivate = activateWorkbench;
	blockInfo[CHEST].onActivate = activateChest;
	blockInfo[FURNACE_OFF].onActivate = activateFurnace;
	blockInfo[FURNACE_ON].onActivate = activateFurnace;

	blockInfo[TORCH].onPlace = placeTorch;

	XMLDocument xml;
	if (xml.LoadFile("data/xml/blocks.xml"))
		throw std::runtime_error("Failed to open 'data/xml/blocks.xml'");
	
	XMLElement* b = xml.RootElement()->FirstChildElement("block");
	while (b) 
	{
		int id = -1;
		if ( b->QueryAttribute("id", &id) == XML_SUCCESS && id >= 0 && id < 256)
		{
			string name = b->Attribute("name");
			itemNames[name] = ItemType(id);

			b->QueryAttribute("lightReduction", &blockInfo[id].lightReduction);
			b->QueryAttribute("lightEmission", &blockInfo[id].blockLightBrightness);
			b->QueryAttribute("hardness", &blockInfo[id].hardness);
			b->QueryAttribute("minHarvestLevel", &blockInfo[id].minHarvestLevel);

			const char* tool = b->Attribute("tool");
			if (tool)
				blockInfo[id].tool = getItemCategory(tool);

			readBlockRenderInfo(id, b);
		}
		b = b->NextSiblingElement("block");
	}

	//Second pass for drop tables
	b = xml.RootElement()->FirstChildElement("block");
	while (b) 
	{
		int id = -1;

		if ( b->QueryAttribute("id", &id) == XML_SUCCESS && id >= 0 && id < 256)
		{
			XMLElement* child = b->FirstChildElement("drop");
			while (child)
			{
				int dropId = 0;
				int probability = 0;

				if ( child->QueryAttribute("item", &dropId) != XML_SUCCESS )
					dropId = (short)getItemType( child->Attribute("item") );

				child->QueryAttribute("probability", &probability);

				if (dropId)
					blockInfo[id].addDrop( ItemType(dropId), probability );

				child = child->NextSiblingElement("drop");
			}
		}
		b = b->NextSiblingElement("block");
	}
}
