#include "../../include/physics2d/physics_raii.hpp"
#include "../../include/physics2d/physics_body_raii.hpp"
#include "../../include/physics2d/physics_snapshot.hpp"

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
    engine.step();

    physics2d::Snapshot snapshot;
    if (!snapshot.capture(engine)) {
        return 3;
    }
    if (snapshot.body_count() != 1) {
        return 4;
    }

    engine.step();
    if (!snapshot.apply(engine)) {
        return 5;
    }
    if (!snapshot.replay(engine, 2)) {
        return 6;
    }
    if (!engine.status().ok()) {
        return 7;
    }
    return 0;
}
