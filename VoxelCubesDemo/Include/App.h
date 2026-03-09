#pragma once

#include "Renderer.h"
#include "ChunkManager.h"
#include "Player.h"
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>

class App {
public:
    App() { }

    void Run();

    ~App() {}

private:
    Renderer _renderer;
    ChunkManager _chunkManager;
    Player _player;
    Physics _physics;

    std::atomic<bool> _running = true; // This is your shared control flag
};