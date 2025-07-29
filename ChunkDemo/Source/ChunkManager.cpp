#include "ChunkManager.h"

void ChunkManager::Update(const glm::vec3& position) {
	GenerateChunk(position);
}

void ChunkManager::GenerateChunk(const glm::vec3& position) {
	float xScale = 100.0f / _width;
	float zScale = 100.0f / _height;
	glm::ivec2 playerChunk = glm::ivec2(std::floor(position.x / (_width * xScale)), std::floor(position.z / (_height * zScale)));  // based on player pos
	int counter = 0;
	for (int x = -_viewDistance; x <= _viewDistance; x++) {
		for (int z = -_viewDistance; z <= _viewDistance; z++) {
			if (glm::abs(x * z) > _viewDistance*_viewDistance/1.5f) continue;
			glm::ivec2 coord = playerChunk + glm::ivec2(x, z);

			// Generate if not yet stored
			if (_chunkMap.find(coord) == _chunkMap.end()) {
				_chunkMap[coord] = std::move(Core::CreateHeightMapPlaneMeshGPU(_width, _height, coord, _scale, _amplitude, _frequency, _octave, _persistance, _lacunarity));
				//return;
				//counter++;
				if (counter > 2) {
					return;
				}
			}
		}
	}
}

void ChunkManager::DestroyChunks() {
	for (auto& [coord, mesh] : _chunkMap) {
		DeleteChunk(mesh);
	}
	_chunkMap.clear();
}

void ChunkManager::DeleteChunk(Core::PlaneMesh& mesh) {
	mesh.vertices.clear();
	mesh.normals.clear();
	mesh.indices.clear();
}