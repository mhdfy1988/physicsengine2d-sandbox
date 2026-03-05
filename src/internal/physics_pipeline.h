#ifndef PHYSICS_INTERNAL_PIPELINE_H
#define PHYSICS_INTERNAL_PIPELINE_H

#include "../../include/physics.h"

typedef struct {
    Vec2 min;
    Vec2 max;
} AABB;

typedef struct {
    int velocity_iterations;
    int position_iterations;
    float dt;
} PhysicsSolverContext;

typedef struct {
    PhysicsEngine* engine_hint;
    CollisionManifold* contacts;
    int contact_count;
    const int* contact_ids;
    Constraint* constraints;
    int constraint_count;
    const int* constraint_ids;
} PhysicsSolverWorldView;

typedef struct {
    PhysicsBroadphaseBuilder build_pairs;
    void* user;
} PhysicsBroadphaseOps;

typedef struct {
    PhysicsNarrowphaseBuilder build_contacts;
    void* user;
} PhysicsNarrowphaseOps;

typedef void (*PhysicsParallelForFn)(int begin, int end, void* user);

int physics_internal_default_build_pairs(PhysicsEngine* engine, void* user);
int physics_internal_default_build_contacts(PhysicsEngine* engine, void* user);
void physics_internal_append_ccd_contacts(PhysicsEngine* engine, float dt);
void physics_internal_bind_default_pipeline(PhysicsEngine* engine);

void physics_internal_step(PhysicsEngine* engine);
int physics_internal_body_has_finite_state(const RigidBody* body);
AABB physics_internal_body_compute_aabb(const RigidBody* body);
void physics_internal_prepare_collision_inputs(PhysicsEngine* engine);
void physics_internal_detect_collisions(PhysicsEngine* engine);
void physics_internal_resolve_collisions(PhysicsEngine* engine, const PhysicsSolverContext* ctx);
void physics_internal_solve_world_view(PhysicsSolverWorldView* view, const PhysicsSolverContext* ctx);
void physics_internal_parallel_for(PhysicsEngine* engine, int count, int grain, PhysicsParallelForFn fn, void* user);
void physics_internal_update_velocities(PhysicsEngine* engine, float dt);
void physics_internal_update_positions(PhysicsEngine* engine, float dt);
void physics_internal_clear_forces(PhysicsEngine* engine);

#endif
