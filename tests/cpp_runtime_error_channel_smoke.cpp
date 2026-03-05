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
    runtime.registry().add_collider_circle(e, 0.4f);
    runtime.registry().add_rigidbody_spec(e, physics2d::ecs::RigidBodySpec{});

    physics2d::runtime::TickSnapshot ok = runtime.tick_snapshot();
    if (ok.stats.runtime_error_count != 0u || !runtime.last_errors().empty()) {
        std::printf("[FAIL] expected no runtime errors on healthy bridge\n");
        return 2;
    }

    if (!runtime.registry().debug_remove_reverse_mapping(e)) {
        std::printf("[FAIL] failed to inject bridge mismatch for test\n");
        return 3;
    }

    physics2d::runtime::TickSnapshot bad = runtime.tick_snapshot();
    if (bad.stats.mapping_errors == 0u || bad.stats.runtime_error_count == 0u) {
        std::printf("[FAIL] expected runtime error channel to capture bridge mismatch\n");
        return 4;
    }

    bool saw_missing_reverse = false;
    for (const auto& err : runtime.last_errors()) {
        if (err.code == physics2d::runtime::RuntimeErrorCode::BridgeMissingReverse) {
            saw_missing_reverse = true;
            break;
        }
    }
    if (!saw_missing_reverse) {
        std::printf("[FAIL] missing BridgeMissingReverse runtime error\n");
        return 5;
    }

    std::printf("[PASS] runtime error channel smoke\n");
    return 0;
}
