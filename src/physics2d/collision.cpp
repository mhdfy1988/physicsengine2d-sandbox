#include <float.h>
#include <math.h>
#include <stddef.h>
#include "../include/physics_core/collision.hpp"
#include "internal/physics_math_internal.hpp"
#include "internal/physics_tuning.hpp"

static float combined_restitution(const RigidBody* a, const RigidBody* b) {
    if (a->type == BODY_DYNAMIC && b->type == BODY_STATIC) {
        return clamp(a->restitution, 0.0f, 1.0f);
    }
    if (b->type == BODY_DYNAMIC && a->type == BODY_STATIC) {
        return clamp(b->restitution, 0.0f, 1.0f);
    }

    int has_circle = (a->shape != NULL && a->shape->type == SHAPE_CIRCLE) ||
                     (b->shape != NULL && b->shape->type == SHAPE_CIRCLE);
    if (has_circle) {
        return clamp(max_f(a->restitution, b->restitution), 0.0f, 1.0f);
    }
    return min_f(a->restitution, b->restitution);
}

static int collision_has_circle(const RigidBody* a, const RigidBody* b) {
    return (a->shape != NULL && a->shape->type == SHAPE_CIRCLE) ||
           (b->shape != NULL && b->shape->type == SHAPE_CIRCLE);
}

static int collision_is_circle_polygon_pair(const RigidBody* a, const RigidBody* b) {
    int a_circle = (a->shape != NULL && a->shape->type == SHAPE_CIRCLE);
    int b_circle = (b->shape != NULL && b->shape->type == SHAPE_CIRCLE);
    return (a_circle ^ b_circle);
}

void collision_resolve(CollisionManifold* manifold) {
    collision_resolve_velocity(manifold);
}

