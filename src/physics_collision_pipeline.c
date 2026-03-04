#include <math.h>
#include <stddef.h>
#include "physics_internal.h"

int physics_internal_body_has_finite_state(const RigidBody* body) {
    int i;
    if (body == NULL || body->shape == NULL) {
        return 0;
    }
    if (!isfinite(body->position.x) || !isfinite(body->position.y) ||
        !isfinite(body->velocity.x) || !isfinite(body->velocity.y) ||
        !isfinite(body->angle) || !isfinite(body->angular_velocity) ||
        !isfinite(body->mass) || !isfinite(body->inv_mass) ||
        !isfinite(body->inertia) || !isfinite(body->inv_inertia) ||
        !isfinite(body->damping)) {
        return 0;
    }
    if (body->shape->type == SHAPE_CIRCLE) {
        if (!isfinite(body->shape->data.circle.radius) || body->shape->data.circle.radius <= 0.0f) {
            return 0;
        }
    } else if (body->shape->type == SHAPE_POLYGON) {
        int n = body->shape->data.polygon.vertex_count;
        if (n <= 0 || n > 8) {
            return 0;
        }
        for (i = 0; i < n; i++) {
            Vec2 v = body->shape->data.polygon.vertices[i];
            if (!isfinite(v.x) || !isfinite(v.y)) {
                return 0;
            }
        }
    } else {
        return 0;
    }
    return 1;
}

AABB physics_internal_body_compute_aabb(const RigidBody* body) {
    AABB box;
    box.min = vec2(0.0f, 0.0f);
    box.max = vec2(0.0f, 0.0f);

    if (body == NULL || body->shape == NULL) {
        return box;
    }
    box.min = body->position;
    box.max = body->position;

    if (body->shape->type == SHAPE_CIRCLE) {
        float r = body->shape->data.circle.radius;
        box.min = vec2(body->position.x - r, body->position.y - r);
        box.max = vec2(body->position.x + r, body->position.y + r);
        return box;
    }

    if (body->shape->type == SHAPE_POLYGON) {
        PolygonShape* poly = (PolygonShape*)&body->shape->data.polygon;
        int i;
        float min_x;
        float min_y;
        float max_x;
        float max_y;
        if (poly->vertex_count <= 0) {
            return box;
        }

        {
            Vec2 p0 = body_get_world_point((RigidBody*)body, poly->vertices[0]);
            min_x = p0.x;
            min_y = p0.y;
            max_x = p0.x;
            max_y = p0.y;
        }
        for (i = 1; i < poly->vertex_count; i++) {
            Vec2 w = body_get_world_point((RigidBody*)body, poly->vertices[i]);
            min_x = min_f(min_x, w.x);
            min_y = min_f(min_y, w.y);
            max_x = max_f(max_x, w.x);
            max_y = max_f(max_y, w.y);
        }
        box.min = vec2(min_x, min_y);
        box.max = vec2(max_x, max_y);
    }

    return box;
}

void physics_internal_bind_default_pipeline(PhysicsEngine* engine) {
    if (engine == NULL) return;
    engine->broadphase_ops.build_pairs = physics_internal_default_build_pairs;
    engine->broadphase_ops.user = NULL;
    engine->narrowphase_ops.build_contacts = physics_internal_default_build_contacts;
    engine->narrowphase_ops.user = NULL;
}

void physics_internal_prepare_collision_inputs(PhysicsEngine* engine) {
    int i;
    if (engine == NULL) {
        return;
    }

    engine->broadphase_pair_count = 0;
    for (i = 0; i < engine->body_count; i++) {
        RigidBody* body = engine->bodies[i];
        if (body == NULL || !body->active || body->shape == NULL || !physics_internal_body_has_finite_state(body)) {
            if (body != NULL) body->active = 0;
            engine->aabbs[i].min = vec2(0.0f, 0.0f);
            engine->aabbs[i].max = vec2(0.0f, 0.0f);
            continue;
        }
        engine->aabbs[i] = physics_internal_body_compute_aabb(body);
    }
}

void physics_internal_detect_collisions(PhysicsEngine* engine) {
    if (engine == NULL) return;
    engine->contact_count = 0;
    physics_internal_prepare_collision_inputs(engine);
    if (engine->broadphase_ops.build_pairs != NULL) {
        engine->broadphase_ops.build_pairs(engine, engine->broadphase_ops.user);
    }
    if (engine->narrowphase_ops.build_contacts != NULL) {
        engine->narrowphase_ops.build_contacts(engine, engine->narrowphase_ops.user);
    }
}
