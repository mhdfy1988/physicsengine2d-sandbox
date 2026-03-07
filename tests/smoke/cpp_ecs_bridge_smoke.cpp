#include "../../include/physics2d/physics2d.hpp"

int main() {
    physics2d::Engine engine = physics2d::Engine::create();
    if (!engine.valid()) {
        return 1;
    }

    physics2d::ecs::Registry registry;
    physics2d::ecs::Pipeline pipeline;

    physics2d::ecs::Entity e = registry.create();
    registry.add_transform(e, physics2d::ecs::Transform{vec2(1.0f, 2.0f), 0.0f});
    registry.add_collider_circle(e, 0.8f);
    registry.add_rigidbody_spec(e, physics2d::ecs::RigidBodySpec{1.0f, BODY_DYNAMIC});

    pipeline.tick(registry, engine.view());
    if (pipeline.stats().spawned_entities != 1u) {
        return 2;
    }
    if (pipeline.stats().synced_entities != 1u) {
        return 3;
    }
    if (pipeline.stats().mapping_errors != 0u) {
        return 4;
    }
    if (registry.runtime_body_count() != 1u) {
        return 5;
    }

    const physics2d::ecs::RuntimeBodyRef* rr = registry.runtime_body(e);
    if (rr == nullptr || rr->body == nullptr) {
        return 6;
    }
    if (registry.entity_of_body(rr->body) != e) {
        return 7;
    }
    if (!registry.validate_bridge().ok()) {
        return 8;
    }

    if (!registry.queue_destroy(e)) {
        return 9;
    }
    pipeline.tick(registry, engine.view());
    if (pipeline.stats().cleaned_entities != 1u) {
        return 10;
    }
    if (registry.runtime_body_count() != 0u) {
        return 11;
    }
    return 0;
}
