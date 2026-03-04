#ifndef PHYSICS_ENGINE_H
#define PHYSICS_ENGINE_H

#include "physics_math.h"
#include "body.h"
#include "shape.h"
#include "collision.h"
#include "constraint.h"

#define MAX_BODIES 1024
#define MAX_CONTACTS 4096
#define MAX_CONSTRAINTS 512
#define MAX_BROADPHASE_PAIRS 16384

typedef struct PhysicsEngine PhysicsEngine;

PhysicsEngine* physics_engine_create(void);
void physics_engine_free(PhysicsEngine* engine);

void physics_engine_set_gravity(PhysicsEngine* engine, Vec2 gravity);
void physics_engine_set_time_step(PhysicsEngine* engine, float dt);
void physics_engine_set_iterations(PhysicsEngine* engine, int iterations);
void physics_engine_set_damping(PhysicsEngine* engine, float damping);
void physics_engine_set_broadphase_cell_size(PhysicsEngine* engine, float cell_size);
void physics_engine_set_broadphase_use_grid(PhysicsEngine* engine, int enable);
Vec2 physics_engine_get_gravity(const PhysicsEngine* engine);
float physics_engine_get_time_step(const PhysicsEngine* engine);
int physics_engine_get_iterations(const PhysicsEngine* engine);
float physics_engine_get_damping(const PhysicsEngine* engine);
float physics_engine_get_broadphase_cell_size(const PhysicsEngine* engine);
int physics_engine_get_broadphase_use_grid(const PhysicsEngine* engine);

int physics_engine_get_body_count(const PhysicsEngine* engine);
RigidBody* physics_engine_get_body(const PhysicsEngine* engine, int index);
int physics_engine_get_contact_count(const PhysicsEngine* engine);
const CollisionManifold* physics_engine_get_contact(const PhysicsEngine* engine, int index);
int physics_engine_get_constraint_count(const PhysicsEngine* engine);
const Constraint* physics_engine_get_constraint(const PhysicsEngine* engine, int index);
int physics_engine_find_constraint_index(const PhysicsEngine* engine, const Constraint* constraint);
int physics_engine_constraint_is_active(const PhysicsEngine* engine, int index);
ConstraintType physics_engine_constraint_get_type(const PhysicsEngine* engine, int index);
float physics_engine_constraint_get_rest_length(const PhysicsEngine* engine, int index);
float physics_engine_constraint_get_stiffness(const PhysicsEngine* engine, int index);
float physics_engine_constraint_get_damping(const PhysicsEngine* engine, int index);
float physics_engine_constraint_get_break_force(const PhysicsEngine* engine, int index);
float physics_engine_constraint_get_last_force(const PhysicsEngine* engine, int index);
int physics_engine_constraint_get_collide_connected(const PhysicsEngine* engine, int index);
void physics_engine_constraint_set_active(PhysicsEngine* engine, int index, int active);
void physics_engine_constraint_set_rest_length(PhysicsEngine* engine, int index, float rest_length);
void physics_engine_constraint_set_stiffness(PhysicsEngine* engine, int index, float stiffness);
void physics_engine_constraint_set_damping(PhysicsEngine* engine, int index, float damping);
void physics_engine_constraint_set_break_force(PhysicsEngine* engine, int index, float break_force);
void physics_engine_constraint_set_collide_connected(PhysicsEngine* engine, int index, int collide_connected);
int physics_engine_get_broadphase_pair_count(const PhysicsEngine* engine);

void physics_engine_add_body(PhysicsEngine* engine, RigidBody* body);
void physics_engine_remove_body(PhysicsEngine* engine, RigidBody* body);
RigidBody* physics_engine_detach_body(PhysicsEngine* engine, RigidBody* body);

Constraint* physics_engine_add_distance_constraint(PhysicsEngine* engine, RigidBody* a, RigidBody* b,
                                                   Vec2 world_anchor_a, Vec2 world_anchor_b,
                                                   float stiffness, int collide_connected);
Constraint* physics_engine_add_spring_constraint(PhysicsEngine* engine, RigidBody* a, RigidBody* b,
                                                 Vec2 world_anchor_a, Vec2 world_anchor_b,
                                                 float rest_length, float stiffness, float damping,
                                                 int collide_connected);
void physics_engine_clear_constraints(PhysicsEngine* engine);

void physics_engine_step(PhysicsEngine* engine);
void physics_engine_update_positions(PhysicsEngine* engine);
void physics_engine_update_velocities(PhysicsEngine* engine);
void physics_engine_detect_collisions(PhysicsEngine* engine);
void physics_engine_resolve_collisions(PhysicsEngine* engine);
void physics_engine_clear_forces(PhysicsEngine* engine);

RigidBody* physics_engine_raycast(PhysicsEngine* engine, Vec2 start, Vec2 end);
int physics_engine_get_bodies_in_area(PhysicsEngine* engine, Vec2 center, float radius, RigidBody** out_bodies, int max_bodies);

#endif
