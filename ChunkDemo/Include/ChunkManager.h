#pragma once

#include <unordered_map>
#include <unordered_set>
#include <glm.hpp>

#include "Core/Core.h"

using ChunkCoord = glm::ivec2;

namespace std {
	template <>
	struct hash<glm::ivec2> {
		size_t operator()(const glm::ivec2& v) const {
			return std::hash<int>()(v.x) ^ (std::hash<int>()(v.y) << 1);
		}
	};
}

class ChunkManager {
public:

	ChunkManager(){}

	void DestroyChunks();

	void Update(glm::vec3& position);

	std::unordered_map<ChunkCoord, Core::PlaneMesh>& GetChunkMap() {
		return _chunkMap;
	}

	std::unordered_set<glm::ivec2>& GetActiveChunkSet() {
		return _activeChunkSet;
	}

private:
	std::unordered_map<ChunkCoord, Core::PlaneMesh> _chunkMap;
	std::unordered_set<glm::ivec2> _activeChunkSet;

	float _scale = 0.1f;
	float _amplitude = 1.0f;
	float _frequency = 1.0f;
	int _octave = 5;
	float _lacunarity = 2.0f;
	float _persistance = 0.5f;
	int _width = 1000;
	int _height = 1000;
	int _viewDistance = 1;

	void SetupChunkRenderData(Core::PlaneMesh& mesh);

	void UpdateActiveChunk(const glm::vec3& position);

	void DeleteChunk(Core::PlaneMesh& mesh);

};