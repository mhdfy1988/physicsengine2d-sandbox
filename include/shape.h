#ifndef PHYSICS_SHAPE_H
#define PHYSICS_SHAPE_H

#include "physics_math.h"

typedef enum {
    SHAPE_CIRCLE,
    SHAPE_POLYGON
} ShapeType;

// 圆形结构
typedef struct {
    float radius;
} CircleShape;

// 多边形结构
typedef struct {
    Vec2 vertices[8];      // 最多8个顶点
    int vertex_count;
} PolygonShape;

// 通用形状结构
typedef struct {
    ShapeType type;
    float friction;        // 摩擦系数
    float restitution;     // 弹性系数
    union {
        CircleShape circle;
        PolygonShape polygon;
    } data;
} Shape;

// 形状创建函数
Shape* shape_create_circle(float radius);
Shape* shape_create_box(float width, float height);
void shape_free(Shape* shape);

// 形状计算函数
float shape_get_area(Shape* shape);
float shape_get_moment_of_inertia(Shape* shape, float mass);
Vec2 shape_get_center_of_mass(Shape* shape);

#endif
