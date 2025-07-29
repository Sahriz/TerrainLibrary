#pragma once
#include <unordered_set>
#include <glm.hpp>

#include "Core/Core.h"
#include "ChunkManager.h"

class ChunkRenderer {
public:
	ChunkRenderer(int width, int height, int viewDistance) : _width(width), _height(height), _viewDistance(viewDistance){}

	void UpdateActiveChunk(const glm::vec3& position, ChunkManager& chunkManager);

	std::unordered_set<glm::ivec2>& GetActiveChunkSet() {
		return _activeChunkSet;
	}
private:
	std::unordered_set<glm::ivec2> _activeChunkSet;
	std::unordered_set<glm::ivec2> _previousFrameActiveChunkSet;
	
	int _width;
	int _height;
	int _viewDistance;

	void SetupChunkRenderData(Core::PlaneMesh& mesh);

	void CleanupChunkRenderData(Core::PlaneMesh& mesh);
};