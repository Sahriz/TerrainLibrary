#pragma once

#include "Renderer.h"
#include "ChunkManager.h"
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>

class App {
public:
    App() { std::cout << "test"; }

    void Run();

    ~App() {}

private:
    Renderer _renderer;
    ChunkManager _chunkManager;

    using Clock = std::chrono::high_resolution_clock;
    using Time = std::chrono::duration<double>;

    const double TICK_RATE = 1.0 / 60.0; // 60 ticks per second

    std::mutex _playerMutex;
    std::atomic<bool> _running{true}; // This is your shared control flag
};