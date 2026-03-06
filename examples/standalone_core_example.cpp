#include <cstdio>

#include "physics_core/physics.h"
#include "runtime_snapshot_repo.hpp"

int main() {
    const char* snapshot_path = "_tmp_standalone_core_example.snapshot";
    PhysicsEngine* engine = physics_engine_create();
    Shape* shape;
    RigidBody* body;

    if (engine == nullptr) {
        std::printf("[FAIL] engine create failed\n");
        return 1;
    }

    shape = shape_create_circle(0.5f);
    body = body_create(0.0f, 1.0f, 1.0f, shape);
    physics_engine_add_body(engine, body);
    physics_engine_step(engine);

    if (!runtime_snapshot_repo_save(engine, snapshot_path)) {
        std::printf("[FAIL] snapshot save failed\n");
        physics_engine_free(engine);
        return 2;
    }

    std::printf("[PASS] standalone core example\n");
    remove(snapshot_path);
    physics_engine_free(engine);
    return 0;
}
