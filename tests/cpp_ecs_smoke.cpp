#include "../cpp/physics2d.hpp"

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

    registry.spawn_rigid_bodies(engine.view());
    if (engine.body_count() != 1) {
        return 2;
    }

    engine.step();
    registry.sync_transforms_from_physics();
    if (registry.transform(e) == nullptr) {
        return 3;
    }
    if (registry.runtime_body(e) == nullptr) {
        return 4;
    }

    if (!registry.destroy(e, engine.view())) {
        return 5;
    }
    if (engine.body_count() != 0) {
        return 6;
    }
    return 0;
}
