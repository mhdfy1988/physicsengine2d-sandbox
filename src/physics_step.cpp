#include <math.h>
#include <stddef.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include "physics_internal.h"
#include "internal/physics_tuning.h"

static double time_now_ms(void) {
#ifdef _WIN32
    static LARGE_INTEGER freq = {};
    LARGE_INTEGER now;
    if (freq.QuadPart == 0) {
        QueryPerformanceFrequency(&freq);
    }
    QueryPerformanceCounter(&now);
    return (double)now.QuadPart * 1000.0 / (double)freq.QuadPart;
#elif defined(CLOCK_MONOTONIC)
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
        return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1000000.0;
    }
    return (double)clock() * 1000.0 / (double)CLOCKS_PER_SEC;
#else
    return (double)clock() * 1000.0 / (double)CLOCKS_PER_SEC;
#endif
}

static void sanitize_dynamic_body_after_step(RigidBody* body, Vec2 prev_pos, float prev_angle) {
    const float max_linear_speed = 120.0f;
    const float max_angular_speed = 40.0f;
    float v_len;
    if (body == NULL || body->type != BODY_DYNAMIC) {
        return;
    }

    if (!isfinite(body->position.x) || !isfinite(body->position.y)) {
        body->position = prev_pos;
        body->velocity = vec2(0.0f, 0.0f);
    }
    if (!isfinite(body->angle)) {
        body->angle = prev_angle;
        body->angular_velocity = 0.0f;
    }
    if (!isfinite(body->velocity.x) || !isfinite(body->velocity.y)) {
        body->velocity = vec2(0.0f, 0.0f);
    }
    if (!isfinite(body->angular_velocity)) {
        body->angular_velocity = 0.0f;
    }

    v_len = vec2_length(body->velocity);
    if (v_len > max_linear_speed && v_len > 1e-6f) {
        body->velocity = vec2_scale(body->velocity, max_linear_speed / v_len);
    }
    if (body->angular_velocity > max_angular_speed) body->angular_velocity = max_angular_speed;
    if (body->angular_velocity < -max_angular_speed) body->angular_velocity = -max_angular_speed;
}

static void step_stage_integrate(PhysicsEngine* engine, float dt, Vec2* prev_pos, float* prev_angle) {
    int i;
    physics_internal_update_velocities(engine, dt);
    physics_internal_update_positions(engine, dt);
    for (i = 0; i < engine->body_count; i++) {
        sanitize_dynamic_body_after_step(engine->bodies[i], prev_pos[i], prev_angle[i]);
        if (engine->bodies[i] != NULL) {
            prev_pos[i] = engine->bodies[i]->position;
            prev_angle[i] = engine->bodies[i]->angle;
        }
    }
}

static void step_stage_solve(PhysicsEngine* engine, float dt) {
    PhysicsSolverContext solver_ctx;
    solver_ctx.velocity_iterations = engine->config.iterations;
    solver_ctx.position_iterations = engine->config.iterations + engine->config.max_position_iterations_bias;
    solver_ctx.dt = dt;
    physics_internal_resolve_collisions(engine, &solver_ctx);
}

static void step_stage_capture_persistent_contacts(PhysicsEngine* engine) {
    int i;
    if (engine == NULL) return;
    engine->persistent_contact_count = 0;
    for (i = 0; i < engine->contact_count && i < MAX_CONTACTS; i++) {
        engine->persistent_contacts[i] = engine->contacts[i];
        engine->persistent_contact_count++;
    }
}

static int step_body_has_contact(const PhysicsEngine* engine, const RigidBody* body) {
    int i;
    if (engine == NULL || body == NULL) return 0;
    for (i = 0; i < engine->contact_count; i++) {
        const CollisionManifold* c = &engine->contacts[i];
        if (c->bodyA == body || c->bodyB == body) return 1;
    }
    return 0;
}

static void step_stage_update_sleep(PhysicsEngine* engine, float dt) {
    int i;
    const float sleep_linear = PHYSICS_TUNE_SLEEP_LINEAR;
    const float sleep_angular = PHYSICS_TUNE_SLEEP_ANGULAR;
    const float contact_sleep_linear = PHYSICS_TUNE_SLEEP_CONTACT_LINEAR;
    const float contact_sleep_angular = PHYSICS_TUNE_SLEEP_CONTACT_ANGULAR;
    const float sleep_time = PHYSICS_TUNE_SLEEP_TIME;
    if (engine == NULL || !engine->experimental.sleep_enabled) return;
    for (i = 0; i < engine->body_count; i++) {
        RigidBody* b = engine->bodies[i];
        Vec2 gravity_force;
        Vec2 external_force;
        float linear_threshold;
        float angular_threshold;
        if (b == NULL || !b->active || b->type != BODY_DYNAMIC) continue;
        gravity_force = vec2_scale(b->use_custom_gravity ? b->gravity : engine->gravity, b->mass);
        external_force = vec2_sub(b->force, gravity_force);
        if (vec2_length_sq(external_force) > 1e-8f || fabsf(b->torque) > 1e-8f) {
            b->sleeping = 0;
            b->sleep_timer = 0.0f;
            continue;
        }
        if (step_body_has_contact(engine, b)) {
            linear_threshold = contact_sleep_linear;
            angular_threshold = contact_sleep_angular;
        } else {
            linear_threshold = sleep_linear;
            angular_threshold = sleep_angular;
        }
        if (vec2_length_sq(b->velocity) <= linear_threshold * linear_threshold &&
            fabsf(b->angular_velocity) <= angular_threshold) {
            b->sleep_timer += dt;
            if (b->sleep_timer >= sleep_time) {
                b->sleeping = 1;
                b->velocity = vec2(0.0f, 0.0f);
                b->angular_velocity = 0.0f;
            }
        } else {
            b->sleeping = 0;
            b->sleep_timer = 0.0f;
        }
    }
}

