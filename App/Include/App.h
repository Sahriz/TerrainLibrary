#pragma once

#include <GLFW/glfw3.h>      // must come before glad
#include <glad/glad.h>

#include "Core/Core.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "glm.hpp"
class App {
public:
    App();

    std::string readFile(const std::string& filePath);

    GLuint compileShader(GLenum type, const char* src);

    GLuint createShaderProgramFromFiles(const std::string& vertPath, const std::string& fragPath);


    void Run();

    ~App() {}

private:
    float _scaleStartValue;
    float _frequencyStartValue;
    int _octaveStartValue;


    void UpdateStartValues(float scale, float frequency, int octaves){
        _scaleStartValue = scale;
        _frequencyStartValue = frequency;
        _octaveStartValue = octaves;
    }
};