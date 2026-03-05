#include "../cpp/physics2d.hpp"

int main() {
    physics2d::World world = physics2d::World::create();
    if (!world.valid()) {
        return 1;
    }

    physics2d::EngineView engine = world.engine();
    if (!engine.valid()) {
        return 2;
    }

    physics2d::Body body = physics2d::Body::create_circle(0.0f, 0.0f, 1.0f, 0.5f);
    if (!body.valid()) {
        return 3;
    }

    engine.add_body(body.release());
    if (engine.body_count() != 1) {
        return 4;
    }

    engine.step();
    if (!engine.status().ok()) {
        return 5;
    }
    return 0;
}
