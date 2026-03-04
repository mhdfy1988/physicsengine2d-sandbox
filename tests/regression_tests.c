#include <math.h>
#include <stdio.h>
#include "../include/physics.h"

#define EPS 1e-3f
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
static int test_circle_circle_detection(void) {
    Shape* s1 = shape_create_circle(2.0f);
    Shape* s2 = shape_create_circle(2.0f);
    RigidBody* a = body_create(0.0f, 0.0f, 1.0f, s1);
    RigidBody* b = body_create(3.0f, 0.0f, 1.0f, s2);

    CollisionInfo info = {0};
    int hit = collision_detect(a, b, &info);

    body_free(a);
    body_free(b);

    if (!hit) {
        printf("[FAIL] circle-circle should collide\n");
        return 0;
    }

    if (fabsf(info.penetration - 1.0f) > EPS) {
        printf("[FAIL] circle-circle penetration expected 1.0 got %.4f\n", info.penetration);
        return 0;
    }

    printf("[PASS] circle-circle detection\n");
    return 1;
}

static int test_circle_inside_polygon(void) {
    Shape* cs = shape_create_circle(1.0f);
    Shape* ps = shape_create_box(10.0f, 10.0f);
    RigidBody* circle = body_create(0.0f, 0.0f, 1.0f, cs);
    RigidBody* box = body_create(0.0f, 0.0f, 1.0f, ps);
    body_set_type(box, BODY_STATIC);

    CollisionInfo info = {0};
    int hit = collision_detect(circle, box, &info);

    body_free(circle);
    body_free(box);

    if (!hit) {
        printf("[FAIL] circle-inside-polygon should collide\n");
        return 0;
    }

    if (info.penetration <= 0.0f) {
        printf("[FAIL] circle-inside-polygon penetration should be positive\n");
        return 0;
    }

    printf("[PASS] circle-inside-polygon detection\n");
    return 1;
}

static int test_collision_detect_null_safety(void) {
    CollisionInfo info = {0};
    Shape* shape = shape_create_circle(1.0f);
    RigidBody* body = body_create(0.0f, 0.0f, 1.0f, shape);
    int pass = 1;

    if (collision_detect(NULL, body, &info) != 0) {
        printf("[FAIL] collision_detect should reject NULL body A\n");
        pass = 0;
    }
    if (collision_detect(body, NULL, &info) != 0) {
        printf("[FAIL] collision_detect should reject NULL body B\n");
        pass = 0;
    }
    if (collision_detect(body, body, NULL) != 0) {
        printf("[FAIL] collision_detect should reject NULL info\n");
        pass = 0;
    }

    body_free(body);
    if (!pass) {
        return 0;
    }

    printf("[PASS] collision_detect null safety\n");
    return 1;
}

static int test_shape_helpers_null_safety(void) {
    Vec2 center = shape_get_center_of_mass(NULL);
    if (fabsf(shape_get_area(NULL)) > EPS ||
        fabsf(shape_get_moment_of_inertia(NULL, 1.0f)) > EPS ||
        fabsf(center.x) > EPS || fabsf(center.y) > EPS) {
        printf("[FAIL] shape helpers should be NULL-safe\n");
        return 0;
    }

    printf("[PASS] shape helper null safety\n");
    return 1;
}

static int test_gravity_override(void) {
    PhysicsEngine* engine = physics_engine_create();
    physics_engine_set_gravity(engine, vec2(0.0f, 20.0f));
    physics_engine_set_time_step(engine, 0.1f);
    physics_engine_set_damping(engine, 1.0f);

    RigidBody* g_default = body_create(0.0f, 0.0f, 1.0f, shape_create_circle(1.0f));
    RigidBody* g_custom = body_create(0.0f, 0.0f, 1.0f, shape_create_circle(1.0f));
    body_set_gravity(g_custom, vec2(0.0f, 0.0f));

    physics_engine_add_body(engine, g_default);
    physics_engine_add_body(engine, g_custom);

    physics_engine_step(engine);

    float vy_default = g_default->velocity.y;
    float vy_custom = g_custom->velocity.y;

    physics_engine_free(engine);

    if (vy_default <= vy_custom + EPS) {
        printf("[FAIL] custom gravity override not applied: default=%.4f custom=%.4f\n", vy_default, vy_custom);
        return 0;
    }

    printf("[PASS] gravity global + custom override\n");
    return 1;
}

