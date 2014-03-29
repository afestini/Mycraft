#ifndef CRAFTING_INCLUDED
#define CRAFTING_INCLUDED

#include <vector>
#include <map>

class Item;
enum ItemType;

class Blueprint 
{
public:
	Blueprint(ItemType product, int amount, int sizeX, int sizeY, const std::vector<ItemType>& items);
	
	bool match(const Item* slots, int areaSize) const;

	int sizeX, sizeY;
	std::vector<ItemType> incredients;
	ItemType product;
	int amount;

	static void init();
	static Item find(const Item* items, size_t craftingArea);
	static std::vector<Blueprint> blueprints;
};

class Recipe 
{
public:
	Recipe(ItemType product, ItemType input);

	static void init();
	static Item find(ItemType in);
	static std::map<ItemType, ItemType> recipes;
};


#endif