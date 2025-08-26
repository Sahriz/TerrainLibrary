#include "App.h"

using Clock = std::chrono::high_resolution_clock;
using Time = std::chrono::duration<double>;

const double TICK_RATE = 1.0 / 60.0; // 60 ticks per second

std::mutex playerMutex;
std::atomic<bool> running(true);

void App::Run() {
	Core::Init();
	auto previous = Clock::now();
	double lag = 0.0;

	_physics = Physics(_chunkManager);
	_player = Player(_renderer.GetWindow(), _physics);
	_renderer.PlayerInit(_player);
	

	// Spawn tick thread
	std::thread tickThread([&]() {
		auto previous = Clock::now();
		double lag = 0.0;

		while (running) {
			auto current = Clock::now();
			Time elapsed = current - previous;
			previous = current;
			lag += elapsed.count();

			while (lag >= TICK_RATE) {
				{
					std::lock_guard<std::mutex> lock(playerMutex);
					_player.UpdatePlayer(lag); // safe update
				}
				lag -= TICK_RATE;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		});

	//Game Loop
	// Main render loop
	while (!glfwWindowShouldClose(_renderer.GetWindow())) {
		glm::vec3 pos = _renderer.GetCameraPosition();
		_chunkManager.Update(pos);

		_renderer.Render(_chunkManager);
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	// Cleanup
	running = false;
	tickThread.join();
	Core::Cleanup();
	_renderer.Cleanup(_chunkManager);

}



