#include "../include/physics2d/physics_raii.hpp"
#include "../include/physics2d/physics_body_raii.hpp"

int main() {
    physics2d::Engine engine = physics2d::Engine::create();
    if (!engine.valid()) {
        return 1;
    }

    physics2d::Body body_a = physics2d::Body::create_circle(-1.0f, 0.0f, 1.0f, 0.5f);
    physics2d::Body body_b = physics2d::Body::create_circle(1.0f, 0.0f, 1.0f, 0.5f);
    if (!body_a.valid() || !body_b.valid()) {
        return 2;
    }

    RigidBody* raw_a = body_a.get();
    RigidBody* raw_b = body_b.get();
    engine.add_body(body_a.release());
    engine.add_body(body_b.release());
    if (engine.body_count() != 2) {
        return 3;
    }

    Constraint* c = engine.add_distance_constraint(
        raw_a, raw_b, vec2(-0.5f, 0.0f), vec2(0.5f, 0.0f), 0.8f, 0);
    if (c == nullptr) {
        return 4;
    }
    if (engine.constraint_count() != 1) {
        return 5;
    }
    if (!engine.constraint_active(0)) {
        return 6;
    }

    engine.set_constraint_active(0, false);
    if (engine.constraint_active(0)) {
        return 7;
    }

    engine.step();
    if (!engine.status().ok()) {
        return 8;
    }
    return 0;
}