extern "C" {

void physics_internal_step(PhysicsEngine* engine) {
    Vec2 saved_force[MAX_BODIES];
    Vec2 prev_pos[MAX_BODIES];
    float prev_angle[MAX_BODIES];
    float saved_torque[MAX_BODIES];
    PhysicsStepProfile profile;
    float full_dt;
    float sub_dt;
    int substeps;
    int i;
    int sub;
    double t0;
    if (engine == NULL) {
        return;
    }
    engine->last_step_config_snapshot.runtime.time_step = engine->config.time_step;
    engine->last_step_config_snapshot.runtime.substeps = engine->config.substeps;
    engine->last_step_config_snapshot.runtime.damping = engine->config.damping;
    engine->last_step_config_snapshot.solver.iterations = engine->config.iterations;
    engine->last_step_config_snapshot.solver.max_position_iterations_bias = engine->config.max_position_iterations_bias;
    engine->last_step_config_snapshot.pipeline.broadphase_cell_size = engine->config.broadphase_cell_size;
    engine->last_step_config_snapshot.pipeline.broadphase_type = engine->config.broadphase_type;
    engine->last_step_config_snapshot.experimental = engine->experimental;

    physics_internal_emit_event(engine, PHYSICS_EVENT_STEP_BEGIN, 0, 0.0, NULL, NULL);

    memset(&profile, 0, sizeof(profile));
    t0 = time_now_ms();
    full_dt = engine->config.time_step;
    substeps = engine->config.substeps;
    if (substeps < 1) substeps = 1;
    if (substeps > 8) substeps = 8;

    for (i = 0; i < engine->body_count; i++) {
        RigidBody* body = engine->bodies[i];
        if (body == NULL) {
            saved_force[i] = vec2(0.0f, 0.0f);
            saved_torque[i] = 0.0f;
            prev_pos[i] = vec2(0.0f, 0.0f);
            prev_angle[i] = 0.0f;
            continue;
        }
        saved_force[i] = body->force;
        saved_torque[i] = body->torque;
        prev_pos[i] = body->position;
        prev_angle[i] = body->angle;
    }

    sub_dt = full_dt / (float)substeps;
    for (sub = 0; sub < substeps; sub++) {
        double ta;
        double tb;
        engine->current_step_dt = sub_dt;
        for (i = 0; i < engine->body_count; i++) {
            RigidBody* body = engine->bodies[i];
            if (body == NULL) continue;
            body->force = saved_force[i];
            body->torque = saved_torque[i];
        }

        ta = time_now_ms();
        step_stage_integrate(engine, sub_dt, prev_pos, prev_angle);
        tb = time_now_ms();
        profile.integrate_ms += (tb - ta);

        physics_internal_scratch_reset(engine);
        engine->contact_count = 0;
        ta = time_now_ms();
        physics_internal_prepare_collision_inputs(engine);
        if (engine->broadphase_ops.build_pairs != NULL) {
            engine->broadphase_ops.build_pairs(engine, engine->broadphase_ops.user);
        }
        tb = time_now_ms();
        profile.broadphase_ms += (tb - ta);
        physics_internal_emit_event(engine, PHYSICS_EVENT_POST_BROADPHASE, engine->broadphase_pair_count, (tb - ta), NULL, NULL);

        ta = time_now_ms();
        if (engine->narrowphase_ops.build_contacts != NULL) {
            engine->narrowphase_ops.build_contacts(engine, engine->narrowphase_ops.user);
        }
        physics_internal_append_ccd_contacts(engine, sub_dt);
        tb = time_now_ms();
        profile.narrowphase_ms += (tb - ta);
        physics_internal_emit_event(engine, PHYSICS_EVENT_POST_NARROWPHASE, engine->contact_count, (tb - ta), NULL, NULL);

        ta = time_now_ms();
        step_stage_solve(engine, sub_dt);
        step_stage_update_sleep(engine, sub_dt);
        tb = time_now_ms();
        profile.solve_ms += (tb - ta);
        step_stage_capture_persistent_contacts(engine);
    }

    {
        double ta = time_now_ms();
        physics_internal_clear_forces(engine);
        profile.clear_forces_ms = time_now_ms() - ta;
    }

    profile.step_index = engine->last_profile.step_index + 1;
    profile.pair_count = engine->broadphase_pair_count;
    profile.contact_count = engine->contact_count;
    profile.total_ms = time_now_ms() - t0;
    engine->last_profile = profile;
    engine->current_step_dt = full_dt;
    physics_internal_emit_event(engine, PHYSICS_EVENT_STEP_END, profile.contact_count, profile.total_ms, NULL, NULL);
}

}  // extern "C"
