#pragma once
#include "ChunkManager.h"


class Physics {
public:
	Physics() {};
	Physics(ChunkManager& chunkManager) {
		_chunkManager = chunkManager;
	}


private:
	ChunkManager _chunkManager;
};