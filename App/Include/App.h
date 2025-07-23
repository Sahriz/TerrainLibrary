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

    std::string readFile(const std::string& filePath);

    GLuint compileShader(GLenum type, const char* src);

    GLuint createShaderProgramFromFiles(const std::string& vertPath, const std::string& fragPath);

    void createPerspectiveMatrix(float fov, float aspect, float near, float far, float right, float left, float top, float bottom);


    void Run();

    ~App() {}

private:
    float _scaleStartValue;
    float _frequencyStartValue;
    int _octaveStartValue;
    glm::mat4 _perspectiveMat;


    void UpdateStartValues(float scale, float frequency, int octaves){
        _scaleStartValue = scale;
        _frequencyStartValue = frequency;
        _octaveStartValue = octaves;
    }
};