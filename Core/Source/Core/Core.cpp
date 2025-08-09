#include "Core.h"

namespace Core {
	//Used for functions that are not exposed to the user, but are used internally in the library. Some are CPU implementation of the GPU functions, that are meant for debugging,
	//while others are just utility functions that are used within the library.
	//If for any reason there would be a need to expose these functions, they can be moved to the Core namespace and made public. Just remember to declare them in the
	//header file Core.h.
	namespace {
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
		int getIndex(int x, int z, int width) {
			return z * width + x;
		}
		PlaneMesh CreatePlaneMeshCPU(int width, int height) {

			std::vector<glm::fvec3> vertices;
			vertices.reserve(width * height); //coordinates (x, y, z)
			std::vector<glm::fvec3> normals;
			normals.reserve(width * height); //coordinates (x, y, z)
			float xScale = 100.f / width;
			float zScale = 100.f / height;
			for (int z = 0; z < height; ++z) {
				for (int x = 0; x < width; ++x) {
					// Calculate the vertex position
					float posX = static_cast<float>(x - width / 2.0f) * xScale;
					float posY = -0.1;
					float posZ = static_cast<float>(z - height / 2.0f) * zScale; // Assuming a flat plane at z=0
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



			return PlaneMesh(vertices, indices, normals);
		}
		void CreateHeightMapPlaneMeshCPU(PlaneMesh& planeData, int height, int width) {

			for (glm::fvec3& point : planeData.vertices) {
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
					if (x > 0) {
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
		int edgeTable[] =  {
			0x0, 0x109, 0x203, 0x30a, 0x406, 0x50f, 0x605, 0x70c,
				0x80c, 0x905, 0xa0f, 0xb06, 0xc0a, 0xd03, 0xe09, 0xf00,
				0x190, 0x99, 0x393, 0x29a, 0x596, 0x49f, 0x795, 0x69c,
				0x99c, 0x895, 0xb9f, 0xa96, 0xd9a, 0xc93, 0xf99, 0xe90,
				0x230, 0x339, 0x33, 0x13a, 0x636, 0x73f, 0x435, 0x53c,
				0xa3c, 0xb35, 0x83f, 0x936, 0xe3a, 0xf33, 0xc39, 0xd30,
				0x3a0, 0x2a9, 0x1a3, 0xaa, 0x7a6, 0x6af, 0x5a5, 0x4ac,
				0xbac, 0xaa5, 0x9af, 0x8a6, 0xfaa, 0xea3, 0xda9, 0xca0,
				0x460, 0x569, 0x663, 0x76a, 0x66, 0x16f, 0x265, 0x36c,
				0xc6c, 0xd65, 0xe6f, 0xf66, 0x86a, 0x963, 0xa69, 0xb60,
				0x5f0, 0x4f9, 0x7f3, 0x6fa, 0x1f6, 0xff, 0x3f5, 0x2fc,
				0xdfc, 0xcf5, 0xfff, 0xef6, 0x9fa, 0x8f3, 0xbf9, 0xaf0,
				0x650, 0x759, 0x453, 0x55a, 0x256, 0x35f, 0x55, 0x15c,
				0xe5c, 0xf55, 0xc5f, 0xd56, 0xa5a, 0xb53, 0x859, 0x950,
				0x7c0, 0x6c9, 0x5c3, 0x4ca, 0x3c6, 0x2cf, 0x1c5, 0xcc,
				0xfcc, 0xec5, 0xdcf, 0xcc6, 0xbca, 0xac3, 0x9c9, 0x8c0,
				0x8c0, 0x9c9, 0xac3, 0xbca, 0xcc6, 0xdcf, 0xec5, 0xfcc,
				0xcc, 0x1c5, 0x2cf, 0x3c6, 0x4ca, 0x5c3, 0x6c9, 0x7c0,
				0x950, 0x859, 0xb53, 0xa5a, 0xd56, 0xc5f, 0xf55, 0xe5c,
				0x15c, 0x55, 0x35f, 0x256, 0x55a, 0x453, 0x759, 0x650,
				0xaf0, 0xbf9, 0x8f3, 0x9fa, 0xef6, 0xfff, 0xcf5, 0xdfc,
				0x2fc, 0x3f5, 0xff, 0x1f6, 0x6fa, 0x7f3, 0x4f9, 0x5f0,
				0xb60, 0xa69, 0x963, 0x86a, 0xf66, 0xe6f, 0xd65, 0xc6c,
				0x36c, 0x265, 0x16f, 0x66, 0x76a, 0x663, 0x569, 0x460,
				0xca0, 0xda9, 0xea3, 0xfaa, 0x8a6, 0x9af, 0xaa5, 0xbac,
				0x4ac, 0x5a5, 0x6af, 0x7a6, 0xaa, 0x1a3, 0x2a9, 0x3a0,
				0xd30, 0xc39, 0xf33, 0xe3a, 0x936, 0x83f, 0xb35, 0xa3c,
				0x53c, 0x435, 0x73f, 0x636, 0x13a, 0x33, 0x339, 0x230,
				0xe90, 0xf99, 0xc93, 0xd9a, 0xa96, 0xb9f, 0x895, 0x99c,
				0x69c, 0x795, 0x49f, 0x596, 0x29a, 0x393, 0x99, 0x190,
				0xf00, 0xe09, 0xd03, 0xc0a, 0xb06, 0xa0f, 0x905, 0x80c,
				0x70c, 0x605, 0x50f, 0x406, 0x30a, 0x203, 0x109, 0x0   };
		int FlatTriTable[256 * 16] = {
		 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0, 1, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		1, 8, 3, 9, 8, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0, 8, 3, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		9, 2, 10, 0, 2, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		2, 8, 3, 2, 10, 8, 10, 9, 8, -1, -1, -1, -1, -1, -1, -1,
		3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0, 11, 2, 8, 11, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		1, 9, 0, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		1, 11, 2, 1, 9, 11, 9, 8, 11, -1, -1, -1, -1, -1, -1, -1,
		3, 10, 1, 11, 10, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0, 10, 1, 0, 8, 10, 8, 11, 10, -1, -1, -1, -1, -1, -1, -1,
		3, 9, 0, 3, 11, 9, 11, 10, 9, -1, -1, -1, -1, -1, -1, -1,
		9, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		4, 3, 0, 7, 3, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0, 1, 9, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		4, 1, 9, 4, 7, 1, 7, 3, 1, -1, -1, -1, -1, -1, -1, -1,
		1, 2, 10, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		3, 4, 7, 3, 0, 4, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1,
		9, 2, 10, 9, 0, 2, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1,
		2, 10, 9, 2, 9, 7, 2, 7, 3, 7, 9, 4, -1, -1, -1, -1,
		8, 4, 7, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		11, 4, 7, 11, 2, 4, 2, 0, 4, -1, -1, -1, -1, -1, -1, -1,
		9, 0, 1, 8, 4, 7, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1,
		4, 7, 11, 9, 4, 11, 9, 11, 2, 9, 2, 1, -1, -1, -1, -1,
		3, 10, 1, 3, 11, 10, 7, 8, 4, -1, -1, -1, -1, -1, -1, -1,
		1, 11, 10, 1, 4, 11, 1, 0, 4, 7, 11, 4, -1, -1, -1, -1,
		4, 7, 8, 9, 0, 11, 9, 11, 10, 11, 0, 3, -1, -1, -1, -1,
		4, 7, 11, 4, 11, 9, 9, 11, 10, -1, -1, -1, -1, -1, -1, -1,
		9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		9, 5, 4, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0, 5, 4, 1, 5, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		8, 5, 4, 8, 3, 5, 3, 1, 5, -1, -1, -1, -1, -1, -1, -1,
		1, 2, 10, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		3, 0, 8, 1, 2, 10, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1,
		5, 2, 10, 5, 4, 2, 4, 0, 2, -1, -1, -1, -1, -1, -1, -1,
		2, 10, 5, 3, 2, 5, 3, 5, 4, 3, 4, 8, -1, -1, -1, -1,
		9, 5, 4, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0, 11, 2, 0, 8, 11, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1,
		0, 5, 4, 0, 1, 5, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1,
		2, 1, 5, 2, 5, 8, 2, 8, 11, 4, 8, 5, -1, -1, -1, -1,
		10, 3, 11, 10, 1, 3, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1,
		4, 9, 5, 0, 8, 1, 8, 10, 1, 8, 11, 10, -1, -1, -1, -1,
		5, 4, 0, 5, 0, 11, 5, 11, 10, 11, 0, 3, -1, -1, -1, -1,
		5, 4, 8, 5, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1,
		9, 7, 8, 5, 7, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		9, 3, 0, 9, 5, 3, 5, 7, 3, -1, -1, -1, -1, -1, -1, -1,
		0, 7, 8, 0, 1, 7, 1, 5, 7, -1, -1, -1, -1, -1, -1, -1,
		1, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		9, 7, 8, 9, 5, 7, 10, 1, 2, -1, -1, -1, -1, -1, -1, -1,
		10, 1, 2, 9, 5, 0, 5, 3, 0, 5, 7, 3, -1, -1, -1, -1,
		8, 0, 2, 8, 2, 5, 8, 5, 7, 10, 5, 2, -1, -1, -1, -1,
		2, 10, 5, 2, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1,
		7, 9, 5, 7, 8, 9, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1,
		9, 5, 7, 9, 7, 2, 9, 2, 0, 2, 7, 11, -1, -1, -1, -1,
		2, 3, 11, 0, 1, 8, 1, 7, 8, 1, 5, 7, -1, -1, -1, -1,
		11, 2, 1, 11, 1, 7, 7, 1, 5, -1, -1, -1, -1, -1, -1, -1,
		9, 5, 8, 8, 5, 7, 10, 1, 3, 10, 3, 11, -1, -1, -1, -1,
		5, 7, 0, 5, 0, 9, 7, 11, 0, 1, 0, 10, 11, 10, 0, -1,
		11, 10, 0, 11, 0, 3, 10, 5, 0, 8, 0, 7, 5, 7, 0, -1,
		11, 10, 5, 7, 11, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0, 8, 3, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		9, 0, 1, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		1, 8, 3, 1, 9, 8, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1,
		1, 6, 5, 2, 6, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		1, 6, 5, 1, 2, 6, 3, 0, 8, -1, -1, -1, -1, -1, -1, -1,
		9, 6, 5, 9, 0, 6, 0, 2, 6, -1, -1, -1, -1, -1, -1, -1,
		5, 9, 8, 5, 8, 2, 5, 2, 6, 3, 2, 8, -1, -1, -1, -1,
		2, 3, 11, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		11, 0, 8, 11, 2, 0, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1,
		0, 1, 9, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1,
		5, 10, 6, 1, 9, 2, 9, 11, 2, 9, 8, 11, -1, -1, -1, -1,
		6, 3, 11, 6, 5, 3, 5, 1, 3, -1, -1, -1, -1, -1, -1, -1,
		0, 8, 11, 0, 11, 5, 0, 5, 1, 5, 11, 6, -1, -1, -1, -1,
		3, 11, 6, 0, 3, 6, 0, 6, 5, 0, 5, 9, -1, -1, -1, -1,
		6, 5, 9, 6, 9, 11, 11, 9, 8, -1, -1, -1, -1, -1, -1, -1,
		5, 10, 6, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		4, 3, 0, 4, 7, 3, 6, 5, 10, -1, -1, -1, -1, -1, -1, -1,
		1, 9, 0, 5, 10, 6, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1,
		10, 6, 5, 1, 9, 7, 1, 7, 3, 7, 9, 4, -1, -1, -1, -1,
		6, 1, 2, 6, 5, 1, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1,
		1, 2, 5, 5, 2, 6, 3, 0, 4, 3, 4, 7, -1, -1, -1, -1,
		8, 4, 7, 9, 0, 5, 0, 6, 5, 0, 2, 6, -1, -1, -1, -1,
		7, 3, 9, 7, 9, 4, 3, 2, 9, 5, 9, 6, 2, 6, 9, -1,
		3, 11, 2, 7, 8, 4, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1,
		5, 10, 6, 4, 7, 2, 4, 2, 0, 2, 7, 11, -1, -1, -1, -1,
		0, 1, 9, 4, 7, 8, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1,
		9, 2, 1, 9, 11, 2, 9, 4, 11, 7, 11, 4, 5, 10, 6, -1,
		8, 4, 7, 3, 11, 5, 3, 5, 1, 5, 11, 6, -1, -1, -1, -1,
		5, 1, 11, 5, 11, 6, 1, 0, 11, 7, 11, 4, 0, 4, 11, -1,
		0, 5, 9, 0, 6, 5, 0, 3, 6, 11, 6, 3, 8, 4, 7, -1,
		6, 5, 9, 6, 9, 11, 4, 7, 9, 7, 11, 9, -1, -1, -1, -1,
		10, 4, 9, 6, 4, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		4, 10, 6, 4, 9, 10, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1,
		10, 0, 1, 10, 6, 0, 6, 4, 0, -1, -1, -1, -1, -1, -1, -1,
		8, 3, 1, 8, 1, 6, 8, 6, 4, 6, 1, 10, -1, -1, -1, -1,
		1, 4, 9, 1, 2, 4, 2, 6, 4, -1, -1, -1, -1, -1, -1, -1,
		3, 0, 8, 1, 2, 9, 2, 4, 9, 2, 6, 4, -1, -1, -1, -1,
		0, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		8, 3, 2, 8, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1,
		10, 4, 9, 10, 6, 4, 11, 2, 3, -1, -1, -1, -1, -1, -1, -1,
		0, 8, 2, 2, 8, 11, 4, 9, 10, 4, 10, 6, -1, -1, -1, -1,
		3, 11, 2, 0, 1, 6, 0, 6, 4, 6, 1, 10, -1, -1, -1, -1,
		6, 4, 1, 6, 1, 10, 4, 8, 1, 2, 1, 11, 8, 11, 1, -1,
		9, 6, 4, 9, 3, 6, 9, 1, 3, 11, 6, 3, -1, -1, -1, -1,
		8, 11, 1, 8, 1, 0, 11, 6, 1, 9, 1, 4, 6, 4, 1, -1,
		3, 11, 6, 3, 6, 0, 0, 6, 4, -1, -1, -1, -1, -1, -1, -1,
		6, 4, 8, 11, 6, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		7, 10, 6, 7, 8, 10, 8, 9, 10, -1, -1, -1, -1, -1, -1, -1,
		0, 7, 3, 0, 10, 7, 0, 9, 10, 6, 7, 10, -1, -1, -1, -1,
		10, 6, 7, 1, 10, 7, 1, 7, 8, 1, 8, 0, -1, -1, -1, -1,
		10, 6, 7, 10, 7, 1, 1, 7, 3, -1, -1, -1, -1, -1, -1, -1,
		1, 2, 6, 1, 6, 8, 1, 8, 9, 8, 6, 7, -1, -1, -1, -1,
		2, 6, 9, 2, 9, 1, 6, 7, 9, 0, 9, 3, 7, 3, 9, -1,
		7, 8, 0, 7, 0, 6, 6, 0, 2, -1, -1, -1, -1, -1, -1, -1,
		7, 3, 2, 6, 7, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		2, 3, 11, 10, 6, 8, 10, 8, 9, 8, 6, 7, -1, -1, -1, -1,
		2, 0, 7, 2, 7, 11, 0, 9, 7, 6, 7, 10, 9, 10, 7, -1,
		1, 8, 0, 1, 7, 8, 1, 10, 7, 6, 7, 10, 2, 3, 11, -1,
		11, 2, 1, 11, 1, 7, 10, 6, 1, 6, 7, 1, -1, -1, -1, -1,
		8, 9, 6, 8, 6, 7, 9, 1, 6, 11, 6, 3, 1, 3, 6, -1,
		0, 9, 1, 11, 6, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		7, 8, 0, 7, 0, 6, 3, 11, 0, 11, 6, 0, -1, -1, -1, -1,
		7, 11, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		3, 0, 8, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0, 1, 9, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		8, 1, 9, 8, 3, 1, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1,
		10, 1, 2, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		1, 2, 10, 3, 0, 8, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1,
		2, 9, 0, 2, 10, 9, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1,
		6, 11, 7, 2, 10, 3, 10, 8, 3, 10, 9, 8, -1, -1, -1, -1,
		7, 2, 3, 6, 2, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		7, 0, 8, 7, 6, 0, 6, 2, 0, -1, -1, -1, -1, -1, -1, -1,
		2, 7, 6, 2, 3, 7, 0, 1, 9, -1, -1, -1, -1, -1, -1, -1,
		1, 6, 2, 1, 8, 6, 1, 9, 8, 8, 7, 6, -1, -1, -1, -1,
		10, 7, 6, 10, 1, 7, 1, 3, 7, -1, -1, -1, -1, -1, -1, -1,
		10, 7, 6, 1, 7, 10, 1, 8, 7, 1, 0, 8, -1, -1, -1, -1,
		0, 3, 7, 0, 7, 10, 0, 10, 9, 6, 10, 7, -1, -1, -1, -1,
		7, 6, 10, 7, 10, 8, 8, 10, 9, -1, -1, -1, -1, -1, -1, -1,
		6, 8, 4, 11, 8, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		3, 6, 11, 3, 0, 6, 0, 4, 6, -1, -1, -1, -1, -1, -1, -1,
		8, 6, 11, 8, 4, 6, 9, 0, 1, -1, -1, -1, -1, -1, -1, -1,
		9, 4, 6, 9, 6, 3, 9, 3, 1, 11, 3, 6, -1, -1, -1, -1,
		6, 8, 4, 6, 11, 8, 2, 10, 1, -1, -1, -1, -1, -1, -1, -1,
		1, 2, 10, 3, 0, 11, 0, 6, 11, 0, 4, 6, -1, -1, -1, -1,
		4, 11, 8, 4, 6, 11, 0, 2, 9, 2, 10, 9, -1, -1, -1, -1,
		10, 9, 3, 10, 3, 2, 9, 4, 3, 11, 3, 6, 4, 6, 3, -1,
		8, 2, 3, 8, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1,
		0, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		1, 9, 0, 2, 3, 4, 2, 4, 6, 4, 3, 8, -1, -1, -1, -1,
		1, 9, 4, 1, 4, 2, 2, 4, 6, -1, -1, -1, -1, -1, -1, -1,
		8, 1, 3, 8, 6, 1, 8, 4, 6, 6, 10, 1, -1, -1, -1, -1,
		10, 1, 0, 10, 0, 6, 6, 0, 4, -1, -1, -1, -1, -1, -1, -1,
		4, 6, 3, 4, 3, 8, 6, 10, 3, 0, 3, 9, 10, 9, 3, -1,
		10, 9, 4, 6, 10, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		4, 9, 5, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0, 8, 3, 4, 9, 5, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1,
		5, 0, 1, 5, 4, 0, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1,
		11, 7, 6, 8, 3, 4, 3, 5, 4, 3, 1, 5, -1, -1, -1, -1,
		9, 5, 4, 10, 1, 2, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1,
		6, 11, 7, 1, 2, 10, 0, 8, 3, 4, 9, 5, -1, -1, -1, -1,
		7, 6, 11, 5, 4, 10, 4, 2, 10, 4, 0, 2, -1, -1, -1, -1,
		3, 4, 8, 3, 5, 4, 3, 2, 5, 10, 5, 2, 11, 7, 6, -1,
		7, 2, 3, 7, 6, 2, 5, 4, 9, -1, -1, -1, -1, -1, -1, -1,
		9, 5, 4, 0, 8, 6, 0, 6, 2, 6, 8, 7, -1, -1, -1, -1,
		3, 6, 2, 3, 7, 6, 1, 5, 0, 5, 4, 0, -1, -1, -1, -1,
		6, 2, 8, 6, 8, 7, 2, 1, 8, 4, 8, 5, 1, 5, 8, -1,
		9, 5, 4, 10, 1, 6, 1, 7, 6, 1, 3, 7, -1, -1, -1, -1,
		1, 6, 10, 1, 7, 6, 1, 0, 7, 8, 7, 0, 9, 5, 4, -1,
		4, 0, 10, 4, 10, 5, 0, 3, 10, 6, 10, 7, 3, 7, 10, -1,
		7, 6, 10, 7, 10, 8, 5, 4, 10, 4, 8, 10, -1, -1, -1, -1,
		6, 9, 5, 6, 11, 9, 11, 8, 9, -1, -1, -1, -1, -1, -1, -1,
		3, 6, 11, 0, 6, 3, 0, 5, 6, 0, 9, 5, -1, -1, -1, -1,
		0, 11, 8, 0, 5, 11, 0, 1, 5, 5, 6, 11, -1, -1, -1, -1,
		6, 11, 3, 6, 3, 5, 5, 3, 1, -1, -1, -1, -1, -1, -1, -1,
		1, 2, 10, 9, 5, 11, 9, 11, 8, 11, 5, 6, -1, -1, -1, -1,
		0, 11, 3, 0, 6, 11, 0, 9, 6, 5, 6, 9, 1, 2, 10, -1,
		11, 8, 5, 11, 5, 6, 8, 0, 5, 10, 5, 2, 0, 2, 5, -1,
		6, 11, 3, 6, 3, 5, 2, 10, 3, 10, 5, 3, -1, -1, -1, -1,
		5, 8, 9, 5, 2, 8, 5, 6, 2, 3, 8, 2, -1, -1, -1, -1,
		9, 5, 6, 9, 6, 0, 0, 6, 2, -1, -1, -1, -1, -1, -1, -1,
		1, 5, 8, 1, 8, 0, 5, 6, 8, 3, 8, 2, 6, 2, 8, -1,
		1, 5, 6, 2, 1, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		1, 3, 6, 1, 6, 10, 3, 8, 6, 5, 6, 9, 8, 9, 6, -1,
		10, 1, 0, 10, 0, 6, 9, 5, 0, 5, 6, 0, -1, -1, -1, -1,
		0, 3, 8, 5, 6, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		10, 5, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		11, 5, 10, 7, 5, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		11, 5, 10, 11, 7, 5, 8, 3, 0, -1, -1, -1, -1, -1, -1, -1,
		5, 11, 7, 5, 10, 11, 1, 9, 0, -1, -1, -1, -1, -1, -1, -1,
		10, 7, 5, 10, 11, 7, 9, 8, 1, 8, 3, 1, -1, -1, -1, -1,
		11, 1, 2, 11, 7, 1, 7, 5, 1, -1, -1, -1, -1, -1, -1, -1,
		0, 8, 3, 1, 2, 7, 1, 7, 5, 7, 2, 11, -1, -1, -1, -1,
		9, 7, 5, 9, 2, 7, 9, 0, 2, 2, 11, 7, -1, -1, -1, -1,
		7, 5, 2, 7, 2, 11, 5, 9, 2, 3, 2, 8, 9, 8, 2, -1,
		2, 5, 10, 2, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1,
		8, 2, 0, 8, 5, 2, 8, 7, 5, 10, 2, 5, -1, -1, -1, -1,
		9, 0, 1, 5, 10, 3, 5, 3, 7, 3, 10, 2, -1, -1, -1, -1,
		9, 8, 2, 9, 2, 1, 8, 7, 2, 10, 2, 5, 7, 5, 2, -1,
		1, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0, 8, 7, 0, 7, 1, 1, 7, 5, -1, -1, -1, -1, -1, -1, -1,
		9, 0, 3, 9, 3, 5, 5, 3, 7, -1, -1, -1, -1, -1, -1, -1,
		9, 8, 7, 5, 9, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		5, 8, 4, 5, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1,
		5, 0, 4, 5, 11, 0, 5, 10, 11, 11, 3, 0, -1, -1, -1, -1,
		0, 1, 9, 8, 4, 10, 8, 10, 11, 10, 4, 5, -1, -1, -1, -1,
		10, 11, 4, 10, 4, 5, 11, 3, 4, 9, 4, 1, 3, 1, 4, -1,
		2, 5, 1, 2, 8, 5, 2, 11, 8, 4, 5, 8, -1, -1, -1, -1,
		0, 4, 11, 0, 11, 3, 4, 5, 11, 2, 11, 1, 5, 1, 11, -1,
		0, 2, 5, 0, 5, 9, 2, 11, 5, 4, 5, 8, 11, 8, 5, -1,
		9, 4, 5, 2, 11, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		2, 5, 10, 3, 5, 2, 3, 4, 5, 3, 8, 4, -1, -1, -1, -1,
		5, 10, 2, 5, 2, 4, 4, 2, 0, -1, -1, -1, -1, -1, -1, -1,
		3, 10, 2, 3, 5, 10, 3, 8, 5, 4, 5, 8, 0, 1, 9, -1,
		5, 10, 2, 5, 2, 4, 1, 9, 2, 9, 4, 2, -1, -1, -1, -1,
		8, 4, 5, 8, 5, 3, 3, 5, 1, -1, -1, -1, -1, -1, -1, -1,
		0, 4, 5, 1, 0, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		8, 4, 5, 8, 5, 3, 9, 0, 5, 0, 3, 5, -1, -1, -1, -1,
		9, 4, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		4, 11, 7, 4, 9, 11, 9, 10, 11, -1, -1, -1, -1, -1, -1, -1,
		0, 8, 3, 4, 9, 7, 9, 11, 7, 9, 10, 11, -1, -1, -1, -1,
		1, 10, 11, 1, 11, 4, 1, 4, 0, 7, 4, 11, -1, -1, -1, -1,
		3, 1, 4, 3, 4, 8, 1, 10, 4, 7, 4, 11, 10, 11, 4, -1,
		4, 11, 7, 9, 11, 4, 9, 2, 11, 9, 1, 2, -1, -1, -1, -1,
		9, 7, 4, 9, 11, 7, 9, 1, 11, 2, 11, 1, 0, 8, 3, -1,
		11, 7, 4, 11, 4, 2, 2, 4, 0, -1, -1, -1, -1, -1, -1, -1,
		11, 7, 4, 11, 4, 2, 8, 3, 4, 3, 2, 4, -1, -1, -1, -1,
		2, 9, 10, 2, 7, 9, 2, 3, 7, 7, 4, 9, -1, -1, -1, -1,
		9, 10, 7, 9, 7, 4, 10, 2, 7, 8, 7, 0, 2, 0, 7, -1,
		3, 7, 10, 3, 10, 2, 7, 4, 10, 1, 10, 0, 4, 0, 10, -1,
		1, 10, 2, 8, 7, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		4, 9, 1, 4, 1, 7, 7, 1, 3, -1, -1, -1, -1, -1, -1, -1,
		4, 9, 1, 4, 1, 7, 0, 8, 1, 8, 7, 1, -1, -1, -1, -1,
		4, 0, 3, 7, 4, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		4, 8, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		9, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		3, 0, 9, 3, 9, 11, 11, 9, 10, -1, -1, -1, -1, -1, -1, -1,
		0, 1, 10, 0, 10, 8, 8, 10, 11, -1, -1, -1, -1, -1, -1, -1,
		3, 1, 10, 11, 3, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		1, 2, 11, 1, 11, 9, 9, 11, 8, -1, -1, -1, -1, -1, -1, -1,
		3, 0, 9, 3, 9, 11, 1, 2, 9, 2, 11, 9, -1, -1, -1, -1,
		0, 2, 11, 8, 0, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		3, 2, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		2, 3, 8, 2, 8, 10, 10, 8, 9, -1, -1, -1, -1, -1, -1, -1,
		9, 10, 2, 0, 9, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		2, 3, 8, 2, 8, 10, 0, 1, 8, 1, 10, 8, -1, -1, -1, -1,
		1, 10, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		1, 3, 8, 9, 1, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0, 9, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0, 3, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };	
	}
	GLuint _vertexInitComputeShaderProgram = 0;
	GLuint _indexInitComputeShaderProgram = 0;
	GLuint _vertexDisplacementComputeShaderProgram = 0;
	GLuint _normalInterpelationComputeShaderProgram = 0;
	GLuint _3dNoiseMapComputeShader = 0;
	GLuint _marchingCubesTriCounterComputeShader = 0;
	GLuint _marchingCubesTriCreatorComputeShader = 0;
	GLuint _voxelCubesGeometryInitComputeShader = 0;
	GLuint _voxelCubesTriangleCounterComputeShader = 0;
	GLuint _normalisedNoiseMapComputeShader = 0;
	GLuint _splineNoiseEvaluationComputeShader = 0;
	void Init() {
		_vertexInitComputeShaderProgram = CreateComputeShaderProgram("../Core/Source/Core/HeightMapVertexInit.comp");
		_indexInitComputeShaderProgram = CreateComputeShaderProgram("../Core/Source/Core/HeightMapIndexInit.comp");
		_vertexDisplacementComputeShaderProgram = CreateComputeShaderProgram("../Core/Source/Core/HeightMapVertexDisplacement.comp");
		_normalInterpelationComputeShaderProgram = CreateComputeShaderProgram("../Core/Source/Core/HeightMapNormal.comp");
		_3dNoiseMapComputeShader = CreateComputeShaderProgram("../Core/Source/Core/Create3DNoise.comp");
		_marchingCubesTriCounterComputeShader = CreateComputeShaderProgram("../Core/Source/Core/MarchingCubesCountTris.comp");
		_marchingCubesTriCreatorComputeShader = CreateComputeShaderProgram("../Core/Source/Core/MarchingCubesCreateTris.comp");
		_voxelCubesGeometryInitComputeShader = CreateComputeShaderProgram("../Core/Source/Core/VoxelCubesGeometryInit.comp");
		_voxelCubesTriangleCounterComputeShader = CreateComputeShaderProgram("../Core/Source/Core/VoxelCubesCountTriangles.comp");
		_normalisedNoiseMapComputeShader = CreateComputeShaderProgram("../Core/Source/Core/NormaliseNoiseValues.comp");
		_splineNoiseEvaluationComputeShader = CreateComputeShaderProgram("../Core/Source/Core/SplineNoiseEvaluation.comp");
	}
	void Cleanup() {
		glDeleteProgram(_vertexInitComputeShaderProgram);
		glDeleteProgram(_indexInitComputeShaderProgram);
		glDeleteProgram(_vertexDisplacementComputeShaderProgram);
		glDeleteProgram(_normalInterpelationComputeShaderProgram);
		glDeleteProgram(_3dNoiseMapComputeShader);
		glDeleteProgram(_marchingCubesTriCounterComputeShader);
		glDeleteProgram(_marchingCubesTriCreatorComputeShader);
		glDeleteProgram(_voxelCubesGeometryInitComputeShader);
		glDeleteProgram(_voxelCubesTriangleCounterComputeShader);
		glDeleteProgram(_normalisedNoiseMapComputeShader);
		glDeleteProgram(_splineNoiseEvaluationComputeShader);
	}

	std::vector<float> CreateFlat2DNoiseMap(const int width, const int height, const int depth, const glm::vec2 offset, bool CleanUp) {
		std::vector<float> noiseMap;
		int paddedWidth = width + 1;
		int paddedHeight = height + 1;
		int sizeOfNoiseMap = paddedWidth + paddedHeight;
		noiseMap.resize(sizeOfNoiseMap);

		GLuint ssboNoise;
		glGenBuffers(1, &ssboNoise);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboNoise);
		glBufferData(GL_SHADER_STORAGE_BUFFER, noiseMap.size() * sizeof(float), noiseMap.data(), GL_DYNAMIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboNoise);

		GLint widthLoc = glGetUniformLocation(_vertexInitComputeShaderProgram, "width");
		GLint heightLoc = glGetUniformLocation(_vertexInitComputeShaderProgram, "height");
		GLint offsetLoc = glGetUniformLocation(_vertexInitComputeShaderProgram, "offset");

		glUseProgram(_vertexInitComputeShaderProgram);

		glUniform1i(widthLoc, width);
		glUniform1i(heightLoc, height);
		glUniform2fv(offsetLoc, 1, &offset[0]);

		return noiseMap;
	}
	std::vector<float> CreateFlat3DNoiseMap(const int width,const int height,const int depth,const glm::vec3 offset, bool CleanUp, const float amplitude, const float frequency, const float persistance, const float lacunarity, const int octaves, const bool useDropoff) {
		std::vector<float> noiseMap;
		int sizeOfNoiseMap = width * height * depth;
		noiseMap.resize(sizeOfNoiseMap);

		GLuint ssboNoise;
		glGenBuffers(1, &ssboNoise);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboNoise);
		glBufferData(GL_SHADER_STORAGE_BUFFER, noiseMap.size() * sizeof(float), noiseMap.data(), GL_DYNAMIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboNoise);

		GLint widthLoc = glGetUniformLocation(_3dNoiseMapComputeShader, "width");
		GLint heightLoc = glGetUniformLocation(_3dNoiseMapComputeShader, "height");
		GLint depthLoc = glGetUniformLocation(_3dNoiseMapComputeShader, "depth");
		GLint offsetLoc = glGetUniformLocation(_3dNoiseMapComputeShader, "offset");
		GLint amiplitudeLoc = glGetUniformLocation(_3dNoiseMapComputeShader, "amplitude");
		GLint frequencyLoc = glGetUniformLocation(_3dNoiseMapComputeShader, "frequency");
		GLint persistanceLoc = glGetUniformLocation(_3dNoiseMapComputeShader, "persistance");
		GLint lacunarityLoc = glGetUniformLocation(_3dNoiseMapComputeShader, "lacunarity");
		GLint octavesLoc = glGetUniformLocation(_3dNoiseMapComputeShader, "octaves");
		GLint dropoffLoc = glGetUniformLocation(_3dNoiseMapComputeShader, "useHeightDropoff");

		glUseProgram(_3dNoiseMapComputeShader);

		glUniform1i(widthLoc, width);
		glUniform1i(heightLoc, height);
		glUniform1i(depthLoc, depth);
		glUniform3fv(offsetLoc, 1, &offset[0]);
		glUniform1f(amiplitudeLoc, amplitude);
		glUniform1f(frequencyLoc, frequency);
		glUniform1f(persistanceLoc, persistance);
		glUniform1f(lacunarityLoc, lacunarity);
		glUniform1i(octavesLoc, octaves);
		glUniform1i(dropoffLoc, useDropoff);

		glDispatchCompute(
			(GLuint)ceil(width / 8.0f),
			(GLuint)ceil(height / 8.0f),
			(GLuint)ceil(depth / 8.0f)
		);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboNoise);
		float* ptr = (float*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
		// Copy or use data
		if (ptr) {
			noiseMap.assign(ptr, ptr + noiseMap.size());
			glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		}
		else {
			std::cout << "Something went wrong in CreateVertices";
		}
		glDeleteBuffers(1, &ssboNoise);
		return noiseMap;
	}
	void CreateFlat3DNoiseMap(NoiseMapData& noiseMapData, const int width, const int height, const int depth, const glm::vec3 offset, bool CleanUp, const float amplitude, const float frequency, const float persistance, const float lacunarity, const int octaves, const bool useDropoff) {
		
		int sizeOfNoiseMap = width * height * depth;
		noiseMapData.noiseMap.resize(sizeOfNoiseMap);

		GLuint ssboNoise;
		glGenBuffers(1, &ssboNoise);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboNoise);
		glBufferData(GL_SHADER_STORAGE_BUFFER, noiseMapData.noiseMap.size() * sizeof(float), noiseMapData.noiseMap.data(), GL_DYNAMIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboNoise);
		
		GLuint ssboMaxValue;
		float maxValue = -1000.0f;
		glGenBuffers(1, &ssboMaxValue);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboMaxValue);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float), &maxValue, GL_DYNAMIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboMaxValue);

