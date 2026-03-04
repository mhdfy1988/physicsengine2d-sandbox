#ifndef PHYSICS_INTERNAL_H
#define PHYSICS_INTERNAL_H

#include "../include/physics.h"

typedef struct {
    Vec2 min;
    Vec2 max;
} AABB;

typedef struct {
    RigidBody* a;
    RigidBody* b;
} BroadphasePair;

struct PhysicsEngine {
    Vec2 gravity;
    float damping;
    float time_step;
    int iterations;

    RigidBody* bodies[MAX_BODIES];
    int body_count;

    CollisionManifold contacts[MAX_CONTACTS];
    int contact_count;

    Constraint constraints[MAX_CONSTRAINTS];
    int constraint_count;

    BroadphasePair broadphase_pairs[MAX_BROADPHASE_PAIRS];
    int broadphase_pair_count;
    float broadphase_cell_size;
    int broadphase_use_grid;
};

void physics_internal_build_broadphase_pairs(PhysicsEngine* engine, AABB* aabbs);
RigidBody* physics_internal_raycast(PhysicsEngine* engine, Vec2 start, Vec2 end);
void physics_internal_step(PhysicsEngine* engine);
int physics_internal_body_has_finite_state(const RigidBody* body);
AABB physics_internal_body_compute_aabb(const RigidBody* body);
void physics_internal_detect_collisions(PhysicsEngine* engine);
void physics_internal_resolve_collisions(PhysicsEngine* engine);
void physics_internal_update_velocities(PhysicsEngine* engine);
void physics_internal_update_positions(PhysicsEngine* engine);
void physics_internal_clear_forces(PhysicsEngine* engine);

#endif
