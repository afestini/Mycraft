#include "player.h"
#include "item.h"

Player player;

bool Player::pickUp(Item& item)
{
	for (int i = 0; i < 36; ++i)
	{
		if (inventory[i].id == item.id)
		{
			if ( item.remove( inventory[i].add(item.count) ) == item.count )
				return true;
		}
	}

	for (int i = 0; i < 36; ++i)
	{
		if (inventory[i].id == 0)
		{
			inventory[i] = item;
			return true;
		}
	}

	return false;
}