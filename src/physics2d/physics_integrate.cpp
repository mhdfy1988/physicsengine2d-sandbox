#include <math.h>
#include <stddef.h>
#include "physics_internal.hpp"

extern "C" {

void physics_internal_update_velocities(PhysicsEngine* engine, float dt) {
    int i;
    if (engine == NULL || dt <= 0.0f) {
        return;
    }

    for (i = 0; i < engine->body_count; i++) {
        RigidBody* body = engine->bodies[i];
        if (body == NULL || body->type != BODY_DYNAMIC) {
            continue;
        }
        if (engine->experimental.sleep_enabled && body->sleeping) {
            if (vec2_length_sq(body->force) > 1e-8f || fabsf(body->torque) > 1e-8f) {
                body->sleeping = 0;
                body->sleep_timer = 0.0f;
            } else {
                body->velocity = vec2(0.0f, 0.0f);
                body->angular_velocity = 0.0f;
                continue;
            }
        }
        if (!physics_internal_body_has_finite_state(body)) {
            body->active = 0;
            continue;
        }

        {
            Vec2 gravity = body->use_custom_gravity ? body->gravity : engine->gravity;
            Vec2 gravity_force = vec2_scale(gravity, body->mass);
            body->force = vec2_add(body->force, gravity_force);
        }

        body->acceleration = vec2_scale(body->force, body->inv_mass);
        body->angular_acceleration = body->torque * body->inv_inertia;

        body->velocity = vec2_add(body->velocity, vec2_scale(body->acceleration, dt));
        body->angular_velocity += body->angular_acceleration * dt;

        body->velocity = vec2_scale(body->velocity, engine->config.damping * body->damping);
        body->angular_velocity *= (engine->config.damping * body->damping);

        if (vec2_length_sq(body->velocity) < 1e-4f) {
            body->velocity = vec2(0.0f, 0.0f);
        }
        if (fabsf(body->angular_velocity) < 0.01f) {
            body->angular_velocity = 0.0f;
        }
    }
}

void physics_internal_update_positions(PhysicsEngine* engine, float dt) {
    int i;
    if (engine == NULL || dt <= 0.0f) {
        return;
    }

    for (i = 0; i < engine->body_count; i++) {
        RigidBody* body = engine->bodies[i];
        if (body == NULL || body->type == BODY_STATIC) {
            continue;
        }
        if (engine->experimental.sleep_enabled && body->type == BODY_DYNAMIC && body->sleeping) {
            continue;
        }
        if (!physics_internal_body_has_finite_state(body)) {
            body->active = 0;
            continue;
        }

        body->position = vec2_add(body->position, vec2_scale(body->velocity, dt));
        body->angle += body->angular_velocity * dt;
    }
}

void physics_internal_clear_forces(PhysicsEngine* engine) {
    int i;
    if (engine == NULL) {
        return;
    }

    for (i = 0; i < engine->body_count; i++) {
        RigidBody* body = engine->bodies[i];
        if (body == NULL) {
            continue;
        }
        body->force = vec2(0.0f, 0.0f);
        body->torque = 0.0f;
    }
}

}  // extern "C"
