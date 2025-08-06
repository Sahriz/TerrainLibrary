#include "ChunkManager.h"

void ChunkManager::Update(const glm::vec3& position) {
	GenerateChunk(position);
}

void ChunkManager::GenerateChunk(const glm::vec3& position) {
	
	glm::vec2 playerChunk = GetChunkCoordFromPosition(position);
	
	for (int x = -_viewDistance; x <= _viewDistance; x++) {
			for (int z = -_viewDistance; z <= _viewDistance; z++) {
				if (glm::abs(x * z) > _viewDistance * _viewDistance / 1.5f) continue;
				glm::vec2 coord = playerChunk + glm::vec2(x, z);
				//std::cout << coord.x << " " << coord.y << " " << coord.z << "\n";
				// Generate if not yet stored
				if (_chunkMap.find(coord) == _chunkMap.end()) {
					glm::vec2 offset = coord * glm::vec2(_width, _depth);
					//Try generating the mesh with and without GPU to see the difference in speed! The function call is the same but the end of
					//the function call is GPU for the gpu implementtion. Please do keep in mind the noise map is still using compute shaders
					//even on the cpu implementation, so that is technically a speedup that should not be granted as a possitive for the CPU part
					//of this code. 
					_chunkMap[coord] = std::move(Core::CreateVoxelCubes3DMesh(_width, _height, _depth, offset, false, 1.0f, 0.1f, 0.5f, 2.0f, 5, true)); 
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