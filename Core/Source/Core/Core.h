#pragma once
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include "vector"
#include "glm.hpp"


#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>

namespace Core {

	struct PlaneMesh
	{
		std::vector<glm::fvec3> vertices; //coordinates (x, y, z)
		std::vector<int> indices; //indices of the vertices to form triangles
		std::vector<glm::fvec3> normals; //normals for each vertex

		GLuint vao = 0;
		GLuint vboVertices = 0;
		GLuint vboNormals = 0;
		GLuint ebo = 0;

		bool gpuLoaded = false;
	};
	extern GLuint _vertexInitComputeShaderProgram;
	extern GLuint _indexInitComputeShaderProgram;
	extern GLuint _vertexDisplacementComputeShaderProgram;
	extern GLuint _normalInterpelationComputeShaderProgram;
	extern GLuint _3dNoiseMapComputeShader;
	extern GLuint _marchingCubesTriCounterComputeShader;
	extern GLuint _voxelCubesGeometryInitComputeShader;
	extern GLuint _smoothMarchingCubesVertCreatorComputeShader;
	extern GLuint _voxelCubesTriangleCounterComputeShader;

	void Init();
	void Cleanup();

	std::vector<float> CreateFlat2DNoiseMap(const int width, const int height, const int depth, const glm::vec2 offset, bool CleanUp);
	std::vector<float> CreateFlat3DNoiseMap(const int width, const int height, const int depth, const glm::vec3 offset, bool CleanUp, const float amplitude = 1.0f, const float frequency = 1.0f, const float persistance = 0.5f, const float lacunarity = 2.0f, const int octaves = 5, const bool useDropoff = false);

	void CreateVertices(PlaneMesh& planeData, int width, int height, glm::ivec2 offset = glm::ivec2(0,0), bool CleanUp = true);
	void CreateIndices(PlaneMesh& planeData, int width, int height, bool CleanUp);
	void DisplaceVertices(PlaneMesh& planeData, int width, int height, float scale = 1.0f, float amplitude = 1.0f, float frequency = 1.0f, int octaves = 5, float persistance = 0.5f, float lacunarity = 2.0f, bool CleanUp = true);
	void InterpolatedNormals(PlaneMesh& planeData, int width, int height, bool CleanUp);
	PlaneMesh CreateHeightMapPlaneMeshGPU(int width, int height, glm::vec2 offset = glm::vec2(0,0), float scale = 0.1f, float amplitude = 1.0f, float frequency = 1.0f, int octaves = 5, float persistance = 0.5f, float lacunarity = 2.0f, bool CleanUp = true);
	
	glm::vec3 VertInterp(float iso, glm::vec3 p1, glm::vec3 p2, float v1, float v2);
	int CountMarchingCubesTriangleCount(int width, int height, int depth, glm::vec3 offset, bool CleanUp, std::vector<float>& noiseMap, float iso);
	PlaneMesh CreateMarchingCubesTriangles(int width, int height, int depth, int offset, bool CleanUp, std::vector<float>& noiseMap, float iso, int triangleCount);
	PlaneMesh CreateVoxel2DMesh(int width, int height, int depth, glm::vec2 offset, bool CleanUp);
	PlaneMesh CreateMarchingCubes3DMesh(int width, int height, int depth, glm::vec3 offset, bool CleanUp);
	PlaneMesh CreateMarchingCubes3DMeshGPU(int width, int height, int depth, glm::vec3 offset, bool CleanUp);
	PlaneMesh CreateMarchingCubes3DMeshSmoothGPU(int width, int height, int depth, glm::vec3 offset, bool CleanUp);

	int VoxelCubesQuadCount(PlaneMesh& planeData, int width, int heigth, int depth, glm::vec3 offset, const std::vector<float>& noiseMap, bool CleanUp);
	void VoxelCubesGeometryInit(PlaneMesh& planeData, int width, int heigth, int depth, glm::vec3 offset, const std::vector<float>& noiseMap, bool CleanUp);
	PlaneMesh CreateVoxelCubes3DMesh(int width, int heigth, int depth, glm::vec2 offset, bool CleanUp, const float amplitude = 1.0f, const float frequency = 1.0f, const float persistance = 0.5f, const float lacunarity = 2.0f, const int octaves = 5, const bool useDropoff = true);
}