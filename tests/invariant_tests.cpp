#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "../include/physics_core/physics.hpp"

static int is_finite_body(const RigidBody* b) {
    if (b == NULL) return 1;
    if (!isfinite(b->position.x) || !isfinite(b->position.y)) return 0;
    if (!isfinite(b->velocity.x) || !isfinite(b->velocity.y)) return 0;
    if (!isfinite(b->angle) || !isfinite(b->angular_velocity)) return 0;
    return 1;
}

int main(void) {
    PhysicsEngine* engine = physics_engine_create();
    int i;
    int step;
    if (engine == NULL) {
        printf("[FAIL] invariant: engine create failed\n");
        return 1;
    }
    srand(1337);
    physics_engine_set_time_step(engine, 1.0f / 120.0f);
    physics_engine_set_gravity(engine, vec2(0.0f, 9.8f));
    for (i = 0; i < 120; i++) {
        float x = (float)(rand() % 100) * 0.7f - 35.0f;
        float y = (float)(rand() % 60) * 0.7f - 20.0f;
        float r = 0.4f + (float)(rand() % 30) / 100.0f;
        RigidBody* b = body_create(x, y, 0.8f + (float)(rand() % 70) / 100.0f, shape_create_circle(r));
        if (b == NULL) continue;
        b->velocity = vec2((float)(rand() % 60) - 30.0f, (float)(rand() % 40) - 20.0f);
        physics_engine_add_body(engine, b);
    }
    {
        RigidBody* floor = body_create(0.0f, 55.0f, 1.0f, shape_create_box(180.0f, 2.0f));
        if (floor != NULL) {
            body_set_type(floor, BODY_STATIC);
            physics_engine_add_body(engine, floor);
        }
    }
    for (step = 0; step < 2500; step++) {
        physics_engine_step(engine);
        for (i = 0; i < physics_engine_get_body_count(engine); i++) {
            const RigidBody* b = physics_engine_get_body(engine, i);
            if (!is_finite_body(b)) {
                printf("[FAIL] invariant: non-finite body state at step %d idx %d\n", step, i);
                physics_engine_free(engine);
                return 1;
            }
        }
    }
    physics_engine_free(engine);
    printf("[PASS] invariant long-run\n");
    return 0;
}

