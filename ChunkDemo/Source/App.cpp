#include "App.h"

void App::Run() {
	Core::Init();
	while (!glfwWindowShouldClose(_renderer.GetWindow())) {
		glm::vec3 pos = _renderer.GetCameraPosition(); // or pass shared
		_chunkManager.Update(pos);
		_renderer.Render(_chunkManager);
	}
	Core::Cleanup();
	_renderer.Cleanup(_chunkManager);

}



