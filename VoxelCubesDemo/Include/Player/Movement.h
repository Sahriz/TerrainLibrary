#pragma once
#include "Physics.h"

class Movement {
public:
	Movement() {

	}
	Movement(Physics& physics) { _physics = physics; }

private:
	Physics _physics;
	float _playerSpeed = 15.0f;
	bool _grounded = false;
};