static int test_raycast_geometry(void) {
    PhysicsEngine* engine = physics_engine_create();
    physics_engine_set_gravity(engine, vec2(0.0f, 0.0f));

    RigidBody* off_ray = body_create(5.0f, 5.0f, 1.0f, shape_create_circle(1.0f));
    RigidBody* on_ray = body_create(7.0f, 0.0f, 1.0f, shape_create_circle(1.0f));

    body_set_type(off_ray, BODY_STATIC);
    body_set_type(on_ray, BODY_STATIC);

    physics_engine_add_body(engine, off_ray);
    physics_engine_add_body(engine, on_ray);

    RigidBody* hit = physics_engine_raycast(engine, vec2(0.0f, 0.0f), vec2(10.0f, 0.0f));
    int pass = (hit == on_ray);

    physics_engine_free(engine);

    if (!pass) {
        printf("[FAIL] raycast should hit on-ray shape, not off-ray shape\n");
        return 0;
    }

    printf("[PASS] raycast geometric hit\n");
    return 1;
}

static int test_polygon_polygon_sat_axes(void) {
    Shape* a_shape = shape_create_box(4.0f, 20.0f);
    Shape* b_shape = shape_create_box(20.0f, 4.0f);
    RigidBody* a = body_create(0.0f, 0.0f, 1.0f, a_shape);
    RigidBody* b = body_create(8.0f, 8.0f, 1.0f, b_shape);
    a->angle = 0.78539816339f;
    b->angle = -0.78539816339f;

    CollisionInfo info = {0};
    int hit = collision_detect(a, b, &info);

    body_free(a);
    body_free(b);

    if (hit) {
        printf("[FAIL] polygon SAT should separate these rotated boxes\n");
        return 0;
    }

    printf("[PASS] polygon SAT dual-axis separation\n");
    return 1;
}

static int test_high_speed_tunnel_regression(void) {
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

static int test_high_stack_stability_regression(void) {
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

static int test_broadphase_grid_filters_pairs(void) {
    PhysicsEngine* engine = physics_engine_create();
    if (engine == NULL) {
        printf("[FAIL] failed to create engine for broadphase test\n");
        return 0;
    }

    physics_engine_set_gravity(engine, vec2(0.0f, 0.0f));
    physics_engine_set_broadphase_use_grid(engine, 1);
    physics_engine_set_broadphase_cell_size(engine, 8.0f);

    for (int i = 0; i < 60; i++) {
        RigidBody* b = body_create(100.0f + i * 20.0f, 0.0f, 1.0f, shape_create_circle(1.0f));
        body_set_type(b, BODY_STATIC);
        physics_engine_add_body(engine, b);
    }

    RigidBody* a = body_create(0.0f, 0.0f, 1.0f, shape_create_circle(2.0f));
    RigidBody* b = body_create(3.0f, 0.0f, 1.0f, shape_create_circle(2.0f));
    physics_engine_add_body(engine, a);
    physics_engine_add_body(engine, b);

    physics_engine_detect_collisions(engine);

    int pass = 1;
    if (physics_engine_get_contact_count(engine) <= 0) {
        printf("[FAIL] broadphase grid missed obvious overlap\n");
        pass = 0;
    } else if (physics_engine_get_broadphase_pair_count(engine) >= 200) {
        printf("[FAIL] broadphase grid candidate pairs too high: %d\n", physics_engine_get_broadphase_pair_count(engine));
        pass = 0;
    }

    physics_engine_free(engine);
    if (!pass) {
        return 0;
    }

    printf("[PASS] broadphase grid filtering\n");
    return 1;
}

static int test_add_body_duplicate_guard(void) {
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

static int test_distance_constraint_stability(void) {
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

static int test_constraint_break_force(void) {
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

int main(void) {
    int passed = 0;
    int total = 0;
#define RUN_TEST_OR_FAIL(fn)                                          \
    do {                                                               \
        total++;                                                       \
        if (!(fn)()) {                                                 \
            printf("\nResult: FAIL (%d/%d)\n", passed, total);        \
            return 1;                                                  \
        }                                                              \
        passed++;                                                      \
    } while (0)

    RUN_TEST_OR_FAIL(test_circle_circle_detection);
    RUN_TEST_OR_FAIL(test_circle_inside_polygon);
    RUN_TEST_OR_FAIL(test_collision_detect_null_safety);
    RUN_TEST_OR_FAIL(test_shape_helpers_null_safety);
    RUN_TEST_OR_FAIL(test_gravity_override);
    RUN_TEST_OR_FAIL(test_raycast_geometry);
    RUN_TEST_OR_FAIL(test_polygon_polygon_sat_axes);
    RUN_TEST_OR_FAIL(test_high_speed_tunnel_regression);
    RUN_TEST_OR_FAIL(test_high_stack_stability_regression);
    RUN_TEST_OR_FAIL(test_broadphase_grid_filters_pairs);
    RUN_TEST_OR_FAIL(test_add_body_duplicate_guard);
    RUN_TEST_OR_FAIL(test_distance_constraint_stability);
    RUN_TEST_OR_FAIL(test_constraint_break_force);

    printf("\nResult: PASS (%d/%d)\n", passed, total);
    return 0;
}
