#include "../cpp/physics2d.hpp"

int main() {
    physics2d::runtime::RuntimeFacade runtime = physics2d::runtime::RuntimeFacade::create();
    if (!runtime.valid()) {
        return 1;
    }

    physics2d::ecs::Entity e = runtime.registry().create();
    runtime.registry().add_transform(e, physics2d::ecs::Transform{vec2(0.0f, 0.0f), 0.0f});
    runtime.registry().add_collider_circle(e, 0.5f);
    runtime.registry().add_rigidbody_spec(e, physics2d::ecs::RigidBodySpec{1.0f, BODY_DYNAMIC});

    physics2d::runtime::TickSnapshot snap1 = runtime.tick_snapshot();
    if (snap1.frame_index != 1u) {
        return 2;
    }
    if (snap1.stats.body_count != 1) {
        return 3;
    }
    if (snap1.stats.spawned_entities != 1u) {
        return 4;
    }
    if (snap1.stats.synced_entities != 1u) {
        return 5;
    }
    if (!snap1.bridge.ok()) {
        return 6;
    }
    if (runtime.last_snapshot().frame_index != snap1.frame_index) {
        return 7;
    }

    if (!runtime.registry().queue_destroy(e)) {
        return 8;
    }
    physics2d::runtime::TickStats s2 = runtime.tick();
    if (s2.cleaned_entities != 1u) {
        return 9;
    }
    if (s2.body_count != 0) {
        return 10;
    }
    if (runtime.last_snapshot().frame_index != 2u) {
        return 11;
    }
    if (!runtime.last_snapshot().bridge.ok()) {
        return 12;
    }
    if (runtime.registry().alive(e)) {
        return 13;
    }
    return 0;
}
