#include "../include/physics2d/physics2d.hpp"

int main() {
    physics2d::Engine engine = physics2d::Engine::create();
    if (!engine.valid()) {
        return 1;
    }

    physics2d::ecs::Registry registry;
    physics2d::ecs::Entity a = registry.create();
    registry.add_transform(a, physics2d::ecs::Transform{vec2(0.0f, 0.0f), 0.0f});
    registry.add_collider_circle(a, 0.5f);
    registry.add_rigidbody_spec(a, physics2d::ecs::RigidBodySpec{1.0f, BODY_DYNAMIC});

    physics2d::ecs::Entity b = registry.create();
    registry.add_transform(b, physics2d::ecs::Transform{vec2(1.0f, 0.0f), 0.0f});
    registry.add_rigidbody_spec(b, physics2d::ecs::RigidBodySpec{1.0f, BODY_DYNAMIC});

    std::size_t spawnable = 0;
    registry.each_spawnable([&](physics2d::ecs::Entity, const physics2d::ecs::Transform&,
                                const physics2d::ecs::Collider&, const physics2d::ecs::RigidBodySpec&) {
        spawnable++;
    });
    if (spawnable != 1u) {
        return 2;
    }

    physics2d::ecs::SpawnRigidBodySystem spawn_sys;
    spawn_sys.run(registry, engine.view());
    if (engine.body_count() != 1) {
        return 3;
    }

    std::size_t runtime = 0;
    registry.each_runtime([&](physics2d::ecs::Entity, const physics2d::ecs::RuntimeBodyRef& rr) {
        if (rr.body != nullptr) {
            runtime++;
        }
    });
    if (runtime != 1u) {
        return 4;
    }

    spawnable = 0;
    registry.each_spawnable([&](physics2d::ecs::Entity, const physics2d::ecs::Transform&,
                                const physics2d::ecs::Collider&, const physics2d::ecs::RigidBodySpec&) {
        spawnable++;
    });
    if (spawnable != 0u) {
        return 5;
    }
    return 0;
}
