#include "Core.h"




namespace Core {
	
	

	void PrintHelloWorld()
	{
		std::cout << "Hello World!\n";
		std::cin.get();
	}
	int getIndex(int x, int z, int width) {
		return z * width + x;
	}
	PlaneMesh CreatePlaneMesh(int width, int height) {

		std::vector<glm::fvec3> vertices;
		vertices.reserve(width*height); //coordinates (x, y, z)
		std::vector<glm::fvec3> normals;
		normals.reserve(width * height); //coordinates (x, y, z)
		float xScale = 100.f / width;
		float zScale = 100.f / height;
		for(int z = 0; z < height; ++z) {
			for(int x = 0; x < width; ++x) {
				// Calculate the vertex position
				float posX = static_cast<float>(x-width/2.0f) * xScale;
				float posY = -0.1;
				float posZ = static_cast<float>(z-height/2.0f) * zScale; // Assuming a flat plane at z=0
				//std::cout << posZ << "\n";
				glm::fvec3 vertex(posX, posY, posZ);
				// Add the vertex to the vector
				vertices.push_back(vertex);
				normals.push_back(glm::fvec3(0.0f, 1.0f, 0.0f)); // Flat normal for the plane
			}
		}
		std::vector<int> indices;
		
		for (int z = 0; z < height - 1; ++z) {
			for (int x = 0; x < width - 1; ++x) {
				int v0 = getIndex(x, z, width);
				int v1 = getIndex(x + 1, z, width);
				int v2 = getIndex(x, z + 1, width);
				int v3 = getIndex(x + 1, z + 1, width);

				// First triangle
				indices.push_back(v0);
				indices.push_back(v2);
				indices.push_back(v1);
				
				// Second triangle
				indices.push_back(v1);
				indices.push_back(v2);
				indices.push_back(v3);
				
			}
		}

		

		return PlaneMesh(vertices,indices, normals);
	}

	std::string readFile(const std::string& filePath) {
		std::ifstream file(filePath);
		std::stringstream buffer;
		if (file) {
			buffer << file.rdbuf();
		}
		else {
			std::cerr << "Failed to open file: " << filePath << "\n";
		}
		return buffer.str();
	}

	GLuint CreateComputeShaderProgram(const std::string& path) {
		GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
		std::string source = readFile(path);
		const char* src = source.c_str();
		glShaderSource(shader, 1, &src, nullptr);
		glCompileShader(shader);

		// Check compilation status
		GLint success;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success) {
			GLint logLength;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
			std::vector<char> log(logLength);
			glGetShaderInfoLog(shader, logLength, nullptr, log.data());
			std::cerr << "Compute Shader compilation failed:\n" << log.data() << std::endl;
			glDeleteShader(shader);
			return 0;
		}

		// Link shader into a program
		GLuint program = glCreateProgram();
		glAttachShader(program, shader);
		glLinkProgram(program);

		glGetProgramiv(program, GL_LINK_STATUS, &success);
		if (!success) {
			GLint logLength;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
			std::vector<char> log(logLength);
			glGetProgramInfoLog(program, logLength, nullptr, log.data());
			std::cerr << "Program linking failed:\n" << log.data() << std::endl;
			glDeleteShader(shader);
			glDeleteProgram(program);
			return 0;
		}

