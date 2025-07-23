#include "Camera.h"

glm::mat4 Camera::GetViewMatrix()
{
	return glm::lookAt(_cameraPos, _cameraPos + _cameraFront, _cameraUp);
}

void Camera::HandleKeyboardInput(float deltaTime, GLFWwindow* window) {
	float cameraSpeed = 15.0f * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		_cameraPos += cameraSpeed * _cameraFront;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		_cameraPos -= cameraSpeed * _cameraFront;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		_cameraPos -= glm::normalize(glm::cross(_cameraFront, _cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		_cameraPos += glm::normalize(glm::cross(_cameraFront, _cameraUp)) * cameraSpeed;
	if( glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		_cameraPos += cameraSpeed * _cameraUp;
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		_cameraPos -= cameraSpeed * _cameraUp;
}

void Camera::ProcessMouseMovement(GLFWwindow* window, double xpos, double ypos)
{
	static float sensitivity = 0.1f;

	if (_firstMouse) {
		_lastX = xpos;
		_lastY = ypos;
		_firstMouse = false;
	}

	float xoffset = xpos - _lastX;
	float yoffset = _lastY - ypos; // reversed since y-coordinates range from bottom to top
	_lastX = xpos;
	_lastY = ypos;

	xoffset *= sensitivity;
	yoffset *= sensitivity;

	_yaw += xoffset;
	_pitch += yoffset;

	// clamp _pitch to avoid gimbal lock
	_pitch = glm::clamp(_pitch, -89.0f, 89.0f);

	// recalculate _cameraFront
	glm::vec3 direction;
	direction.x = cos(glm::radians(_yaw)) * cos(glm::radians(_pitch));
	direction.y = 0;
	direction.z = sin(glm::radians(_yaw)) * cos(glm::radians(_pitch));
	_cameraFront = glm::normalize(direction);
}