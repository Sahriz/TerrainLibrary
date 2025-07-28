#include "ChunkManager.h"

void ChunkManager::Update(glm::vec3& position) {
	UpdateActiveChunk(position);
}

void ChunkManager::SetupChunkRenderData(Core::PlaneMesh& mesh) {
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
}

void ChunkManager::UpdateActiveChunk(const glm::vec3& position) {
	float xScale = 500.0f / _width;
	float zScale = 500.0f / _height;
	glm::ivec2 playerChunk = glm::ivec2(std::floor(position.x / (_width * xScale)), std::floor(position.z / (_height * zScale)));  // based on player pos

	_activeChunkSet.clear();

	for (int x = -_viewDistance; x <= _viewDistance; x++) {
		for (int z = -_viewDistance; z <= _viewDistance; z++) {
			glm::ivec2 coord = playerChunk + glm::ivec2(x, z);
			glm::ivec2 offset = playerChunk + glm::ivec2(x * xScale, z * zScale);

			// Generate if not yet stored
			if (_chunkMap.find(coord) == _chunkMap.end()) {
				_chunkMap[coord] = std::move(Core::CreateHeightMapPlaneMeshGPU(_width, _height, coord, _scale, _amplitude, _frequency, _octave, _persistance, _lacunarity));
				SetupChunkRenderData(_chunkMap[coord]);
			}

			// Add an active chunk
			_activeChunkSet.insert(coord);
		}
	}
}

void ChunkManager::DestroyChunks() {
	for (auto& [coord, mesh] : _chunkMap) {
		DeleteChunk(mesh);
	}
	_chunkMap.clear();
	_activeChunkSet.clear();
}

void ChunkManager::DeleteChunk(Core::PlaneMesh& mesh) {
	if (mesh.ebo != 0) {
		glDeleteBuffers(1, &mesh.ebo);
		mesh.ebo = 0;
	}
	if (mesh.vboVertices != 0) {
		glDeleteBuffers(1, &mesh.vboVertices);
		mesh.vboVertices = 0;
	}
	if (mesh.vboNormals != 0) {
		glDeleteBuffers(1, &mesh.vboNormals);
		mesh.vboNormals = 0;
	}
	if (mesh.vao != 0) {
		glDeleteVertexArrays(1, &mesh.vao);
		mesh.vao = 0;
	}

	mesh.vertices.clear();
	mesh.normals.clear();
	mesh.indices.clear();
}