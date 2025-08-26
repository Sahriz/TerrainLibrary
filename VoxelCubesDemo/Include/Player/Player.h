#pragma once
#include "Inventory.h"
#include "Camera.h"
#include "Movement.h"
#include "iostream"


class Player {
public:
	Player() {

	}
	Player(GLFWwindow* window, Physics& physics) {
		_camera = Camera(window);
		_inventory = Inventory();
		_movement = Movement(physics);
	}

	/*Player Game loop using tickSystem*/
	void UpdatePlayer(double deltaTime);


	/*Camera Related functionallity related to a specific player*/
	const glm::vec3& GetCameraPosition();

	glm::mat4 GetViewMatrix();

	void UpdateCursorState(GLFWwindow* window);

	void HandleKeyboardInput(float deltaTime, GLFWwindow* window);

	void ProcessMouseMovement(GLFWwindow* window, double xpos, double ypos);

	

private:
	Camera _camera;
	Inventory _inventory;
	Movement _movement;
};