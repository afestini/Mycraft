#include "JSFramework.h"
#include "world.h"
#include "ProfileCounter.h"
#include "picking.h"

#include "block.h"
#include "blockinfo.h"
#include "blockrenderinfo.h"

#include "crafting.h"

#include "render/defaultrenderer.h"
#include "tools/templatetools.h"

#include <sstream>
#include <iomanip>
#include <vector>
#include <tbb/task_scheduler_init.h>

#include <filesystem>

#include "player.h"
#include "gui/gui.h"
#include "eventmanager.h"
#include "../physics/charactercontroller.h"

using namespace std;

InventoryWindow inventoryWnd(Gui::windows(), 336, 420, 354, 290);
QuickSlotBar quickSlots(Gui::root(), 330, 716, 364, 44);


const float pickRange = 15.f;

void drawSelectionMarker()
{
	World& world = World::instance();

	PickResult& blockDesc = world.selection;
	if (blockDesc.chunk)
	{
		const Chunk* chunk = blockDesc.chunk;
		const Block* block = blockDesc.block();
		const float x = float(chunk->x * CHUNK_SIZE + blockDesc.x);
		const float y = float(blockDesc.y);
		const float z = float(chunk->z * CHUNK_SIZE + blockDesc.z);

		if (block)
		{
			glColor3f(0,0,0);

			glBegin(GL_LINE_LOOP);

			for (int i=0; i<4; ++i)
			{
				const Vertex& v = blockDesc.face->vertex[i];
				const Vector3 vec = Vector3(x, y, z) + Vector3(v.x, v.y, v.z) + Vector3(v.nx, v.ny, v.nz) * .01f;
				glVertex3fv(&vec.x);
			}
			glEnd();

			glColor3f(1.f, 1.f, 1.f);
		}
	}

	if (Globals::DrawFps)
	{
		JSApp::EnterHUDMode();
		glEnable(GL_TEXTURE_2D);

		const int bx = int(JSApp::Camera.Position[0]) - (JSApp::Camera.Position[0] < 0 ? 1 : 0);
		const int by = int(JSApp::Camera.Position[1]);
		const int bz = int(JSApp::Camera.Position[2]) - (JSApp::Camera.Position[2] < 0 ? 1 : 0);

		if ( Block* block = getBlockAt(bx, by, bz).block() )
		{
			Font::Print(string("Light: ") + to_string((int)block->light), 35.0f, 48.0f, 1.75f);
			Font::Print(string("BlockLight: ") + to_string((int)block->blocklight), 35.0f, 50.0f, 1.75f);
		}

		JSApp::LeaveHUDMode();
	}
}


void drawSelectionInfo()
{
	PickResult& blockDesc = World::instance().selection;
	const Chunk* chunk = blockDesc.chunk;
	const Block* block = blockDesc.block();

	if ( chunk && block )
	{
		const float x = float(chunk->x * CHUNK_SIZE + blockDesc.x);
		const float y = float(blockDesc.y);
		const float z = float(chunk->z * CHUNK_SIZE + blockDesc.z);

		JSApp::EnterHUDMode();
		glEnable(GL_TEXTURE_2D);
		Font::Print(string("Type: ") + getItemName(ItemType(block->type)) + " (" + to_string((int)block->type) + ")", 52.0f, 44.0f, 1.75f);
		Font::Print(string("Data: ") + to_string((int)block->data), 52.0f, 46.0f, 1.75f);
		Font::Print(string("Variant: ") + to_string((int)block->variant), 52.0f, 48.0f, 1.75f);
		Font::Print(string("Light: ") + to_string((int)block->light), 52.0f, 50.0f, 1.75f);
		Font::Print(string("BlockLight: ") + to_string((int)block->blocklight), 52.0f, 52.0f, 1.75f);
		Font::Print(string("Chunk: ") + to_string((int)chunk->x) + "/" + to_string((int)chunk->z), 52.0f, 54.0f, 1.75f);
		Font::Print(string("Chunk pos: ") + to_string((int)blockDesc.x) + "/"+ to_string((int)blockDesc.z), 52.0f, 56.0f, 1.75f);
		Font::Print(string("World pos: ") + to_string((int)x) + "/" + to_string((int)y) + "/" + to_string((int)z), 52.0f, 58.0f, 1.75f);
		JSApp::LeaveHUDMode();
	}
	
	JSApp::EnterHUDMode();
	glEnable(GL_TEXTURE_2D);

	const int bx = int(JSApp::Camera.Position[0]) - (JSApp::Camera.Position[0] < 0 ? 1 : 0);
	const int by = int(JSApp::Camera.Position[1]);
	const int bz = int(JSApp::Camera.Position[2]) - (JSApp::Camera.Position[2] < 0 ? 1 : 0);

	if ( Block* block = getBlockAt(bx, by, bz).block() )
	{
		Font::Print(string("Light: ") + to_string((int)block->light), 35.0f, 48.0f, 1.75f);
		Font::Print(string("BlockLight: ") + to_string((int)block->blocklight), 35.0f, 50.0f, 1.75f);
	}

	JSApp::LeaveHUDMode();
}


