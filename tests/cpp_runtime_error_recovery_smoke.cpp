#include <cstdio>
#include "physics_runtime_facade.hpp"

int main() {
    physics2d::runtime::RuntimeFacade runtime = physics2d::runtime::RuntimeFacade::create();
    if (!runtime.valid()) {
        std::printf("[FAIL] runtime facade create failed\n");
        return 1;
    }

    physics2d::ecs::Entity e = runtime.registry().create();
    runtime.registry().add_transform(e, physics2d::ecs::Transform{});
    runtime.registry().add_collider_circle(e, 0.5f);
    runtime.registry().add_rigidbody_spec(e, physics2d::ecs::RigidBodySpec{});

    physics2d::runtime::TickSnapshot s0 = runtime.tick_snapshot();
    if (s0.stats.runtime_error_count != 0u) {
        std::printf("[FAIL] expected no error before injection\n");
        return 2;
    }

    if (!runtime.registry().debug_remove_reverse_mapping(e)) {
        std::printf("[FAIL] failed to inject mismatch\n");
        return 3;
    }
    physics2d::runtime::TickSnapshot s1 = runtime.tick_snapshot();
    if (s1.stats.runtime_error_count == 0u) {
        std::printf("[FAIL] expected error after mismatch injection\n");
        return 4;
    }

    runtime.registry().add_collider_circle(e, 0.6f);
    physics2d::runtime::TickSnapshot s2 = runtime.tick_snapshot();
    if (s2.stats.runtime_error_count != 0u || !runtime.last_errors().empty()) {
        std::printf("[FAIL] expected runtime error channel recovery\n");
        return 5;
    }

    std::printf("[PASS] runtime error recovery smoke\n");
    return 0;
}
