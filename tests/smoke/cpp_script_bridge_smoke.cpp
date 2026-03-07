#include <cstdio>
#include "physics2d/physics_script_bridge.hpp"

static bool nearly_equal(float a, float b) {
    float delta = a - b;
    if (delta < 0.0f) {
        delta = -delta;
    }
    return delta <= 0.001f;
}

int main() {
    physics2d::runtime::RuntimeFacade runtime = physics2d::runtime::RuntimeFacade::create();
    if (!runtime.valid()) {
        std::printf("[FAIL] runtime facade create failed\n");
        return 1;
    }

    physics2d::script::ScriptBridge bridge(runtime);
    physics2d::script::EntitySpawnSpec spec;
    spec.name = "hero";
    spec.transform = physics2d::ecs::Transform{vec2(0.0f, 0.0f), 0.0f};
    spec.body = physics2d::ecs::RigidBodySpec{1.0f, BODY_DYNAMIC};
    spec.collider_shape = physics2d::script::ColliderShape::Box;
    spec.size_a = 1.0f;
    spec.size_b = 0.5f;

    if (bridge.spawn_entity(spec) == physics2d::ecs::kInvalidEntity) {
        std::printf("[FAIL] failed to spawn script entity\n");
        return 2;
    }

    physics2d::runtime::TickSnapshot snap0 = bridge.tick();
    if (snap0.stats.body_count != 1) {
        std::printf("[FAIL] expected spawned runtime body after script tick\n");
        return 3;
    }

    physics2d::script::EntityState s0 = bridge.read_state("hero");
    if (!s0.alive || !s0.runtime_ready) {
        std::printf("[FAIL] script entity should be alive and runtime-ready\n");
        return 4;
    }

    physics2d::ecs::PipelineConfig cfg = runtime.pipeline().config();
    cfg.run_step = false;
    runtime.pipeline().set_config(cfg);
    if (!bridge.set_transform("hero", physics2d::ecs::Transform{vec2(2.0f, 3.0f), 0.25f})) {
        std::printf("[FAIL] failed to set transform through script bridge\n");
        return 5;
    }
    (void)bridge.tick();
    physics2d::script::EntityState s1 = bridge.read_state("hero");
    if (!nearly_equal(s1.transform.position.x, 2.0f) ||
        !nearly_equal(s1.transform.position.y, 3.0f) ||
        !nearly_equal(s1.transform.angle, 0.25f)) {
        std::printf("[FAIL] transform sync through script bridge mismatch\n");
        return 6;
    }

    cfg.run_step = true;
    runtime.pipeline().set_config(cfg);
    if (!bridge.set_linear_velocity("hero", vec2(6.0f, 0.0f))) {
        std::printf("[FAIL] failed to set velocity through script bridge\n");
        return 7;
    }
    (void)bridge.tick();
    physics2d::script::EntityState s2 = bridge.read_state("hero");
    if (s2.transform.position.x <= s1.transform.position.x) {
        std::printf("[FAIL] expected script-driven body to advance after step\n");
        return 8;
    }

    if (!bridge.apply_impulse("hero", vec2(1.0f, 0.0f))) {
        std::printf("[FAIL] failed to apply impulse through script bridge\n");
        return 9;
    }
    (void)bridge.tick();
    physics2d::script::EntityState s3 = bridge.read_state("hero");
    if (s3.velocity.x <= s2.velocity.x) {
        std::printf("[FAIL] expected script impulse to increase linear velocity\n");
        return 10;
    }

    if (!bridge.queue_destroy("hero")) {
        std::printf("[FAIL] failed to queue destroy through script bridge\n");
        return 11;
    }
    (void)bridge.tick();
    if (bridge.has_entity("hero")) {
        std::printf("[FAIL] expected script entity to be destroyed after tick\n");
        return 12;
    }

    std::printf("[PASS] cpp script bridge smoke\n");
    return 0;
}
