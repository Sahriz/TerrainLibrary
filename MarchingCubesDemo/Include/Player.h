#pragma once

#include "glm.hpp"

class Player {
public:

	Player() {
		_position = glm::vec3(0.0f);
	}

	void UpdatePosition(glm::vec3 pos) {
		_position = pos;
	}

	glm::vec3 GetPosition() {
		return _position;
	}

private:
	glm::vec3 _position;

};