class ActiveBlock
{
public:
	ActiveBlock() : block(0), totalDamage(0), totalTime(0) {}

	void drawCracks(Renderer* renderer);
	void update(double time, const ItemInfo& toolInfo, const BlockInfo& blockInfo);

	Block* block;
	float totalDamage;
	double totalTime;
} activeBlock;


void ActiveBlock::update(double time, const ItemInfo& toolInfo, const BlockInfo& blockInfo)
{
	totalTime += time;
	while (totalTime > .05)
	{
		totalTime -= .05;
		const bool needTool = (blockInfo.minHarvestLevel > 0);
		const bool correctTool = (blockInfo.tool == toolInfo.category);

		float damage = needTool ? .3f : 1.0f;
		if (correctTool && toolInfo.harvestLevel >= blockInfo.minHarvestLevel)
			damage = toolInfo.damageBlock;

		totalDamage += damage;
	}
}


void ActiveBlock::drawCracks(Renderer* renderer)
{
	World& world = World::instance();

	PickResult& blockDesc = world.selection;

	if (blockDesc.chunk && block && block->type && BlockInfo::get(block->type).hardness > 0)
	{
		float percent = min(totalDamage/BlockInfo::get(block->type).hardness, 1.0f);
		renderer->renderBlockCracks(*block, *blockDesc.chunk, blockDesc.x, blockDesc.y, blockDesc.z, percent);
	}
}

void drawReticle()
{
	JSApp::EnterHUDMode();
		
	glDisable(GL_TEXTURE_2D);

	glLineWidth(1.5f);

	glBegin(GL_LINES);
	glVertex3f(49.25f, 50, 0); glVertex3f(50.75f, 50, 0);
	glVertex3f(50, 49, 0); glVertex3f(50, 51, 0);
	glEnd();

	JSApp::LeaveHUDMode();
}





void KeyCallback(int key, int state)
{
	if ( !JSApp::CursorFree || !Gui::OnKey(key, state) )
		JSApp::OnKey(key, state);
}

void MouseBtnCallback(int btn, int state)
{
	if ( !JSApp::CursorFree || !Gui::OnMouseBtn(btn, state) )
		JSApp::OnMouseBtn(btn, state);
}

void MouseMoveCallback(int x, int y)
{
	if (JSApp::CursorFree)
		Gui::OnMouseMove(x, y);

	JSApp::OnMouseMove(x, y);
}

void MouseWheelCallback(int pos)
{
	if ( !JSApp::CursorFree || !Gui::OnMouseWheel(pos) )
		JSApp::OnMouseWheel(pos);
}


void updateCrafting()
{
	Item product = Blueprint::find(&player.inventory[80], 2);
	player.inventory[99] = product;
}

EventManager& eventManager() 
{
	static EventManager manager;
	return manager;
}

struct WorldDesc 
{
	WorldDesc(const string& filename, const string& path) 
		: filename(filename), path(path) {}

	string filename, path; 
};

