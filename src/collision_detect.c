#include <float.h>
#include <math.h>
#include <stddef.h>
#include "../include/collision.h"
#include "internal/physics_tuning.h"

static void project_polygon_on_axis(RigidBody* body, PolygonShape* poly, Vec2 axis, float* min_p, float* max_p) {
    float min_v = FLT_MAX;
    float max_v = -FLT_MAX;

    for (int i = 0; i < poly->vertex_count; i++) {
        Vec2 w = body_get_world_point(body, poly->vertices[i]);
        float p = vec2_dot(w, axis);
        min_v = min_f(min_v, p);
        max_v = max_f(max_v, p);
    }

    *min_p = min_v;
    *max_p = max_v;
}

static int update_sat_axis(RigidBody* a, PolygonShape* poly_a, RigidBody* b, PolygonShape* poly_b,
                           Vec2 axis, float* best_overlap, Vec2* best_axis) {
    float min_a, max_a, min_b, max_b;
    project_polygon_on_axis(a, poly_a, axis, &min_a, &max_a);
    project_polygon_on_axis(b, poly_b, axis, &min_b, &max_b);

    if (max_a < min_b || max_b < min_a) {
        return 0;
    }

    float overlap = min_f(max_a - min_b, max_b - min_a);
    if (overlap < *best_overlap) {
        *best_overlap = overlap;
        *best_axis = axis;
    }

    return 1;
}

