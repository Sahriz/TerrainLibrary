#include "ChunkManager.h"

void ChunkManager::Update(const glm::vec3& position, Physics& physics) {
	GenerateChunk(position);
	GetCPUContent(position, physics);
	UnloadFarChunks(position); // Keep the VRAM clean!
}

void ChunkManager::QueueCPUContent(const glm::vec3& coord) {
	// This function can be used to queue chunks for CPU readback. 
	_chunkCoordsToGenerateToCPU.push_back(coord);
}

void ChunkManager::GetCPUContent(const glm::vec3& position, Physics& physics) {
	for (auto it = _chunkCoordsToGenerateToCPU.begin(); it != _chunkCoordsToGenerateToCPU.end(); ){
		Core::VoxelMesh* chunkData = _chunkMap[*it];

		if (Core::PollAsyncReadback(*chunkData)) {
			// 2. Erase it, and let C++ give us the iterator to the next item
			PhysicsJob job;
			job.type = PhysicsJob::Type::AddOrUpdate;
			job.coord = *it;
			job.vertices = chunkData->cpuMesh.vertices; // copy
			job.normals = chunkData->cpuMesh.normals;   // optional
			physics.EnqueueJob(std::move(job));

			it = _chunkCoordsToGenerateToCPU.erase(it);

			
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
					_chunkMap[coord] = std::move(Core::CreateMarchingCubes3DMeshGPU(_width, _height, _depth, offset, false, _scale, _frequency, _persistance, _lacunarity, _octave)); 
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

void ChunkManager::UnloadFarChunks(const glm::vec3& playerPosition) {
	// 1. Get player's chunk coordinate (use your existing function)
	glm::ivec3 playerChunk = GetChunkCoordFromPosition(playerPosition);

	// 2. Set unload distance slightly larger than view distance to prevent flickering
	int unloadDist = _viewDistance + 2;

	for (auto it = _chunkMap.begin(); it != _chunkMap.end(); ) {
		glm::ivec3 chunkCoord = it->first;

		// 3. Calculate cubic distance on the grid
		int distX = std::abs(chunkCoord.x - playerChunk.x);
		int distY = std::abs(chunkCoord.y - playerChunk.y);
		int distZ = std::abs(chunkCoord.z - playerChunk.z);

		// If it falls outside our cube...
		if (distX > unloadDist || distY > unloadDist || distZ > unloadDist) {

			// 4. FIX THE GHOST QUEUE: Remove it from the pending CPU readback list!
			auto queueIt = std::find_if(_chunkCoordsToGenerateToCPU.begin(), _chunkCoordsToGenerateToCPU.end(),
				[&chunkCoord](const glm::vec3& v) {
					return v.x == chunkCoord.x && v.y == chunkCoord.y && v.z == chunkCoord.z;
				});

			if (queueIt != _chunkCoordsToGenerateToCPU.end()) {
				_chunkCoordsToGenerateToCPU.erase(queueIt);
			}

			// 5. Nuke everything safely
			DeleteChunk(it->second);
			it = _chunkMap.erase(it);
		}
		else {
			++it;
		}
	}
}

void ChunkManager::DeleteChunk(Core::VoxelMesh* mesh) {
	if (!mesh) return;

	// Free standard GPU memory
	if (mesh->vboVertices) glDeleteBuffers(1, &mesh->vboVertices);
	if (mesh->vboNormals) glDeleteBuffers(1, &mesh->vboNormals);
	if (mesh->vao) glDeleteVertexArrays(1, &mesh->vao);
	if (mesh->indirectBuffer) glDeleteBuffers(1, &mesh->indirectBuffer);
	if (mesh->ssboIndices) glDeleteBuffers(1, &mesh->ssboIndices);

	// --- THE LEAK FIX: Free the massive Staging Buffers! ---
	if (mesh->stagingVertices) glDeleteBuffers(1, &mesh->stagingVertices);
	if (mesh->stagingNormals) glDeleteBuffers(1, &mesh->stagingNormals);
	if (mesh->stagingIndirect) glDeleteBuffers(1, &mesh->stagingIndirect);

	if (mesh->syncObj) glDeleteSync(mesh->syncObj);

	// Delete the C++ object from RAM
	delete mesh;
}