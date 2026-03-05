#ifndef PHYSICS_SNAPSHOT_HPP
#define PHYSICS_SNAPSHOT_HPP

#include <cstring>
#include "physics_raii.hpp"

namespace physics2d {

class Snapshot {
public:
    Snapshot() noexcept {
        std::memset(&data_, 0, sizeof(data_));
        data_.version = PHYSICS_WORLD_SNAPSHOT_VERSION;
    }

    int body_count() const noexcept {
        return data_.body_count;
    }

    unsigned int version() const noexcept {
        return data_.version;
    }

    const PhysicsWorldSnapshot* get() const noexcept {
        return &data_;
    }

    PhysicsWorldSnapshot* get() noexcept {
        return &data_;
    }

    bool capture(const Engine& engine) noexcept {
        return physics_engine_capture_snapshot(engine.get(), &data_) != 0;
    }

    bool apply(Engine& engine) const noexcept {
        return physics_engine_apply_snapshot(engine.get(), &data_) != 0;
    }

    bool replay(Engine& engine, int steps) const noexcept {
        return physics_engine_replay_from_snapshot(engine.get(), &data_, steps) != 0;
    }

private:
    PhysicsWorldSnapshot data_;
};

}  // namespace physics2d

#endif
