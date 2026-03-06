#include <math.h>
#include <stddef.h>
#include "../include/constraint.hpp"
#include "internal/physics_math_internal.hpp"
#include "internal/physics_tuning.hpp"

static float constraint_mass_denom(const RigidBody* a, const RigidBody* b, Vec2 ra, Vec2 rb, Vec2 n) {
    float ra_cn = vec2_cross(ra, n);
    float rb_cn = vec2_cross(rb, n);
    return a->inv_mass + b->inv_mass + ra_cn * ra_cn * a->inv_inertia + rb_cn * rb_cn * b->inv_inertia;
}

static int constraint_compute_axis(Constraint* c, Vec2* out_ra, Vec2* out_rb, Vec2* out_n, float* out_dist) {
    RigidBody* a;
    RigidBody* b;
    Vec2 ra;
    Vec2 rb;
    Vec2 pa;
    Vec2 pb;
    Vec2 delta;
    float dist;
    if (c == NULL || out_ra == NULL || out_rb == NULL || out_n == NULL || out_dist == NULL) {
        return 0;
    }
    if (!c->active || c->body_a == NULL || c->body_b == NULL) {
        return 0;
    }
    a = c->body_a;
    b = c->body_b;
    ra = vec2_rotate(c->local_anchor_a, a->angle);
    rb = vec2_rotate(c->local_anchor_b, b->angle);
    pa = vec2_add(a->position, ra);
    pb = vec2_add(b->position, rb);
    delta = vec2_sub(pb, pa);
    dist = vec2_length(delta);
    if (dist < 1e-6f) {
        return 0;
    }
    *out_ra = ra;
    *out_rb = rb;
    *out_n = vec2_scale(delta, 1.0f / dist);
    *out_dist = dist;
    return 1;
}

void constraint_warm_start(Constraint* c) {
    RigidBody* a;
    RigidBody* b;
    Vec2 ra;
    Vec2 rb;
    Vec2 n;
    float dist;
    Vec2 impulse;
    if (c == NULL || !c->active || c->body_a == NULL || c->body_b == NULL) {
        return;
    }
    if (fabsf(c->last_impulse) < 1e-6f) {
        return;
    }
    if (!constraint_compute_axis(c, &ra, &rb, &n, &dist)) {
        return;
    }
    a = c->body_a;
    b = c->body_b;
    impulse = vec2_scale(n, c->last_impulse);
    a->velocity = vec2_sub(a->velocity, vec2_scale(impulse, a->inv_mass));
    b->velocity = vec2_add(b->velocity, vec2_scale(impulse, b->inv_mass));
    a->angular_velocity -= vec2_cross(ra, impulse) * a->inv_inertia;
    b->angular_velocity += vec2_cross(rb, impulse) * b->inv_inertia;
}

void constraint_init_distance(Constraint* c, RigidBody* a, RigidBody* b,
                              Vec2 world_anchor_a, Vec2 world_anchor_b,
                              float stiffness, int collide_connected) {
    if (c == NULL || a == NULL || b == NULL) {
        return;
    }

    c->type = CONSTRAINT_DISTANCE;
    c->body_a = a;
    c->body_b = b;
    c->local_anchor_a = body_get_local_point(a, world_anchor_a);
    c->local_anchor_b = body_get_local_point(b, world_anchor_b);
    c->rest_length = vec2_length(vec2_sub(world_anchor_b, world_anchor_a));
    c->stiffness = clamp(stiffness, PHYSICS_TUNE_CONSTRAINT_MIN_STIFFNESS, 1.0f);
    c->damping = 0.0f;
    c->break_force = 0.0f;
    c->last_impulse = 0.0f;
    c->last_force = 0.0f;
    c->collide_connected = collide_connected ? 1 : 0;
    c->active = 1;
}

