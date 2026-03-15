#pragma once
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include "Core/Core.h"
#include "Jolt/Jolt.h"
#include <Jolt/Physics/Collision/Shape/MeshShape.h>

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

            if (job.type == PhysicsJob::Type::AddOrUpdate) {
                JPH::TriangleList joltTriangles;
                joltTriangles.reserve(job.vertices.size() / 3);

                // 1. Convert GLM vertices to Jolt Triangles
                for (size_t i = 0; i < job.vertices.size(); i += 3) {
                    JPH::Triangle tri(
                        JPH::Float3(job.vertices[i].x, job.vertices[i].y, job.vertices[i].z),
                        JPH::Float3(job.vertices[i + 1].x, job.vertices[i + 1].y, job.vertices[i + 1].z),
                        JPH::Float3(job.vertices[i + 2].x, job.vertices[i + 2].y, job.vertices[i + 2].z)
                    );
                    joltTriangles.push_back(tri);
                }

                // 2. Define the settings for the Mesh Shape
                JPH::MeshShapeSettings meshSettings(joltTriangles);

                // 3. Bake the actual collision shape!
                // (In a real engine, you'd pass this shape to Jolt's BodyInterface to spawn it in the world)
                JPH::ShapeSettings::ShapeResult shapeResult = meshSettings.Create();

                if (shapeResult.HasError()) {
                    // Handle the error (e.g., invalid triangles)
                }
                else {
                    JPH::ShapeRefC terrainShape = shapeResult.Get();
                    // DONE: You now have a highly optimized Jolt Triangle Mesh!
                }
            }

        }
    }

private:

    std::queue<PhysicsJob> _queue;
    std::mutex _mutex;
    std::condition_variable _cv;
    std::thread _worker;
    std::atomic<bool> _running;
};