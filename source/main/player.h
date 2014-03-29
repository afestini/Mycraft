#ifndef PLAYER_INCLUDED
#define PLAYER_INCLUDED

#include <vector>

#include "item.h"
#include "VectorX.h"

class Player
{
public:
	Player() : inventory(128), selectedSlot(0) {}

	bool pickUp(Item& item);
	std::vector<Item> inventory;
	int selectedSlot;
	Vector3 velocity, position;
};

extern Player player;

#endif