void getWorldFoldersFrom(const string& path, vector<WorldDesc>* worldFolders)
{
	using namespace filesystem;

	if (exists(path))
	{
		for (directory_iterator it(path); it != directory_iterator(); ++it)
		{
			if ( is_directory(it->path()) && exists(it->path().string() + "/region") )
			{
				const string filename = it->path().filename().string();
				worldFolders->push_back( WorldDesc(filename, path + filename) );
			}
		}
	}
}

void getWorldFolders(vector<WorldDesc>* worldFolders)
{
	getWorldFoldersFrom("data/worlds/", worldFolders);

	//No longer compatible with latest MC format
	//const char* AppData = getenv("APPDATA");
	//getWorldFoldersFrom(string(AppData) + "/.minecraft/saves/", worldFolders);
}

void renderWorldFolders(const vector<WorldDesc>& worldFolders)
{
	JSApp::EnterHUDMode();
	glEnable(GL_TEXTURE_2D);
	
	Font::Print("Enter world to load", 40.0f, 16.0f, 2.0f);
	for (size_t i=0; i<worldFolders.size(); ++i)
		Font::Print(to_string(i+1) + ": " + worldFolders[i].filename, 40.0f, 20.0f + 2*i, 2.0f);
	
	JSApp::LeaveHUDMode();
}

string selectWorld()
{
	string worldFile;
	
	vector<WorldDesc> worldFolders;
	getWorldFolders(&worldFolders);

	if (!worldFolders.empty())
	{
		while (worldFile.empty() && !JSApp::Quit)
		{
			renderWorldFolders(worldFolders);

			JSApp::Tick();

			for (size_t k=0; k<9; ++k)
				if ( JSApp::KeyState['1' + k] && k < worldFolders.size() )
					worldFile = worldFolders[k].path;
		}
	}

	return worldFile;
}

#include "../physics/collision.h"
#include "blockrenderinfo.h"

void determinePotentialColliders(const Ellipsoid& e, const Vector3& velocity, vector<Quad>* faces)
{
	Vector3 minExtend = e.center - e.radii;
	Vector3 maxExtend = e.center + e.radii;

	((velocity.x < 0) ? minExtend.x : maxExtend.x) += velocity.x;
	((velocity.y < 0) ? minExtend.y : maxExtend.y) += velocity.y;
	((velocity.z < 0) ? minExtend.z : maxExtend.z) += velocity.z;

	for (float y = maxExtend.y; y >= minExtend.y - 1; --y)
	for (float x = minExtend.x; x <= maxExtend.x + 1; ++x)
	for (float z = minExtend.z; z <= maxExtend.z + 1; ++z)
	{
		PickResult pr = getBlockAt(x, y, z);
		if (pr.block() && BlockRenderInfo::get(pr.block()->type).collisionType != BlockRenderInfo::CollisionType::NONE)
		{
			const Vector3 blockPos = Vector3((float)(pr.chunk->x * CHUNK_SIZE + pr.x),
											 (float)pr.y,
											 (float)(pr.chunk->z * CHUNK_SIZE + pr.z));

			for (const Face& f : BlockRenderInfo::get(pr.block()->type).geometryVariants[pr.block()->variant])
			{
				faces->push_back( Quad( Vector3(&f.vertex[0].x) + blockPos,
										Vector3(&f.vertex[1].x) + blockPos,
										Vector3(&f.vertex[2].x) + blockPos,
										Vector3(&f.vertex[3].x) + blockPos,
										Vector3(&f.vertex[0].nx)));
			}
		}
	}
}


Vector3 handleCollision(Vector3 position, Vector3 velocity, int recursionDepth = 0)
{
	if (velocity.lensq() < .000001f || recursionDepth > 5)
		return position;

	const Ellipsoid ellipsoid( position + Vector3(0, .9f, 0), Vector3(.25f, .9f, .25f) );
	const Vector3 destination = ellipsoid.center + velocity;

	vector<Quad> colliders;
	determinePotentialColliders(ellipsoid, velocity, &colliders);

	float minT = 1.0f;
	Vector3 newVelocity;

	for (const Quad& q : colliders)
	{
		Vector3 tmpNewVelocity;
		const float t = collide( velocity, ellipsoid, q, &tmpNewVelocity);

		if ( t >= .0f && t < minT )
		{
			minT = t;
			newVelocity = tmpNewVelocity;
		}
	}

	if (minT >= 1.0f)
		return position + velocity;

	velocity *= minT;
	position += velocity;

	if ( minT < 1.0f && newVelocity.lensq() > 0 )
		position = handleCollision( position, newVelocity, ++recursionDepth );
	
	return position;
}


