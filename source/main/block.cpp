#include "block.h"
#include "defines.h"
#include "chunk.h"
#include "picking.h"
#include "world.h"

#include "gui/gui.h"
#include "gui/window.h"

#include "tools/templatetools.h"

using namespace std;


typedef std::map<TileEntity*, Window*> TileEntityWindowTable;
TileEntityWindowTable tileEntityWindows;

void closeWindow(Window* window)
{
	for (auto& pair : tileEntityWindows)
	{
		if ( pair.second == window )
		{
			delete pair.second;
			tileEntityWindows.erase(pair.first);
			break;
		}
	}

	bool openWindows = false;
	for (GuiElement* e : Gui::windows().children)
		if ( e->visible() ) openWindows = true;

	JSApp::FreeCursor( openWindows );
}

void onWindowClose(const EventWindowClose& evt)
{
	closeWindow(evt.window);
}

void placeWindowOverPosition( Window* wnd, float x, float y, float z )
{
	const float coord[] = {x, y, z};
	int s[2] = {0};
	JSApp::Camera.World2Screen(coord, s);

	s[1] = min(JSApp::Config["ScreenY"] - s[1], Gui::mouseY - 20);
	s[0] = clamp( s[0] - wnd->w/2, 0, JSApp::Config["ScreenX"] - wnd->w );
	s[1] = clamp( s[1] - wnd->h, 0, JSApp::Config["ScreenY"] - wnd->h );

	wnd->x = s[0];
	wnd->y = s[1];
}

void activateDoor(PickResult& block)
{
	block.block()->data ^= 0x4;

	PickResult pr;
	if (block.block()->data & 8) // Top half
		pr = getNeighbor(block, DIR_NY);
	else
		pr = getNeighbor(block, DIR_PY);

	if (Block* b = pr.block())
		b->data ^= 0x4;

	block.chunk->setDirty();
	World::instance().hasDirtyChunks = true;
}

void activateWorkbench(PickResult& block)
{
	Workbench* t = dynamic_cast<Workbench*>( block.chunk->getTileEntity(block.x, block.y, block.z) );
	if (!t)
	{
		t = new Workbench;
		t->x = block.chunk->x * CHUNK_SIZE + block.x;
		t->z = block.chunk->z * CHUNK_SIZE + block.z;
		t->y = block.y;

		block.chunk->tileEntities.emplace_back(t);
	}

	TileEntityWindowTable::iterator it = tileEntityWindows.find(t);
	if ( it == tileEntityWindows.end() )
	{
		WorkbenchWindow* wnd = new WorkbenchWindow(Gui::windows(), 350, 200, 356, 156);
		placeWindowOverPosition( wnd, t->x+.5f, t->y+.90f, t->z+.5f );
		wnd->setTexture( MaterialManager::LoadTexture("data/textures/crafting.png", false) );
		wnd->setInventory( t->items );
		tileEntityWindows[t] = wnd;
		JSApp::FreeCursor(true);
	}

	else if ( !JSApp::CursorFree )
		JSApp::FreeCursor(true);

	else
		closeWindow(it->second);
}

void activateChest(PickResult& block)
{
	Chest* t = dynamic_cast<Chest*>( block.chunk->getTileEntity(block.x, block.y, block.z) );
	if (!t)
	{
		t = new Chest;
		t->x = block.chunk->x * CHUNK_SIZE + block.x;
		t->z = block.chunk->z * CHUNK_SIZE + block.z;
		t->y = block.y;

		block.chunk->tileEntities.emplace_back(t);
	}

	TileEntityWindowTable::iterator it = tileEntityWindows.find(t);
	if ( it == tileEntityWindows.end() )
	{
		ChestWindow* wnd = new ChestWindow(Gui::windows(), 350, 200, 354, 158);
		placeWindowOverPosition( wnd, t->x+.5f, t->y+.90f, t->z+.5f );
		wnd->setTexture( MaterialManager::LoadTexture("data/textures/container.png", false) );
		wnd->setInventory( t->items );
		tileEntityWindows[t] = wnd;
		JSApp::FreeCursor(true);
	}

	else if ( !JSApp::CursorFree )
		JSApp::FreeCursor(true);

	else
		closeWindow(it->second);
}

void activateFurnace(PickResult& block)
{
	Furnace* t = dynamic_cast<Furnace*>( block.chunk->getTileEntity(block.x, block.y, block.z) );
	if (!t)
	{
		t = new Furnace;
		t->x = block.chunk->x * CHUNK_SIZE + block.x;
		t->z = block.chunk->z * CHUNK_SIZE + block.z;
		t->y = block.y;

		block.chunk->tileEntities.emplace_back(t);
	}
	
	TileEntityWindowTable::iterator it = tileEntityWindows.find(t);
	if ( it == tileEntityWindows.end() )
	{
		FurnaceWindow* wnd = new FurnaceWindow(Gui::windows(), 350, 200, 352, 154, *t);
		placeWindowOverPosition( wnd, t->x+.5f, t->y+.90f, t->z+.5f );
		wnd->setTexture( MaterialManager::LoadTexture("data/textures/furnace.png", false) );
		tileEntityWindows[t] = wnd;
		JSApp::FreeCursor(true);
	}

	else if ( !JSApp::CursorFree )
		JSApp::FreeCursor(true);

	else
		closeWindow(it->second);
}

bool placeTorch(PickResult& block, DIRECTION d)
{
	bool canPlace = true;
	switch (d)
	{
	case DIR_PX: block.block()->data = 1; break;
	case DIR_NX: block.block()->data = 2; break;
	case DIR_PZ: block.block()->data = 3; break;
	case DIR_NZ: block.block()->data = 4; break;
	case DIR_PY: block.block()->data = 5; break;
	default: canPlace = false;
	}
	return canPlace;
}
