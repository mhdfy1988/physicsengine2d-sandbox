#include "../cpp/physics2d.hpp"

int main() {
    physics2d::runtime::RuntimeFacade runtime = physics2d::runtime::RuntimeFacade::create();
    if (!runtime.valid()) {
        return 1;
    }

    physics2d::ecs::Entity a = runtime.registry().create();
    runtime.registry().add_transform(a, physics2d::ecs::Transform{vec2(0.0f, 0.0f), 0.0f});
    runtime.registry().add_collider_circle(a, 1.0f);
    runtime.registry().add_rigidbody_spec(a, physics2d::ecs::RigidBodySpec{1.0f, BODY_DYNAMIC});

    physics2d::ecs::Entity b = runtime.registry().create();
    runtime.registry().add_transform(b, physics2d::ecs::Transform{vec2(0.5f, 0.0f), 0.0f});
    runtime.registry().add_collider_circle(b, 1.0f);
    runtime.registry().add_rigidbody_spec(b, physics2d::ecs::RigidBodySpec{1.0f, BODY_DYNAMIC});

    runtime.tick_snapshot();
    const auto& events1 = runtime.last_events();
    bool has_contact = false;
    for (const auto& e : events1) {
        if (e.kind == physics2d::runtime::RuntimeEventKind::ContactCreated &&
            ((e.entity_a == a && e.entity_b == b) || (e.entity_a == b && e.entity_b == a))) {
            has_contact = true;
            break;
        }
    }
    if (!has_contact) {
        return 2;
    }

    const physics2d::ecs::RuntimeBodyRef* ra = runtime.registry().runtime_body(a);
    if (ra == nullptr || ra->body == nullptr) {
        return 3;
    }
    ra->body->sleeping = 1;

    runtime.tick_snapshot();
    const auto& events2 = runtime.last_events();
    bool has_sleep = false;
    for (const auto& e : events2) {
        if (e.kind == physics2d::runtime::RuntimeEventKind::BodySleep && e.entity_a == a) {
            has_sleep = true;
            break;
        }
    }
    if (!has_sleep) {
        return 4;
    }

    ra->body->sleeping = 0;
    runtime.tick_snapshot();
    const auto& events3 = runtime.last_events();
    bool has_wake = false;
    for (const auto& e : events3) {
        if (e.kind == physics2d::runtime::RuntimeEventKind::BodyWake && e.entity_a == a) {
            has_wake = true;
            break;
        }
    }
    if (!has_wake) {
        return 5;
    }

    return 0;
}
