#ifndef UPDATE_PIPELINE_INCLUDED
#define UPDATE_PIPELINE_INCLUDED

class World;

void executeUpdatePipeline(World& world);

bool executeUpdatePipelineAsync(World& world);
void updateUpdatePipelineAsync(int maxTimeForUpdate);

#endif
