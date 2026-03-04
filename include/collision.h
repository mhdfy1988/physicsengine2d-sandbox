#ifndef PHYSICS_COLLISION_H
#define PHYSICS_COLLISION_H

#include "physics_math.h"
#include "body.h"

typedef struct {
    Vec2 point;            // 碰撞点
    Vec2 normal;           // 碰撞法线
    float penetration;      // 穿透深度
    int has_collision;     // 是否碰撞
} CollisionInfo;

typedef struct {
    RigidBody* bodyA;
    RigidBody* bodyB;
    CollisionInfo info;
} CollisionManifold;

// 碰撞检测函数
int collision_circle_circle(RigidBody* a, RigidBody* b, CollisionInfo* info);
int collision_circle_polygon(RigidBody* a, RigidBody* b, CollisionInfo* info);
int collision_polygon_polygon(RigidBody* a, RigidBody* b, CollisionInfo* info);
int collision_detect(RigidBody* a, RigidBody* b, CollisionInfo* info);

// SAT (Separating Axis Theorem) 分离轴定理测试
int sat_test_circle(RigidBody* a, RigidBody* b, CollisionInfo* info);
int sat_test_polygon(RigidBody* a, RigidBody* b, CollisionInfo* info);

// 碰撞响应
void collision_resolve(CollisionManifold* manifold);
void collision_resolve_velocity(CollisionManifold* manifold);
void collision_resolve_position(CollisionManifold* manifold);

#endif
