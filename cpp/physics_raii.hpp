#ifndef PHYSICS_RAII_HPP
#define PHYSICS_RAII_HPP

#include <memory>
#include "../include/physics.h"

namespace physics2d {

struct EngineDeleter {
    void operator()(PhysicsEngine* engine) const noexcept {
        if (engine != nullptr) {
            physics_engine_free(engine);
        }
    }
};

class Engine {
public:
    using Handle = std::unique_ptr<PhysicsEngine, EngineDeleter>;

    Engine() noexcept = default;
    explicit Engine(PhysicsEngine* raw) noexcept : handle_(raw) {}

    static Engine create() noexcept {
        return Engine(physics_engine_create());
    }

    bool valid() const noexcept {
        return handle_ != nullptr;
    }

    PhysicsEngine* get() const noexcept {
        return handle_.get();
    }

    PhysicsEngine* release() noexcept {
        return handle_.release();
    }

    void reset(PhysicsEngine* raw = nullptr) noexcept {
        handle_.reset(raw);
    }

    void set_gravity(Vec2 gravity) noexcept {
        physics_engine_set_gravity(handle_.get(), gravity);
    }

    void step() noexcept {
        physics_engine_step(handle_.get());
    }

private:
    Handle handle_;
};

}  // namespace physics2d

#endif
