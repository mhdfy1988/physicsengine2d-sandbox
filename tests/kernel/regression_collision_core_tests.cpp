#include <math.h>
#include <stdio.h>
#include "../../include/physics_core/physics.hpp"
#include "../regression_tests.hpp"

#define COLLISION_CORE_EPS 1e-3f

int regression_test_circle_circle_detection(void) {
    Shape* s1 = shape_create_circle(2.0f);
    Shape* s2 = shape_create_circle(2.0f);
    RigidBody* a = body_create(0.0f, 0.0f, 1.0f, s1);
    RigidBody* b = body_create(3.0f, 0.0f, 1.0f, s2);

    CollisionInfo info{};
    int hit = collision_detect(a, b, &info);

    body_free(a);
    body_free(b);

    if (!hit) {
        printf("[FAIL] circle-circle should collide\n");
        return 0;
    }

    if (fabsf(info.penetration - 1.0f) > COLLISION_CORE_EPS) {
        printf("[FAIL] circle-circle penetration expected 1.0 got %.4f\n", info.penetration);
        return 0;
    }

    printf("[PASS] circle-circle detection\n");
    return 1;
}

int regression_test_circle_inside_polygon(void) {
    Shape* cs = shape_create_circle(1.0f);
    Shape* ps = shape_create_box(10.0f, 10.0f);
    RigidBody* circle = body_create(0.0f, 0.0f, 1.0f, cs);
    RigidBody* box = body_create(0.0f, 0.0f, 1.0f, ps);
    body_set_type(box, BODY_STATIC);

    CollisionInfo info{};
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

int regression_test_collision_detect_null_safety(void) {
    CollisionInfo info{};
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

int regression_test_shape_helpers_null_safety(void) {
    Vec2 center = shape_get_center_of_mass(NULL);
    if (fabsf(shape_get_area(NULL)) > COLLISION_CORE_EPS ||
        fabsf(shape_get_moment_of_inertia(NULL, 1.0f)) > COLLISION_CORE_EPS ||
        fabsf(center.x) > COLLISION_CORE_EPS || fabsf(center.y) > COLLISION_CORE_EPS) {
        printf("[FAIL] shape helpers should be NULL-safe\n");
        return 0;
    }

    printf("[PASS] shape helper null safety\n");
    return 1;
}

int regression_test_gravity_override(void) {
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

    if (vy_default <= vy_custom + COLLISION_CORE_EPS) {
        printf("[FAIL] custom gravity override not applied: default=%.4f custom=%.4f\n", vy_default, vy_custom);
        return 0;
    }

    printf("[PASS] gravity global + custom override\n");
    return 1;
}

int regression_test_raycast_geometry(void) {
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

int regression_test_polygon_polygon_sat_axes(void) {
    Shape* a_shape = shape_create_box(4.0f, 20.0f);
    Shape* b_shape = shape_create_box(20.0f, 4.0f);
    RigidBody* a = body_create(0.0f, 0.0f, 1.0f, a_shape);
    RigidBody* b = body_create(8.0f, 8.0f, 1.0f, b_shape);
    a->angle = 0.78539816339f;
    b->angle = -0.78539816339f;

    CollisionInfo info{};
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
