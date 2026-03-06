#include "../include/physics2d/physics2d.hpp"

int main() {
    physics2d::runtime::RuntimeFacade runtime = physics2d::runtime::RuntimeFacade::create();
    if (!runtime.valid()) {
        return 1;
    }

    physics2d::ecs::Entity e = runtime.registry().create();
    runtime.registry().add_transform(e, physics2d::ecs::Transform{vec2(0.0f, 0.0f), 0.0f});
    runtime.registry().add_collider_circle(e, 0.5f);
    runtime.registry().add_rigidbody_spec(e, physics2d::ecs::RigidBodySpec{1.0f, BODY_DYNAMIC});

    physics2d::runtime::TickSnapshot s0 = runtime.tick_snapshot();
    if (s0.stats.body_count != 1) {
        return 2;
    }
    const physics2d::ecs::RuntimeBodyRef* rr = runtime.registry().runtime_body(e);
    if (rr == nullptr || rr->body == nullptr) {
        return 3;
    }

    rr->body->velocity = vec2(5.0f, 0.0f);
    const float x0 = rr->body->position.x;

    physics2d::ecs::PipelineConfig cfg = runtime.pipeline().config();
    cfg.run_step = false;
    runtime.pipeline().set_config(cfg);
    runtime.tick_snapshot();
    rr = runtime.registry().runtime_body(e);
    if (rr == nullptr || rr->body == nullptr) {
        return 4;
    }
    if (rr->body->position.x != x0) {
        return 5;
    }

    cfg.run_step = true;
    runtime.pipeline().set_config(cfg);
    runtime.tick_snapshot();
    rr = runtime.registry().runtime_body(e);
    if (rr == nullptr || rr->body == nullptr) {
        return 6;
    }
    if (rr->body->position.x <= x0) {
        return 7;
    }
    const float x1 = rr->body->position.x;

    cfg.run_step = false;
    runtime.pipeline().set_config(cfg);
    runtime.tick_snapshot();
    rr = runtime.registry().runtime_body(e);
    if (rr == nullptr || rr->body == nullptr) {
        return 8;
    }
    if (rr->body->position.x != x1) {
        return 9;
    }

    return 0;
}
