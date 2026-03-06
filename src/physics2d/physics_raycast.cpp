#include <float.h>
#include <math.h>
#include <stddef.h>
#include "physics_internal.hpp"

static int point_in_polygon_world(RigidBody* body, Vec2 p) {
    PolygonShape* poly;
    int inside = 0;
    int i;
    int j;
    if (body == NULL || body->shape == NULL || body->shape->type != SHAPE_POLYGON) {
        return 0;
    }

    poly = &body->shape->data.polygon;
    for (i = 0, j = poly->vertex_count - 1; i < poly->vertex_count; j = i++) {
        Vec2 vi = body_get_world_point(body, poly->vertices[i]);
        Vec2 vj = body_get_world_point(body, poly->vertices[j]);
        int crosses = ((vi.y > p.y) != (vj.y > p.y));
        if (crosses) {
            float x_at_y = (vj.x - vi.x) * (p.y - vi.y) / (vj.y - vi.y) + vi.x;
            if (p.x < x_at_y) {
                inside = !inside;
            }
        }
    }

    return inside;
}

static int intersect_segments(Vec2 p, Vec2 r, Vec2 q, Vec2 s, float* out_t) {
    float denom = vec2_cross(r, s);
    Vec2 qp;
    float t;
    float u;
    if (fabsf(denom) < 1e-6f) {
        return 0;
    }

    qp = vec2_sub(q, p);
    t = vec2_cross(qp, s) / denom;
    u = vec2_cross(qp, r) / denom;
    if (t < 0.0f || t > 1.0f || u < 0.0f || u > 1.0f) {
        return 0;
    }

    *out_t = t;
    return 1;
}

static int raycast_circle(RigidBody* body, Vec2 start, Vec2 end, float* out_t) {
    float radius = body->shape->data.circle.radius;
    Vec2 d = vec2_sub(end, start);
    Vec2 f = vec2_sub(start, body->position);
    float a = vec2_dot(d, d);
    float b;
    float c;
    float disc;
    float sqrt_disc;
    float t1;
    float t2;
    float hit_t = FLT_MAX;
    if (a < 1e-6f) {
        return 0;
    }

    if (vec2_dot(f, f) <= radius * radius) {
        *out_t = 0.0f;
        return 1;
    }

    b = 2.0f * vec2_dot(f, d);
    c = vec2_dot(f, f) - radius * radius;
    disc = b * b - 4.0f * a * c;
    if (disc < 0.0f) {
        return 0;
    }

    sqrt_disc = sqrtf(disc);
    t1 = (-b - sqrt_disc) / (2.0f * a);
    t2 = (-b + sqrt_disc) / (2.0f * a);
    if (t1 >= 0.0f && t1 <= 1.0f) {
        hit_t = t1;
    }
    if (t2 >= 0.0f && t2 <= 1.0f) {
        hit_t = min_f(hit_t, t2);
    }
    if (hit_t == FLT_MAX) {
        return 0;
    }

    *out_t = hit_t;
    return 1;
}

static int raycast_polygon(RigidBody* body, Vec2 start, Vec2 end, float* out_t) {
    PolygonShape* poly = &body->shape->data.polygon;
    Vec2 r;
    float best_t = FLT_MAX;
    int i;
    if (point_in_polygon_world(body, start)) {
        *out_t = 0.0f;
        return 1;
    }

    r = vec2_sub(end, start);
    for (i = 0; i < poly->vertex_count; i++) {
        Vec2 v1 = body_get_world_point(body, poly->vertices[i]);
        Vec2 v2 = body_get_world_point(body, poly->vertices[(i + 1) % poly->vertex_count]);
        Vec2 s = vec2_sub(v2, v1);
        float t = 0.0f;
        if (intersect_segments(start, r, v1, s, &t)) {
            best_t = min_f(best_t, t);
        }
    }

    if (best_t == FLT_MAX) {
        return 0;
    }
    *out_t = best_t;
    return 1;
}

extern "C" {

RigidBody* physics_internal_raycast(PhysicsEngine* engine, Vec2 start, Vec2 end) {
    RigidBody* closest = NULL;
    float closest_t = FLT_MAX;
    int i;
    if (engine == NULL) {
        return NULL;
    }

    for (i = 0; i < engine->body_count; i++) {
        RigidBody* body = engine->bodies[i];
        float t = 0.0f;
        int hit = 0;
        if (body == NULL || body->shape == NULL || !body->active) {
            continue;
        }
        if (body->shape->type == SHAPE_CIRCLE) {
            hit = raycast_circle(body, start, end, &t);
        } else if (body->shape->type == SHAPE_POLYGON) {
            hit = raycast_polygon(body, start, end, &t);
        }

        if (hit && t < closest_t) {
            closest_t = t;
            closest = body;
        }
    }

    return closest;
}

}  // extern "C"
