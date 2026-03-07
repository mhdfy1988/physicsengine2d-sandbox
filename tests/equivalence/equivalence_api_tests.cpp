#include <math.h>
#include <stdio.h>
#include "../../include/physics_core/physics.hpp"
#include "../regression_tests.hpp"

#define EPS_EQ 1e-5f

int regression_test_equivalence_runtime_setters(void) {
    PhysicsEngine* legacy = physics_engine_create();
    PhysicsEngine* layered = physics_engine_create();
    PhysicsConfig a;
    PhysicsConfig b;
    PhysicsConfig base;
    int pass = 1;

    if (legacy == NULL || layered == NULL) {
        printf("[FAIL] failed to create engines for equivalence runtime setter test\n");
        if (legacy != NULL) physics_engine_free(legacy);
        if (layered != NULL) physics_engine_free(layered);
        return 0;
    }

    physics_engine_set_time_step(legacy, 1.0f / 120.0f);
    physics_engine_set_iterations(legacy, 9);
    physics_engine_set_damping(legacy, 0.97f);
    physics_engine_set_broadphase_cell_size(legacy, 7.5f);
    physics_engine_set_broadphase_use_grid(legacy, 1);

    physics_engine_get_config(layered, &base);
    {
        PhysicsRuntimeConfig rcfg;
        PhysicsSolverConfig scfg;
        PhysicsPipelineConfig pcfg;
        rcfg.time_step = 1.0f / 120.0f;
        rcfg.substeps = base.substeps;
        rcfg.damping = 0.97f;
        scfg.iterations = 9;
        scfg.max_position_iterations_bias = base.max_position_iterations_bias;
        pcfg.broadphase_cell_size = 7.5f;
        pcfg.broadphase_type = PHYSICS_BROADPHASE_GRID;
        physics_engine_set_runtime_config(layered, &rcfg);
        physics_engine_set_solver_config(layered, &scfg);
        physics_engine_set_pipeline_config(layered, &pcfg);
    }

    physics_engine_get_config(legacy, &a);
    physics_engine_get_config(layered, &b);

    if (fabsf(a.time_step - b.time_step) > EPS_EQ ||
        a.substeps != b.substeps ||
        a.iterations != b.iterations ||
        fabsf(a.damping - b.damping) > EPS_EQ ||
        fabsf(a.broadphase_cell_size - b.broadphase_cell_size) > EPS_EQ ||
        a.broadphase_type != b.broadphase_type) {
        printf("[FAIL] runtime setter equivalence mismatch\n");
        pass = 0;
    }

    physics_engine_free(legacy);
    physics_engine_free(layered);
    if (!pass) return 0;
    printf("[PASS] equivalence runtime setters\n");
    return 1;
}

int regression_test_equivalence_remove_body_paths(void) {
    PhysicsEngine* detach_path = physics_engine_create();
    PhysicsEngine* remove_path = physics_engine_create();
    RigidBody* a1;
    RigidBody* b1;
    RigidBody* a2;
    RigidBody* b2;
    PhysicsBodyHandle h1;
    PhysicsBodyHandle h2;
    int pass = 1;

    if (detach_path == NULL || remove_path == NULL) {
        printf("[FAIL] failed to create engines for body remove equivalence test\n");
        if (detach_path != NULL) physics_engine_free(detach_path);
        if (remove_path != NULL) physics_engine_free(remove_path);
        return 0;
    }

    a1 = body_create(0.0f, 0.0f, 1.0f, shape_create_circle(1.0f));
    b1 = body_create(2.0f, 0.0f, 1.0f, shape_create_circle(1.0f));
    a2 = body_create(0.0f, 0.0f, 1.0f, shape_create_circle(1.0f));
    b2 = body_create(2.0f, 0.0f, 1.0f, shape_create_circle(1.0f));
    if (a1 == NULL || b1 == NULL || a2 == NULL || b2 == NULL) {
        printf("[FAIL] failed to create bodies for body remove equivalence test\n");
        physics_engine_free(detach_path);
        physics_engine_free(remove_path);
        return 0;
    }

    physics_engine_add_body(detach_path, a1);
    physics_engine_add_body(detach_path, b1);
    physics_engine_add_body(remove_path, a2);
    physics_engine_add_body(remove_path, b2);
    h1 = physics_engine_get_body_handle(detach_path, a1);
    h2 = physics_engine_get_body_handle(remove_path, a2);

    {
        RigidBody* detached = physics_engine_detach_body(detach_path, a1);
        if (detached != NULL) {
            body_free(detached);
        }
    }
    physics_engine_remove_body(remove_path, a2);

    if (physics_engine_get_body_count(detach_path) != physics_engine_get_body_count(remove_path)) {
        printf("[FAIL] body count mismatch after remove-path equivalence\n");
        pass = 0;
    }
    if (physics_engine_resolve_body_handle(detach_path, h1) != NULL ||
        physics_engine_resolve_body_handle(remove_path, h2) != NULL) {
        printf("[FAIL] stale handle should be invalid in both remove paths\n");
        pass = 0;
    }

    physics_engine_free(detach_path);
    physics_engine_free(remove_path);
    if (!pass) return 0;
    printf("[PASS] equivalence remove body paths\n");
    return 1;
}
