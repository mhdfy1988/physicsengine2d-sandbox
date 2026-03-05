#include <math.h>
#include <stdio.h>
#include "../include/physics.h"
#include "regression_tests.h"

#define HS_STEPS 240
#define HS_TUNNEL_X 62.0f
#define HS_TUNNEL_Y_MIN 14.0f
#define HS_TUNNEL_Y_MAX 42.0f

#define STACK_COLUMNS 3
#define STACK_ROWS 12
#define STACK_STEPS 720
#define STACK_MAX_SPEED 260.0f
#define STACK_MIN_X -10.0f
#define STACK_MAX_X 110.0f
#define STACK_MIN_Y -20.0f
#define STACK_MAX_Y 70.0f

int regression_test_high_speed_tunnel_regression(void) {
    PhysicsEngine* engine = physics_engine_create();
    Shape* thin_shape = shape_create_box(1.2f, 24.0f);
    RigidBody* thin_wall = body_create(58.0f, 28.0f, 1000.0f, thin_shape);
    Shape* fast_shape_a = shape_create_circle(1.0f);
    Shape* fast_shape_b = shape_create_circle(1.0f);
    RigidBody* ball_a = body_create(10.0f, 30.0f, 0.6f, fast_shape_a);
    RigidBody* ball_b = body_create(10.0f, 34.0f, 0.6f, fast_shape_b);
    int i;
    int peak_contacts = 0;
    int tunnel_signals = 0;

    if (engine == NULL) {
        printf("[FAIL] failed to create engine for high-speed regression\n");
        return 0;
    }

    physics_engine_set_gravity(engine, vec2(0.0f, 9.8f));
    physics_engine_set_time_step(engine, 1.0f / 120.0f);
    physics_engine_set_damping(engine, 0.997f);
    physics_engine_set_iterations(engine, 16);

    body_set_type(thin_wall, BODY_STATIC);
    fast_shape_a->friction = 0.05f;
    fast_shape_b->friction = 0.05f;
    fast_shape_a->restitution = 0.75f;
    fast_shape_b->restitution = 0.75f;
    ball_a->velocity = vec2(105.0f, -4.0f);
    ball_b->velocity = vec2(127.0f, -2.0f);

    physics_engine_add_body(engine, thin_wall);
    physics_engine_add_body(engine, ball_a);
    physics_engine_add_body(engine, ball_b);

    for (i = 0; i < HS_STEPS; i++) {
        physics_engine_step(engine);
        if (physics_engine_get_contact_count(engine) > peak_contacts) {
            peak_contacts = physics_engine_get_contact_count(engine);
        }
        if (ball_a->position.x > HS_TUNNEL_X &&
            ball_a->position.y > HS_TUNNEL_Y_MIN &&
            ball_a->position.y < HS_TUNNEL_Y_MAX) {
            tunnel_signals++;
        }
        if (ball_b->position.x > HS_TUNNEL_X &&
            ball_b->position.y > HS_TUNNEL_Y_MIN &&
            ball_b->position.y < HS_TUNNEL_Y_MAX) {
            tunnel_signals++;
        }
    }

    physics_engine_free(engine);

    if (peak_contacts <= 0) {
        printf("[FAIL] high-speed regression expected wall contact but got none\n");
        return 0;
    }
    if (tunnel_signals > 0) {
        printf("[FAIL] high-speed regression detected tunnel-through (%d samples)\n", tunnel_signals);
        return 0;
    }

    printf("[PASS] high-speed tunnel regression\n");
    return 1;
}

int regression_test_high_stack_stability_regression(void) {
    PhysicsEngine* engine = physics_engine_create();
    int c;
    int r;
    int i;
    int dynamic_count = 0;

    if (engine == NULL) {
        printf("[FAIL] failed to create engine for high-stack regression\n");
        return 0;
    }

    physics_engine_set_gravity(engine, vec2(0.0f, 9.8f));
    physics_engine_set_time_step(engine, 1.0f / 60.0f);
    physics_engine_set_damping(engine, 0.998f);
    physics_engine_set_iterations(engine, 22);

    {
        Shape* ground_shape = shape_create_box(120.0f, 2.0f);
        RigidBody* ground = body_create(50.0f, 45.0f, 1000.0f, ground_shape);
        body_set_type(ground, BODY_STATIC);
        physics_engine_add_body(engine, ground);
    }

    for (c = 0; c < STACK_COLUMNS; c++) {
        for (r = 0; r < STACK_ROWS; r++) {
            Shape* box_shape = shape_create_box(4.6f, 4.6f);
            RigidBody* box = body_create(38.0f + c * 12.0f, 43.0f - r * 4.8f, 4.0f, box_shape);
            box_shape->friction = 0.38f;
            box_shape->restitution = 0.05f;
            physics_engine_add_body(engine, box);
            dynamic_count++;
        }
    }

    for (i = 0; i < STACK_STEPS; i++) {
        int bi;
        physics_engine_step(engine);

        for (bi = 0; bi < physics_engine_get_body_count(engine); bi++) {
            RigidBody* b = physics_engine_get_body(engine, bi);
            float speed_sq;
            if (b == NULL || b->type != BODY_DYNAMIC) continue;

            if (!isfinite(b->position.x) || !isfinite(b->position.y) ||
                !isfinite(b->velocity.x) || !isfinite(b->velocity.y)) {
                physics_engine_free(engine);
                printf("[FAIL] high-stack regression produced NaN/Inf at step %d\n", i);
                return 0;
            }

            speed_sq = vec2_length_sq(b->velocity);
            if (speed_sq > STACK_MAX_SPEED * STACK_MAX_SPEED) {
                physics_engine_free(engine);
                printf("[FAIL] high-stack regression unstable speed %.3f at step %d\n", sqrtf(speed_sq), i);
                return 0;
            }

            if (b->position.x < STACK_MIN_X || b->position.x > STACK_MAX_X ||
                b->position.y < STACK_MIN_Y || b->position.y > STACK_MAX_Y) {
                physics_engine_free(engine);
                printf("[FAIL] high-stack regression body escaped bounds at step %d\n", i);
                return 0;
            }
        }
    }

    {
        int alive_dynamic = 0;
        for (i = 0; i < physics_engine_get_body_count(engine); i++) {
            RigidBody* b = physics_engine_get_body(engine, i);
            if (b != NULL && b->type == BODY_DYNAMIC) alive_dynamic++;
        }
        if (alive_dynamic != dynamic_count) {
            physics_engine_free(engine);
            printf("[FAIL] high-stack regression body-count drift: expected %d got %d\n", dynamic_count, alive_dynamic);
            return 0;
        }
    }

    physics_engine_free(engine);
    printf("[PASS] high-stack stability regression\n");
    return 1;
}