void constraint_init_spring(Constraint* c, RigidBody* a, RigidBody* b,
                            Vec2 world_anchor_a, Vec2 world_anchor_b,
                            float rest_length, float stiffness, float damping,
                            int collide_connected) {
    if (c == NULL || a == NULL || b == NULL) {
        return;
    }

    c->type = CONSTRAINT_SPRING;
    c->body_a = a;
    c->body_b = b;
    c->local_anchor_a = body_get_local_point(a, world_anchor_a);
    c->local_anchor_b = body_get_local_point(b, world_anchor_b);
    c->rest_length = max_f(rest_length, 0.0f);
    c->stiffness = max_f(stiffness, 0.0f);
    c->damping = max_f(damping, 0.0f);
    c->break_force = 0.0f;
    c->last_impulse = 0.0f;
    c->last_force = 0.0f;
    c->collide_connected = collide_connected ? 1 : 0;
    c->active = 1;
}

void constraint_init_rope(Constraint* c, RigidBody* a, RigidBody* b,
                          Vec2 world_anchor_a, Vec2 world_anchor_b,
                          float max_length, float stiffness, int collide_connected) {
    if (c == NULL || a == NULL || b == NULL) {
        return;
    }
    c->type = CONSTRAINT_ROPE;
    c->body_a = a;
    c->body_b = b;
    c->local_anchor_a = body_get_local_point(a, world_anchor_a);
    c->local_anchor_b = body_get_local_point(b, world_anchor_b);
    c->rest_length = max_f(max_length, 0.0f);
    c->stiffness = clamp(stiffness, PHYSICS_TUNE_CONSTRAINT_MIN_STIFFNESS, 1.0f);
    c->damping = 0.0f;
    c->break_force = 0.0f;
    c->last_impulse = 0.0f;
    c->last_force = 0.0f;
    c->collide_connected = collide_connected ? 1 : 0;
    c->active = 1;
}

void constraint_solve_velocity(Constraint* c, float dt) {
    const float wake_impulse_threshold = PHYSICS_TUNE_WAKE_IMPULSE_THRESHOLD;
    if (c == NULL || !c->active || c->body_a == NULL || c->body_b == NULL) {
        return;
    }

    RigidBody* a = c->body_a;
    RigidBody* b = c->body_b;

    Vec2 ra;
    Vec2 rb;
    Vec2 n;
    float dist;
    if (!constraint_compute_axis(c, &ra, &rb, &n, &dist)) {
        return;
    }

    float denom = constraint_mass_denom(a, b, ra, rb, n);
    if (denom <= 1e-6f) {
        return;
    }

    Vec2 va = vec2_add(a->velocity, physics_cross_scalar_vec(a->angular_velocity, ra));
    Vec2 vb = vec2_add(b->velocity, physics_cross_scalar_vec(b->angular_velocity, rb));
    float rel_n = vec2_dot(vec2_sub(vb, va), n);
    float x = dist - c->rest_length;
    float j = 0.0f;

    if (c->type == CONSTRAINT_DISTANCE) {
        float effective_stiffness = max_f(c->stiffness, PHYSICS_TUNE_CONSTRAINT_MIN_STIFFNESS);
        float inv_dt = (dt > 1e-6f) ? (1.0f / dt) : 0.0f;
        float target_rel = -effective_stiffness * x * inv_dt;
        j = (target_rel - rel_n) / denom;
    } else if (c->type == CONSTRAINT_ROPE) {
        float effective_stiffness = max_f(c->stiffness, PHYSICS_TUNE_CONSTRAINT_MIN_STIFFNESS);
        float inv_dt = (dt > 1e-6f) ? (1.0f / dt) : 0.0f;
        if (x <= 0.0f) {
            c->last_impulse = 0.0f;
            c->last_force = 0.0f;
            return;
        }
        {
            float target_rel = -effective_stiffness * x * inv_dt;
            j = (target_rel - rel_n) / denom;
            if (j > 0.0f) {
                j = 0.0f;
            }
        }
    } else {
        float effective_stiffness = max_f(c->stiffness, PHYSICS_TUNE_CONSTRAINT_SPRING_MIN_STIFFNESS);
        float desired_force = -effective_stiffness * x - c->damping * rel_n;
        j = (desired_force * dt) / denom;
    }
    if (!isfinite(j)) {
        c->last_impulse = 0.0f;
        c->last_force = 0.0f;
        return;
    }

    c->last_impulse += j;
    c->last_impulse = clamp(c->last_impulse, -PHYSICS_TUNE_CONSTRAINT_IMPULSE_LIMIT,
                            PHYSICS_TUNE_CONSTRAINT_IMPULSE_LIMIT);
    c->last_force = (dt > 1e-6f) ? (fabsf(j) / dt) : 0.0f;
    if (c->break_force > 0.0f && c->last_force > c->break_force) {
        c->active = 0;
        c->last_impulse = 0.0f;
        return;
    }

    Vec2 impulse = vec2_scale(n, j);
    a->velocity = vec2_sub(a->velocity, vec2_scale(impulse, a->inv_mass));
    b->velocity = vec2_add(b->velocity, vec2_scale(impulse, b->inv_mass));
    a->angular_velocity -= vec2_cross(ra, impulse) * a->inv_inertia;
    b->angular_velocity += vec2_cross(rb, impulse) * b->inv_inertia;
    if (fabsf(j) > wake_impulse_threshold) {
        a->sleeping = 0;
        b->sleeping = 0;
        a->sleep_timer = 0.0f;
        b->sleep_timer = 0.0f;
    }
}

