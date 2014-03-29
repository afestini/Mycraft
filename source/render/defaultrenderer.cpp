#include "defaultrenderer.h"

#include "main/world.h"
#include "main/chunk.h"
#include "main/Loader.h"
#include "main/blockrenderinfo.h"
#include "main/vertex.h"
#include "gui/gui.h"

#include "main/ProfileCounter.h"
#include "main/Material.h"

using namespace std;


#undef TRANSPARENT
#undef OPAQUE

struct OccQuery 
{
	enum QUERY_TYPE { OPAQUE = 0, TRANSPARENT, NONE };

	OccQuery() : visible(1)
	{
		glGenQueries(2, query);
		queryOpen[0] = queryOpen[1] = false;
	}

	GLuint query[2], visible;
	bool queryOpen[2];

	void begin(QUERY_TYPE type) 
	{
		glBeginQuery(GL_SAMPLES_PASSED, query[type]);
		queryOpen[type] = true;
	}

	void end() 
	{
		glEndQuery(GL_SAMPLES_PASSED); 
	}

	GLuint getOcclusionQueryResult()
	{
		if (visible)
			visible = pollOcclusionResult(OPAQUE) || pollOcclusionResult(TRANSPARENT);

		return visible;
	}

	GLuint pollOcclusionResult(QUERY_TYPE type)
	{
		GLuint visible = 1;
		
		if ( queryOpen[type] )
		{
			GLuint available = 0;
			while (!available && !glGetError())
				glGetQueryObjectuiv(query[type], GL_QUERY_RESULT_AVAILABLE, &available);

			glGetQueryObjectuiv(query[type], GL_QUERY_RESULT, &visible);

			queryOpen[type] = false;
		}

		return visible;
	}	
};

static map<const Chunk*, OccQuery> queryData;



DefaultRenderer::~DefaultRenderer()
{
		Loader::UnloadAll();
}

void DefaultRenderer::init()
{
	terrainTexture = loadTextureArray("data/textures/terrain.png");
	itemTexture = MaterialManager::LoadTexture("data/textures/items.png", false);
	
	float lightVals[16];// = {0, .03, .05, .07, .09, .11, .13, .17, .21, .26, .33, .41 ,.51, .64, .8, 1.0};
	for (int i=0; i<16; ++i) lightVals[i] = (float)pow(.8f, 15-i);


	floatShader = Loader::LoadShaderProgram("data/shaders/shader-vertex.vs", "data/shaders/shader-fragment.fs");
	MaterialManager::BindShader(floatShader);

	glUniform1fv( glGetUniformLocation(floatShader, "blocklightVals"), 16, lightVals );

	floatVertexLocation = glGetAttribLocation(floatShader, "vertex");
	floatTexCoordLocation = glGetAttribLocation(floatShader, "texcoord");
	floatLightLocation = glGetAttribLocation(floatShader, "lights");
	floatTextureLocation = glGetUniformLocation(floatShader, "texture");
	floatMatrixLocation = glGetUniformLocation(floatShader, "mvp");
	floatChunkLocation = glGetUniformLocation(floatShader, "chunkPos");
	floatSunlightLocation = glGetUniformLocation(floatShader, "sunlightVals");
	floatPlayerLightLocation = glGetUniformLocation(floatShader, "playerLight");
	floatFogLocation = glGetUniformLocation(floatShader, "fogDensity");
	

	Vector4 faceCoords[6*4] =
	{
		Vector4(0,1,0,0), Vector4(0,0,0,0), Vector4(0,1,1,0), Vector4(0,0,1,0),
		Vector4(1,1,1,0), Vector4(1,0,1,0), Vector4(1,1,0,0), Vector4(1,0,0,0),
		Vector4(0,1,1,0), Vector4(0,0,1,0), Vector4(1,1,1,0), Vector4(1,0,1,0),
		Vector4(1,1,0,0), Vector4(1,0,0,0), Vector4(0,1,0,0), Vector4(0,0,0,0),
		Vector4(0,1,0,0), Vector4(0,1,1,0), Vector4(1,1,0,0), Vector4(1,1,1,0),
		Vector4(1,0,0,0), Vector4(1,0,1,0), Vector4(0,0,0,0), Vector4(0,0,1,0),
	};

	geoShader = Loader::LoadShaderProgram("data/shaders/shader-geo.vs", "data/shaders/shader-geo.fs", "data/shaders/shader-geo.gs");
	MaterialManager::BindShader(geoShader);

	glUniform1fv( glGetUniformLocation(geoShader, "blocklightVals"), 16, lightVals );
	glUniformMatrix4fv( glGetUniformLocation(geoShader, "faceCoords"), 6, false, &faceCoords[0].x );

	geoVertexLocation = glGetAttribLocation(geoShader, "vertex");
	geoLightLocation = glGetAttribLocation(geoShader, "light");	
	geoMatrixLocation = glGetUniformLocation(geoShader, "mvp");
	geoTextureLocation = glGetUniformLocation(geoShader, "texture");
	geoChunkLocation = glGetUniformLocation(geoShader, "chunkPos");
	geoSunlightLocation = glGetUniformLocation(geoShader, "sunlightVals");
	geoPlayerLightLocation = glGetUniformLocation(geoShader, "playerLight");
	geoEyePosLocation = glGetUniformLocation(geoShader, "eyePos");
	
	occShader = Loader::LoadShaderProgram("data/shaders/shader-occ.vs", "data/shaders/shader-occ.fs", "data/shaders/shader-occ.gs");
	MaterialManager::BindShader(occShader);
	occMatrixLocation = glGetUniformLocation(occShader, "mvp");

	MaterialManager::DisableShader();
	glAlphaFunc(GL_GREATER, .5);
}

