#include "../cpp/physics_raii.hpp"

int main() {
    physics2d::Engine engine = physics2d::Engine::create();
    if (!engine.valid()) {
        return 1;
    }

    engine.clear_error();
    if (!engine.status().ok()) {
        return 2;
    }

    if (engine.add_broadphase_pair(-1, 0)) {
        return 3;
    }

    physics2d::Status s = engine.status();
    if (s.code != PHYSICS_ERROR_INVALID_ARGUMENT) {
        return 4;
    }
    return 0;
}
