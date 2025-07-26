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
    float _scaleStartValue =35.0f;
	float _amplitudeStartValue = 0.5f;
    float _frequencyStartValue = 4.0f;
    int _octaveStartValue = 5;
    float _lacunarityStartValue = 2.0f;
    float _persistanceStartValue = 0.5f;
    glm::mat4 _perspectiveMat;

    std::string readFile(const std::string& filePath);

    GLuint compileShader(GLenum type, const std::string& source);

    GLuint createShaderProgram(const std::string& vertexPath, const std::string& fragmentPath);

    void updatePlaneMesh(Core::PlaneMesh& planeData);
    void UpdateStartValues(float scale, float frequency, int octaves){
        _scaleStartValue = scale;
        _frequencyStartValue = frequency;
        _octaveStartValue = octaves;
    }
};