struct Distance	{
        bool operator()(const Chunk* a, const Chunk* b)	const {
                const Vector2 pos(JSApp::Camera.Position[0], JSApp::Camera.Position[2]);
                const Vector2 v1(a->x * 16.0f + CHUNK_SIZE/2, a->z * 16.0f + CHUNK_SIZE/2);
                const Vector2 v2(b->x * 16.0f + CHUNK_SIZE/2, b->z * 16.0f + CHUNK_SIZE/2);
                return (v1 - pos).lensq() < (v2 - pos).lensq();
        }
};

void DefaultRenderer::renderWorld(const World& world) 
{
	if (Globals::ExtCam) 
	{
		CameraGL tmp(JSApp::Camera);
		tmp.reset();
		tmp.move(0,500,0);
		tmp.rotate(90, 1,0,0);
		tmp.setView();
		mvp = Matrix44(tmp.modelviewprojection());
	}
	else
		mvp = Matrix44(JSApp::Camera.modelviewprojection());

	darkness = 1.0f - pow(.8f, world.darkness());

	cullChunks();

	numQueries = 0;
	numQueriesPassed = 0;
	numDrawnChunks = 0;
	numChunksInFrustum = visibleChunks.size();

	sort(visibleChunks.begin(), visibleChunks.end(), Distance());

	glDisable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);

	//Always has to be done before rendering solids (update visible info or set 1)
	getLastFrameOcclusion();

	renderSolids(visibleChunks);

	if (!prevOccludedChunks.empty())
	{
		static ProfileCounter occCullCounter("Occlusion culling", .2f, 5);
		occCullCounter.Start();

		renderOcclusionVolumes(prevOccludedChunks);

		getOcclusionVolumeResults();

		occCullCounter.Stop();

		renderSolids(visibleAfterQueryChunks);
	}

	renderTransparents(visibleChunks);

	MaterialManager::DisableShader();
	Buffer::Unbind(VERTEX_BUFFER);

	glDisable(GL_ALPHA_TEST);
}

void DefaultRenderer::cullChunks()
{
	static ProfileCounter cullCounter("Frustum culling", .2f, 5);
	cullCounter.Start();

	visibleChunks.clear();

	for (const Chunk& chunk : World::instance().chunks)
	{
		if ( chunk.loaded )
		{
			float bb[6] = { 
				float(chunk.x*CHUNK_SIZE), 
				float(chunk.lowestVisBlock), 
				float(chunk.z*CHUNK_SIZE), 
				float((chunk.x+1) * CHUNK_SIZE), 
				float(chunk.highestVisBlock), 
				float((chunk.z+1) * CHUNK_SIZE)
			};

			if (!Globals::FrustumCulling || JSApp::Camera.AABBVisible(bb))
				visibleChunks.push_back(&chunk);
		}
	}

	cullCounter.Stop();
}

