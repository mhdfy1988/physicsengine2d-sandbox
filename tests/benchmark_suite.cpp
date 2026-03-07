#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../include/physics_core/physics.hpp"

static double now_ms(void) {
    return (double)clock() * 1000.0 / (double)CLOCKS_PER_SEC;
}

int main(void) {
    PhysicsEngine* engine = physics_engine_create();
    int i;
    int steps = 600;
    double t0;
    double avg_ms;
    double gate_ms = 8.0;
    const char* env_gate = getenv("PHYSICS_BENCH_GATE_MS");
    if (env_gate != NULL) {
        double v = atof(env_gate);
        if (v > 0.0) gate_ms = v;
    }
    if (engine == NULL) {
        printf("[FAIL] benchmark: engine create failed\n");
        return 1;
    }

    physics_engine_set_gravity(engine, vec2(0.0f, 9.8f));
    physics_engine_set_time_step(engine, 1.0f / 120.0f);
    for (i = 0; i < 180; i++) {
        float x = (float)(i % 18) * 2.1f;
        float y = (float)(i / 18) * 2.1f;
        RigidBody* b = body_create(x, y, 1.0f, shape_create_circle(0.8f));
        if (b != NULL) physics_engine_add_body(engine, b);
    }
    {
        RigidBody* floor = body_create(18.0f, 45.0f, 1.0f, shape_create_box(80.0f, 2.0f));
        if (floor != NULL) {
            body_set_type(floor, BODY_STATIC);
            physics_engine_add_body(engine, floor);
        }
    }

    t0 = now_ms();
    for (i = 0; i < steps; i++) {
        physics_engine_step(engine);
    }
    avg_ms = (now_ms() - t0) / (double)steps;
    printf("[INFO] benchmark avg_step_ms=%.4f gate=%.4f\n", avg_ms, gate_ms);
    physics_engine_free(engine);
    if (avg_ms > gate_ms) {
        printf("[FAIL] benchmark gate exceeded\n");
        return 1;
    }
    printf("[PASS] benchmark gate\n");
    return 0;
}

