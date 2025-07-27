#pragma once

#include <unordered_map>
#include <unordered_set>
#include <GLFW/glfw3.h>      // must come before glad
#include <glad/glad.h>

#include "Core/Core.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "vector"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>

#include "glm.hpp"
#include "matrix_transform.hpp"
#include "type_ptr.hpp"

#include "Camera.h"

using ChunkCoord = glm::ivec2;

namespace std {
    template <>
    struct hash<glm::ivec2> {
        size_t operator()(const glm::ivec2& v) const {
            return std::hash<int>()(v.x) ^ (std::hash<int>()(v.y) << 1);
        }
    };
}
class App {
public:
    App();

    void Run();

    ~App() {}

private:
    float _scale = 0.1f;
	float _amplitude = 1.0f;
    float _frequency = 1.0f;
    int _octave = 5;
    float _lacunarity = 2.0f;
    float _persistance = 0.5f;
    int _width = 1000;
    int _height = 1000;
    int _viewDistance = 1;

    int _screenWidth = 1280;
    int _screenHeight = 720;
    const glm::vec2 _imguiWindowSize = glm::vec2(10.0f,10.0f);
    glm::mat4 _perspectiveMat;
    GLFWwindow* _window;
    Camera _camera;
    GLuint _shaderProgram;

    std::unordered_map<ChunkCoord, Core::PlaneMesh> _chunkMap;
    std::unordered_set<glm::ivec2> _activeChunkSet;

    void UpdateChunks();

    void SetupMesh(Core::PlaneMesh& mesh);

    void RenderChunks(const Core::PlaneMesh& planeData);

    void init();

    std::string ReadFile(const std::string& filePath);

    GLuint CompileShader(GLenum type, const std::string& source);

    GLuint CreateShaderProgram(const std::string& vertexPath, const std::string& fragmentPath);

    void UpdatePlaneMesh(Core::PlaneMesh& planeData, GLuint& VAO, GLuint& VBOVertex, GLuint& VBONormals, GLuint& EBO);

    void ProgramSetup(Core::PlaneMesh& planeData, GLuint& VAO, GLuint& VBOVertex, GLuint& VBONormals, GLuint& EBO);

    void ProgramInit(Core::PlaneMesh& planeData, GLuint& VAO, GLuint& VBOVertex, GLuint& VBONormals, GLuint& EBO);

    void DeleteChunk(Core::PlaneMesh& mesh);

    void Cleanup(GLFWwindow* window, GLuint& shaderProgram);

    void ResetToStartValues();
};