void DefaultRenderer::getLastFrameOcclusion()
{	
	prevOccludedChunks.clear();

	vector<const Chunk*>::const_iterator it = visibleChunks.begin();

	if ( !visibleChunks.empty() )
		queryData[*it++].visible = 1;

	for (it; it != visibleChunks.end(); ++it)
	{
		OccQuery& query = queryData[*it];
		query.visible = occlusionCulling_ ? query.getOcclusionQueryResult() : 1;

		if (!query.visible)
				prevOccludedChunks.push_back(*it);
	}
}

void DefaultRenderer::renderOcclusionVolumes(const vector<const Chunk*>& list)
{
	MaterialManager::BindShader(occShader);
	glUniformMatrix4fv(occMatrixLocation, 1, false, mvp);

	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_FALSE);
	glDisable(GL_CULL_FACE);

	numQueries = list.size();

	for (const Chunk* chunk : list)
	{
		OccQuery& query = queryData[chunk];
	
		query.begin(OccQuery::OPAQUE);
		glBegin(GL_POINTS);
		glVertexAttribI4iEXT(0, chunk->x*16, chunk->lowestVisBlock, chunk->z*16, chunk->highestVisBlock - chunk->lowestVisBlock);
		glEnd();
		query.end();
	}

	glEnable(GL_CULL_FACE);
	glDepthMask(GL_TRUE);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

void DefaultRenderer::getOcclusionVolumeResults()
{
	visibleAfterQueryChunks.clear();

	for (const Chunk* chunk : prevOccludedChunks)
	{
		OccQuery& query = queryData[chunk];
		query.visible = query.pollOcclusionResult(OccQuery::OPAQUE);

		if (query.visible)
			visibleAfterQueryChunks.push_back(chunk);
	}

	numQueriesPassed = visibleAfterQueryChunks.size();
}

void DefaultRenderer::renderSolids(const vector<const Chunk*>& list)
{
	float sunlightVals[16];
	for (int i=0; i<16; ++i) sunlightVals[i] = max(.0f, (float)pow(.8f, 15-i) - darkness);

	MaterialManager::BindShader(geoShader);

	glUniformMatrix4fv(geoMatrixLocation, 1, false, mvp);
	glUniform1i(geoTextureLocation, 0);
	glUniform1fv(geoSunlightLocation, 16, sunlightVals);
	glUniform1f(geoPlayerLightLocation, World::instance().playerLight);
	glUniform4fv(geoEyePosLocation, 1, JSApp::Camera.Position);

	glBindTexture(GL_TEXTURE_2D_ARRAY, terrainTexture);

	glEnableVertexAttribArray(geoVertexLocation);
	glEnableVertexAttribArray(geoLightLocation);

	for (const Chunk* chunk : list)
	{
		OccQuery& query = queryData[chunk];
		if (query.visible)
		{
			if (occlusionCulling_) query.begin(OccQuery::OPAQUE);
			renderSolidChunk(*chunk);
			if (occlusionCulling_) query.end();
			JSApp::numTriangles += chunk->numVertices/4;
			++numDrawnChunks;
		}
	}

	glDisableVertexAttribArray(geoVertexLocation);
	glDisableVertexAttribArray(geoLightLocation);
}

