#include "physics_core/physics.hpp"

int main() {
    PhysicsEngine* engine = physics_engine_create();
    if (engine == nullptr) {
        return 1;
    }

    physics_engine_set_gravity(engine, vec2(0.0f, 9.8f));
    physics_engine_step(engine);
    physics_engine_free(engine);
    return 0;
}
