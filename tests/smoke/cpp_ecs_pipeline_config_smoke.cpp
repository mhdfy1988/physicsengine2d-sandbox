#include "../../include/physics2d/physics2d.hpp"

int main() {
    physics2d::Engine engine = physics2d::Engine::create();
    if (!engine.valid()) {
        return 1;
    }

    physics2d::ecs::Registry registry;
    physics2d::ecs::Pipeline pipeline;

    physics2d::ecs::Entity e = registry.create();
    registry.add_transform(e, physics2d::ecs::Transform{vec2(0.0f, 0.0f), 0.0f});
    registry.add_collider_circle(e, 0.5f);
    registry.add_rigidbody_spec(e, physics2d::ecs::RigidBodySpec{1.0f, BODY_DYNAMIC});

    pipeline.tick(registry, engine.view());
    if (engine.body_count() != 1) {
        return 2;
    }

    if (!registry.queue_destroy(e)) {
        return 3;
    }

    physics2d::ecs::PipelineConfig cfg = pipeline.config();
    cfg.run_cleanup = false;
    pipeline.set_config(cfg);
    pipeline.tick(registry, engine.view());
    if (pipeline.stats().cleaned_entities != 0u) {
        return 4;
    }
    if (engine.body_count() != 1) {
        return 5;
    }
    if (!registry.alive(e)) {
        return 6;
    }

    cfg.run_cleanup = true;
    pipeline.set_config(cfg);
    pipeline.tick(registry, engine.view());
    if (pipeline.stats().cleaned_entities != 1u) {
        return 7;
    }
    if (engine.body_count() != 0) {
        return 8;
    }
    if (registry.alive(e)) {
        return 9;
    }
    return 0;
}