void DefaultRenderer::renderTransparents(const vector<const Chunk*>& list)
{
	float sunlightVals[16];
	for (int i=0; i<16; ++i) sunlightVals[i] = max(.0f, (float)pow(.8f, 15-i) - darkness);

	MaterialManager::BindShader(floatShader);
	glUniform1i(floatTextureLocation, 0);
	glUniformMatrix4fv(floatMatrixLocation, 1, false, mvp);
	glUniform1fv(floatSunlightLocation, 16, sunlightVals);
	glUniform1f(floatPlayerLightLocation, World::instance().playerLight);
	glUniform1f(floatFogLocation, fogDensity_*fogDensity_);

	glEnableVertexAttribArray(floatVertexLocation);
	glEnableVertexAttribArray(floatTexCoordLocation);
	glEnableVertexAttribArray(floatLightLocation);

	glEnable(GL_BLEND);

	for (const Chunk* chunk : list)
	{
		OccQuery& query = queryData[chunk];
		if (query.visible)
		{
			if (occlusionCulling_) query.begin(OccQuery::TRANSPARENT);
			renderTransparentChunk(*chunk);
			if (occlusionCulling_) query.end();
			JSApp::numTriangles += chunk->numFloatVertices/2;
		}
	}

	glDisableVertexAttribArray(floatVertexLocation);
	glDisableVertexAttribArray(floatTexCoordLocation);
	glDisableVertexAttribArray(floatLightLocation);
}

void DefaultRenderer::renderSolidChunk(const Chunk& chunk)
{
	chunk.vertexBuffer.Bind();

	glVertexAttribIPointerEXT(geoVertexLocation, 4, GL_UNSIGNED_BYTE, sizeof(CompVertex), (void*)offsetof(CompVertex, xz));
	glVertexAttribIPointerEXT(geoLightLocation, 4, GL_UNSIGNED_BYTE, sizeof(CompVertex), (void*)offsetof(CompVertex, light));
	
	glUniform3i(geoChunkLocation, chunk.x*CHUNK_SIZE, 0, chunk.z*CHUNK_SIZE);
	glDrawArrays(GL_POINTS, 0, chunk.numVertices);
}

void DefaultRenderer::renderTransparentChunk(const Chunk& chunk)
{
	chunk.vertexTransBuffer.Bind();

	glVertexAttribPointer(floatVertexLocation, 3, GL_SHORT, GL_FALSE, sizeof(FloatVertex), (void*)offsetof(FloatVertex, x));
	glVertexAttribPointer(floatTexCoordLocation, 3, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(FloatVertex), (void*)offsetof(FloatVertex, u));
	glVertexAttribPointer(floatLightLocation, 1, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(FloatVertex), (void*)offsetof(FloatVertex, lights));

	glUniform3i(floatChunkLocation, chunk.x*CHUNK_SIZE, 0, chunk.z*CHUNK_SIZE);
	glDrawArrays(GL_QUADS, 0, chunk.numFloatVertices);
}

void DefaultRenderer::renderBlockIcon(ItemType type, int x, int y, int w, int h)
{
	const BlockRenderInfo& info = BlockRenderInfo::get(type);

	glDisable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);

	glViewport(x, JSApp::Config["ScreenY"] - y - h, w, h);

	Matrix44 mvp = Matrix44(-1.5f,0,0,0, 0,1.5f,0,0, 0,0,-1,0, 0,0,0,1);
	if (!info.spriteIcon)
	{
		mvp = Matrix44(-1.25f,0,0,0, 0,1.25f,0,0, 0,0,-1,0, 0,0,0,1);
		mvp.RotateX(-30*3.1415f/180);
		mvp.RotateY(45*3.1415f/180);
	}
	mvp *= Matrix44::Translation(-.5f, -.5f, -.5f);

	MaterialManager::BindShader(floatShader);
	glBindTexture(GL_TEXTURE_2D_ARRAY, terrainTexture);
	
	glUniformMatrix4fv(floatMatrixLocation, 1, false, mvp);
	glUniform1i(floatTextureLocation, 0);
	glUniform3i(floatChunkLocation, 0,0,0);
	glUniform1f(floatPlayerLightLocation, 0);

	glEnableVertexAttribArray(floatVertexLocation);
	glEnableVertexAttribArray(floatTexCoordLocation);
	glEnableVertexAttribArray(floatLightLocation);

	vector<FloatVertex> vertices;

	const int var = BlockRenderInfo::get(type).itemVariant;
	vector<Face>::const_iterator it = info.geometryVariants[var].begin();
	
	for (int light = 0; it != info.geometryVariants[var].end(); ++it, ++light)
	{
		const int brightness =  min(15, 13+(light/2));
		for (int i=0; i<4; ++i)
			vertices.push_back(FloatVertex( it->vertex[i].x, it->vertex[i].y, it->vertex[i].z,
											it->vertex[i].u, it->vertex[i].v, it->vertex[i].tz, 
											brightness, 0) );
	}
	glVertexAttribPointer(floatVertexLocation, 3, GL_SHORT, GL_FALSE, sizeof(FloatVertex), &vertices[0].x);
	glVertexAttribPointer(floatTexCoordLocation, 3, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(FloatVertex), &vertices[0].u);
	glVertexAttribPointer(floatLightLocation, 1, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(FloatVertex), &vertices[0].lights);

	glDrawArrays(GL_QUADS, 0, info.geometryVariants[var].size()*4);
	
	glDisableVertexAttribArray(floatVertexLocation);
	glDisableVertexAttribArray(floatTexCoordLocation);
	glDisableVertexAttribArray(floatLightLocation);

	glDisable(GL_ALPHA_TEST);
	glEnable(GL_BLEND);
	MaterialManager::DisableShader();

	glViewport(0,0,JSApp::Config["ScreenX"],JSApp::Config["ScreenY"]);
}

