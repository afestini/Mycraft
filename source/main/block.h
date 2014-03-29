#ifndef BLOCK_INCLUDED
#define BLOCK_INCLUDED

#include "picking.h"

class Window;
struct EventWindowClose;
struct PickResult;

struct Block
{
	unsigned char type;
	unsigned char data : 4;
	unsigned char variant : 4;
	unsigned char light : 4;
	unsigned char blocklight : 4;	
};

extern void closeWindow(Window* window);
extern void onWindowClose(const EventWindowClose& evt);

extern void activateDoor(PickResult& block);
extern void activateWorkbench(PickResult& block);
extern void activateChest(PickResult& block);
extern void activateFurnace(PickResult& block);
extern bool placeTorch(PickResult& block, DIRECTION d);

/*
+X south, -X north
+Z west, -Z east 

Direction the painting faces: 0 is east, 1 is north, 2 is west, and 3 is south.

Minecart Tracks

    * 0x0: flat track going east-west
    * 0x1: flat track going north-south
    * 0x2: track ascending to the south
    * 0x3: track ascending to the north
    * 0x4: track ascending to the east
    * 0x5: track ascending to the west 

Make a circle from four rails:

    * 0x6: Northeast corner
    * 0x7: Southeast corner
    * 0x8: Southwest corner
    * 0x9: Northwest corner 

Levers

    * 0x8: If this bit is set, the lever has been thrown and is providing power. 

Wall levers:

    * 0x1: Facing south
    * 0x2: Facing north
    * 0x3: Facing west
    * 0x4: Facing east 

Ground levers:

    * 0x5: Lever points west when off.
    * 0x6: Lever points south when off. (Note that unlike the other types of switch, this version will not power wires around the block it is sitting on.) 

Buttons

    * 0x8 If this bit is set, the button has been pressed. If this bit is set in a saved level, the button will remain pressed for an undefined length of time after the level is loaded. 

Button direction:

    * 0x1: Facing south
    * 0x2: Facing north
    * 0x3: Facing west
    * 0x4: Facing east 

Sign Posts

    * 0x0: West
    * 0x1: West-Northwest
    * 0x2: Northwest
    * 0x3: North-Northwest
    * 0x4: North
    * 0x5: North-Northeast
    * 0x6: Northeast
    * 0x7: East-Northeast
    * 0x8: East
    * 0x9: East-Southeast
    * 0xA: Southeast
    * 0xB: South-Southeast
    * 0xC: South
    * 0xD: South-Southwest
    * 0xE: Southwest
    * 0xF: West-Southwest 

Wall Signs

    * 0x2: Facing east
    * 0x3: Facing west
    * 0x4: Facing north
    * 0x5: Facing south 

Pumpkins and Jack-o-Lanterns

    * 0x0: Facing east
    * 0x1: Facing south
    * 0x2: Facing west
    * 0x3: Facing north 
*/

#endif
