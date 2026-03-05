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

    physics2d::runtime::TickStats s1 = runtime.tick();
    if (s1.body_count != 1) {
        return 2;
    }

    if (!runtime.registry().queue_destroy(e)) {
        return 3;
    }
    physics2d::runtime::TickStats s2 = runtime.tick();
    if (s2.cleaned_entities != 1u) {
        return 4;
    }
    if (s2.body_count != 0) {
        return 5;
    }
    if (runtime.registry().alive(e)) {
        return 6;
    }
    return 0;
}