void DefaultRenderer::renderBlockCracks(const Block& block, const Chunk& chunk, float x, float y, float z, float percent)
{
	const BlockRenderInfo& info = BlockRenderInfo::get(block.type);

	MaterialManager::BindShader(floatShader);
	glBindTexture(GL_TEXTURE_2D_ARRAY, terrainTexture);

	glPushAttrib(GL_COLOR_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ZERO, GL_SRC_COLOR);

	Matrix44 mvp;
	memcpy(mvp, JSApp::Camera.modelviewprojection(), sizeof(mvp));
	
	glUniformMatrix4fv(floatMatrixLocation, 1, false, mvp);
	glUniform1i(floatTextureLocation, 0);
	glUniform3i(floatChunkLocation, chunk.x * CHUNK_SIZE, 0, chunk.z * CHUNK_SIZE);
	glUniform1f(floatPlayerLightLocation, 0);
	glUniform1f(floatFogLocation, fogDensity_*fogDensity_);

	glEnableVertexAttribArray(floatVertexLocation);
	glEnableVertexAttribArray(floatTexCoordLocation);
	glEnableVertexAttribArray(floatLightLocation);

	vector<FloatVertex> vertices;

	const float texture = 240 + (percent*10);
	vector<Face>::const_iterator it = info.geometryVariants[block.variant].begin();

	for (; it != info.geometryVariants[block.variant].end(); ++it)
	{
		for (int i=0; i<4; ++i)
		{
			Vector3 p = Vector3(it->vertex[i].x + x, it->vertex[i].y + y, it->vertex[i].z + z);
			p += Vector3(it->vertex[i].nx, it->vertex[i].ny, it->vertex[i].nz) * .01f;
			const float u = it->vertex[i].u;
			const float v = it->vertex[i].v;
			vertices.push_back(FloatVertex( p.x, p.y, p.z, u, v, texture, 15, 15) );
		}
	}

	glVertexAttribPointer(floatVertexLocation, 3, GL_SHORT, GL_FALSE, sizeof(FloatVertex), &vertices[0].x);
	glVertexAttribPointer(floatTexCoordLocation, 3, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(FloatVertex), &vertices[0].u);
	glVertexAttribPointer(floatLightLocation, 1, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(FloatVertex), &vertices[0].lights);

	glDrawArrays(GL_QUADS, 0, info.geometryVariants[block.variant].size()*4);
	
	glDisableVertexAttribArray(floatVertexLocation);
	glDisableVertexAttribArray(floatTexCoordLocation);
	glDisableVertexAttribArray(floatLightLocation);

	glDisable(GL_BLEND);
	glPopAttrib();

	MaterialManager::DisableShader();
}


void traverseGui(GuiElement& node, GuiVisitor& v)
{
	if ( node.visible() )
	{
		glPushMatrix();
		glTranslatef(float(node.x), float(node.y), 0);

		node.invite(v);

		list<GuiElement*>::reverse_iterator it = node.children.rbegin();
		for (it; it != node.children.rend(); ++it) 
			traverseGui(**it, v);

		glPopMatrix();
	}
}