static int point_in_polygon(RigidBody* body, PolygonShape* poly, Vec2 p) {
    int inside = 0;

    for (int i = 0, j = poly->vertex_count - 1; i < poly->vertex_count; j = i++) {
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

static void polygon_extreme_band(RigidBody* body, PolygonShape* poly, Vec2 normal, Vec2 tangent, int find_max,
                                 float* out_extreme_n, float* out_tmin, float* out_tmax) {
    const float eps = PHYSICS_TUNE_POLYGON_BAND_EPS;
    float best_n = find_max ? -FLT_MAX : FLT_MAX;

    for (int i = 0; i < poly->vertex_count; i++) {
        Vec2 w = body_get_world_point(body, poly->vertices[i]);
        float pn = vec2_dot(w, normal);
        if (find_max) {
            best_n = max_f(best_n, pn);
        } else {
            best_n = min_f(best_n, pn);
        }
    }

    float tmin = FLT_MAX;
    float tmax = -FLT_MAX;
    int count = 0;

    for (int i = 0; i < poly->vertex_count; i++) {
        Vec2 w = body_get_world_point(body, poly->vertices[i]);
        float pn = vec2_dot(w, normal);
        if (fabsf(pn - best_n) <= eps) {
            float pt = vec2_dot(w, tangent);
            tmin = min_f(tmin, pt);
            tmax = max_f(tmax, pt);
            count++;
        }
    }

    if (count <= 0) {
        for (int i = 0; i < poly->vertex_count; i++) {
            Vec2 w = body_get_world_point(body, poly->vertices[i]);
            float pn = vec2_dot(w, normal);
            if (is_equal(pn, best_n, 1e-4f)) {
                float pt = vec2_dot(w, tangent);
                tmin = min_f(tmin, pt);
                tmax = max_f(tmax, pt);
            }
        }
        if (tmin == FLT_MAX || tmax == -FLT_MAX) {
            tmin = 0.0f;
            tmax = 0.0f;
        }
    }

    *out_extreme_n = best_n;
    *out_tmin = tmin;
    *out_tmax = tmax;
}

static Vec2 build_polygon_contact_point(RigidBody* a, PolygonShape* poly_a, RigidBody* b, PolygonShape* poly_b,
                                        Vec2 normal, float* penetration_io) {
    Vec2 tangent = vec2(-normal.y, normal.x);

    float a_n, a_tmin, a_tmax;
    float b_n, b_tmin, b_tmax;
    polygon_extreme_band(a, poly_a, normal, tangent, 1, &a_n, &a_tmin, &a_tmax);
    polygon_extreme_band(b, poly_b, normal, tangent, 0, &b_n, &b_tmin, &b_tmax);

    float overlap_t_min = max_f(a_tmin, b_tmin);
    float overlap_t_max = min_f(a_tmax, b_tmax);
    float contact_t;
    if (overlap_t_min <= overlap_t_max) {
        contact_t = 0.5f * (overlap_t_min + overlap_t_max);
    } else {
        contact_t = 0.25f * (a_tmin + a_tmax + b_tmin + b_tmax);
    }

    float contact_n = 0.5f * (a_n + b_n);
    float extra_penetration = max_f(a_n - b_n, 0.0f);
    if (extra_penetration > 0.0f) {
        *penetration_io = min_f(*penetration_io, extra_penetration);
    }

    return vec2_add(vec2_scale(normal, contact_n), vec2_scale(tangent, contact_t));
}

int collision_circle_circle(RigidBody* a, RigidBody* b, CollisionInfo* info) {
    if (a->shape->type != SHAPE_CIRCLE || b->shape->type != SHAPE_CIRCLE) {
        return 0;
    }

    Vec2 delta = vec2_sub(b->position, a->position);
    float dist_sq = vec2_length_sq(delta);

    float r_a = a->shape->data.circle.radius;
    float r_b = b->shape->data.circle.radius;
    float min_dist = r_a + r_b;

    if (dist_sq > min_dist * min_dist) {
        return 0;
    }

    float dist = sqrtf(dist_sq);
    Vec2 normal;

    if (dist < 1e-6f) {
        normal = vec2(1, 0);
        dist = 0.0f;
    } else {
        normal = vec2_scale(delta, 1.0f / dist);
    }

    info->has_collision = 1;
    info->normal = normal;
    info->penetration = min_dist - dist;
    info->point = vec2_add(a->position, vec2_scale(normal, r_a));

    return 1;
}

int collision_circle_polygon(RigidBody* a, RigidBody* b, CollisionInfo* info) {
    if (a->shape->type != SHAPE_CIRCLE || b->shape->type != SHAPE_POLYGON) {
        return 0;
    }

    float radius = a->shape->data.circle.radius;
    PolygonShape* poly = &b->shape->data.polygon;

    float min_dist = FLT_MAX;
    Vec2 closest_point = b->position;

    for (int i = 0; i < poly->vertex_count; i++) {
        Vec2 v1 = body_get_world_point(b, poly->vertices[i]);
        Vec2 v2 = body_get_world_point(b, poly->vertices[(i + 1) % poly->vertex_count]);

        Vec2 edge = vec2_sub(v2, v1);
        float edge_len_sq = vec2_length_sq(edge);
        if (edge_len_sq < 1e-6f) {
            continue;
        }

        Vec2 to_center = vec2_sub(a->position, v1);
        float t = clamp(vec2_dot(to_center, edge) / edge_len_sq, 0.0f, 1.0f);
        Vec2 closest = vec2_add(v1, vec2_scale(edge, t));

        float dist = vec2_length(vec2_sub(a->position, closest));
        if (dist < min_dist) {
            min_dist = dist;
            closest_point = closest;
        }
    }

    int inside = point_in_polygon(b, poly, a->position);
    if (!inside && min_dist > radius) {
        return 0;
    }

    Vec2 normal = vec2_sub(closest_point, a->position);
    float normal_len = vec2_length(normal);

    if (normal_len < 1e-6f) {
        Vec2 center_dir = vec2_sub(b->position, a->position);
        if (vec2_length_sq(center_dir) < 1e-6f) {
            normal = vec2(1, 0);
        } else {
            normal = vec2_normalize(center_dir);
        }
    } else {
        normal = vec2_scale(normal, 1.0f / normal_len);
    }

    info->has_collision = 1;
    info->normal = normal;
    info->penetration = inside ? (radius + min_dist) : (radius - min_dist);
    info->point = closest_point;

    return 1;
}

int collision_polygon_polygon(RigidBody* a, RigidBody* b, CollisionInfo* info) {
    if (a->shape->type != SHAPE_POLYGON || b->shape->type != SHAPE_POLYGON) {
        return 0;
    }

    PolygonShape* poly_a = &a->shape->data.polygon;
    PolygonShape* poly_b = &b->shape->data.polygon;

    float best_overlap = FLT_MAX;
    Vec2 best_axis = vec2(1, 0);

    for (int i = 0; i < poly_a->vertex_count; i++) {
        Vec2 v1 = body_get_world_point(a, poly_a->vertices[i]);
        Vec2 v2 = body_get_world_point(a, poly_a->vertices[(i + 1) % poly_a->vertex_count]);
        Vec2 edge = vec2_sub(v2, v1);
        Vec2 axis = vec2_normalize(vec2(-edge.y, edge.x));

        if (!update_sat_axis(a, poly_a, b, poly_b, axis, &best_overlap, &best_axis)) {
            return 0;
        }
    }

    for (int i = 0; i < poly_b->vertex_count; i++) {
        Vec2 v1 = body_get_world_point(b, poly_b->vertices[i]);
        Vec2 v2 = body_get_world_point(b, poly_b->vertices[(i + 1) % poly_b->vertex_count]);
        Vec2 edge = vec2_sub(v2, v1);
        Vec2 axis = vec2_normalize(vec2(-edge.y, edge.x));

        if (!update_sat_axis(a, poly_a, b, poly_b, axis, &best_overlap, &best_axis)) {
            return 0;
        }
    }

    Vec2 center_delta = vec2_sub(b->position, a->position);
    if (vec2_dot(center_delta, best_axis) < 0.0f) {
        best_axis = vec2_negate(best_axis);
    }

    info->has_collision = 1;
    info->normal = best_axis;
    info->penetration = best_overlap;
    info->point = build_polygon_contact_point(a, poly_a, b, poly_b, info->normal, &info->penetration);

    return 1;
}

int sat_test_circle(RigidBody* a, RigidBody* b, CollisionInfo* info) {
    if (a->shape->type == SHAPE_CIRCLE && b->shape->type == SHAPE_CIRCLE) {
        return collision_circle_circle(a, b, info);
    }

    if (a->shape->type == SHAPE_CIRCLE && b->shape->type == SHAPE_POLYGON) {
        return collision_circle_polygon(a, b, info);
    }

    if (a->shape->type == SHAPE_POLYGON && b->shape->type == SHAPE_CIRCLE) {
        int hit = collision_circle_polygon(b, a, info);
        if (hit) {
            info->normal = vec2_negate(info->normal);
        }
        return hit;
    }

    return 0;
}

int sat_test_polygon(RigidBody* a, RigidBody* b, CollisionInfo* info) {
    if (a->shape->type != SHAPE_POLYGON || b->shape->type != SHAPE_POLYGON) {
        return 0;
    }

    return collision_polygon_polygon(a, b, info);
}

int collision_detect(RigidBody* a, RigidBody* b, CollisionInfo* info) {
    if (a == NULL || b == NULL || info == NULL || a->shape == NULL || b->shape == NULL) {
        return 0;
    }

    if (!a->active || !b->active) {
        return 0;
    }

    if (a->shape->type == SHAPE_CIRCLE && b->shape->type == SHAPE_CIRCLE) {
        return collision_circle_circle(a, b, info);
    }

    if (a->shape->type == SHAPE_CIRCLE && b->shape->type == SHAPE_POLYGON) {
        return collision_circle_polygon(a, b, info);
    }

    if (a->shape->type == SHAPE_POLYGON && b->shape->type == SHAPE_CIRCLE) {
        int result = collision_circle_polygon(b, a, info);
        if (result) {
            info->normal = vec2_negate(info->normal);
        }
        return result;
    }

    if (a->shape->type == SHAPE_POLYGON && b->shape->type == SHAPE_POLYGON) {
        return collision_polygon_polygon(a, b, info);
    }

    return 0;
}
