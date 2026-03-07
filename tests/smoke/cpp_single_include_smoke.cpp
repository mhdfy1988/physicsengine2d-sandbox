#include "../../include/physics2d/physics2d.hpp"

int main() {
    physics2d::Engine engine = physics2d::Engine::create();
    if (!engine.valid()) {
        return 1;
    }

    physics2d::Body body = physics2d::Body::create_box(0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    if (!body.valid()) {
        return 2;
    }

    physics2d::Snapshot snap;
    engine.add_body(body.release());
    if (!snap.capture(engine)) {
        return 3;
    }
    if (snap.body_count() != 1) {
        return 4;
    }
    return 0;
}
