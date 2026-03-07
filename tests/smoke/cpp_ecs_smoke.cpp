#include "../../include/physics2d/physics2d.hpp"

int main() {
    physics2d::Engine engine = physics2d::Engine::create();
    if (!engine.valid()) {
        return 1;
    }

    physics2d::ecs::Registry registry;
    physics2d::ecs::Entity e = registry.create();
    registry.add_transform(e, physics2d::ecs::Transform{vec2(0.0f, 0.0f), 0.0f});
    registry.add_collider_circle(e, 0.5f);
    registry.add_rigidbody_spec(e, physics2d::ecs::RigidBodySpec{1.0f, BODY_DYNAMIC});

    physics2d::ecs::SpawnRigidBodySystem spawn_sys;
    physics2d::ecs::PhysicsStepSystem step_sys;
    physics2d::ecs::SyncTransformSystem sync_sys;
    physics2d::ecs::CleanupSystem cleanup_sys;

    spawn_sys.run(registry, engine.view());
    if (engine.body_count() != 1) {
        return 2;
    }

    step_sys.run(engine.view());
    sync_sys.run(registry);
    if (registry.transform(e) == nullptr) {
        return 3;
    }
    if (registry.runtime_body(e) == nullptr) {
        return 4;
    }

    if (!registry.queue_destroy(e)) {
        return 5;
    }
    if (cleanup_sys.run(registry, engine.view()) != 1u) {
        return 6;
    }
    if (engine.body_count() != 0) {
        return 7;
    }
    if (registry.alive(e)) {
        return 8;
    }
    return 0;
}