void DefaultRenderer::renderGui()
{
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	
	traverseGui(Gui::root(), *this);

	glEnable(GL_DEPTH_TEST);
}


void DefaultRenderer::visit(Tooltip& tooltip)
{
	int mx = Gui::mouseX, my = Gui::mouseY;

	tooltip.x = mx + 12;
	tooltip.y = my - 12;
	tooltip.w = int(.5f + Font::GetWidth(tooltip.text(), 15.f)) + 10;

	glDisable(GL_TEXTURE_2D);
	glColor4f(.1f,.1f,.1f,.8f);
		
	glPushMatrix();
	glLoadIdentity();

	glBegin(GL_QUADS);
	glVertex2i(tooltip.x, tooltip.y-tooltip.h);
	glVertex2i(tooltip.x, tooltip.y);
	glVertex2i(tooltip.x+tooltip.w, tooltip.y);
	glVertex2i(tooltip.x+tooltip.w, tooltip.y-tooltip.h);
	glEnd();

	glColor3f(1, 1, 1);
	glEnable(GL_TEXTURE_2D);
	
	Font::Print(tooltip.text(), float(tooltip.x+5), float(tooltip.y-20), 15.f);

	glPopMatrix();
}

void DefaultRenderer::visit(Button& btn)
{
	if (btn.highlighted)
	{
		glDisable(GL_TEXTURE_2D);	
		glColor4f(1,1,1,.1f);

		glBegin(GL_QUADS);
		glTexCoord2f(.0f,.0f); glVertex3i(0, 0, 0);
		glTexCoord2f(.0f,1.f); glVertex3i(0, btn.h, 0);	
		glTexCoord2f(1.f,1.f); glVertex3i(btn.w, btn.h, 0);
		glTexCoord2f(1.f,.0f); glVertex3i(btn.w, 0, 0);
		glEnd();

		glColor4f(1,1,1,1);
		glEnable(GL_TEXTURE_2D);
	}
}

void DefaultRenderer::visit(Window& window)
{
	glBindTexture(GL_TEXTURE_2D, window.texture);

	const float u1 = window.w/512.f;
	const float v1 = window.h/512.f;

	glBegin(GL_QUADS);
	glTexCoord2f(.0f,.0f); glVertex3i(0, 0, 0);
	glTexCoord2f(.0f,v1); glVertex3i(0, window.h, 0);	
	glTexCoord2f(u1,v1); glVertex3i(window.w, window.h, 0);
	glTexCoord2f(u1,.0f); glVertex3i(window.w, 0, 0);
	glEnd();
}

void DefaultRenderer::visit(ProgressBar& pb)
{
	glBindTexture(GL_TEXTURE_2D, pb.texture);

	const int w = pb.vertical ? pb.w : int( pb.percent*pb.w/100.f );
	const int h = pb.vertical ? int( pb.percent*pb.h/100.f ) : pb.h;
	const int t = pb.h - h;

	const float u0 = pb.u0;
	const float v1 = pb.v1;
	const float u1 = u0 + w/512.f;
	const float v0 = v1 - h/512.f;
	

	glBegin(GL_QUADS);
	glTexCoord2f(u0,v0); glVertex3i(0, t, 0);
	glTexCoord2f(u0,v1); glVertex3i(0, pb.h, 0);	
	glTexCoord2f(u1,v1); glVertex3i(w, pb.h, 0);
	glTexCoord2f(u1,v0); glVertex3i(w, t, 0);
	glEnd();

}

