#pragma once
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include "Core/Core.h"

struct PhysicsJob {
    enum class Type { AddOrUpdate, Remove } type;
    glm::ivec3 coord;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals; 
};

class Physics {
public:
    Physics() : _running(false) {}
    ~Physics() { Stop(); }

    void Start() {
        _running = true;
        _worker = std::thread(&Physics::ThreadLoop, this);
    }

    void Stop() {
        if (!_running) return;
        _running = false;
        _cv.notify_all();
        if (_worker.joinable()) _worker.join();
    }

    // Called from render/main thread
    void EnqueueJob(PhysicsJob&& job) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _queue.push(std::move(job));
        }
        _cv.notify_one();
    }

private:
    void ThreadLoop() {
        while (_running) {
            PhysicsJob job;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                _cv.wait(lock, [&] { return !_queue.empty() || !_running; });
                if (!_running) break;

                job = std::move(_queue.front());
                _queue.pop();
            }

            // TODO: Create or update collider for job.coord using job.vertices
            // TODO: Handle remove jobs
        }
    }

private:

    std::queue<PhysicsJob> _queue;
    std::mutex _mutex;
    std::condition_variable _cv;
    std::thread _worker;
    std::atomic<bool> _running;
};