void collision_resolve_velocity(CollisionManifold* manifold) {
    RigidBody* a = manifold->bodyA;
    RigidBody* b = manifold->bodyB;
    CollisionInfo* info = &manifold->info;
    const float wake_impulse_threshold = PHYSICS_TUNE_WAKE_IMPULSE_THRESHOLD;

    float inv_mass_sum = a->inv_mass + b->inv_mass;
    if (inv_mass_sum <= 1e-6f) {
        return;
    }

    Vec2 contact = info->point;
    Vec2 ra = vec2_sub(contact, a->position);
    Vec2 rb = vec2_sub(contact, b->position);

    Vec2 va = vec2_add(a->velocity, physics_cross_scalar_vec(a->angular_velocity, ra));
    Vec2 vb = vec2_add(b->velocity, physics_cross_scalar_vec(b->angular_velocity, rb));
    Vec2 rel_vel = vec2_sub(vb, va);
    float vel_along_normal = vec2_dot(rel_vel, info->normal);
    if (vel_along_normal >= 0.0f) {
        return;
    }

    float ra_cn = vec2_cross(ra, info->normal);
    float rb_cn = vec2_cross(rb, info->normal);
    float normal_denom = inv_mass_sum +
                         ra_cn * ra_cn * a->inv_inertia +
                         rb_cn * rb_cn * b->inv_inertia;

    if (normal_denom <= 1e-6f) {
        return;
    }

    int has_circle = collision_has_circle(a, b);
    int circle_polygon_pair = collision_is_circle_polygon_pair(a, b);
    float restitution = combined_restitution(a, b);
    float bounce_threshold = has_circle
                                 ? (circle_polygon_pair ? PHYSICS_TUNE_BOUNCE_THRESHOLD_CIRCLE_POLYGON
                                                        : PHYSICS_TUNE_BOUNCE_THRESHOLD_CIRCLE)
                                 : PHYSICS_TUNE_BOUNCE_THRESHOLD_POLYGON;
    float impact_speed = -vel_along_normal;
    if (circle_polygon_pair) {
        restitution *= PHYSICS_TUNE_RESTITUTION_CIRCLE_POLYGON_SCALE;
    }
    if (fabsf(vel_along_normal) < bounce_threshold) {
        restitution = 0.0f;
    }
    float j = -(1.0f + restitution) * vel_along_normal / normal_denom;
    float base_j = j;
    Vec2 impulse = vec2_scale(info->normal, j);
    manifold->cached_normal_impulse = clamp(manifold->cached_normal_impulse + j,
                                            -PHYSICS_TUNE_CONTACT_CACHE_NORMAL_LIMIT,
                                            PHYSICS_TUNE_CONTACT_CACHE_NORMAL_LIMIT);

    a->velocity = vec2_sub(a->velocity, vec2_scale(impulse, a->inv_mass));
    b->velocity = vec2_add(b->velocity, vec2_scale(impulse, b->inv_mass));
    a->angular_velocity -= vec2_cross(ra, impulse) * a->inv_inertia;
    b->angular_velocity += vec2_cross(rb, impulse) * b->inv_inertia;

    // Keep bounce visible for circle contacts by enforcing a minimum separating speed.
    if (has_circle && restitution > 0.0f && impact_speed > bounce_threshold) {
        Vec2 va_after = vec2_add(a->velocity, physics_cross_scalar_vec(a->angular_velocity, ra));
        Vec2 vb_after = vec2_add(b->velocity, physics_cross_scalar_vec(b->angular_velocity, rb));
        float rel_n_after = vec2_dot(vec2_sub(vb_after, va_after), info->normal);
        float target_separating = restitution * impact_speed;
        if (circle_polygon_pair) {
            target_separating *= PHYSICS_TUNE_TARGET_SEPARATION_CIRCLE_POLYGON_SCALE;
        }

        if (rel_n_after < target_separating) {
            float delta_v = target_separating - rel_n_after;
            float extra_j = delta_v / normal_denom;
            extra_j = min_f(extra_j,
                            j * (circle_polygon_pair ? PHYSICS_TUNE_EXTRA_IMPULSE_SCALE_CIRCLE_POLYGON
                                                     : PHYSICS_TUNE_EXTRA_IMPULSE_SCALE_DEFAULT));
            if (extra_j > 0.0f) {
                Vec2 extra_impulse = vec2_scale(info->normal, extra_j);
                a->velocity = vec2_sub(a->velocity, vec2_scale(extra_impulse, a->inv_mass));
                b->velocity = vec2_add(b->velocity, vec2_scale(extra_impulse, b->inv_mass));
                a->angular_velocity -= vec2_cross(ra, extra_impulse) * a->inv_inertia;
                b->angular_velocity += vec2_cross(rb, extra_impulse) * b->inv_inertia;
                j += extra_j;
                manifold->cached_normal_impulse = clamp(manifold->cached_normal_impulse + extra_j,
                                                        -PHYSICS_TUNE_CONTACT_CACHE_NORMAL_LIMIT,
                                                        PHYSICS_TUNE_CONTACT_CACHE_NORMAL_LIMIT);
            }
        }
    }

    va = vec2_add(a->velocity, physics_cross_scalar_vec(a->angular_velocity, ra));
    vb = vec2_add(b->velocity, physics_cross_scalar_vec(b->angular_velocity, rb));
    rel_vel = vec2_sub(vb, va);

    Vec2 tangent = vec2_sub(rel_vel, vec2_scale(info->normal, vec2_dot(rel_vel, info->normal)));
    float t_len_sq = vec2_length_sq(tangent);
    if (t_len_sq <= 1e-6f) {
        return;
    }
    tangent = vec2_scale(tangent, 1.0f / sqrtf(t_len_sq));

    float ra_ct = vec2_cross(ra, tangent);
    float rb_ct = vec2_cross(rb, tangent);
    float tangent_denom = inv_mass_sum +
                          ra_ct * ra_ct * a->inv_inertia +
                          rb_ct * rb_ct * b->inv_inertia;

    if (tangent_denom <= 1e-6f) {
        return;
    }

    float jt = -vec2_dot(rel_vel, tangent) / tangent_denom;
    float mu = sqrtf(max_f(a->friction, 0.0f) * max_f(b->friction, 0.0f));
    float mu_static = circle_polygon_pair ? (mu * PHYSICS_TUNE_STATIC_FRICTION_SCALE_CIRCLE_POLYGON)
                                          : (has_circle ? (mu * PHYSICS_TUNE_STATIC_FRICTION_SCALE_CIRCLE)
                                                        : (mu * PHYSICS_TUNE_STATIC_FRICTION_SCALE_POLYGON));
    float mu_dynamic = circle_polygon_pair ? (mu * PHYSICS_TUNE_DYNAMIC_FRICTION_SCALE_CIRCLE_POLYGON)
                                           : (has_circle ? (mu * PHYSICS_TUNE_DYNAMIC_FRICTION_SCALE_CIRCLE)
                                                         : (mu * PHYSICS_TUNE_DYNAMIC_FRICTION_SCALE_POLYGON));
    float friction_ref_j = has_circle ? base_j : j;
    float max_static = friction_ref_j * mu_static;
    float max_dynamic = friction_ref_j * mu_dynamic;

    if (fabsf(jt) <= max_static) {
        // Static friction: cancel tangential relative motion when possible.
    } else {
        // Dynamic friction: slide with bounded friction force.
        jt = (jt < 0.0f) ? -max_dynamic : max_dynamic;
    }

    Vec2 friction_impulse = vec2_scale(tangent, jt);
    manifold->cached_tangent_impulse = clamp(manifold->cached_tangent_impulse + jt,
                                             -PHYSICS_TUNE_CONTACT_CACHE_TANGENT_LIMIT,
                                             PHYSICS_TUNE_CONTACT_CACHE_TANGENT_LIMIT);
    a->velocity = vec2_sub(a->velocity, vec2_scale(friction_impulse, a->inv_mass));
    b->velocity = vec2_add(b->velocity, vec2_scale(friction_impulse, b->inv_mass));
    a->angular_velocity -= vec2_cross(ra, friction_impulse) * a->inv_inertia;
    b->angular_velocity += vec2_cross(rb, friction_impulse) * b->inv_inertia;

    // Rolling resistance: bleed residual spin while objects remain in contact.
    float roll_mu = PHYSICS_TUNE_ROLLING_RESISTANCE_SCALE * mu;
    float max_roll_impulse = j * roll_mu;
    if (max_roll_impulse > 0.0f) {
        if (a->type == BODY_DYNAMIC && a->inv_inertia > 0.0f) {
            float max_dw_a = max_roll_impulse * a->inv_inertia;
            if (a->angular_velocity > 0.0f) {
                a->angular_velocity = max_f(0.0f, a->angular_velocity - max_dw_a);
            } else {
                a->angular_velocity = min_f(0.0f, a->angular_velocity + max_dw_a);
            }
        }
        if (b->type == BODY_DYNAMIC && b->inv_inertia > 0.0f) {
            float max_dw_b = max_roll_impulse * b->inv_inertia;
            if (b->angular_velocity > 0.0f) {
                b->angular_velocity = max_f(0.0f, b->angular_velocity - max_dw_b);
            } else {
                b->angular_velocity = min_f(0.0f, b->angular_velocity + max_dw_b);
            }
        }
    }
    if (fabsf(j) > wake_impulse_threshold || fabsf(jt) > wake_impulse_threshold) {
        a->sleeping = 0;
        b->sleeping = 0;
        a->sleep_timer = 0.0f;
        b->sleep_timer = 0.0f;
    }
}

