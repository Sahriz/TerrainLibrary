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
		float xScale = 1.f / width/3;
		float zScale = 1.f / height;
		for(int z = 0; z < height; ++z) {
			for(int x = 0; x < width; ++x) {
				// Calculate the vertex position
				float posX = static_cast<float>(x-width/2.0f);
				float posY = -0.1;
				float posZ = static_cast<float>(z-height/2.0f); // Assuming a flat plane at z=0
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
	struct alignas(16) Vec3Aligned {
		glm::fvec3 value;
		float padding; // Padding to ensure 16-byte alignment
	};
	void ApplyHeightMap(PlaneMesh& planeData) {
		GLuint ssbo;
		glGenBuffers(1, &ssbo);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
		glBufferData(GL_SHADER_STORAGE_BUFFER, planeData.vertices.size() * sizeof(glm::fvec3), planeData.vertices.data(), GL_DYNAMIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);

		GLuint computeShaderProgram = CreateComputeShaderProgram("../Core/Source/Core/HeightMap.txt");

		glUseProgram(computeShaderProgram);

		GLuint numGroups = (planeData.vertices.size() + 63) / 64;
		glDispatchCompute(numGroups, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
		glm::fvec3* ptr = (glm::fvec3*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

		// Copy or use data
		if (ptr) {
			planeData.vertices.assign(ptr, ptr + planeData.vertices.size());
			glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		}
		else {
			std::cerr << "Failed to map SSBO for reading!" << std::endl;
		}
		

	}

}