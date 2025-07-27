#pragma once

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
    const glm::vec2 _imguiWindowSize = glm::vec2(10.0f,10.0f);
    glm::mat4 _perspectiveMat;
    int _maxBufferSize = 2000;

    std::string ReadFile(const std::string& filePath);

    GLuint CompileShader(GLenum type, const std::string& source);

    GLuint CreateShaderProgram(const std::string& vertexPath, const std::string& fragmentPath);

    void UpdatePlaneMesh(Core::PlaneMesh& planeData, GLuint& VAO, GLuint& VBOVertex, GLuint& VBONormals, GLuint& EBO);

    void ProgramSetup(Core::PlaneMesh& planeData, GLuint& VAO, GLuint& VBOVertex, GLuint& VBONormals, GLuint& EBO);

    void ProgramInit(Core::PlaneMesh& planeData, GLuint& VAO, GLuint& VBOVertex, GLuint& VBONormals, GLuint& EBO);

    void Cleanup(GLuint& VAO, GLuint& VBOVertex, GLuint& VBONormals, GLuint& EBO, GLFWwindow* window, GLuint& shaderProgram);

    void ResetToStartValues();
};