int regression_test_add_body_duplicate_guard(void) {
    PhysicsEngine* engine = physics_engine_create();
    RigidBody* body;
    if (engine == NULL) {
        printf("[FAIL] failed to create engine for duplicate-body test\n");
        return 0;
    }

    body = body_create(0.0f, 0.0f, 1.0f, shape_create_circle(1.0f));
    if (body == NULL) {
        physics_engine_free(engine);
        printf("[FAIL] failed to create body for duplicate-body test\n");
        return 0;
    }

    physics_engine_add_body(engine, body);
    physics_engine_add_body(engine, body);

    if (physics_engine_get_body_count(engine) != 1) {
        physics_engine_free(engine);
        printf("[FAIL] duplicate body insertion should be ignored\n");
        return 0;
    }

    physics_engine_free(engine);
    printf("[PASS] duplicate body insertion guard\n");
    return 1;
}

int regression_test_distance_constraint_stability(void) {
    PhysicsEngine* engine = physics_engine_create();
    if (engine == NULL) {
        printf("[FAIL] failed to create engine for constraint test\n");
        return 0;
    }

    physics_engine_set_gravity(engine, vec2(0.0f, 0.0f));
    physics_engine_set_time_step(engine, 1.0f / 120.0f);
    physics_engine_set_damping(engine, 1.0f);
    physics_engine_set_iterations(engine, 14);

    RigidBody* a = body_create(-5.0f, 0.0f, 1.0f, shape_create_circle(0.8f));
    RigidBody* b = body_create(5.0f, 0.0f, 1.0f, shape_create_circle(0.8f));
    physics_engine_add_body(engine, a);
    physics_engine_add_body(engine, b);

    if (physics_engine_add_distance_constraint(engine, a, b, a->position, b->position, 0.95f, 0) == NULL) {
        physics_engine_free(engine);
        printf("[FAIL] failed to add distance constraint\n");
        return 0;
    }

    a->velocity = vec2(24.0f, 0.0f);
    b->velocity = vec2(-4.0f, 0.0f);

    for (int i = 0; i < 240; i++) {
        physics_engine_step(engine);
    }

    float final_dist = vec2_length(vec2_sub(a->position, b->position));
    physics_engine_free(engine);

    if (fabsf(final_dist - 10.0f) > 0.55f) {
        printf("[FAIL] distance constraint drifted: got %.4f expected near 10.0\n", final_dist);
        return 0;
    }

    printf("[PASS] distance constraint stability\n");
    return 1;
}

int regression_test_constraint_break_force(void) {
    PhysicsEngine* engine = physics_engine_create();
    if (engine == NULL) {
        printf("[FAIL] failed to create engine for break-force test\n");
        return 0;
    }

    physics_engine_set_gravity(engine, vec2(0.0f, 0.0f));
    physics_engine_set_time_step(engine, 1.0f / 120.0f);
    physics_engine_set_damping(engine, 1.0f);
    physics_engine_set_iterations(engine, 12);

    RigidBody* a = body_create(-4.0f, 0.0f, 1.0f, shape_create_circle(0.8f));
    RigidBody* b = body_create(4.0f, 0.0f, 1.0f, shape_create_circle(0.8f));
    physics_engine_add_body(engine, a);
    physics_engine_add_body(engine, b);

    Constraint* c = physics_engine_add_distance_constraint(engine, a, b, a->position, b->position, 1.0f, 0);
    if (c == NULL) {
        physics_engine_free(engine);
        printf("[FAIL] failed to add constraint for break-force test\n");
        return 0;
    }
    c->break_force = 40.0f;
    a->velocity = vec2(120.0f, 0.0f);
    b->velocity = vec2(-120.0f, 0.0f);

    for (int i = 0; i < 60; i++) {
        physics_engine_step(engine);
        if (!c->active) break;
    }

    int pass = !c->active;
    physics_engine_free(engine);
    if (!pass) {
        printf("[FAIL] break-force constraint should deactivate under high load\n");
        return 0;
    }

    printf("[PASS] constraint break-force deactivation\n");
    return 1;
}
