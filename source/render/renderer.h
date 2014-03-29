#ifndef RENDERER_INCLUDED
#define RENDERER_INCLUDED

#include "gui/guivisitor.h"
#include "main/defines.h"

class World;
class Chunk;
struct Block;
struct PickResult;

int loadTextureArray(const char* filename);

class Renderer : public GuiVisitor
{
public:
	virtual ~Renderer() {}

	virtual void init() = 0;

	virtual void renderWorld(const World& world) = 0;
	virtual void renderBlockIcon(ItemType type, int x, int y, int w = 64, int h = 64) = 0;
	virtual void renderBlockCracks(const Block& block, const Chunk& chunk, float x, float y, float z, float percent) = 0;

	virtual void renderGui() = 0;

	virtual bool occlusionCulling() const = 0;
	virtual void occlusionCulling(bool on) = 0;

	virtual float fogDensity() const = 0;
	virtual void fogDensity(float fd) = 0;
};

#endif
