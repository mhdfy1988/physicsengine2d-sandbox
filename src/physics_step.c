#include <math.h>
#include <stddef.h>
#include "physics_internal.h"

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

void physics_internal_step(PhysicsEngine* engine) {
    enum { PHYSICS_SUBSTEPS = 3 };
    Vec2 saved_force[MAX_BODIES];
    Vec2 prev_pos[MAX_BODIES];
    float prev_angle[MAX_BODIES];
    float saved_torque[MAX_BODIES];
    float full_dt;
    int i;
    int sub;
    if (engine == NULL) {
        return;
    }

    full_dt = engine->time_step;
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

    engine->time_step = full_dt / (float)PHYSICS_SUBSTEPS;
    for (sub = 0; sub < PHYSICS_SUBSTEPS; sub++) {
        for (i = 0; i < engine->body_count; i++) {
            RigidBody* body = engine->bodies[i];
            if (body == NULL) continue;
            body->force = saved_force[i];
            body->torque = saved_torque[i];
        }
        physics_internal_update_velocities(engine);
        physics_internal_update_positions(engine);
        physics_internal_detect_collisions(engine);
        physics_internal_resolve_collisions(engine);
        for (i = 0; i < engine->body_count; i++) {
            sanitize_dynamic_body_after_step(engine->bodies[i], prev_pos[i], prev_angle[i]);
            if (engine->bodies[i] != NULL) {
                prev_pos[i] = engine->bodies[i]->position;
                prev_angle[i] = engine->bodies[i]->angle;
            }
        }
    }
    engine->time_step = full_dt;
    physics_internal_clear_forces(engine);
}
