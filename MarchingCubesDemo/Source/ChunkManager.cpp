#include "ChunkManager.h"

void ChunkManager::Update(const glm::vec3& position) {
	GenerateChunk(position);
	//GetCPUContent(position);
}

void ChunkManager::QueueCPUContent(const glm::vec3& coord) {
	// This function can be used to queue chunks for CPU readback. 
	_chunkCoordsToGenerateToCPU.push_back(coord);
}

void ChunkManager::GetCPUContent(const glm::vec3& position) {
	for (auto it = _chunkCoordsToGenerateToCPU.begin(); it != _chunkCoordsToGenerateToCPU.end(); ){
		Core::VoxelMesh& chunkData = _chunkMap[*it];

		if (Core::PollAsyncReadback(chunkData)) {
			std::cout << "fetched!\n"; // Fixed typo here!

			// 2. Erase it, and let C++ give us the iterator to the next item
			it = _chunkCoordsToGenerateToCPU.erase(it);
			break;
		}
		else {
			// 3. Only move forward if we DID NOT erase anything
			++it;
		}
	}
}

void ChunkManager::GenerateChunk(const glm::vec3& position) {
	
	glm::vec3 playerChunk = GetChunkCoordFromPosition(position);
	int numberOfGeneratedChunksOnThisFrame = 0;
	for (int x = -_viewDistance; x <= _viewDistance; x++) {
		for (int y = -_viewDistance; y <= _viewDistance; y++) {
			for (int z = -_viewDistance; z <= _viewDistance; z++) {
				if (numberOfGeneratedChunksOnThisFrame > 1) return;
				//if (glm::abs(x * y * z) > _viewDistance * _viewDistance * _viewDistance / 1.5f) continue;
				glm::vec3 coord = playerChunk + glm::vec3(x, y, z);
				//std::cout << coord.x << " " << coord.y << " " << coord.z << "\n";
				// Generate if not yet stored
				if (_chunkMap.find(coord) == _chunkMap.end()) {
					glm::vec3 offset = coord * glm::vec3(_width, _height, _depth);
					//Try generating the mesh with and without GPU to see the difference in speed! The function call is the same but the end of
					//the function call is GPU for the gpu implementtion. Please do keep in mind the noise map is still using compute shaders
					//even on the cpu implementation, so that is technically a speedup that should not be granted as a possitive for the CPU part
					//of this code. 
					_chunkMap[coord] = std::move(*Core::CreateMarchingCubes3DMeshGPU(_width, _height, _depth, offset, false, _scale, _frequency, _persistance, _lacunarity, _octave)); 
					numberOfGeneratedChunksOnThisFrame++;
					QueueCPUContent(coord);
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

void ChunkManager::DeleteChunk(Core::VoxelMesh& mesh) {

	// Free GPU memory
	if (mesh.vboVertices) glDeleteBuffers(1, &mesh.vboVertices);
	if (mesh.vboNormals) glDeleteBuffers(1, &mesh.vboNormals);
	if (mesh.vao) glDeleteVertexArrays(1, &mesh.vao);
	if (mesh.indirectBuffer) glDeleteBuffers(1, &mesh.indirectBuffer);
	if (mesh.ssboIndices) glDeleteBuffers(1, &mesh.ssboIndices);
	


	mesh.vboNormals = 0;
	mesh.vboVertices = 0;
	mesh.vao = 0;

	mesh.gpuLoaded = false;
}