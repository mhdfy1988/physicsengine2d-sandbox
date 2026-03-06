#ifndef PHYSICS_SHAPE_H
#define PHYSICS_SHAPE_H

#include "physics_math.hpp"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SHAPE_CIRCLE,
    SHAPE_POLYGON
} ShapeType;

/* Circle shape payload */
typedef struct {
    float radius;
} CircleShape;

/* Convex polygon payload */
typedef struct {
    Vec2 vertices[8];      /* Up to 8 vertices */
    int vertex_count;
} PolygonShape;

/* Generic shape descriptor */
typedef struct {
    ShapeType type;
    float friction;        /* Friction coefficient */
    float restitution;     /* Restitution coefficient */
    union {
        CircleShape circle;
        PolygonShape polygon;
    } data;
} Shape;

/* Shape creation */
Shape* shape_create_circle(float radius);
Shape* shape_create_box(float width, float height);
void shape_free(Shape* shape);

/* Shape properties */
float shape_get_area(Shape* shape);
float shape_get_moment_of_inertia(Shape* shape, float mass);
Vec2 shape_get_center_of_mass(Shape* shape);

#ifdef __cplusplus
}
#endif

#endif
