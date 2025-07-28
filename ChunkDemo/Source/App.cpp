#include "App.h"

void App::Run() {
	
	while (!glfwWindowShouldClose(_renderer.GetWindow())) {
		_chunkManager.Update(_renderer.GetCameraPosition());
		_renderer.Render(_chunkManager);
	}
	_renderer.Cleanup(_chunkManager);

}



