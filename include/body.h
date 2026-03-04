#ifndef PHYSICS_BODY_H
#define PHYSICS_BODY_H

#include "physics_math.h"
#include "shape.h"

typedef enum {
    BODY_DYNAMIC,
    BODY_STATIC,
    BODY_KINEMATIC
} BodyType;

typedef struct {
    float mass;
    float inv_mass;
    float inertia;
    float inv_inertia;

    Vec2 position;
    float angle;

    Vec2 velocity;
    float angular_velocity;
    Vec2 acceleration;
    float angular_acceleration;

    Vec2 force;
    float torque;

    float friction;
    float restitution;
    float damping;
    Vec2 gravity;
    int use_custom_gravity;

    BodyType type;
    Shape* shape;
    int owns_shape;

    int active;
} RigidBody;

RigidBody* body_create(float x, float y, float mass, Shape* shape);
void body_free(RigidBody* body);

void body_set_type(RigidBody* body, BodyType type);
void body_set_gravity(RigidBody* body, Vec2 g);
void body_set_friction(RigidBody* body, float friction);
void body_set_restitution(RigidBody* body, float restitution);
void body_set_shape_ownership(RigidBody* body, int owns_shape);

void body_apply_force(RigidBody* body, Vec2 force);
void body_apply_force_at(RigidBody* body, Vec2 force, Vec2 point);
void body_apply_impulse(RigidBody* body, Vec2 impulse);
void body_apply_angular_impulse(RigidBody* body, float impulse);
void body_apply_torque(RigidBody* body, float torque);

Vec2 body_get_velocity_at(RigidBody* body, Vec2 point);
Vec2 body_get_world_point(RigidBody* body, Vec2 local_point);
Vec2 body_get_local_point(RigidBody* body, Vec2 world_point);

#endif
