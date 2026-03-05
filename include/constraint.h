#ifndef PHYSICS_CONSTRAINT_H
#define PHYSICS_CONSTRAINT_H

#include "body.h"

typedef enum {
    CONSTRAINT_DISTANCE,
    CONSTRAINT_SPRING,
    CONSTRAINT_ROPE
} ConstraintType;

typedef struct {
    ConstraintType type;
    RigidBody* body_a;
    RigidBody* body_b;

    Vec2 local_anchor_a;
    Vec2 local_anchor_b;

    float rest_length;
    float stiffness;
    float damping;
    float break_force;
    float last_impulse;
    float last_force;

    int collide_connected;
    int active;
} Constraint;

void constraint_init_distance(Constraint* c, RigidBody* a, RigidBody* b,
                              Vec2 world_anchor_a, Vec2 world_anchor_b,
                              float stiffness, int collide_connected);
void constraint_init_spring(Constraint* c, RigidBody* a, RigidBody* b,
                            Vec2 world_anchor_a, Vec2 world_anchor_b,
                            float rest_length, float stiffness, float damping,
                            int collide_connected);
void constraint_init_rope(Constraint* c, RigidBody* a, RigidBody* b,
                          Vec2 world_anchor_a, Vec2 world_anchor_b,
                          float max_length, float stiffness, int collide_connected);
void constraint_warm_start(Constraint* c);
void constraint_solve_velocity(Constraint* c, float dt);
void constraint_solve_position(Constraint* c);

#endif
