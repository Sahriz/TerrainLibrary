#include "ChunkRenderer.h"



void ChunkRenderer::UpdateActiveChunk(const glm::vec3& position, ChunkManager& chunkManager) {
	
	glm::vec3 playerChunk = chunkManager.GetChunkCoordFromPosition(position);
	_previousFrameActiveChunkSet = _activeChunkSet;
	_activeChunkSet.clear();
	auto& chunkMap = chunkManager.GetChunkMap();
	for (int x = -_viewDistance; x <= _viewDistance; x++) {
		for (int y = -_viewDistance; y <= _viewDistance; y++) {
			for (int z = -_viewDistance; z <= _viewDistance; z++) {
				if (glm::abs(x * y * z) > _viewDistance * _viewDistance / 1.5f) continue;
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

void ChunkRenderer::SetupChunkRenderData(Core::PlaneMesh& mesh) {
	
	glGenVertexArrays(1, &mesh.vao);
	glBindVertexArray(mesh.vao);

	glGenBuffers(1, &mesh.vboVertices);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vboVertices);
	glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(glm::fvec3), mesh.vertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr); // location 0
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &mesh.vboNormals);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vboNormals);
	glBufferData(GL_ARRAY_BUFFER, mesh.normals.size() * sizeof(glm::fvec3), mesh.normals.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr); // location 1
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &mesh.ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(int), mesh.indices.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);

	mesh.gpuLoaded = true;
}

void ChunkRenderer::CleanupChunkRenderData(Core::PlaneMesh& mesh) {
	// Delete the EBO, VBOs, and VAO
	glDeleteBuffers(1, &mesh.ebo);
	glDeleteBuffers(1, &mesh.vboNormals);
	glDeleteBuffers(1, &mesh.vboVertices);
	glDeleteVertexArrays(1, &mesh.vao);

	// Optional: Reset IDs to 0 (OpenGL default for "none")
	mesh.ebo = 0;
	mesh.vboNormals = 0;
	mesh.vboVertices = 0;
	mesh.vao = 0;

	mesh.gpuLoaded = false;
}

