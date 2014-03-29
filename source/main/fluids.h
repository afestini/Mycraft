#ifndef FLUIDS_INCLUDED
#define FLUIDS_INCLUDED

#include <vector>
#include "vertex.h"

class Chunk;

void appendFluidGeometry(int x, int y, int z, std::vector<FloatVertex>& vertexBuffer, unsigned visibleDirections, const Chunk& chunk);

#endif