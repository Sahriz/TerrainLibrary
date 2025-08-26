#include "Player.h"

void Player::UpdatePlayer(double deltaTime) {
	
}


/*Camera Related functionallity related to a specific player*/
const glm::vec3& Player::GetCameraPosition() {
	return _camera.GetPosition();
}

glm::mat4 Player::GetViewMatrix() {
	return _camera.GetViewMatrix();
}

void Player::UpdateCursorState(GLFWwindow* window) {
	_camera.UpdateCursorState(window);
}

void Player::HandleKeyboardInput(float deltaTime, GLFWwindow* window) {
	_camera.HandleKeyboardInput(deltaTime, window);
}

void Player::ProcessMouseMovement(GLFWwindow* window, double xpos, double ypos) {
	_camera.ProcessMouseMovement(window, xpos, ypos);
}

