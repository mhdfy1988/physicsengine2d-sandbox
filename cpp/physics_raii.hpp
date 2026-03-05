#ifndef PHYSICS_RAII_HPP
#define PHYSICS_RAII_HPP

#include <memory>
#include "../include/physics.h"
#include "physics_status.hpp"

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

    int body_count() const noexcept {
        return physics_engine_get_body_count(handle_.get());
    }

    void add_body(RigidBody* body) noexcept {
        physics_engine_add_body(handle_.get(), body);
    }

    void remove_body(RigidBody* body) noexcept {
        physics_engine_remove_body(handle_.get(), body);
    }

    RigidBody* detach_body(RigidBody* body) noexcept {
        return physics_engine_detach_body(handle_.get(), body);
    }

    bool add_broadphase_pair(int index_a, int index_b) noexcept {
        return physics_engine_add_broadphase_pair(handle_.get(), index_a, index_b) != 0;
    }

    void clear_error() noexcept {
        physics_engine_clear_error(handle_.get());
    }

    Status status() const noexcept {
        return status_from_engine(handle_.get());
    }

private:
    Handle handle_;
};

}  // namespace physics2d

#endif
