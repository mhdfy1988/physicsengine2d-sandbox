#include "../include/physics2d/physics2d.hpp"

int main() {
    physics2d::Engine engine = physics2d::Engine::create();
    if (!engine.valid()) {
        return 1;
    }

    physics2d::ecs::Registry registry;
    physics2d::ecs::Pipeline pipeline;

    physics2d::ecs::PipelineConfig cfg = pipeline.config();
    cfg.run_step = false;
    cfg.run_sync = false;
    pipeline.set_config(cfg);

    physics2d::ecs::Entity e = registry.create();
    registry.add_transform(e, physics2d::ecs::Transform{vec2(0.0f, 0.0f), 0.0f});
    registry.add_collider_circle(e, 0.5f);
    registry.add_rigidbody_spec(e, physics2d::ecs::RigidBodySpec{1.0f, BODY_DYNAMIC});

    pipeline.tick(registry, engine.view());
    if (pipeline.stats().spawned_entities != 1u) {
        return 2;
    }
    const physics2d::ecs::RuntimeBodyRef* rr = registry.runtime_body(e);
    if (rr == nullptr || rr->body == nullptr) {
        return 3;
    }
    RigidBody* original = rr->body;

    registry.add_transform(e, physics2d::ecs::Transform{vec2(3.0f, -2.0f), 0.25f});
    registry.add_rigidbody_spec(e, physics2d::ecs::RigidBodySpec{1.0f, BODY_STATIC});
    pipeline.tick(registry, engine.view());
    rr = registry.runtime_body(e);
    if (rr == nullptr || rr->body == nullptr) {
        return 4;
    }
    if (rr->body->position.x != 3.0f || rr->body->position.y != -2.0f) {
        return 5;
    }
    if (rr->body->angle != 0.25f) {
        return 6;
    }
    if (rr->body->type != BODY_STATIC) {
        return 7;
    }
    if (pipeline.stats().pre_synced_entities < 2u) {
        return 8;
    }

    registry.add_collider_box(e, 2.0f, 1.0f);
    pipeline.tick(registry, engine.view());
    rr = registry.runtime_body(e);
    if (rr == nullptr || rr->body == nullptr) {
        return 9;
    }
    if (rr->body == original) {
        return 10;
    }
    if (pipeline.stats().respawned_entities != 1u) {
        return 11;
    }
    if (!registry.validate_bridge().ok()) {
        return 12;
    }

    return 0;
}
