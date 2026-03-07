#include "../../include/physics2d/physics_raii.hpp"

int main() {
    physics2d::Engine engine = physics2d::Engine::create();
    if (!engine.valid()) {
        return 1;
    }

    engine.set_gravity(vec2(0.0f, 9.8f));
    engine.step();
    return 0;
}
