#include <math.h>
#include <stdio.h>
#include "../include/physics.hpp"
#include "regression_tests.hpp"

#define EPS 1e-3f

int regression_test_layered_config_snapshot(void) {
    PhysicsEngine* engine = physics_engine_create();
    PhysicsRuntimeConfig rcfg;
    PhysicsSolverConfig scfg;
    PhysicsPipelineConfig pcfg;
    PhysicsExperimentalConfig ecfg;
    PhysicsConfigSnapshot snap;
    int pass = 1;
    if (engine == NULL) {
        printf("[FAIL] failed to create engine for layered-config test\n");
        return 0;
    }

    rcfg.time_step = 1.0f / 120.0f;
    rcfg.substeps = 2;
    rcfg.damping = 0.97f;
    scfg.iterations = 9;
    scfg.max_position_iterations_bias = 4;
    pcfg.broadphase_cell_size = 7.5f;
    pcfg.broadphase_type = PHYSICS_BROADPHASE_GRID;
    ecfg.ccd_enabled = 1;
    ecfg.sleep_enabled = 0;
    ecfg.threading_enabled = 1;
    ecfg.worker_count = 2;

    physics_engine_set_runtime_config(engine, &rcfg);
    physics_engine_set_solver_config(engine, &scfg);
    physics_engine_set_pipeline_config(engine, &pcfg);
    physics_engine_set_experimental_config(engine, &ecfg);
    physics_engine_step(engine);
    physics_engine_get_step_config_snapshot(engine, &snap);

    if (fabsf(snap.runtime.time_step - rcfg.time_step) > EPS ||
        snap.runtime.substeps != rcfg.substeps ||
        fabsf(snap.runtime.damping - rcfg.damping) > EPS ||
        snap.solver.iterations != scfg.iterations ||
        snap.solver.max_position_iterations_bias != scfg.max_position_iterations_bias ||
        fabsf(snap.pipeline.broadphase_cell_size - pcfg.broadphase_cell_size) > EPS ||
        snap.pipeline.broadphase_type != pcfg.broadphase_type ||
        snap.experimental.ccd_enabled != ecfg.ccd_enabled ||
        snap.experimental.threading_enabled != ecfg.threading_enabled ||
        snap.experimental.worker_count != ecfg.worker_count) {
        printf("[FAIL] layered config snapshot mismatch\n");
        pass = 0;
    }

    physics_engine_free(engine);
    if (!pass) return 0;
    printf("[PASS] layered config snapshot\n");
    return 1;
}

int regression_test_generation_handles(void) {
    PhysicsEngine* engine = physics_engine_create();
    RigidBody* a;
    RigidBody* b;
    Constraint* c;
    PhysicsBodyHandle bh;
    PhysicsConstraintHandle ch;
    int pass = 1;
    if (engine == NULL) {
        printf("[FAIL] failed to create engine for generation-handle test\n");
        return 0;
    }

    a = body_create(0.0f, 0.0f, 1.0f, shape_create_circle(1.0f));
    b = body_create(2.0f, 0.0f, 1.0f, shape_create_circle(1.0f));
    physics_engine_add_body(engine, a);
    physics_engine_add_body(engine, b);
    bh = physics_engine_get_body_handle(engine, a);
    if (physics_engine_resolve_body_handle(engine, bh) != a) {
        printf("[FAIL] body handle should resolve before mutation\n");
        pass = 0;
    }

    c = physics_engine_add_distance_constraint(engine, a, b, a->position, b->position, 0.8f, 1);
    ch = physics_engine_get_constraint_handle(engine, c);
    if (physics_engine_resolve_constraint_handle(engine, ch) == NULL) {
        printf("[FAIL] constraint handle should resolve before mutation\n");
        pass = 0;
    }

    physics_engine_detach_body(engine, a);
    body_free(a);
    if (physics_engine_resolve_body_handle(engine, bh) != NULL) {
        printf("[FAIL] stale body handle should be invalid after mutation\n");
        pass = 0;
    }
    if (physics_engine_resolve_constraint_handle(engine, ch) != NULL) {
        printf("[FAIL] stale constraint handle should be invalid after related mutation\n");
        pass = 0;
    }

    physics_engine_free(engine);
    if (!pass) return 0;
    printf("[PASS] generation handles\n");
    return 1;
}

