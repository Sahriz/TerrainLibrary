#pragma once

#include "Renderer.h"

class App {
public:
    App() { }

    void Run();

    ~App() {}

private:
    Renderer _renderer;

};