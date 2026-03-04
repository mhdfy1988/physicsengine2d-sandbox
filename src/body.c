#include <stdlib.h>
#include "../include/body.h"

RigidBody* body_create(float x, float y, float mass, Shape* shape) {
    if (shape == NULL) {
        return NULL;
    }

    RigidBody* body = (RigidBody*)malloc(sizeof(RigidBody));
    if (body == NULL) {
        return NULL;
    }

    body->position = vec2(x, y);
    body->angle = 0.0f;
    body->velocity = vec2(0, 0);
    body->angular_velocity = 0.0f;
    body->acceleration = vec2(0, 0);
    body->angular_acceleration = 0.0f;
    body->force = vec2(0, 0);
    body->torque = 0.0f;

    body->mass = mass;
    body->inv_mass = (mass > 0.0f) ? 1.0f / mass : 0.0f;

    body->shape = shape;
    body->owns_shape = 1;
    body->inertia = shape_get_moment_of_inertia(shape, mass);
    body->inv_inertia = (body->inertia > 0.0f) ? 1.0f / body->inertia : 0.0f;

    body->friction = shape->friction;
    body->restitution = shape->restitution;
    body->damping = 1.0f;
    body->gravity = vec2(0, 9.8f);
    body->use_custom_gravity = 0;

    body->type = BODY_DYNAMIC;
    body->active = 1;

    return body;
}

void body_free(RigidBody* body) {
    if (body == NULL) {
        return;
    }

    if (body->owns_shape && body->shape != NULL) {
        shape_free(body->shape);
        body->shape = NULL;
    }

    free(body);
}

void body_set_type(RigidBody* body, BodyType type) {
    if (body == NULL) {
        return;
    }

    body->type = type;
    if (type == BODY_STATIC) {
        body->inv_mass = 0.0f;
        body->inv_inertia = 0.0f;
    } else if (body->mass > 0.0f) {
        body->inv_mass = 1.0f / body->mass;
        body->inv_inertia = (body->inertia > 0.0f) ? 1.0f / body->inertia : 0.0f;
    }
}

void body_set_gravity(RigidBody* body, Vec2 g) {
    if (body == NULL) {
        return;
    }

    body->gravity = g;
    body->use_custom_gravity = 1;
}

void body_set_friction(RigidBody* body, float friction) {
    if (body == NULL) {
        return;
    }

    body->friction = friction;
}

void body_set_restitution(RigidBody* body, float restitution) {
    if (body == NULL) {
        return;
    }

    body->restitution = restitution;
}

void body_set_shape_ownership(RigidBody* body, int owns_shape) {
    if (body == NULL) {
        return;
    }

    body->owns_shape = owns_shape ? 1 : 0;
}

void body_apply_force(RigidBody* body, Vec2 force) {
    if (body == NULL) {
        return;
    }

    body->force = vec2_add(body->force, force);
}

void body_apply_force_at(RigidBody* body, Vec2 force, Vec2 point) {
    if (body == NULL) {
        return;
    }

    body_apply_force(body, force);

    Vec2 r = vec2_sub(point, body->position);
    float torque = vec2_cross(r, force);
    body->torque += torque;
}

void body_apply_impulse(RigidBody* body, Vec2 impulse) {
    if (body == NULL) {
        return;
    }

    body->velocity = vec2_add(body->velocity, vec2_scale(impulse, body->inv_mass));
}

void body_apply_angular_impulse(RigidBody* body, float impulse) {
    if (body == NULL) {
        return;
    }

    body->angular_velocity += impulse * body->inv_inertia;
}

void body_apply_torque(RigidBody* body, float torque) {
    if (body == NULL) {
        return;
    }

    body->torque += torque;
}

Vec2 body_get_velocity_at(RigidBody* body, Vec2 point) {
    if (body == NULL) {
        return vec2(0, 0);
    }

    Vec2 r = vec2_sub(point, body->position);
    Vec2 angular_vel = vec2(-r.y * body->angular_velocity, r.x * body->angular_velocity);
    return vec2_add(body->velocity, angular_vel);
}

Vec2 body_get_world_point(RigidBody* body, Vec2 local_point) {
    if (body == NULL) {
        return local_point;
    }

    Vec2 rotated = vec2_rotate(local_point, body->angle);
    return vec2_add(body->position, rotated);
}

Vec2 body_get_local_point(RigidBody* body, Vec2 world_point) {
    if (body == NULL) {
        return world_point;
    }

    Vec2 relative = vec2_sub(world_point, body->position);
    return vec2_rotate(relative, -body->angle);
}