int regression_test_ccd_swept_circle_contact(void) {
    PhysicsEngine* engine = physics_engine_create();
    PhysicsExperimentalConfig ecfg;
    RigidBody* a;
    RigidBody* b;
    int pass = 1;
    if (engine == NULL) {
        printf("[FAIL] failed to create engine for CCD test\n");
        return 0;
    }
    physics_engine_set_gravity(engine, vec2(0.0f, 0.0f));
    physics_engine_set_time_step(engine, 1.0f / 60.0f);
    ecfg.ccd_enabled = 1;
    ecfg.sleep_enabled = 0;
    ecfg.threading_enabled = 0;
    ecfg.worker_count = 1;
    physics_engine_set_experimental_config(engine, &ecfg);

    a = body_create(0.0f, 0.0f, 1.0f, shape_create_circle(0.8f));
    b = body_create(-3.0f, 0.0f, 1.0f, shape_create_circle(0.8f));
    a->velocity = vec2(400.0f, 0.0f);
    b->velocity = vec2(0.0f, 0.0f);
    body_set_type(b, BODY_STATIC);
    physics_engine_add_body(engine, a);
    physics_engine_add_body(engine, b);

    /* Discrete overlap is absent, CCD pass should still append a contact from swept test. */
    physics_engine_detect_collisions(engine);
    if (physics_engine_get_contact_count(engine) <= 0) {
        printf("[FAIL] CCD expected swept contact but got none\n");
        pass = 0;
    }

    physics_engine_free(engine);
    if (!pass) return 0;
    printf("[PASS] ccd swept circle contact\n");
    return 1;
}

int regression_test_island_solver_decomposition(void) {
    PhysicsEngine* engine = physics_engine_create();
    RigidBody* a1;
    RigidBody* b1;
    RigidBody* a2;
    RigidBody* b2;
    int pass = 1;
    if (engine == NULL) {
        printf("[FAIL] failed to create engine for island decomposition test\n");
        return 0;
    }
    physics_engine_set_gravity(engine, vec2(0.0f, 0.0f));
    physics_engine_set_time_step(engine, 1.0f / 60.0f);
    physics_engine_set_iterations(engine, 8);

    a1 = body_create(0.0f, 0.0f, 1.0f, shape_create_circle(0.7f));
    b1 = body_create(1.0f, 0.0f, 1.0f, shape_create_circle(0.7f));
    a2 = body_create(40.0f, 0.0f, 1.0f, shape_create_circle(0.7f));
    b2 = body_create(41.0f, 0.0f, 1.0f, shape_create_circle(0.7f));
    if (a1 == NULL || b1 == NULL || a2 == NULL || b2 == NULL) {
        physics_engine_free(engine);
        printf("[FAIL] failed to create bodies for island decomposition test\n");
        return 0;
    }
    a1->velocity = vec2(3.0f, 0.0f);
    b1->velocity = vec2(-3.0f, 0.0f);
    a2->velocity = vec2(2.5f, 0.0f);
    b2->velocity = vec2(-2.5f, 0.0f);
    body_set_restitution(a1, 1.0f);
    body_set_restitution(b1, 1.0f);
    body_set_restitution(a2, 1.0f);
    body_set_restitution(b2, 1.0f);

    physics_engine_add_body(engine, a1);
    physics_engine_add_body(engine, b1);
    physics_engine_add_body(engine, a2);
    physics_engine_add_body(engine, b2);

    physics_engine_detect_collisions(engine);
    if (physics_engine_get_contact_count(engine) < 2) {
        printf("[FAIL] expected two island contacts before solve\n");
        pass = 0;
    } else {
        physics_engine_resolve_collisions(engine);
        if (!(a1->velocity.x < 0.0f && b1->velocity.x > 0.0f)) {
            printf("[FAIL] first island did not resolve as expected (%.3f, %.3f)\n", a1->velocity.x, b1->velocity.x);
            pass = 0;
        }
        if (!(a2->velocity.x < 0.0f && b2->velocity.x > 0.0f)) {
            printf("[FAIL] second island did not resolve as expected (%.3f, %.3f)\n", a2->velocity.x, b2->velocity.x);
            pass = 0;
        }
    }

    physics_engine_free(engine);
    if (!pass) return 0;
    printf("[PASS] island solver decomposition\n");
    return 1;
}
