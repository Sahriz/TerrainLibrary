#pragma once

#include <unordered_map>
#include <unordered_set>
#include <glm.hpp>

#include "Core/Core.h"

using ChunkCoord = glm::ivec3;

namespace std {
	template <>
	struct hash<glm::ivec3> {
		size_t operator()(const glm::ivec3& k) const {
			std::size_t h1 = std::hash<int>()(k.x);
			std::size_t h2 = std::hash<int>()(k.y);
			std::size_t h3 = std::hash<int>()(k.z);
			// Combine hashes
			return ((h1 ^ (h2 << 1)) >> 1) ^ (h3 << 1);
		}
	};
}

class ChunkManager {
public:

	ChunkManager() {}

	void DestroyChunks();

	void Update(const glm::vec3& position);

	void GenerateChunk(const glm::vec3& position);

	glm::vec3 GetChunkCoordFromPosition(const glm::vec3& position) const {
		float xScale = 1.0f / _width;
		float yScale = 1.0f / _height;
		float zScale = 1.0f / _depth;

		return glm::ivec3(
			std::floor(position.x / (_width )),
			std::floor(position.y / (_height )),
			std::floor(position.z / (_depth ))
		);
	}

	void UpdateSettings(float scale, float amplitude, float frequency, int octaves, float lacunarity, float persistance, int width, int height, int depth, int viewDistance) {
		_scale = scale;
		_amplitude = amplitude;
		_frequency = frequency;
		_octave = octaves;
		_lacunarity = lacunarity;
		_persistance = persistance;
		_width = width;
		_height = height;
		_depth = depth;
		_viewDistance = viewDistance;

	}

	std::unordered_map<ChunkCoord, Core::PlaneMesh>& GetChunkMap() {
		return _chunkMap;
	}

private:
	std::unordered_map<ChunkCoord, Core::PlaneMesh> _chunkMap;

	float _scale = 0.1f;
	float _amplitude = 1.0f;
	float _frequency = 1.0f;
	int _octave = 5;
	float _lacunarity = 2.0f;
	float _persistance = 0.5f;
	int _width = 32;
	int _height = 32;
	int _depth = 32;
	int _viewDistance = 2;

	void DeleteChunk(Core::PlaneMesh& mesh);

};