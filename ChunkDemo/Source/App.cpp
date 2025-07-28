#include "App.h"

void App::Run() {
	
	while (!glfwWindowShouldClose(_renderer.GetWindow())) {
		_renderer.Render();
	}
	_renderer.Cleanup();

}