int main(int, char *[]) 
{
	tbb::task_scheduler_init schedulerInit;
	Globals::NThreads = schedulerInit.default_num_threads();

	JSApp::Init();

	glfwSetKeyCallback(KeyCallback);
	glfwSetMouseButtonCallback(MouseBtnCallback);
	glfwSetMousePosCallback(MouseMoveCallback);
	glfwSetMouseWheelCallback(MouseWheelCallback);

	const int screenWidth = JSApp::Config["ScreenX"];
	const int screenHeight = JSApp::Config["ScreenY"];
	inventoryWnd.setPosition( (screenWidth - inventoryWnd.w)/2, (screenHeight - inventoryWnd.h)/2 );
	quickSlots.setPosition( (screenWidth - quickSlots.w)/2, screenHeight - quickSlots.h - (screenHeight/50) );

	eventManager().registerHandler(onWindowClose);

	player.position = Vector3(JSApp::Camera.Position) - Vector3(0, 1.7f, 0);

	Renderer* renderer = new DefaultRenderer();
	renderer->init();

	renderer->occlusionCulling( JSApp::Config["OcclusionCulling"] != 0);
	ambientOcclusion = (JSApp::Config["AmbientOcclusion"] != 0);

	string worldFile = selectWorld();
	if (worldFile.empty())
		return 0;

	World& world = World::instance();
	world.init();
	world.load(worldFile);

	quickSlots.setInventory( player.inventory );
	quickSlots.setTexture( MaterialManager::LoadTexture("data/textures/gui.png", false) );

	Gui::windows().visible(false);

	inventoryWnd.visible(false);
	inventoryWnd.setInventory( player.inventory );
	inventoryWnd.setTexture( MaterialManager::LoadTexture("data/textures/inventory.png", false) );

	Slot::handSlot = new HandSlot( Gui::root(), 0, 0, &player.inventory.back() );

	while(!JSApp::Quit)
	{
		Vector3 skyColor = Vector3(.4f, .7f, .9f) * (1.0f - world.darkness());
		glClearColor(skyColor.x, skyColor.y, skyColor.z, 1.0f);

		Vector3 oldCamPosition(JSApp::Camera.Position);

		JSApp::Tick();
		if (JSApp::FrameTime > .02f)
			JSApp::FrameTime = .02f;

		if (Globals::NoClip)
		{
			if (JSApp::Actions["Left"].active)
				JSApp::Camera.move(-JSApp::MoveSpeed * JSApp::FrameTime, 0, 0);

			if (JSApp::Actions["Right"].active)
				JSApp::Camera.move(JSApp::MoveSpeed * JSApp::FrameTime, 0, 0);

			if (JSApp::Actions["Forward"].active)
				JSApp::Camera.move(0, 0, JSApp::MoveSpeed * JSApp::FrameTime);

			if (JSApp::Actions["Backward"].active)
				JSApp::Camera.move(0, 0, -JSApp::MoveSpeed * JSApp::FrameTime);
		}
		else
		{
			player.velocity.x = player.velocity.z = 0;

			//Cheap trick to ignore gravity if already on the ground
			//@TODO only check block beneath
			const Vector3 pos = handleCollision(player.position, Vector3(0, -.01f, 0));
			const bool onGround = abs(player.position.y - pos.y) < 0.000001f;

			Vector3 right(JSApp::Camera.Right[0], 0, JSApp::Camera.Right[2]);
			if (right.lensq() != .0f)
			{
				right *= JSApp::MoveSpeed / right.len();

				if (JSApp::Actions["Left"].active)  player.velocity -= right;
				if (JSApp::Actions["Right"].active) player.velocity += right;
			}
			
			Vector3 fwd(JSApp::Camera.Forward[0], 0, JSApp::Camera.Forward[2]);
			if (fwd.lensq() != .0f)
			{
				fwd *= JSApp::MoveSpeed / fwd.len();

				if (JSApp::Actions["Backward"].active) player.velocity -= fwd;
				if (JSApp::Actions["Forward"].active)  player.velocity += fwd;				
			}

			if (JSApp::Actions["Jump"].active && !JSApp::Actions["Jump"].handled)
			{
				player.velocity.y = 3.5f;
				JSApp::Actions["Jump"].handled = true;
			}

			else if (!onGround)
				player.velocity.y += -9.81f * JSApp::FrameTime;
				

			const float oldYPos = player.position.y;

			player.position = handleCollision(player.position, player.velocity * JSApp::FrameTime);
			player.velocity.y = (player.position.y - oldYPos) / JSApp::FrameTime;

			JSApp::Camera.Position[0] = player.position.x;
			JSApp::Camera.Position[1] = player.position.y + 1.7f;
			JSApp::Camera.Position[2] = player.position.z;
			JSApp::Camera.setView();
		}

		const Chunk* currentChunk = world.getChunkFromPosition(JSApp::Camera.Position[0], JSApp::Camera.Position[2]);
		if (currentChunk)
			world.setPosition(currentChunk->x - MAP_SIZE / 2, currentChunk->z - MAP_SIZE / 2);

		static ProfileCounter updateCounter("Update world", .2f, 5);
		updateCounter.Start();
		world.update(JSApp::FrameTime);
		updateCounter.Stop();

		eventManager().processEvents();

		static ProfileCounter renderCounter("Render world", .2f, 5);
		renderCounter.Start();
		JSApp::numTriangles = 0;
		renderer->renderWorld(world);
		renderCounter.Stop();

		if (JSApp::KeyState['L'])
		{
			JSApp::KeyState['L'] = 0;
			if (world.playerLight == .0f)
				world.playerLight = PLAYER_LIGHT;
			else if (world.playerLight == PLAYER_LIGHT)
				world.playerLight = 1.0f;
			else
				world.playerLight = 0;
		}
		if (JSApp::KeyState['A'])
		{
			JSApp::KeyState['A'] = 0;
			ambientOcclusion = !ambientOcclusion;
			world.rebuildChunks();
		}
		if (JSApp::KeyState['O'])
		{
			JSApp::KeyState['O'] = 0;
			renderer->occlusionCulling( !renderer->occlusionCulling() );
		}
		if (JSApp::KeyState['T'])
		{
			JSApp::KeyState['T'] = 0;
			if (world.timeFactor == .0f)
				world.timeFactor = 800.0f;
			else
				world.timeFactor = .0f;
		}
		if (JSApp::KeyState['I'])
		{
			JSApp::KeyState['I'] = 0;
			inventoryWnd.visible( !inventoryWnd.visible() || !JSApp::CursorFree );

			if ( inventoryWnd.visible() )
				JSApp::FreeCursor(true);
			else
				closeWindow(&inventoryWnd);
		}
		if (JSApp::KeyState['F'])
		{
			JSApp::KeyState['F'] = 0;
			float fog = renderer->fogDensity();
			fog = (fog > 0) ? 0 : .0035f;
			renderer->fogDensity(fog);
		}
		if (JSApp::KeyState['J'])
		{
			JSApp::KeyState['J'] = 0;
			player.velocity.y += .075f;
		}

		if (JSApp::AxisState[2])
		{
			player.selectedSlot += JSApp::AxisState[2];
			while ( player.selectedSlot < 0 ) player.selectedSlot += 9;
			while ( player.selectedSlot >= 9 ) player.selectedSlot -= 9;
			quickSlots.selection = player.selectedSlot;
			
			activeBlock.totalDamage = 0;
		}

		//Add damage to block until >= hardness
		if (JSApp::Actions["LeftClick"].active)
		{
			PickResult& blockDesc = World::instance().selection;
			Block* block = blockDesc.block();

			if (block != activeBlock.block)
			{
				activeBlock.block = block;
				activeBlock.totalDamage = 0;
			}

			if (block && BlockInfo::get(block->type).hardness >= 0)
			{
				Item& tool = player.inventory[player.selectedSlot];
				const ItemInfo& toolInfo = ItemInfo::get(tool.id);
				const BlockInfo& blkInfo = BlockInfo::get(block->type);
				
				activeBlock.update( JSApp::FrameTime, toolInfo, blkInfo );

				if (activeBlock.totalDamage >= blkInfo.hardness)
				{
					const bool needTool = (blkInfo.minHarvestLevel > 0);
					const bool correctTool = (blkInfo.tool == toolInfo.category);
					const bool sufficientHarvestLevel = (toolInfo.harvestLevel >= blkInfo.minHarvestLevel);
					
					if ( !needTool || (correctTool && sufficientHarvestLevel) )
					{
						ItemType id = blkInfo.drop();
						if (id)
						{
							Item dropped(id, 1);
							player.pickUp(dropped);
						}
					}
					
					blockDesc.chunk->removeBlock(blockDesc.x, blockDesc.y, blockDesc.z);
					
					if (tool.id && toolInfo.durability > 0)
					{
						tool.damage += correctTool ? 1 : 2;
						if (tool.damage > toolInfo.durability)
							tool.remove(1);
					}
				}
			}
		}
		else
		{
			activeBlock.totalDamage = 0;
			activeBlock.totalTime = 0;
		}

		if (JSApp::Actions["RightClick"].active)
		{
			JSApp::Actions["RightClick"].handled = true;
			PickResult& blockDesc = World::instance().selection;

			Block* block = blockDesc.block();
			if (block && BlockInfo::get(block->type).onActivate)
			{
				BlockInfo::get(block->type).onActivate(blockDesc);
				blockDesc.chunk->setDirty();
				world.hasDirtyChunks = true;
			}
			else
			{
				PickResult newDesc = getNeighbor(blockDesc, blockDesc.hitDir);
				if (newDesc.chunk)
				{
					Item* item = &player.inventory.back();
					if (!item->id)
						item = &player.inventory[player.selectedSlot];
					
					if ( item->id > 0 && item->id < 256 )
					{
						const BlockInfo& info = BlockInfo::get(item->id);
						if ( !info.onPlace || info.onPlace(newDesc, blockDesc.hitDir) )
						{
							if ( newDesc.chunk->addBlock(newDesc.x, newDesc.y, newDesc.z, item->id) )
								item->remove(1);
						}
					}
				}
			}
		}

		updateCrafting();

		if (activeBlock.totalDamage > 0)
			activeBlock.drawCracks(renderer);
		
		else if (!Gui::elementUnderMouse)
			drawSelectionMarker();

		drawReticle();

		JSApp::Enter2D();

		Gui::windows().visible( JSApp::CursorFree );

		if ( !JSApp::CursorFree )
			Gui::OnMouseMove(JSApp::Config["ScreenX"]/2, JSApp::Config["ScreenY"]/2);

		Gui::update( JSApp::FrameTime );

		renderer->renderGui();
		JSApp::LeaveHUDMode();

		JSApp::EnterHUDMode();
		glEnable(GL_TEXTURE_2D);
		
		if (Globals::DrawFps) 
		{
			drawSelectionInfo();

			unsigned daytime = world.daytime();
			unsigned h = daytime/1000;
			unsigned m = ((daytime%1000)*60/1000)%60;
		
			stringstream str;
			str << setfill('0') << setw(2) <<  h << ":" << setw(2) << m;
			Font::Print(string("Daytime: ") + str.str(), 65.0f, 3.0f, 1.75f);
			Font::Print(string("Skylight: ") + to_string(15 - world.darkness()), 85.0f, 3.0f, 1.75f);

			DefaultRenderer* r = (DefaultRenderer*)renderer;
			Font::Print(string("Occlusion queries: ") + to_string(r->numQueriesPassed) + 
								" / " + to_string(r->numQueries) + "\n", .5f, 2.5f, 2.0f);
			Font::Print(string("Chunks visible: ") + to_string(r->numDrawnChunks) + 
								" / " + to_string(r->numChunksInFrustum) + "\n", .5f, 4.5f, 2.0f);
		}

		
		JSApp::LeaveHUDMode();
/**/
		JSApp::ShowStats();
	}

	JSApp::Shutdown();
	return 0;
}
