#include "ChunkRenderer.h"



void ChunkRenderer::UpdateActiveChunk(const glm::vec3& position, ChunkManager& chunkManager) {
	
	glm::vec3 playerChunk = chunkManager.GetChunkCoordFromPosition(position);
	_previousFrameActiveChunkSet = _activeChunkSet;
	_activeChunkSet.clear();
	auto& chunkMap = chunkManager.GetChunkMap();
	for (int x = -_viewDistance; x <= _viewDistance; x++) {
		for (int y = -_viewDistance; y <= _viewDistance; y++) {
			for (int z = -_viewDistance; z <= _viewDistance; z++) {
				if (glm::abs(x * y * z) > _viewDistance * _viewDistance * _viewDistance / 1.5f) continue;
				glm::vec3 coord = playerChunk + glm::vec3(x, y, z);
				//std::cout << coord.x << " " << coord.y << " " << coord.z << "\n";
				
				if (chunkMap.find(coord) != chunkMap.end()) {
					_activeChunkSet.insert(coord);
					// Generate if not yet stored
					
					
					if (!chunkMap[coord].gpuLoaded) {
						SetupChunkRenderData(chunkMap[coord]);
					}
				}
			}
		}	
	}
	for (const glm::ivec3& coord : _previousFrameActiveChunkSet) {
		if (_activeChunkSet.find(coord) == _activeChunkSet.end() && chunkMap[coord].gpuLoaded) {
			CleanupChunkRenderData(chunkMap[coord]);
		}
	}
	
}

void ChunkRenderer::SetupChunkRenderData(Core::VoxelMesh& mesh) {
	
	glGenVertexArrays(1, &mesh.vao);
	glBindVertexArray(mesh.vao);

	glBindBuffer(GL_ARRAY_BUFFER, mesh.vboVertices);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0); // location 0
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, mesh.vboNormals);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0); // location 1
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	mesh.gpuLoaded = true;
}

void ChunkRenderer::CleanupChunkRenderData(Core::VoxelMesh& mesh) {
	if (!mesh.gpuLoaded) return;

	// Delete everything including the new Indirect Buffer
	glDeleteBuffers(1, &mesh.vboVertices);
	glDeleteBuffers(1, &mesh.vboNormals);
	glDeleteBuffers(1, &mesh.indirectBuffer);

	// If you kept ssboIndices for specific reasons:
	if (mesh.ssboIndices) glDeleteBuffers(1, &mesh.ssboIndices);

	glDeleteVertexArrays(1, &mesh.vao);

	// Reset handles
	mesh.vboVertices = 0;
	mesh.vboNormals = 0;
	mesh.indirectBuffer = 0;
	mesh.vao = 0;

	mesh.gpuLoaded = false;
}

