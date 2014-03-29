#ifndef DEFAULT_RENDERER_INCLUDED
#define DEFAULT_RENDERER_INCLUDED

#include "renderer.h"

#include <vector>

#include "main/OpenGL.h"
#include "main/MatrixX.h"

enum ItemType;

class DefaultRenderer : public Renderer
{
public:
	DefaultRenderer() :
		numDrawnChunks(0),
		numChunksInFrustum(0),
		numQueries(0),
		numQueriesPassed(0),
		darkness(0),
		terrainTexture(0), 
		itemTexture(0), 
		fogDensity_(0),
		occlusionCulling_(true),
		floatShader(0),
		floatTextureLocation(0),
		floatVertexLocation(0),
		floatTexCoordLocation(0),
		floatLightLocation(0),
		floatMatrixLocation(0),
		floatChunkLocation(0),
		floatSunlightLocation(0),
		floatPlayerLightLocation(0),
		floatFogLocation(0),
		geoShader(0),
		geoVertexLocation(0),
		geoLightLocation(0),
		geoMatrixLocation(0),
		geoTextureLocation(0),
		geoChunkLocation(0),
		geoSunlightLocation(0),
		geoPlayerLightLocation(0),
		geoEyePosLocation(0),
		occShader(0),
		occMatrixLocation(0)
	{}

	~DefaultRenderer();

	virtual void init();

	virtual bool occlusionCulling() const { return occlusionCulling_; }
	virtual void occlusionCulling(bool on) { occlusionCulling_ = on; }

	virtual float fogDensity() const { return fogDensity_; }
	virtual void fogDensity(float fd) { fogDensity_ = fd; }

	virtual void renderSolidChunk(const Chunk& chunk);
	virtual void renderTransparentChunk(const Chunk& chunk);
	virtual void renderWorld(const World& world);
	virtual void renderBlockIcon(ItemType type, int x, int y, int w = 64, int h = 64);
	virtual void renderBlockCracks(const Block& block, const Chunk& chunk, float x, float y, float z, float percent);
	
	virtual void renderGui();
	virtual void visit(Tooltip& tooltip);
	virtual void visit(Button& btn);
	virtual void visit(QuickSlotBar& window);
	virtual void visit(Window& window);
	virtual void visit(ProgressBar&);
	virtual void visit(HandSlot& slot);
	virtual void visit(Slot& slot);

	size_t numDrawnChunks;
	size_t numChunksInFrustum;
	size_t numQueries;
	size_t numQueriesPassed;

private:
	void cullChunks();
	void getLastFrameOcclusion();
	void renderSolids(const std::vector<const Chunk*>& list);
	void renderTransparents(const std::vector<const Chunk*>& list);
	void renderOcclusionVolumes(const std::vector<const Chunk*>& list);
	void getOcclusionVolumeResults();

	std::vector<const Chunk*> visibleChunks;
	std::vector<const Chunk*> prevOccludedChunks;
	std::vector<const Chunk*> visibleAfterQueryChunks;

	Matrix44 mvp;
	float darkness;
	int terrainTexture;
	int itemTexture;

	float fogDensity_;
	bool occlusionCulling_;

	int floatShader;
	int floatTextureLocation;
	int floatVertexLocation;
	int floatTexCoordLocation;
	int floatLightLocation;
	int floatMatrixLocation;
	int floatChunkLocation;
	int floatSunlightLocation;
	int floatPlayerLightLocation;
	int floatFogLocation;

	int geoShader;
	int geoVertexLocation;
	int geoLightLocation;
	int geoMatrixLocation;
	int geoTextureLocation;
	int geoChunkLocation;
	int geoSunlightLocation;
	int geoPlayerLightLocation;
	int geoEyePosLocation;

	int occShader;
	int occMatrixLocation;
};

#endif