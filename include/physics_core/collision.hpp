#ifndef PHYSICS_COLLISION_H
#define PHYSICS_COLLISION_H

#include "physics_core/physics_math.hpp"
#include "physics_core/body.hpp"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    Vec2 point;             /* Contact point */
    Vec2 normal;            /* Contact normal */
    float penetration;      /* Penetration depth */
    int has_collision;      /* Non-zero when colliding */
} CollisionInfo;

typedef struct {
    RigidBody* bodyA;
    RigidBody* bodyB;
    CollisionInfo info;
    float cached_normal_impulse;
    float cached_tangent_impulse;
} CollisionManifold;

/* Narrow-phase collision tests */
int collision_circle_circle(RigidBody* a, RigidBody* b, CollisionInfo* info);
int collision_circle_polygon(RigidBody* a, RigidBody* b, CollisionInfo* info);
int collision_polygon_polygon(RigidBody* a, RigidBody* b, CollisionInfo* info);
int collision_detect(RigidBody* a, RigidBody* b, CollisionInfo* info);

/* SAT (Separating Axis Theorem) tests */
int sat_test_circle(RigidBody* a, RigidBody* b, CollisionInfo* info);
int sat_test_polygon(RigidBody* a, RigidBody* b, CollisionInfo* info);

/* Collision resolution */
void collision_resolve(CollisionManifold* manifold);
void collision_resolve_velocity(CollisionManifold* manifold);
void collision_resolve_position(CollisionManifold* manifold);

#ifdef __cplusplus
}
#endif

#endif
