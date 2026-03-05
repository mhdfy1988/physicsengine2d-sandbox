#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "../include/shape.h"

Shape* shape_create_circle(float radius) {
    Shape* shape = (Shape*)malloc(sizeof(Shape));
    if (shape == NULL) {
        return NULL;
    }
    shape->type = SHAPE_CIRCLE;
    shape->friction = 0.3f;
    shape->restitution = 0.6f;
    shape->data.circle.radius = radius;
    return shape;
}

Shape* shape_create_box(float width, float height) {
    Shape* shape = (Shape*)malloc(sizeof(Shape));
    if (shape == NULL) {
        return NULL;
    }
    shape->type = SHAPE_POLYGON;
    shape->friction = 0.3f;
    shape->restitution = 0.6f;

    float w = width / 2.0f;
    float h = height / 2.0f;
    shape->data.polygon.vertices[0] = vec2(-w, -h);
    shape->data.polygon.vertices[1] = vec2(w, -h);
    shape->data.polygon.vertices[2] = vec2(w, h);
    shape->data.polygon.vertices[3] = vec2(-w, h);
    shape->data.polygon.vertex_count = 4;

    return shape;
}

void shape_free(Shape* shape) {
    if (shape) {
        free(shape);
    }
}

float shape_get_area(Shape* shape) {
    if (shape == NULL) {
        return 0.0f;
    }

    if (shape->type == SHAPE_CIRCLE) {
        float r = shape->data.circle.radius;
        return PI * r * r;
    } else if (shape->type == SHAPE_POLYGON) {
        float area = 0.0f;
        PolygonShape* poly = &shape->data.polygon;
        if (poly->vertex_count <= 0) {
            return 0.0f;
        }

        for (int i = 0; i < poly->vertex_count; i++) {
            int next = (i + 1) % poly->vertex_count;
            area += vec2_cross(poly->vertices[i], poly->vertices[next]);
        }
        return fabsf(area) / 2.0f;
    }
    return 0.0f;
}

float shape_get_moment_of_inertia(Shape* shape, float mass) {
    if (shape == NULL) {
        return 0.0f;
    }

    if (shape->type == SHAPE_CIRCLE) {
        float r = shape->data.circle.radius;
        return mass * r * r / 2.0f;
    } else if (shape->type == SHAPE_POLYGON) {
        PolygonShape* poly = &shape->data.polygon;
        float width = 0.0f;
        float height = 0.0f;
        if (poly->vertex_count <= 0) {
            return 0.0f;
        }

        for (int i = 0; i < poly->vertex_count; i++) {
            if (fabsf(poly->vertices[i].x) * 2.0f > width) {
                width = fabsf(poly->vertices[i].x) * 2.0f;
            }
            if (fabsf(poly->vertices[i].y) * 2.0f > height) {
                height = fabsf(poly->vertices[i].y) * 2.0f;
            }
        }
        return mass * (width * width + height * height) / 12.0f;
    }
    return 0.0f;
}

Vec2 shape_get_center_of_mass(Shape* shape) {
    if (shape == NULL) {
        return vec2(0, 0);
    }

    if (shape->type == SHAPE_CIRCLE) {
        return vec2(0, 0);
    } else if (shape->type == SHAPE_POLYGON) {
        Vec2 center = vec2(0, 0);
        PolygonShape* poly = &shape->data.polygon;
        if (poly->vertex_count <= 0) {
            return vec2(0, 0);
        }

        for (int i = 0; i < poly->vertex_count; i++) {
            center = vec2_add(center, poly->vertices[i]);
        }
        return vec2_scale(center, 1.0f / poly->vertex_count);
    }
    return vec2(0, 0);
}