		GLuint ssboMinValue;
		float minValue = 1000.0f;
		glGenBuffers(1, &ssboMinValue);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboMinValue);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float), &minValue, GL_DYNAMIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssboMinValue);

		GLint widthLoc = glGetUniformLocation(_3dNoiseMapComputeShader, "width");
		GLint heightLoc = glGetUniformLocation(_3dNoiseMapComputeShader, "height");
		GLint depthLoc = glGetUniformLocation(_3dNoiseMapComputeShader, "depth");
		GLint offsetLoc = glGetUniformLocation(_3dNoiseMapComputeShader, "offset");
		GLint amiplitudeLoc = glGetUniformLocation(_3dNoiseMapComputeShader, "amplitude");
		GLint frequencyLoc = glGetUniformLocation(_3dNoiseMapComputeShader, "frequency");
		GLint persistanceLoc = glGetUniformLocation(_3dNoiseMapComputeShader, "persistance");
		GLint lacunarityLoc = glGetUniformLocation(_3dNoiseMapComputeShader, "lacunarity");
		GLint octavesLoc = glGetUniformLocation(_3dNoiseMapComputeShader, "octaves");
		GLint dropoffLoc = glGetUniformLocation(_3dNoiseMapComputeShader, "useHeightDropoff");

		glUseProgram(_3dNoiseMapComputeShader);

		glUniform1i(widthLoc, width);
		glUniform1i(heightLoc, height);
		glUniform1i(depthLoc, depth);
		glUniform3fv(offsetLoc, 1, &offset[0]);
		glUniform1f(amiplitudeLoc, amplitude);
		glUniform1f(frequencyLoc, frequency);
		glUniform1f(persistanceLoc, persistance);
		glUniform1f(lacunarityLoc, lacunarity);
		glUniform1i(octavesLoc, octaves);
		glUniform1i(dropoffLoc, useDropoff);

		glDispatchCompute(
			(GLuint)ceil(width / 8.0f),
			(GLuint)ceil(height / 8.0f),
			(GLuint)ceil(depth / 8.0f)
		);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboNoise);
		float* ptr = (float*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
		// Copy or use data
		if (ptr) {
			noiseMapData.noiseMap.assign(ptr, ptr + noiseMapData.noiseMap.size());
			glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		}
		else {
			std::cout << "Something went wrong in CreateVertices";
		}
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboMaxValue);
		float* maxPtr = (float*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
		if (maxPtr) {
			noiseMapData.maxValue = *maxPtr;
			glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		}
		else {
			std::cout << "Something went wrong in CreateVertices";
		}
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboMinValue);
		float* minPtr = (float*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
		if (minPtr) {
			noiseMapData.minValue = *minPtr;
			glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		}
		else {
			std::cout << "Something went wrong in CreateVertices";
		}

		glDeleteBuffers(1, &ssboNoise);
		glDeleteBuffers(1, &ssboMaxValue);
		glDeleteBuffers(1, &ssboMinValue);
	}
	void NormaliseNoiseMap(NoiseMapData& noiseMapData, int width, int height, int depth, bool CleanUp) {
		GLuint ssboNoise;
		glGenBuffers(1, &ssboNoise);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboNoise);
		glBufferData(GL_SHADER_STORAGE_BUFFER, noiseMapData.noiseMap.size() * sizeof(float), noiseMapData.noiseMap.data(), GL_DYNAMIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboNoise);

		GLint widthLoc = glGetUniformLocation(_normalisedNoiseMapComputeShader, "width");
		GLint heightLoc = glGetUniformLocation(_normalisedNoiseMapComputeShader, "height");
		GLint depthLoc = glGetUniformLocation(_normalisedNoiseMapComputeShader, "depth");
		GLint maxLoc = glGetUniformLocation(_normalisedNoiseMapComputeShader, "maxValue");
		GLint minLoc = glGetUniformLocation(_normalisedNoiseMapComputeShader, "minValue");

		glUseProgram(_normalisedNoiseMapComputeShader);

		glUniform1i(widthLoc, width);
		glUniform1i(heightLoc, height);
		glUniform1i(depthLoc, depth);
		glUniform1f(maxLoc, noiseMapData.maxValue);
		glUniform1f(minLoc, noiseMapData.minValue);

		glDispatchCompute(
			(GLuint)ceil(width / 8.0f),
			(GLuint)ceil(height / 8.0f),
			(GLuint)ceil(depth / 8.0f)
		);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboNoise);
		float* ptr = (float*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
		// Copy or use data
		if (ptr) {
			noiseMapData.noiseMap.assign(ptr, ptr + noiseMapData.noiseMap.size());
			glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
			glDeleteBuffers(1, &ssboNoise);
		}
		else {
			std::cout << "Something went wrong in CreateVertices";
		}
		
	}
	void SampleSplineCurve(NoiseMapData& noiseMapData, int width, int height, int depth,const Spline& spline) {
		GLuint ssboNoise;
		GLuint ssboSplinePoints;

		glGenBuffers(1, &ssboNoise);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboNoise);
		glBufferData(GL_SHADER_STORAGE_BUFFER, noiseMapData.noiseMap.size() * sizeof(float), noiseMapData.noiseMap.data(), GL_DYNAMIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboNoise);

		glGenBuffers(1, &ssboSplinePoints);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboSplinePoints);
		glBufferData(GL_SHADER_STORAGE_BUFFER, spline.points.size() * sizeof(glm::vec2), spline.points.data(), GL_DYNAMIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboSplinePoints);

		GLint widthLoc = glGetUniformLocation(_splineNoiseEvaluationComputeShader, "width");
		GLint heightLoc = glGetUniformLocation(_splineNoiseEvaluationComputeShader, "height");
		GLint depthLoc = glGetUniformLocation(_splineNoiseEvaluationComputeShader, "depth");
		GLint splinePointsLoc = glGetUniformLocation(_splineNoiseEvaluationComputeShader, "splinePointsCount");
		glUseProgram(_splineNoiseEvaluationComputeShader);

		glUniform1i(widthLoc, width);
		glUniform1i(heightLoc, height);
		glUniform1i(depthLoc, depth);
		glUniform1i(splinePointsLoc, (int)spline.points.size());

		glDispatchCompute(
			(GLuint)ceil(width / 8.0f),
			(GLuint)ceil(height / 8.0f),
			(GLuint)ceil(depth / 8.0f)
		);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboNoise);
		float* ptr = (float*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
		// Copy or use data
		if (ptr) {
			noiseMapData.noiseMap.assign(ptr, ptr + noiseMapData.noiseMap.size());
			glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
			glDeleteBuffers(1, &ssboNoise);
			glDeleteBuffers(1, &ssboSplinePoints);
		}
		else {
			std::cout << "Something went wrong in CreateVertices";
		}
	}

	void CreateVertices(PlaneMesh& planeData, int width, int height, glm::ivec2 offset, bool CleanUp) {
		GLuint ssboVertices;
		glGenBuffers(1, &ssboVertices);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboVertices);
		glBufferData(GL_SHADER_STORAGE_BUFFER, planeData.vertices.size() * 3 * sizeof(float), planeData.vertices.data(), GL_DYNAMIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboVertices);



		GLint widthLoc = glGetUniformLocation(_vertexInitComputeShaderProgram, "width");
		GLint heightLoc = glGetUniformLocation(_vertexInitComputeShaderProgram, "height");
		GLint offsetLoc = glGetUniformLocation(_vertexInitComputeShaderProgram, "offset");

		glUseProgram(_vertexInitComputeShaderProgram);

		glUniform1i(widthLoc, width);
		glUniform1i(heightLoc, height);
		glUniform2iv(offsetLoc, 1, &offset[0]);

		glDispatchCompute((GLuint)ceil(width / 16.0f),
			(GLuint)ceil(height / 16.0f), 1);
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
		glDeleteBuffers(1, &ssboVertices);

			
	}

	void CreateIndices(PlaneMesh& planeData, int width, int height, bool CleanUp) {
		GLuint ssboIndices;
		glGenBuffers(1, &ssboIndices);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboIndices);
		glBufferData(GL_SHADER_STORAGE_BUFFER, planeData.indices.size() * sizeof(int), planeData.indices.data(), GL_DYNAMIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboIndices);

		

		GLint widthLoc = glGetUniformLocation(_indexInitComputeShaderProgram, "width");
		GLint heightLoc = glGetUniformLocation(_indexInitComputeShaderProgram, "height");

		glUseProgram(_indexInitComputeShaderProgram);

		glUniform1i(widthLoc, width);
		glUniform1i(heightLoc, height);

		GLuint numGroups = (((width) * (height)) + 63) / 64;
		glDispatchCompute((GLuint)ceil(width / 16.0f),
			(GLuint)ceil(height / 16.0f), 1);
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
		glDeleteBuffers(1, &ssboIndices);

		
	}
	
	void DisplaceVertices(PlaneMesh& planeData, int width, int height, float scale, float amplitude, float frequency, int octaves, float persistance, float lacunarity, bool CleanUp) {
		GLuint ssboVertices;
		if ((width + 1) * (height + 1) != planeData.vertices.size()) {
			std::cout << "wrong sizes!";
		}
		glGenBuffers(1, &ssboVertices);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboVertices);
		glBufferData(GL_SHADER_STORAGE_BUFFER, planeData.vertices.size() * 3 * sizeof(float), planeData.vertices.data(), GL_DYNAMIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboVertices);

		GLint widthLoc = glGetUniformLocation(_vertexDisplacementComputeShaderProgram, "width");
		GLint heightLoc = glGetUniformLocation(_vertexDisplacementComputeShaderProgram, "height");
		GLint scaleLoc = glGetUniformLocation(_vertexDisplacementComputeShaderProgram, "scale");
		GLint amplitudeLoc = glGetUniformLocation(_vertexDisplacementComputeShaderProgram, "amplitude");
		GLint frequencyLoc = glGetUniformLocation(_vertexDisplacementComputeShaderProgram, "frequency");
		GLint octavesLoc = glGetUniformLocation(_vertexDisplacementComputeShaderProgram, "octaves");
		GLint persistanceLoc = glGetUniformLocation(_vertexDisplacementComputeShaderProgram, "persistance");
		GLint lacunarityLoc = glGetUniformLocation(_vertexDisplacementComputeShaderProgram, "lacunarity");

		glUseProgram(_vertexDisplacementComputeShaderProgram);

		glUniform1i(widthLoc, width);
		glUniform1i(heightLoc, height);
		glUniform1f(scaleLoc, scale);
		glUniform1f(amplitudeLoc, amplitude);
		glUniform1f(frequencyLoc, frequency);
		glUniform1i(octavesLoc, octaves);
		glUniform1f(persistanceLoc, persistance);
		glUniform1f(lacunarityLoc, lacunarity);

		GLuint numGroups = (((width + 1) * (height + 1)) + 63) / 64;
		glDispatchCompute((GLuint)ceil(width / 16.0f),
			(GLuint)ceil(height / 16.0f), 1);
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
		glDeleteBuffers(1, &ssboVertices);

		
		
	}

	void InterpolatedNormals(PlaneMesh& planeData, int width, int height, bool CleanUp){
		GLuint ssboVertices, ssboNormals;
		if ((width + 1) * (height + 1) != planeData.vertices.size()) {
			std::cout << "wrong sizes!";
		}
		if ((width + 1) * (height + 1) != planeData.normals.size()) {
			std::cout << "wrong sizes!";
		}
		glGenBuffers(1, &ssboVertices);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboVertices);
		glBufferData(GL_SHADER_STORAGE_BUFFER, planeData.vertices.size() * 3 * sizeof(float), planeData.vertices.data(), GL_DYNAMIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboVertices);

		glGenBuffers(1, &ssboNormals);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboNormals);
		glBufferData(GL_SHADER_STORAGE_BUFFER, planeData.normals.size() * 3 * sizeof(float), planeData.normals.data(), GL_DYNAMIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboNormals);

		GLint widthLoc = glGetUniformLocation(_normalInterpelationComputeShaderProgram, "width");
		GLint heightLoc = glGetUniformLocation(_normalInterpelationComputeShaderProgram, "height");

		glUseProgram(_normalInterpelationComputeShaderProgram);

		glUniform1i(widthLoc, width);
		glUniform1i(heightLoc, height);

		GLuint numGroups = (((width + 1) * (height + 1)) + 63) / 64;
		glDispatchCompute((GLuint)ceil(width / 16.0f),
			(GLuint)ceil(height / 16.0f), 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboNormals);
		glm::fvec3* ptrNormals = (glm::fvec3*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

		// Copy or use data
		planeData.normals.assign(ptrNormals, ptrNormals + planeData.normals.size());
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		glDeleteBuffers(1, &ssboVertices);
		glDeleteBuffers(1, &ssboNormals);

	}
	
	PlaneMesh CreateHeightMapPlaneMeshGPU(int width, int height, glm::vec2 offset, float scale, float amplitude, float frequency, int octaves, float persistance, float lacunarity, bool CleanUp) {
		PlaneMesh planeData;
		if(CleanUp)
			Init();
		std::vector<glm::fvec3> vertices;
		vertices.resize((width+1) * (height+1));
		std::vector<int> indices;
		indices.resize((width) * (height) * 6);
		std::vector<glm::fvec3> normals;
		normals.resize((width+1)*(height+1));

		planeData.vertices = vertices;
		planeData.indices = indices;
		planeData.normals = normals;

		CreateVertices(planeData, width, height, offset, CleanUp);
		CreateIndices(planeData, width, height, CleanUp);
		DisplaceVertices(planeData, width, height, scale, amplitude, frequency, octaves, persistance, lacunarity, CleanUp);
		InterpolatedNormals(planeData, width, height, CleanUp);
		//CalculateNormalsHeightMap(planeData, width, height);
		if (CleanUp)
			Cleanup();
		return planeData;
	}	

	int CountMarchingCubesTriangleCount(int width, int height, int depth, glm::vec3 offset, bool CleanUp,std::vector<float>& noiseMap, float iso) {
		GLuint ssboCounter;
		int initial = 0;
		glGenBuffers(1, &ssboCounter);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCounter);
		glBufferData(GL_SHADER_STORAGE_BUFFER, 1 * sizeof(int), &initial, GL_DYNAMIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboCounter);

		GLuint ssboNoise;
		glGenBuffers(1, &ssboNoise);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboNoise);
		glBufferData(GL_SHADER_STORAGE_BUFFER, noiseMap.size() * sizeof(float), noiseMap.data(), GL_DYNAMIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboNoise);

		GLuint ssboFlatTriTable;
		glGenBuffers(1, &ssboFlatTriTable);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboFlatTriTable);
		glBufferData(GL_SHADER_STORAGE_BUFFER, 256*16 * sizeof(int), FlatTriTable, GL_STATIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssboFlatTriTable);

		GLint widthLoc = glGetUniformLocation(_marchingCubesTriCounterComputeShader, "width");
		GLint heightLoc = glGetUniformLocation(_marchingCubesTriCounterComputeShader, "height");
		GLint depthLoc = glGetUniformLocation(_marchingCubesTriCounterComputeShader, "depth");
		GLint offsetLoc = glGetUniformLocation(_marchingCubesTriCounterComputeShader, "offset");
		GLint isoLevelLoc = glGetUniformLocation(_marchingCubesTriCounterComputeShader, "isoLevel");

		glUseProgram(_marchingCubesTriCounterComputeShader);

		glUniform1i(widthLoc, width);
		glUniform1i(heightLoc, height);
		glUniform1i(depthLoc, depth);
		glUniform3fv(offsetLoc, 1, &offset[0]);
		glUniform1f(isoLevelLoc, iso);

		glDispatchCompute((GLuint)ceil(width / 8.0f),
			(GLuint)ceil(height / 8.0f), (GLuint)ceil(depth / 8.0f));
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCounter);
		int* ptr = (int*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

		// Copy or use data
		int vertexCount = 0;
		if (ptr) {
			vertexCount = *ptr;
			glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		}
		else {
			std::cout << "Something went wrong in CreateVertices";
		}
		glDeleteBuffers(1, &ssboCounter);
		glDeleteBuffers(1, &ssboNoise);
		return vertexCount;
	}
	PlaneMesh CreateMarchingCubesTriangles(int width, int height, int depth, glm::vec3 offset, bool CleanUp, std::vector<float>& noiseMap, float iso, int triangleCount) {
		PlaneMesh planeData;
		int vectorSize = triangleCount;
		std::vector<glm::fvec3> vertices;
		vertices.resize(vectorSize);
		std::vector<int> indices;
		indices.resize(vectorSize);
		std::vector<glm::fvec3> normals;
		normals.resize(vectorSize);

		GLuint ssboNoise;
		GLuint ssboVertices;
		GLuint ssboNormals;
		GLuint ssboIndices;
		GLuint ssboCounter;
		GLuint ssboFlatTriTable;
		

		glGenBuffers(1, &ssboNoise);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboNoise);
		glBufferData(GL_SHADER_STORAGE_BUFFER, noiseMap.size() * sizeof(float), noiseMap.data(), GL_DYNAMIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboNoise);

		glGenBuffers(1, &ssboVertices);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboVertices);
		glBufferData(GL_SHADER_STORAGE_BUFFER, vertices.size() * 3 * sizeof(float), vertices.data(), GL_DYNAMIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboVertices);

		glGenBuffers(1, &ssboNormals);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboNormals);
		glBufferData(GL_SHADER_STORAGE_BUFFER, normals.size() * 3 * sizeof(float),normals.data(), GL_DYNAMIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssboNormals);

		glGenBuffers(1, &ssboIndices);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboIndices);
		glBufferData(GL_SHADER_STORAGE_BUFFER, indices.size() * sizeof(int), indices.data(), GL_DYNAMIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssboIndices);

		int initial = 0;
		glGenBuffers(1, &ssboCounter);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCounter);
		glBufferData(GL_SHADER_STORAGE_BUFFER, 1 * sizeof(int), &initial, GL_DYNAMIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ssboCounter);

		glGenBuffers(1, &ssboFlatTriTable);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboFlatTriTable);
		glBufferData(GL_SHADER_STORAGE_BUFFER, 256 * 16 * sizeof(int), FlatTriTable, GL_STATIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, ssboFlatTriTable);

		GLint widthLoc = glGetUniformLocation(_marchingCubesTriCreatorComputeShader, "width");
		GLint heightLoc = glGetUniformLocation(_marchingCubesTriCreatorComputeShader, "height");
		GLint depthLoc = glGetUniformLocation(_marchingCubesTriCreatorComputeShader, "depth");
		GLint offsetLoc = glGetUniformLocation(_marchingCubesTriCreatorComputeShader, "offset");
		GLint isoLevelLoc = glGetUniformLocation(_marchingCubesTriCreatorComputeShader, "isoLevel");

		glUseProgram(_marchingCubesTriCreatorComputeShader);

		glUniform1i(widthLoc, width);
		glUniform1i(heightLoc, height);
		glUniform1i(depthLoc, depth);
		glUniform3fv(offsetLoc, 1, &offset[0]);
		glUniform1f(isoLevelLoc, iso);

		glDispatchCompute((GLuint)ceil(width / 8.0f),
			(GLuint)ceil(height / 8.0f), (GLuint)ceil(depth / 8.0f));

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboVertices);
		glm::fvec3* ptrVerts = (glm::fvec3*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

		if(ptrVerts) {
			planeData.vertices.assign(ptrVerts, ptrVerts + vertices.size());
		}

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboNormals);
		glm::fvec3* ptrNormals = (glm::fvec3*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

		if(ptrNormals) {
			planeData.normals.assign(ptrNormals, ptrNormals + normals.size());
		}

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboIndices);
		int* ptrIndices = (int*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

		if (ptrIndices) {
			planeData.indices.assign(ptrIndices, ptrIndices + indices.size());
		}

		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		glDeleteBuffers(1, &ssboNoise);
		glDeleteBuffers(1, &ssboVertices);
		glDeleteBuffers(1, &ssboNormals);
		glDeleteBuffers(1, &ssboIndices);
		glDeleteBuffers(1, &ssboCounter);

		return planeData;
	}
	PlaneMesh CreateMarchingCubes3DMesh(int width, int height, int depth, glm::vec3 offset, bool CleanUp) {
		PlaneMesh planeData;
		int paddedHeight = height + 1;
		int paddedWidth = width + 1;
		int paddedDepth = depth + 1;

		std::vector<float> noiseMap = CreateFlat3DNoiseMap(paddedHeight, paddedWidth, paddedDepth, offset, true);

		/*for (const auto& temp : noiseMap) {
			if(temp < 0.0f)
				std::cout << temp << "\n";
		}*/

		std::vector<glm::fvec3> vertices;
		std::vector<int> indices;
		std::vector<glm::fvec3> normals;


		glm::ivec3 cornerOffsets[] =
		{
			glm::ivec3(0, 0, 0),
			glm::ivec3(1, 0, 0),
			glm::ivec3(1, 0, 1),
			glm::ivec3(0, 0, 1),
			glm::ivec3(0, 1, 0),
			glm::ivec3(1, 1, 0),
			glm::ivec3(1, 1, 1),
			glm::ivec3(0, 1, 1),
		};
		float isoLevel = 0.1f; // threshold


		float scale = 5.0f;
		for (int x = 0; x < width; x++)
		{
			for (int y = 0; y < height; y++)
			{
				for (int z = 0; z < depth; z++)
				{
					// 1. Get cube corners
					glm::vec3 cubeCorners[8];
					float cubeValues[8];

					for (int i = 0; i < 8; i++)
					{
						glm::vec3 corner = glm::vec3(x, y, z) + glm::vec3(cornerOffsets[i]);
						int ix = (int)corner.x;
						int iy = (int)corner.y;
						int iz = (int)corner.z;

						int idx = ix + iy * paddedWidth + iz * paddedWidth * paddedHeight;
						cubeCorners[i] = (corner + offset); // optionally scale it

						cubeValues[i] = noiseMap[idx];


					}

					// 2. Calculate cubeIndex
					int cubeIndex = 0;
					for (int i = 0; i < 8; i++)
					{
						if (cubeValues[i] < isoLevel)
							cubeIndex |= (1 << i);
					}

					// 3. If no intersection, skip
					if (edgeTable[cubeIndex] == 0)
						continue;

					// 4. Compute interpolated vertex positions
					glm::vec3 vertList[12];
					if ((edgeTable[cubeIndex] & 1) != 0)
						vertList[0] = VertInterp(isoLevel, cubeCorners[0], cubeCorners[1], cubeValues[0], cubeValues[1]);
					if ((edgeTable[cubeIndex] & 2) != 0)
						vertList[1] = VertInterp(isoLevel, cubeCorners[1], cubeCorners[2], cubeValues[1], cubeValues[2]);
					if ((edgeTable[cubeIndex] & 4) != 0)
						vertList[2] = VertInterp(isoLevel, cubeCorners[2], cubeCorners[3], cubeValues[2], cubeValues[3]);
					if ((edgeTable[cubeIndex] & 8) != 0)
						vertList[3] = VertInterp(isoLevel, cubeCorners[3], cubeCorners[0], cubeValues[3], cubeValues[0]);
					if ((edgeTable[cubeIndex] & 16) != 0)
						vertList[4] = VertInterp(isoLevel, cubeCorners[4], cubeCorners[5], cubeValues[4], cubeValues[5]);
					if ((edgeTable[cubeIndex] & 32) != 0)
						vertList[5] = VertInterp(isoLevel, cubeCorners[5], cubeCorners[6], cubeValues[5], cubeValues[6]);
					if ((edgeTable[cubeIndex] & 64) != 0)
						vertList[6] = VertInterp(isoLevel, cubeCorners[6], cubeCorners[7], cubeValues[6], cubeValues[7]);
					if ((edgeTable[cubeIndex] & 128) != 0)
						vertList[7] = VertInterp(isoLevel, cubeCorners[7], cubeCorners[4], cubeValues[7], cubeValues[4]);
					if ((edgeTable[cubeIndex] & 256) != 0)
						vertList[8] = VertInterp(isoLevel, cubeCorners[0], cubeCorners[4], cubeValues[0], cubeValues[4]);
					if ((edgeTable[cubeIndex] & 512) != 0)
						vertList[9] = VertInterp(isoLevel, cubeCorners[1], cubeCorners[5], cubeValues[1], cubeValues[5]);
					if ((edgeTable[cubeIndex] & 1024) != 0)
						vertList[10] = VertInterp(isoLevel, cubeCorners[2], cubeCorners[6], cubeValues[2], cubeValues[6]);
					if ((edgeTable[cubeIndex] & 2048) != 0)
						vertList[11] = VertInterp(isoLevel, cubeCorners[3], cubeCorners[7], cubeValues[3], cubeValues[7]);

					// 5. Emit triangles
					int triIndexBase = cubeIndex * 16;

					for (int i = 0; FlatTriTable[triIndexBase + i] != -1; i += 3)
					{
						int indexStart = vertices.size();

						glm::fvec3 v0 = vertList[FlatTriTable[triIndexBase + i]];
						glm::fvec3 v1 = vertList[FlatTriTable[triIndexBase + i + 1]];
						glm::fvec3 v2 = vertList[FlatTriTable[triIndexBase + i + 2]];

						vertices.push_back(v0);
						vertices.push_back(v1);
						vertices.push_back(v2);

						glm::fvec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
						normals.push_back(normal);
						normals.push_back(normal);
						normals.push_back(normal);

						indices.push_back(indexStart);
						indices.push_back(indexStart + 1);
						indices.push_back(indexStart + 2);
					}
				}
			}
		}
		planeData.vertices = vertices;
		planeData.indices = indices;
		planeData.normals = normals;

		//std::cout << vertices.size() << "\n";

		return planeData;
	}

	PlaneMesh CreateMarchingCubes3DMeshGPU(int width, int height, int depth, glm::vec3 offset, bool CleanUp, const float amplitude, const float frequency, const float persistance, const float lacunarity, const int octaves) {
		
		int paddedHeight = height + 1;
		int paddedWidth = width + 1;
		int paddedDepth = depth + 1;

		std::vector<float> noiseMap = CreateFlat3DNoiseMap(paddedHeight, paddedWidth, paddedDepth, offset, true, amplitude, frequency, persistance, lacunarity, octaves);
		int triCount = CountMarchingCubesTriangleCount(paddedWidth, paddedHeight, paddedDepth, offset, CleanUp, noiseMap, 0.0f);
		PlaneMesh planeData = CreateMarchingCubesTriangles(paddedWidth, paddedHeight, paddedDepth, offset, CleanUp, noiseMap, 0.0f, triCount);

		return planeData;
	}
	glm::vec3 VertInterp(float iso, glm::vec3 p1, glm::vec3 p2, float v1, float v2)
	{
		// Handle edge cases to match Bourke's paper
		if (std::abs(iso - v1) < 0.00001f)
			return p1;
		if (std::abs(iso - v2) < 0.00001f)
			return p2;
		if (std::abs(v1 - v2) < 0.00001f)
			return p1;

		float mu = (iso - v1) / (v2 - v1);
		return p1 + mu * (p2 - p1);
	}

	PlaneMesh CreateMarchingCubes3DMeshSmoothGPU(int width, int height, int depth, glm::vec3 offset, bool CleanUp) {
		PlaneMesh planeData;


		return planeData;
	}

	int VoxelCubesQuadCount(int width, int heigth, int depth, glm::vec3 offset,const std::vector<float>& noiseMap, bool CleanUp) {
		GLuint ssboCounter;
		GLuint ssboNoise;

		glGenBuffers(1, &ssboNoise);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboNoise);
		glBufferData(GL_SHADER_STORAGE_BUFFER, noiseMap.size() * sizeof(float), noiseMap.data(), GL_DYNAMIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboNoise);

		int initial = 0;
		glGenBuffers(1, &ssboCounter);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCounter);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int), &initial, GL_DYNAMIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboCounter);

		GLint widthLoc = glGetUniformLocation(_voxelCubesTriangleCounterComputeShader, "gridWidth");
		GLint heightLoc = glGetUniformLocation(_voxelCubesTriangleCounterComputeShader, "gridHeight");
		GLint depthLoc = glGetUniformLocation(_voxelCubesTriangleCounterComputeShader, "gridDepth");

		glUseProgram(_voxelCubesTriangleCounterComputeShader);
		
		glUniform1i(widthLoc, width);
		glUniform1i(heightLoc, heigth);
		glUniform1i(depthLoc, depth);

		glDispatchCompute((GLuint)ceil((width) / 8.0f),
			(GLuint)ceil((heigth) / 8.0f), (GLuint)ceil((depth) / 8.0f));
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCounter);
		int* ptr = (int*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

		int vertexCount = 0;
		if (ptr) {
			vertexCount = *ptr;
			glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		}
		else {
			std::cout << "Something went wrong in CreateVertices";
		}
		glDeleteBuffers(1, &ssboCounter);
		glDeleteBuffers(1, &ssboNoise);
		return vertexCount;
	}

	void VoxelCubesGeometryInit(PlaneMesh& planeData,int width, int heigth, int depth, glm::vec3 offset, const std::vector<float>& noiseMap, int quadCount, bool CleanUp) {
		std::vector<glm::vec3> vertices;
		std::vector<glm::vec3> normals;
		std::vector<int> indices;

		vertices.resize(quadCount * 4);
		normals.resize(quadCount * 4);
		indices.resize(quadCount * 6);

		GLuint ssboNoise;
		GLuint ssboVertex;
		GLuint ssboNormal;
		GLuint ssboIndex;
		GLuint ssboIndexCounter;
		GLuint ssboVertexCounter;

		glGenBuffers(1, &ssboNoise);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboNoise);
		glBufferData(GL_SHADER_STORAGE_BUFFER, noiseMap.size() * sizeof(float), noiseMap.data(), GL_DYNAMIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboNoise);

		glGenBuffers(1, &ssboVertex);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboVertex);
		glBufferData(GL_SHADER_STORAGE_BUFFER, 3 * vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboVertex);

		glGenBuffers(1, &ssboNormal);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboNormal);
		glBufferData(GL_SHADER_STORAGE_BUFFER, 3 * normals.size() * sizeof(float), normals.data(), GL_DYNAMIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssboNormal);

		glGenBuffers(1, &ssboIndex);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboIndex);
		glBufferData(GL_SHADER_STORAGE_BUFFER, indices.size() * sizeof(int), indices.data(), GL_DYNAMIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssboIndex);

		int initialIndex = 0;
		glGenBuffers(1, &ssboIndexCounter);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboIndexCounter);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int), &initialIndex, GL_DYNAMIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ssboIndexCounter);

		int initialVertex = 0;
		glGenBuffers(1, &ssboVertexCounter);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboVertexCounter);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int), &initialVertex, GL_DYNAMIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, ssboVertexCounter);

		GLint widthLoc = glGetUniformLocation(_voxelCubesGeometryInitComputeShader, "gridWidth");
		GLint heightLoc = glGetUniformLocation(_voxelCubesGeometryInitComputeShader, "gridHeight");
		GLint depthLoc = glGetUniformLocation(_voxelCubesGeometryInitComputeShader, "gridDepth");
		GLint offsetLoc = glGetUniformLocation(_voxelCubesGeometryInitComputeShader, "offset");

		glUseProgram(_voxelCubesGeometryInitComputeShader);

		glUniform1i(widthLoc, width);
		glUniform1i(heightLoc, heigth);
		glUniform1i(depthLoc, depth);
		glUniform3fv(offsetLoc, 1, &offset[0]);

		glDispatchCompute((GLuint)ceil((width) / 8.0f),
			(GLuint)ceil((heigth) / 8.0f), (GLuint)ceil((depth) / 8.0f));
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboVertex);
		glm::vec3* vertexPtr = (glm::vec3*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
		if (vertexPtr) {
			planeData.vertices.assign(vertexPtr, vertexPtr + vertices.size());
		}
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboNormal);
		glm::vec3* normalsPtr = (glm::vec3*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
		if (vertexPtr) {
			planeData.normals.assign(normalsPtr, normalsPtr + normals.size());
		}
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboIndex);
		int* IndexPtr = (int*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
		if (vertexPtr) {
			planeData.indices.assign(IndexPtr, IndexPtr + indices.size());
		}
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		glDeleteBuffers(1, &ssboNoise);
		glDeleteBuffers(1, &ssboVertex);
		glDeleteBuffers(1, &ssboNormal);
		glDeleteBuffers(1, &ssboIndex);
		glDeleteBuffers(1, &ssboIndexCounter);
		glDeleteBuffers(1, &ssboVertexCounter);
	}



	PlaneMesh CreateVoxelCubes3DMesh(int width, int height, int depth, glm::vec2 offset, bool CleanUp, const float amplitude, const float frequency, const float persistance, const float lacunarity, const int octaves, const bool useDropoff) {
		PlaneMesh planeData;
		int paddedWidth = width + 2;
		int paddedHeight = height + 2;
		int paddedDepth = depth + 2;

		NoiseMapData noiseMapData;

		glm::vec3 offset3D = glm::vec3(offset.x, 0, offset.y);

		Spline spline;
		spline.points.push_back(SplinePoint(0.0f,0.0f));
		//spline.points.push_back(SplinePoint(0.1f, 0.4f));
		//spline.points.push_back(SplinePoint(0.2f, 1.0f));
		//spline.points.push_back(SplinePoint(0.4f, 0.4f));
		//spline.points.push_back(SplinePoint(0.5f, 1.0f));
		spline.points.push_back(SplinePoint(1.0f,1.0f));

		CreateFlat3DNoiseMap(noiseMapData, paddedWidth, paddedHeight, paddedDepth, offset3D, true, amplitude, frequency, persistance, lacunarity, octaves, true);
		NormaliseNoiseMap(noiseMapData, paddedWidth, paddedHeight, paddedDepth, true);
		SampleSplineCurve(noiseMapData, paddedWidth, paddedHeight, paddedDepth, spline);
		//if(noiseMapData.minValue < -0.866f || noiseMapData.maxValue > 0.866f)
			//std::cout << "MinValue: " << noiseMapData.minValue << " | MaxValue: " << noiseMapData.maxValue << "\n";
		
		int quadCount = VoxelCubesQuadCount(paddedWidth, paddedHeight, paddedDepth, offset3D, noiseMapData.noiseMap, CleanUp);
		VoxelCubesGeometryInit(planeData, paddedWidth, paddedHeight, paddedDepth, offset3D, noiseMapData.noiseMap, quadCount, CleanUp);


		return planeData;
	}
}