#include "ChunkRenderer.h"

void ChunkRenderer::UpdateActiveChunk(const glm::vec3& position, ChunkManager& chunkManager) {
	glm::vec2 playerChunk = chunkManager.GetChunkCoordFromPosition(position);	
	_previousFrameActiveChunkSet = _activeChunkSet;
	_activeChunkSet.clear();
	auto& chunkMap = chunkManager.GetChunkMap();
	for (int x = -_viewDistance; x <= _viewDistance; x++) {
			for (int z = -_viewDistance; z <= _viewDistance; z++) {
				if (glm::abs(x * z) > _viewDistance * _viewDistance / 1.5f) continue;
				glm::vec2 coord = playerChunk + glm::vec2(x, z);
				//std::cout << coord.x << " " << coord.y << " " << coord.z << "\n";
				
				if (chunkMap.find(coord) != chunkMap.end()) {
					_activeChunkSet.insert(coord);
					// Generate if not yet stored
					if (!chunkMap[coord]->gpuLoaded) {
						SetupChunkRenderData(*chunkMap[coord]);
					}
				}
			}
		}	
	for (const glm::vec2& coord : _previousFrameActiveChunkSet) {
		if (_activeChunkSet.find(coord) == _activeChunkSet.end() && chunkMap[coord]->gpuLoaded) {
			CleanupChunkRenderData(chunkMap[coord]);
		}
	}
}

void ChunkRenderer::SetupChunkRenderData(Core::VoxelCubeMesh& mesh) {
	
	glBindVertexArray(mesh.vao);

	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);

	// Position (Location 0) - 3 floats
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Core::VoxelCubeCombinedVertex), (void*)offsetof(Core::VoxelCubeCombinedVertex, position));
	glEnableVertexAttribArray(0);

	// Normal (Location 1) - 3 floats
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Core::VoxelCubeCombinedVertex), (void*)offsetof(Core::VoxelCubeCombinedVertex, normal));
	glEnableVertexAttribArray(1);

	// UV (Location 2) - 2 floats
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Core::VoxelCubeCombinedVertex), (void*)offsetof(Core::VoxelCubeCombinedVertex, uv));
	glEnableVertexAttribArray(2);

	// IMPORTANT: Bind the index buffer to the VAO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);

	glBindVertexArray(0);
	mesh.gpuLoaded = true;
}

void ChunkRenderer::CleanupChunkRenderData(Core::VoxelCubeMesh* mesh) {
	
}