void collision_resolve_position(CollisionManifold* manifold) {
    RigidBody* a = manifold->bodyA;
    RigidBody* b = manifold->bodyB;
    CollisionInfo* info = &manifold->info;

    float slop = PHYSICS_TUNE_POS_SLOP_DEFAULT;
    float percent = PHYSICS_TUNE_POS_PERCENT_DEFAULT;
    float max_correction = PHYSICS_TUNE_POS_MAX_CORRECTION_DEFAULT;
    float correction = max_f(info->penetration - slop, 0.0f) * percent;
    int circle_polygon_pair = collision_is_circle_polygon_pair(a, b);
    if (circle_polygon_pair) {
        // Circle-vs-polygon deep overlap (rope/link particles into boxes) needs stronger separation.
        slop = PHYSICS_TUNE_POS_SLOP_CIRCLE_POLYGON;
        percent = PHYSICS_TUNE_POS_PERCENT_CIRCLE_POLYGON;
        max_correction = PHYSICS_TUNE_POS_MAX_CORRECTION_CIRCLE_POLYGON;
        correction = max_f(info->penetration - slop, 0.0f) * percent;
    }
    correction = min_f(correction, max_correction);
    float weight_a = a->inv_mass;
    float weight_b = b->inv_mass;
    if (circle_polygon_pair) {
        int a_circle = (a->shape != NULL && a->shape->type == SHAPE_CIRCLE);
        float circle_bias = PHYSICS_TUNE_POS_WEIGHT_BIAS_CIRCLE;
        float poly_bias = PHYSICS_TUNE_POS_WEIGHT_BIAS_POLYGON;
        if (a_circle) {
            weight_a *= circle_bias;
            weight_b *= poly_bias;
        } else {
            weight_a *= poly_bias;
            weight_b *= circle_bias;
        }
    }

    float total_inv_mass = weight_a + weight_b;

    if (total_inv_mass <= 1e-6f) {
        return;
    }

    Vec2 correction_vec = vec2_scale(info->normal, correction / total_inv_mass);
    a->position = vec2_sub(a->position, vec2_scale(correction_vec, weight_a));
    b->position = vec2_add(b->position, vec2_scale(correction_vec, weight_b));
}
