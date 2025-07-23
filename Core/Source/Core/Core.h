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
	};

	
	PlaneMesh CreatePlaneMesh(int width, int height);
	std::string readFile(const std::string& filePath);
	GLuint CreateComputeShaderProgram(const std::string& path);
	void ApplyHeightMapCPU(PlaneMesh& planeData);
	void ApplyHeightMap(PlaneMesh& planeData);
}