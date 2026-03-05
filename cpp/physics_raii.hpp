#ifndef PHYSICS_RAII_HPP
#define PHYSICS_RAII_HPP

#include <memory>
#include "physics.h"
#include "physics_status.hpp"

namespace physics2d {

class EngineView {
public:
    EngineView() noexcept = default;
    explicit EngineView(PhysicsEngine* raw) noexcept : ptr_(raw) {}

    bool valid() const noexcept {
        return ptr_ != nullptr;
    }

    PhysicsEngine* get() const noexcept {
        return ptr_;
    }

    void set_gravity(Vec2 gravity) noexcept {
        physics_engine_set_gravity(ptr_, gravity);
    }

    void step() noexcept {
        physics_engine_step(ptr_);
    }

    int body_count() const noexcept {
        return physics_engine_get_body_count(ptr_);
    }

    void add_body(RigidBody* body) noexcept {
        physics_engine_add_body(ptr_, body);
    }

    void remove_body(RigidBody* body) noexcept {
        physics_engine_remove_body(ptr_, body);
    }

    Status status() const noexcept {
        return status_from_engine(ptr_);
    }

private:
    PhysicsEngine* ptr_ = nullptr;
};

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

    EngineView view() noexcept {
        return EngineView(handle_.get());
    }

    EngineView view() const noexcept {
        return EngineView(handle_.get());
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

    RigidBody* body(int index) const noexcept {
        return physics_engine_get_body(handle_.get(), index);
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

    int constraint_count() const noexcept {
        return physics_engine_get_constraint_count(handle_.get());
    }

    Constraint* add_distance_constraint(RigidBody* a, RigidBody* b,
                                        Vec2 world_anchor_a, Vec2 world_anchor_b,
                                        float stiffness, int collide_connected) noexcept {
        return physics_engine_add_distance_constraint(handle_.get(), a, b,
                                                      world_anchor_a, world_anchor_b,
                                                      stiffness, collide_connected);
    }

    Constraint* add_spring_constraint(RigidBody* a, RigidBody* b,
                                      Vec2 world_anchor_a, Vec2 world_anchor_b,
                                      float rest_length, float stiffness, float damping,
                                      int collide_connected) noexcept {
        return physics_engine_add_spring_constraint(handle_.get(), a, b,
                                                    world_anchor_a, world_anchor_b,
                                                    rest_length, stiffness, damping,
                                                    collide_connected);
    }

    Constraint* add_rope_constraint(RigidBody* a, RigidBody* b,
                                    Vec2 world_anchor_a, Vec2 world_anchor_b,
                                    float max_length, float stiffness, int collide_connected) noexcept {
        return physics_engine_add_rope_constraint(handle_.get(), a, b,
                                                  world_anchor_a, world_anchor_b,
                                                  max_length, stiffness, collide_connected);
    }

    bool constraint_active(int index) const noexcept {
        return physics_engine_constraint_is_active(handle_.get(), index) != 0;
    }

    void set_constraint_active(int index, bool active) noexcept {
        physics_engine_constraint_set_active(handle_.get(), index, active ? 1 : 0);
    }

    void clear_constraints() noexcept {
        physics_engine_clear_constraints(handle_.get());
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
