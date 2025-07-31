#include "ChunkManager.h"

void ChunkManager::Update(const glm::vec3& position) {
	GenerateChunk(position);
}

void ChunkManager::GenerateChunk(const glm::vec3& position) {
	
	glm::vec3 playerChunk = GetChunkCoordFromPosition(position);
	
	for (int x = -_viewDistance; x <= _viewDistance; x++) {
		for (int y = -_viewDistance; y <= _viewDistance; y++) {
			for (int z = -_viewDistance; z <= _viewDistance; z++) {
				if (glm::abs(x * y * z) > _viewDistance * _viewDistance * _viewDistance / 1.5f) continue;
				glm::vec3 coord = playerChunk + glm::vec3(x, y, z);
				//std::cout << coord.x << " " << coord.y << " " << coord.z << "\n";
				// Generate if not yet stored
				if (_chunkMap.find(coord) == _chunkMap.end()) {
					glm::vec3 offset = coord * glm::vec3(_width, _height, _depth);
					//Try generating the mesh with and without GPU to see the difference in speed! The function call is the same but the end of
					//the function call is GPU for the gpu implementtion. Please do keep in mind the noise map is still using compute shaders
					//even on the cpu implementation, so that is technically a speedup that should not be granted as a possitive for the CPU part
					//of this code. 
					_chunkMap[coord] = std::move(Core::CreateMarchingCubes3DMeshGPU(_width, _height, _depth, offset, false)); 
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