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
	};

	
	PlaneMesh CreatePlaneMeshCPU(int width, int height);
	std::string readFile(const std::string& filePath);
	GLuint CreateComputeShaderProgram(const std::string& path);
	void CreateHeightMapPlaneMeshCPU(PlaneMesh& planeData, int width, int height);
	void CreateVertices(PlaneMesh& planeData, int width, int height);
	void CreateIndices(PlaneMesh& planeData, int width, int height);
	void DisplaceVertices(PlaneMesh& planeData, int width, int height, float scale = 1.0f, float amplitude = 1.0f, float frequency = 1.0f, int octaves = 5, float persistance = 0.5f, float lacunarity = 2.0f);
	PlaneMesh CreateHeightMapPlaneMeshGPU(int width, int height, float scale = 0.1f, float amplitude = 1.0f, float frequency = 1.0f, int octaves = 5, float persistance = 0.5f, float lacunarity = 2.0f);
	
	
}