		glDeleteShader(shader); // Safe to delete after linking
		return program;
	}

	void ApplyHeightMapCPU(PlaneMesh& planeData, int height, int width) {

		for(glm::fvec3& point : planeData.vertices) {
			// Apply some height map logic here, for example, a simple sine wave
			point.y = point.y + 3.0f * sin(point.x * 0.1f) + 3.0f * cos(point.z * 0.1f);
		}
		for (int z = 0; z < height; ++z) {
			for (int x = 0; x < width; ++x) {
				int v0 = getIndex(x, z, width);
				int v1 = getIndex(x, z, width);
				int v2 = getIndex(x, z, width);
				int v3 = getIndex(x, z, width);
				if (z > 0) {
					v0 = getIndex(x, z - 1, width);
				}
				if (z < height - 1) {
					v1 = getIndex(x, z + 1, width);
				}
				if( x > 0) {
					v2 = getIndex(x - 1, z, width);
				}
				if (x < width - 1) {
					v3 = getIndex(x + 1, z, width);
				}

				glm::vec3 line1 = planeData.vertices[v1] - planeData.vertices[v0];
				glm::vec3 line2 = planeData.vertices[v3] - planeData.vertices[v2];
				glm::vec3 normal = glm::normalize(glm::cross(line1, line2));
				planeData.normals[getIndex(x, z, width)] = normal; // Assign the normal to the corresponding vertex
			}
		}
	}
	
	void CreateVertices(PlaneMesh& planeData, int width, int height) {
		GLuint ssboVertices;
		glGenBuffers(1, &ssboVertices);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboVertices);
		glBufferData(GL_SHADER_STORAGE_BUFFER, planeData.vertices.size() * 3 * sizeof(float), planeData.vertices.data(), GL_DYNAMIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboVertices);

		GLuint computeShaderProgram = CreateComputeShaderProgram("../Core/Source/Core/HeightMapVertexInit.comp");

		GLint widthLoc = glGetUniformLocation(computeShaderProgram, "width");
		GLint heightLoc = glGetUniformLocation(computeShaderProgram, "height");

		glUseProgram(computeShaderProgram);

		glUniform1i(widthLoc, width);
		glUniform1i(heightLoc, height);

		GLuint numGroups = ((width * height) + 63) / 64;
		glDispatchCompute(numGroups, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboVertices);
		glm::fvec3* ptr = (glm::fvec3*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

		// Copy or use data
		if (ptr) {
			
			
			planeData.vertices.assign(ptr, ptr + planeData.vertices.size());
			glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		}
		else {
			std::cout << "Something went wrong in CreateVertices";
		}
	}

	void CreateIndices(PlaneMesh& planeData, int width, int height) {
		GLuint ssboIndices;
		glGenBuffers(1, &ssboIndices);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboIndices);
		glBufferData(GL_SHADER_STORAGE_BUFFER, planeData.indices.size() * sizeof(int), planeData.indices.data(), GL_DYNAMIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboIndices);

		GLuint computeShaderProgram = CreateComputeShaderProgram("../Core/Source/Core/HeightMapIndexInit.comp");

		GLint widthLoc = glGetUniformLocation(computeShaderProgram, "width");
		GLint heightLoc = glGetUniformLocation(computeShaderProgram, "height");

		glUseProgram(computeShaderProgram);

		glUniform1i(widthLoc, width);
		glUniform1i(heightLoc, height);

		GLuint numGroups = (((width-1) * (height-1)) + 63) / 64;
		glDispatchCompute(numGroups, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboIndices);
		int* ptr = (int*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

		// Copy or use data
		if (ptr) {
			planeData.indices.assign(ptr, ptr + planeData.indices.size());
			glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		}
		else {
			std::cout << "Something went wrong in DisplaceVertices";
		}
	}
	
	void DisplaceVertices(PlaneMesh& planeData, int width, int height) {
		GLuint ssboVertices;
		if (width * height != planeData.vertices.size()) {
			std::cout << "wrong sizes!";
		}
		glGenBuffers(1, &ssboVertices);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboVertices);
		glBufferData(GL_SHADER_STORAGE_BUFFER, planeData.vertices.size() * 3 * sizeof(float), planeData.vertices.data(), GL_DYNAMIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboVertices);

		GLuint computeShaderProgram = CreateComputeShaderProgram("../Core/Source/Core/HeightMapVertexDisplacement.comp");

		GLint widthLoc = glGetUniformLocation(computeShaderProgram, "width");
		GLint heightLoc = glGetUniformLocation(computeShaderProgram, "height");

		glUseProgram(computeShaderProgram);

		glUniform1i(widthLoc, width);
		glUniform1i(heightLoc, height);

		GLuint numGroups = ((width * height) + 63) / 64;
		glDispatchCompute(numGroups, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboVertices);
		glm::fvec3* ptr = (glm::fvec3*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

		// Copy or use data
		if (ptr) {
			planeData.vertices.assign(ptr, ptr + planeData.vertices.size());
			glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		}
		else {
			std::cout << "Something went wrong in DisplaceVertices";
		}
		
	}

	void InterpolatedNormals(PlaneMesh& planeData, int width, int height){
		GLuint ssboVertices, ssboNormals;
		if (width * height != planeData.vertices.size()) {
			std::cout << "wrong sizes!";
		}
		if (width * height != planeData.normals.size()) {
			std::cout << "wrong sizes!";
		}
		glGenBuffers(1, &ssboVertices);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboVertices);
		glBufferData(GL_SHADER_STORAGE_BUFFER, width * height * 3 * sizeof(float), planeData.vertices.data(), GL_DYNAMIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboVertices);

		glGenBuffers(1, &ssboNormals);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboNormals);
		glBufferData(GL_SHADER_STORAGE_BUFFER, width*height * 3 * sizeof(float), planeData.normals.data(), GL_DYNAMIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboNormals);

		GLuint computeShaderProgram = CreateComputeShaderProgram("../Core/Source/Core/HeightMapNormal.comp");

		GLint widthLoc = glGetUniformLocation(computeShaderProgram, "width");
		GLint heightLoc = glGetUniformLocation(computeShaderProgram, "height");

		glUseProgram(computeShaderProgram);

		glUniform1i(widthLoc, width);
		glUniform1i(heightLoc, height);

		GLuint numGroups = ((width * height) + 63) / 64;
		glDispatchCompute(numGroups, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboNormals);
		glm::fvec3* ptrNormals = (glm::fvec3*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

		// Copy or use data
		planeData.normals.assign(ptrNormals, ptrNormals + planeData.normals.size());
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	}
	
	PlaneMesh GetHeightMapPlane(int width, int height) {
		PlaneMesh planeData;

		std::vector<glm::fvec3> vertices;
		vertices.resize(width * height);
		std::vector<int> indices;
		indices.resize((width - 1) * (width - 1) * 6);
		std::vector<glm::fvec3> normals;
		normals.resize(width*height);

		planeData.vertices = vertices;
		planeData.indices = indices;
		planeData.normals = normals;

		CreateVertices(planeData, width, height);
		CreateIndices(planeData, width, height);
		DisplaceVertices(planeData, width, height);
		InterpolatedNormals(planeData, width, height);
		//CalculateNormalsHeightMap(planeData, width, height);

		return planeData;

	}
	namespace {
		void CalculateNormalsHeightMap(PlaneMesh& planeData, int width, int height) {
			//Cases for accumulating normals and getting the average normal per vertex
			//First case: The vertex is not on the edge of the mesh or the corner. This means it has 6 neighboring triangles. 
			//Second case: The vertex is on the edge of the mesh, which means it has 3 neighboring triangles.
			//Third case: The vertex is on the corner of the mesh, which means it has either 1 or 2 neighboring triangles. 
			// If z and x are both 0 or height-1 and width-1 respectively then there is only one triangle. 
			// If z is 0 while x is width-1 or vice versa then there are two triangles.
			//Start with the first case, which is the most common one, then handle the other cases as edge cases (pun intended).
			for (int z = 0; z < height; ++z) {
				for (int x = 0; x < width; ++x) {

					//Bottom-left corner
					if (z == 0 && x == 0) {
						int v0 = getIndex(x, z, width);
						int v1 = getIndex(x + 1, z, width);
						int v2 = getIndex(x, z + 1, width);

						glm::vec3 triNormal = glm::normalize(glm::cross(planeData.vertices[v2] - planeData.vertices[v1], planeData.vertices[v1] - planeData.vertices[v0]));

						glm::vec3 combinedNormal = glm::normalize(triNormal);

						planeData.normals[v0] = combinedNormal;
					}
					//Bottom-right corner
					else if (z == 0 && x == width - 1) {
						int v0 = getIndex(x, z, width);
						int v1 = getIndex(x, z + 1, width);
						int v2 = getIndex(x - 1, z, width);
						int v3 = getIndex(x - 1, z + 1, width);

						glm::vec3 triNormal1 = glm::normalize(glm::cross(planeData.vertices[v3] - planeData.vertices[v1], planeData.vertices[v1] - planeData.vertices[v0]));
						glm::vec3 triNormal2 = glm::normalize(glm::cross(planeData.vertices[v2] - planeData.vertices[v3], planeData.vertices[v3] - planeData.vertices[v0]));

						glm::vec3 combinedNormal = glm::normalize(triNormal1 + triNormal2);

						planeData.normals[v0] = combinedNormal;

					}
					//Top-left corner
					else if (z == height - 1 && x == 0) {
						int v0 = getIndex(x, z, width);
						int v1 = getIndex(x, z - 1, width);
						int v2 = getIndex(x + 1, z, width);
						int v3 = getIndex(x + 1, z - 1, width);

						glm::vec3 triNormal1 = glm::normalize(glm::cross(planeData.vertices[v3] - planeData.vertices[v1], (planeData.vertices[v1] - planeData.vertices[v0])));
						glm::vec3 triNormal2 = glm::normalize(glm::cross(planeData.vertices[v2] - planeData.vertices[v3], planeData.vertices[v3] - planeData.vertices[v0]));

						glm::vec3 combinedNormal = glm::normalize(triNormal1 + triNormal2);

						planeData.normals[v0] = combinedNormal;
					}
					//Top-right corner
					else if (z == height - 1 && x == width - 1) {
						int v0 = getIndex(x, z, width);
						int v1 = getIndex(x - 1, z, width);
						int v2 = getIndex(x, z - 1, width);

						glm::vec3 triNormal = glm::normalize(glm::cross(planeData.vertices[v2] - planeData.vertices[v1], planeData.vertices[v1] - planeData.vertices[v0]));

						glm::vec3 combinedNormal = glm::normalize(triNormal);

						planeData.normals[v0] = combinedNormal;
					}
					//Left edge
					else if (x == 0) {
						int v0 = getIndex(x, z, width);
						int v1 = getIndex(x, z - 1, width);
						int v2 = getIndex(x + 1, z - 1, width);
						int v3 = getIndex(x + 1, z, width);
						int v4 = getIndex(x, z + 1, width);

						glm::vec3 triNormal1 = glm::normalize(glm::cross(planeData.vertices[v2] - planeData.vertices[v1], planeData.vertices[v1] - planeData.vertices[v0]));
						glm::vec3 triNormal2 = glm::normalize(glm::cross(planeData.vertices[v3] - planeData.vertices[v2], planeData.vertices[v2] - planeData.vertices[v0]));
						glm::vec3 triNormal3 = glm::normalize(glm::cross(planeData.vertices[v4] - planeData.vertices[v3], planeData.vertices[v3] - planeData.vertices[v0]));

						glm::vec3 combinedNormal = glm::normalize(triNormal1 + triNormal2 + triNormal3);

						planeData.normals[v0] = combinedNormal;
					}
					//right edge
					else if (x == width - 1) {
						int v0 = getIndex(x, z, width);
						int v1 = getIndex(x, z + 1, width);
						int v2 = getIndex(x - 1, z + 1, width);
						int v3 = getIndex(x - 1, z, width);
						int v4 = getIndex(x, z - 1, width);

						glm::vec3 triNormal1 = glm::normalize(glm::cross(planeData.vertices[v2] - planeData.vertices[v1], planeData.vertices[v1] - planeData.vertices[v0]));
						glm::vec3 triNormal2 = glm::normalize(glm::cross(planeData.vertices[v3] - planeData.vertices[v2], planeData.vertices[v2] - planeData.vertices[v0]));
						glm::vec3 triNormal3 = glm::normalize(glm::cross(planeData.vertices[v4] - planeData.vertices[v3], planeData.vertices[v3] - planeData.vertices[v0]));

						glm::vec3 combinedNormal = glm::normalize(triNormal1 + triNormal2 + triNormal3);

						planeData.normals[v0] = combinedNormal;
					}
					//bottom edge
					else if (z == 0) {
						int v0 = getIndex(x, z, width);
						int v1 = getIndex(x + 1, z, width);
						int v2 = getIndex(x, z + 1, width);
						int v3 = getIndex(x - 1, z + 1, width);
						int v4 = getIndex(x - 1, z, width);

						glm::vec3 triNormal1 = glm::normalize(glm::cross(planeData.vertices[v2] - planeData.vertices[v1], planeData.vertices[v1] - planeData.vertices[v0]));
						glm::vec3 triNormal2 = glm::normalize(glm::cross(planeData.vertices[v3] - planeData.vertices[v2], planeData.vertices[v2] - planeData.vertices[v0]));
						glm::vec3 triNormal3 = glm::normalize(glm::cross(planeData.vertices[v4] - planeData.vertices[v3], planeData.vertices[v3] - planeData.vertices[v0]));

						glm::vec3 combinedNormal = glm::normalize(triNormal1 + triNormal2 + triNormal3);

						planeData.normals[v0] = combinedNormal;
					}
					//Top edge
					else if (z == height - 1) {
						int v0 = getIndex(x, z, width);
						int v1 = getIndex(x - 1, z, width);
						int v2 = getIndex(x, z - 1, width);
						int v3 = getIndex(x + 1, z - 1, width);
						int v4 = getIndex(x + 1, z, width);

						glm::vec3 triNormal1 = glm::normalize(glm::cross(planeData.vertices[v2] - planeData.vertices[v1], planeData.vertices[v1] - planeData.vertices[v0]));
						glm::vec3 triNormal2 = glm::normalize(glm::cross(planeData.vertices[v3] - planeData.vertices[v2], planeData.vertices[v2] - planeData.vertices[v0]));
						glm::vec3 triNormal3 = glm::normalize(glm::cross(planeData.vertices[v4] - planeData.vertices[v3], planeData.vertices[v3] - planeData.vertices[v0]));

						glm::vec3 combinedNormal = glm::normalize(triNormal1 + triNormal2 + triNormal3);

						planeData.normals[v0] = combinedNormal;
					}
					//Normal case
					else {
						// Normal case, has 6 neighbors
						int v0 = getIndex(x, z, width);
						int v1 = getIndex(x, z + 1, width);
						int v2 = getIndex(x + 1, z, width);
						int v3 = getIndex(x + 1, z + 1, width);
						int v4 = getIndex(x, z - 1, width);
						int v5 = getIndex(x - 1, z, width);
						int v6 = getIndex(x - 1, z - 1, width);

						glm::vec3 triNormal1 = glm::normalize(glm::cross(planeData.vertices[v1] - planeData.vertices[v0], planeData.vertices[v2] - planeData.vertices[v0]));
						glm::vec3 triNormal2 = glm::normalize(glm::cross(planeData.vertices[v2] - planeData.vertices[v3], planeData.vertices[v3] - planeData.vertices[v0]));
						glm::vec3 triNormal3 = glm::normalize(glm::cross(planeData.vertices[v3] - planeData.vertices[v4], planeData.vertices[v4] - planeData.vertices[v0]));
						glm::vec3 triNormal4 = glm::normalize(glm::cross(planeData.vertices[v4] - planeData.vertices[v5], planeData.vertices[v5] - planeData.vertices[v0]));
						glm::vec3 triNormal5 = glm::normalize(glm::cross(planeData.vertices[v5] - planeData.vertices[v6], planeData.vertices[v6] - planeData.vertices[v0]));
						glm::vec3 triNormal6 = glm::normalize(glm::cross(planeData.vertices[v6] - planeData.vertices[v1], planeData.vertices[v1] - planeData.vertices[v0]));

						glm::vec3 combinedNormal = glm::normalize(triNormal1 + triNormal2 + triNormal3 + triNormal4 + triNormal5 + triNormal6);

						planeData.normals[v0] = combinedNormal;
					}
				}
			}


		}
	}
}