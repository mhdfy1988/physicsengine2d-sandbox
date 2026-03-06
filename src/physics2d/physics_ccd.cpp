#include <math.h>
#include <stddef.h>
#include "physics_internal.hpp"

static int ccd_pair_already_contacted(const PhysicsEngine* engine, int ia, int ib) {
    int i;
    RigidBody* a = physics_internal_body_from_id(engine, ia);
    RigidBody* b = physics_internal_body_from_id(engine, ib);
    if (a == NULL || b == NULL) return 1;
    for (i = 0; i < engine->contact_count; i++) {
        const CollisionManifold* m = &engine->contacts[i];
        if ((m->bodyA == a && m->bodyB == b) || (m->bodyA == b && m->bodyB == a)) {
            return 1;
        }
    }
    return 0;
}

static int ccd_swept_circle_circle(const RigidBody* a, const RigidBody* b, float dt, CollisionInfo* out) {
    float ra;
    float rb;
    float rs;
    Vec2 p0a;
    Vec2 p0b;
    Vec2 s;
    Vec2 v;
    float A;
    float B;
    float C;
    float disc;
    float t;
    Vec2 pa;
    Vec2 pb;
    Vec2 n;
    if (a == NULL || b == NULL || out == NULL) return 0;
    if (a->shape == NULL || b->shape == NULL) return 0;
    if (a->shape->type != SHAPE_CIRCLE || b->shape->type != SHAPE_CIRCLE) return 0;
    if (dt <= 0.0f) return 0;
    ra = a->shape->data.circle.radius;
    rb = b->shape->data.circle.radius;
    rs = ra + rb;
    p0a = vec2_sub(a->position, vec2_scale(a->velocity, dt));
    p0b = vec2_sub(b->position, vec2_scale(b->velocity, dt));
    s = vec2_sub(p0b, p0a);
    v = vec2_sub(b->velocity, a->velocity);
    A = vec2_dot(v, v);
    B = 2.0f * vec2_dot(s, v);
    C = vec2_dot(s, s) - rs * rs;
    if (C <= 0.0f) {
        return 0;
    }
    if (A <= 1e-8f) {
        return 0;
    }
    disc = B * B - 4.0f * A * C;
    if (disc < 0.0f) {
        return 0;
    }
    t = (-B - sqrtf(disc)) / (2.0f * A);
    if (t < 0.0f || t > dt) {
        return 0;
    }
    pa = vec2_add(p0a, vec2_scale(a->velocity, t));
    pb = vec2_add(p0b, vec2_scale(b->velocity, t));
    n = vec2_sub(pb, pa);
    if (vec2_length_sq(n) < 1e-8f) {
        n = vec2(1.0f, 0.0f);
    } else {
        n = vec2_normalize(n);
    }
    out->has_collision = 1;
    out->normal = n;
    out->penetration = 0.001f;
    out->point = vec2_add(pa, vec2_scale(n, ra));
    return 1;
}

extern "C" {

void physics_internal_append_ccd_contacts(PhysicsEngine* engine, float dt) {
    int i;
    if (engine == NULL) return;
    if (!engine->experimental.ccd_enabled) return;
    if (dt <= 0.0f) return;
    for (i = 0; i < engine->broadphase_pair_count; i++) {
        int ia = engine->broadphase_pairs[i].ia;
        int ib = engine->broadphase_pairs[i].ib;
        RigidBody* a = physics_internal_body_from_id(engine, ia);
        RigidBody* b = physics_internal_body_from_id(engine, ib);
        CollisionInfo info = {};
        if (a == NULL || b == NULL) continue;
        if (a->type == BODY_STATIC && b->type == BODY_STATIC) continue;
        if (ccd_pair_already_contacted(engine, ia, ib)) continue;
        if (!ccd_swept_circle_circle(a, b, dt, &info)) continue;
        (void)physics_engine_add_contact(engine, a, b, &info);
    }
}

}  // extern "C"
