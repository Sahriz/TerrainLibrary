#pragma once

#include <GLFW/glfw3.h>      // must come before glad
#include <glad/glad.h>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "glm.hpp"
#include "matrix_transform.hpp"
#include "type_ptr.hpp"

#include "vector"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>

#include "ChunkManager.h"
#include "Camera.h"

using ChunkCoord = glm::ivec2;



class Renderer {
public:
    Renderer() {
        Init();
    }
    
    void Render();

    void Cleanup();
    GLFWwindow* GetWindow() {
        return _window;
    }
    
private:
    ChunkManager _chunkManager;

    glm::mat4 _identity;
    glm::mat4 _view;
    glm::mat4 _model;
    glm::mat3 _normalMatrix;
    float _prevTime = 0.0f;

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
    glm::mat4 _perspectiveMat;
    
    Camera _camera;
    GLuint _shaderProgram;
    GLFWwindow* _window;

    GLint _widthLocation;
    GLint _heightLocation;
    GLint _timeLocation;
    GLint _projMLocation;
    GLint _modelMLocation;
    GLint _viewLoc;
    GLint _normalMatrixLocation;

    void Init();
    std::string ReadFile(const std::string& filePath);
    GLuint CompileShader(GLenum type, const std::string& source);
    GLuint CreateShaderProgram(const std::string& vertexPath, const std::string& fragmentPath);
    void ResetToStartValues();
    

};