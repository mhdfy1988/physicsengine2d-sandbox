#include <cstdio>
#include "physics2d/physics_runtime_facade.hpp"

int main() {
    physics2d::runtime::RuntimeFacade runtime = physics2d::runtime::RuntimeFacade::create();
    if (!runtime.valid()) {
        std::printf("[FAIL] runtime facade create failed\n");
        return 1;
    }

    physics2d::ecs::Entity e1 = runtime.registry().create();
    physics2d::ecs::Entity e2 = runtime.registry().create();
    runtime.registry().add_transform(e1, physics2d::ecs::Transform{});
    runtime.registry().add_transform(e2, physics2d::ecs::Transform{});
    runtime.registry().add_collider_circle(e1, 0.4f);
    runtime.registry().add_collider_circle(e2, 0.4f);
    runtime.registry().add_rigidbody_spec(e1, physics2d::ecs::RigidBodySpec{});
    runtime.registry().add_rigidbody_spec(e2, physics2d::ecs::RigidBodySpec{});

    (void)runtime.tick_snapshot();
    (void)runtime.registry().debug_remove_reverse_mapping(e1);
    (void)runtime.registry().debug_remove_reverse_mapping(e2);

    physics2d::runtime::TickSnapshot bad = runtime.tick_snapshot();
    if (bad.stats.runtime_error_count < 2u) {
        std::printf("[FAIL] expected multiple runtime error entries\n");
        return 2;
    }

    bool saw_missing_reverse = false;
    bool saw_refcount_mismatch = false;
    for (const auto& err : runtime.last_errors()) {
        if (err.code == physics2d::runtime::RuntimeErrorCode::BridgeMissingReverse && err.count >= 2u) {
            saw_missing_reverse = true;
        }
        if (err.code == physics2d::runtime::RuntimeErrorCode::BridgeRefCountMismatch && err.count >= 2u) {
            saw_refcount_mismatch = true;
        }
    }
    if (!saw_missing_reverse || !saw_refcount_mismatch) {
        std::printf("[FAIL] expected missing_reverse and ref_count_mismatch entries\n");
        return 3;
    }

    std::printf("[PASS] runtime multi error smoke\n");
    return 0;
}
