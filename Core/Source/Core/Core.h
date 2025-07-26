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

	
	PlaneMesh CreatePlaneMesh(int width, int height);
	std::string readFile(const std::string& filePath);
	GLuint CreateComputeShaderProgram(const std::string& path);
	void ApplyHeightMapCPU(PlaneMesh& planeData, int width, int height);
	void CreateVertices(PlaneMesh& planeData, int width, int height);
	void CreateIndices(PlaneMesh& planeData, int width, int height);
	void DisplaceVertices(PlaneMesh& planeData, int width, int height, float scale = 1.0f, float amplitude = 1.0f, float frequency = 1.0f, int octaves = 5, float persistance = 0.5f, float lacunarity = 2.0f);
	PlaneMesh GetHeightMapPlane(int width, int height, float scale = 35.0f, float amplitude = 1.0f, float frequency = 4.0f, int octaves = 5, float persistance = 0.5f, float lacunarity = 2.0f);
	namespace {
		void CalculateNormalsHeightMap(PlaneMesh& planeData, int width, int height);
	}
	
}