void DefaultRenderer::visit(QuickSlotBar& window)
{
	glBindTexture(GL_TEXTURE_2D, window.texture);

	float u0 = 1/256.f, u1 = 181/256.f;
	float v0 = 1/256.f, v1 = 21/256.f;

	glBegin(GL_QUADS);
	glTexCoord2f(u0,v0); glVertex3i(1, 1, 0);
	glTexCoord2f(u0,v1); glVertex3i(1, window.h-1, 0);	
	glTexCoord2f(u1,v1); glVertex3i(window.w-1, window.h-1, 0);
	glTexCoord2f(u1,v0); glVertex3i(window.w-1, 1, 0);

	u0 = 1/256.f;
	u1 = u0 + 22/256.f;
	v0 = 23/256.f;
	v1 = v0 + 22/256.f;

	const int x = window.selection*40;

	glTexCoord2f(u0,v0); glVertex3i(x, 0, 0);
	glTexCoord2f(u0,v1); glVertex3i(x, window.h, 0);	
	glTexCoord2f(u1,v1); glVertex3i(x+44, window.h, 0);
	glTexCoord2f(u1,v0); glVertex3i(x+44, 0, 0);

	glEnd();
}


void DefaultRenderer::visit(HandSlot& slot)
{
	slot.x = Gui::mouseX - (slot.parent->x + slot.w/2);
	slot.y = Gui::mouseY - (slot.parent->y + slot.h/2);

	Slot tmp(*slot.parent, slot.x, slot.y, slot.item);
	visit(tmp);
}

void DefaultRenderer::visit(Slot& slot)
{
	const int x = slot.x;
	const int y = slot.y;

	if (slot.item->id == 0 || slot.item->id >= 256)
	{
		glBindTexture(GL_TEXTURE_2D, itemTexture);

		const int t = slot.item->id ? ItemInfo::get(slot.item->id).texture : slot.background;
		const float u0 = t%16 * 1.0f/16, u1 = u0 + 1.0f/16;
		const float v0 = t/16 * 1.0f/16, v1 = v0 + 1.0f/16;

		glBegin(GL_QUADS);
	
		glTexCoord2f(u0,v0);	glVertex3i(0, 0, 0);
		glTexCoord2f(u0,v1);	glVertex3i(0, slot.h, 0);
		glTexCoord2f(u1,v1);	glVertex3i(slot.w, slot.h, 0);
		glTexCoord2f(u1,v0);	glVertex3i(slot.w, 0, 0);

		glEnd();
	}
	else
	{
		int abs_x = x, abs_y = y;
		GuiElement *p = slot.parent;
		while(p)
		{
			abs_x += p->x;
			abs_y += p->y;
			p = p->parent;
		}
		renderBlockIcon((ItemType)slot.item->id, abs_x, abs_y, 32, 32);
	}

	
	glDisable(GL_TEXTURE_2D);
	
	if ( slot.item->damage )
	{
		int l = 3;
		int r = l + slot.w - 6;
		int b = slot.h - 1;
		int t = b - 5;

		const int durability = ItemInfo::get(slot.item->id).durability;
		const float condition = 1.0f - float(slot.item->damage) / durability;
		const int barWidth = int((slot.w - 6) * condition);

		glBegin(GL_QUADS);

		glColor3f(.1f,.1f,.1f);

		glVertex2i(l, t);
		glVertex2i(l, b);
		glVertex2i(r, b);
		glVertex2i(r, t);

		++l; ++t; --b;
		r = l + barWidth;

		const float red = min(.9f, (1.f-condition) * 1.8f);
		const float green = min(.9f, condition * 1.8f);

		glColor3f(red, green, 0);

		glVertex2i(l, t);
		glVertex2i(l, b);
		glVertex2i(r, b);
		glVertex2i(r, t);

		glColor3f(1.f,1.f,1.f);

		glEnd();
	}

	if (slot.highlighted)
	{
		const int x0 = 0, x1 = x0 + slot.w;
		const int y0 = 0, y1 = y0 + slot.h;

		glColor4f(1,1,1,.1f);
		glBegin(GL_QUADS);
		glVertex2i(x0-1, y0-1);
		glVertex2i(x0-1, y1+1);
		glVertex2i(x1+1, y1+1);
		glVertex2i(x1+1, y0-1);
		glEnd();
		glColor3f(1,1,1);
	}

	glEnable(GL_TEXTURE_2D);

	if (slot.item->count > 1)
		Font::Print(to_string((int)slot.item->count), float(slot.w), float(slot.h/2), float(slot.h/2), true);
}
