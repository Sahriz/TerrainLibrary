#include "ChunkManager.h"

void ChunkManager::Update(const glm::vec3& position) {
	GenerateChunk(position);
}

void ChunkManager::GenerateChunk(const glm::vec3& position) {
	
	glm::ivec3 playerChunk = GetChunkCoordFromPosition(position);
	
	for (int x = -_viewDistance; x <= _viewDistance; x++) {
		for (int y = -_viewDistance; y <= _viewDistance; y++) {
			for (int z = -_viewDistance; z <= _viewDistance; z++) {
				if (glm::abs(x * y * z) > _viewDistance * _viewDistance / 1.5f) continue;
				glm::ivec3 coord = playerChunk + glm::ivec3(x, y, z);
				//std::cout << coord.x << " " << coord.y << " " << coord.z << "\n";
				// Generate if not yet stored
				if (_chunkMap.find(coord) == _chunkMap.end()) {
					_chunkMap[coord] = std::move(Core::CreateVoxel3DMesh(_width, _height, _depth, coord, false));
					
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