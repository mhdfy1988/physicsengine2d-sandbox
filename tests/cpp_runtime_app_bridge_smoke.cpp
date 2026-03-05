#include <cstdio>
#include "physics_runtime_app_bridge.hpp"

static void noop(void* user) {
    (void)user;
}

int main() {
    physics2d::runtime::RuntimeFacade facade = physics2d::runtime::RuntimeFacade::create();
    if (!facade.valid()) {
        std::printf("[FAIL] facade create failed\n");
        return 1;
    }

    AppRuntime app_rt;
    AppCommandCallbacks callbacks;
    AppEvent ev;
    callbacks.toggle_run = noop;
    callbacks.step_once = noop;
    callbacks.reset_scene = noop;
    callbacks.spawn_circle = noop;
    callbacks.spawn_box = noop;
    callbacks.user = nullptr;
    app_runtime_init(&app_rt, callbacks);

    physics2d::ecs::Entity e = facade.registry().create();
    facade.registry().add_transform(e, physics2d::ecs::Transform{});
    facade.registry().add_collider_circle(e, 0.5f);
    facade.registry().add_rigidbody_spec(e, physics2d::ecs::RigidBodySpec{});
    (void)facade.tick_snapshot();
    (void)facade.registry().debug_remove_reverse_mapping(e);
    (void)facade.tick_snapshot();
    if (facade.last_errors().empty()) {
        std::printf("[FAIL] expected facade errors before bridge ingest\n");
        return 2;
    }

    physics2d::runtime::app_runtime_ingest_facade_errors(&app_rt, facade);
    app_runtime_report_tick(&app_rt, nullptr, 0, 0.25f);

    while (app_runtime_pop_event(&app_rt, &ev)) {
        if (ev.type == APP_EVENT_RUNTIME_TICK) {
            if (ev.runtime_snapshot.runtime_error_item_count <= 0) {
                std::printf("[FAIL] expected bridged runtime errors in app snapshot\n");
                return 3;
            }
            if (ev.runtime_snapshot.runtime_errors[0].code != APP_RUNTIME_ERROR_CODE_BRIDGE_MISSING_REVERSE) {
                std::printf("[FAIL] bridge code mapping mismatch: got %d\n", ev.runtime_snapshot.runtime_errors[0].code);
                return 4;
            }
            if (ev.runtime_snapshot.runtime_errors[0].severity != APP_RUNTIME_ERROR_WARNING) {
                std::printf("[FAIL] bridge severity mapping mismatch: got %d\n", ev.runtime_snapshot.runtime_errors[0].severity);
                return 5;
            }
            std::printf("[PASS] runtime app bridge smoke\n");
            return 0;
        }
    }

    std::printf("[FAIL] missing runtime tick event\n");
    return 6;
}
