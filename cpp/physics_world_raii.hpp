#ifndef PHYSICS_WORLD_RAII_HPP
#define PHYSICS_WORLD_RAII_HPP

#include <memory>
#include "physics_world.h"
#include "physics_raii.hpp"

namespace physics2d {

struct WorldDeleter {
    void operator()(PhysicsWorld* world) const noexcept {
        if (world != nullptr) {
            physics_world_free(world);
        }
    }
};

class World {
public:
    using Handle = std::unique_ptr<PhysicsWorld, WorldDeleter>;

    World() noexcept = default;
    explicit World(PhysicsWorld* raw) noexcept : handle_(raw) {}

    static World create() noexcept {
        return World(physics_world_create());
    }

    bool valid() const noexcept {
        return handle_ != nullptr;
    }

    PhysicsWorld* get() const noexcept {
        return handle_.get();
    }

    PhysicsWorld* release() noexcept {
        return handle_.release();
    }

    void reset(PhysicsWorld* raw = nullptr) noexcept {
        handle_.reset(raw);
    }

    void step() noexcept {
        physics_world_step(handle_.get());
    }

    EngineView engine() const noexcept {
        return EngineView(physics_world_engine(handle_.get()));
    }

    Status status() const noexcept {
        return engine().status();
    }

private:
    Handle handle_;
};

}  // namespace physics2d

#endif