void constraint_solve_position(Constraint* c) {
    if (c == NULL || !c->active || c->body_a == NULL || c->body_b == NULL) {
        return;
    }

    RigidBody* a = c->body_a;
    RigidBody* b = c->body_b;

    Vec2 ra = vec2_rotate(c->local_anchor_a, a->angle);
    Vec2 rb = vec2_rotate(c->local_anchor_b, b->angle);
    Vec2 pa = vec2_add(a->position, ra);
    Vec2 pb = vec2_add(b->position, rb);
    Vec2 delta = vec2_sub(pb, pa);
    float dist = vec2_length(delta);
    if (dist < 1e-6f) {
        return;
    }

    Vec2 n = vec2_scale(delta, 1.0f / dist);
    float error = dist - c->rest_length;
    float inv_mass_sum = a->inv_mass + b->inv_mass;
    if (inv_mass_sum <= 1e-6f) {
        return;
    }

    float stiffness = 0.0f;
    if (c->type == CONSTRAINT_DISTANCE) {
        stiffness = max_f(c->stiffness, PHYSICS_TUNE_CONSTRAINT_MIN_STIFFNESS);
    } else if (c->type == CONSTRAINT_ROPE) {
        if (error <= 0.0f) {
            return;
        }
        stiffness = max_f(c->stiffness, PHYSICS_TUNE_CONSTRAINT_MIN_STIFFNESS);
    } else {
        // Spring constraints should not be hard-snapped in the position solver.
        // Keep only a very small drift correction, otherwise it behaves like a rigid rod.
        stiffness = clamp(max_f(c->stiffness, PHYSICS_TUNE_CONSTRAINT_SPRING_MIN_STIFFNESS) *
                              PHYSICS_TUNE_CONSTRAINT_SPRING_POS_CORRECTION_SCALE,
                          0.0f, PHYSICS_TUNE_CONSTRAINT_SPRING_POS_CORRECTION_MAX);
    }
    float correction = error * stiffness;
    Vec2 correction_vec = vec2_scale(n, correction / inv_mass_sum);

    a->position = vec2_add(a->position, vec2_scale(correction_vec, a->inv_mass));
    b->position = vec2_sub(b->position, vec2_scale(correction_vec, b->inv_mass));
}
