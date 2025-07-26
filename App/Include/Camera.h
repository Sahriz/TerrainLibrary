#pragma once
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include <GLFW/glfw3.h>

class Camera
{
public:

	Camera(GLFWwindow* window) {
		_cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
		_cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
		_cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
		_yaw = -90.0f;
		_pitch = 0.0f;
		_lastX = 400;
		_lastY = 300;
		_firstMouse = true;
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

	void UpdateCursorState(GLFWwindow* window);

	void HandleKeyboardInput(float deltaTime, GLFWwindow* window);
	
	void ProcessMouseMovement(GLFWwindow* window, double xpos, double ypos);



	glm::mat4 GetViewMatrix();

	
	private:
	glm::vec3 _cameraPos;
	glm::vec3 _cameraFront;
	glm::vec3 _cameraUp;

	float _yaw;
	float _pitch;
	float _lastX, _lastY;
	bool _firstMouse;

	bool _cursorEnabled = false;
	int _oldState = GLFW_RELEASE;
};