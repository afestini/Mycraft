#ifndef BLOCK_INFO_INCLUDED
#define BLOCK_INFO_INCLUDED

#include <vector>
#include "defines.h"
#include "picking.h"

struct PickResult;

class BlockInfo
{
public:
	typedef void(*HitFunc)(PickResult& block);
	typedef void(*ActivateFunc)(PickResult& block);
	typedef bool(*PlacementFunc)(PickResult& block, DIRECTION d);
	typedef bool(*RemovalFunc)(PickResult& block);
		
	BlockInfo() 
		: onHit(0), onActivate(0), onPlace(0), onRemove(0), 
		  lightReduction(15), blockLightBrightness(0), 
		  tool(ItemCategory(0)), hardness(3), minHarvestLevel(0) {}

	static void init();
	static const BlockInfo& get(int);

	HitFunc onHit;
	ActivateFunc onActivate;
	PlacementFunc onPlace;
	RemovalFunc onRemove;
	ItemType drop() const;

	void addDrop(ItemType type, int prob);

	struct DropInfo 
	{
		DropInfo(ItemType type, int prob) : type(type), prob(prob) {}
		ItemType type; 
		int prob; 
	};

	std::vector<DropInfo> dropTable;

	int lightReduction;
	int blockLightBrightness;
	ItemCategory tool;
	int hardness;
	int minHarvestLevel;
};

#endif
