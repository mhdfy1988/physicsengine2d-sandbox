#include "../include/physics2d/physics_raii.hpp"
#include "../include/physics2d/physics_body_raii.hpp"

int main() {
    physics2d::Engine engine = physics2d::Engine::create();
    if (!engine.valid()) {
        return 1;
    }

    physics2d::Body body = physics2d::Body::create_circle(0.0f, 0.0f, 1.0f, 0.5f);
    if (!body.valid()) {
        return 2;
    }

    engine.add_body(body.release());
    if (engine.body_count() != 1) {
        return 3;
    }

    engine.step();
    if (!engine.status().ok()) {
        return 4;
    }
    return 0;
}
