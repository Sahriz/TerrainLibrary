#pragma once

#include "Renderer.h"
#include "ChunkManager.h"
#include <thread>
#include <atomic>

class App {
public:
    App() { }

    void Run();

    ~App() {}

private:
    Renderer _renderer;
    ChunkManager _chunkManager;

    std::atomic<bool> _running = true; // This is your shared control flag
};