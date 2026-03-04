#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "../include/physics.h"

#define GRID_BUCKETS 2048
#define GRID_MAX_ENTRIES 32768
#define GRID_MAX_SPAN 24

typedef struct {
    int body_index;
    int next;
} GridEntry;

typedef struct {
    Vec2 min;
    Vec2 max;
} AABB;

typedef struct {
    RigidBody* a;
    RigidBody* b;
} BroadphasePair;

struct PhysicsEngine {
    Vec2 gravity;
    float damping;
    float time_step;
    int iterations;

    RigidBody* bodies[MAX_BODIES];
    int body_count;

    CollisionManifold contacts[MAX_CONTACTS];
    int contact_count;

    Constraint constraints[MAX_CONSTRAINTS];
    int constraint_count;

    BroadphasePair broadphase_pairs[MAX_BROADPHASE_PAIRS];
    int broadphase_pair_count;
    float broadphase_cell_size;
    int broadphase_use_grid;
};

static unsigned char g_pair_flags[MAX_BODIES][MAX_BODIES];

static void sanitize_dynamic_body_after_step(RigidBody* body, Vec2 prev_pos, float prev_angle) {
    const float max_linear_speed = 120.0f;
    const float max_angular_speed = 40.0f;
    float v_len;
    if (body == NULL || body->type != BODY_DYNAMIC) {
        return;
    }

    if (!isfinite(body->position.x) || !isfinite(body->position.y)) {
        body->position = prev_pos;
        body->velocity = vec2(0.0f, 0.0f);
    }
    if (!isfinite(body->angle)) {
        body->angle = prev_angle;
        body->angular_velocity = 0.0f;
    }
    if (!isfinite(body->velocity.x) || !isfinite(body->velocity.y)) {
        body->velocity = vec2(0.0f, 0.0f);
    }
    if (!isfinite(body->angular_velocity)) {
        body->angular_velocity = 0.0f;
    }

    v_len = vec2_length(body->velocity);
    if (v_len > max_linear_speed && v_len > 1e-6f) {
        body->velocity = vec2_scale(body->velocity, max_linear_speed / v_len);
    }
    if (body->angular_velocity > max_angular_speed) body->angular_velocity = max_angular_speed;
    if (body->angular_velocity < -max_angular_speed) body->angular_velocity = -max_angular_speed;
}

static int body_has_finite_state(const RigidBody* body) {
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

static int point_in_polygon_world(RigidBody* body, Vec2 p) {
    if (body == NULL || body->shape == NULL || body->shape->type != SHAPE_POLYGON) {
        return 0;
    }

    PolygonShape* poly = &body->shape->data.polygon;
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

static AABB body_compute_aabb(const RigidBody* body) {
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
        if (poly->vertex_count <= 0) {
            return box;
        }

        Vec2 p0 = body_get_world_point((RigidBody*)body, poly->vertices[0]);
        float min_x = p0.x;
        float min_y = p0.y;
        float max_x = p0.x;
        float max_y = p0.y;
        for (int i = 1; i < poly->vertex_count; i++) {
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

static int aabb_overlaps(AABB a, AABB b) {
    if (a.max.x < b.min.x || b.max.x < a.min.x) return 0;
    if (a.max.y < b.min.y || b.max.y < a.min.y) return 0;
    return 1;
}

static int physics_engine_pair_blocked_by_constraint(const PhysicsEngine* engine, const RigidBody* a, const RigidBody* b) {
    for (int i = 0; i < engine->constraint_count; i++) {
        const Constraint* c = &engine->constraints[i];
        if (!c->active || c->collide_connected) {
            continue;
        }
        if ((c->body_a == a && c->body_b == b) || (c->body_a == b && c->body_b == a)) {
            return 1;
        }
    }
    return 0;
}

static int broadphase_add_pair(PhysicsEngine* engine, int ia, int ib, AABB* aabbs) {
    if (ia == ib) {
        return 0;
    }
    if (ia > ib) {
        int t = ia;
        ia = ib;
        ib = t;
    }
    if (g_pair_flags[ia][ib]) {
        return 0;
    }
    g_pair_flags[ia][ib] = 1;

    RigidBody* a = engine->bodies[ia];
    RigidBody* b = engine->bodies[ib];
    if (a == NULL || b == NULL || !a->active || !b->active || a->shape == NULL || b->shape == NULL) {
        return 0;
    }
    if (a->type == BODY_STATIC && b->type == BODY_STATIC) {
        return 0;
    }
    if (physics_engine_pair_blocked_by_constraint(engine, a, b)) {
        return 0;
    }
    if (!aabb_overlaps(aabbs[ia], aabbs[ib])) {
        return 0;
    }
    if (engine->broadphase_pair_count >= MAX_BROADPHASE_PAIRS) {
        return 0;
    }

    engine->broadphase_pairs[engine->broadphase_pair_count].a = a;
    engine->broadphase_pairs[engine->broadphase_pair_count].b = b;
    engine->broadphase_pair_count++;
    return 1;
}

static int grid_hash(int x, int y) {
    unsigned int hx = (unsigned int)(x * 73856093);
    unsigned int hy = (unsigned int)(y * 19349663);
    return (int)((hx ^ hy) % GRID_BUCKETS);
}

static void broadphase_build_bruteforce(PhysicsEngine* engine, AABB* aabbs) {
    for (int i = 0; i < engine->body_count; i++) {
        for (int j = i + 1; j < engine->body_count; j++) {
            broadphase_add_pair(engine, i, j, aabbs);
        }
    }
}

static void broadphase_build_grid(PhysicsEngine* engine, AABB* aabbs, float cell_size) {
    int bucket_heads[GRID_BUCKETS];
    GridEntry entries[GRID_MAX_ENTRIES];
    int entry_count = 0;
    memset(bucket_heads, -1, sizeof(bucket_heads));

    for (int i = 0; i < engine->body_count; i++) {
        RigidBody* body = engine->bodies[i];
        if (body == NULL || !body->active || body->shape == NULL) {
            continue;
        }

        int min_cx = (int)floorf(aabbs[i].min.x / cell_size);
        int max_cx = (int)floorf(aabbs[i].max.x / cell_size);
        int min_cy = (int)floorf(aabbs[i].min.y / cell_size);
        int max_cy = (int)floorf(aabbs[i].max.y / cell_size);

        if ((max_cx - min_cx) > GRID_MAX_SPAN || (max_cy - min_cy) > GRID_MAX_SPAN) {
            broadphase_build_bruteforce(engine, aabbs);
            return;
        }

        for (int cy = min_cy; cy <= max_cy; cy++) {
            for (int cx = min_cx; cx <= max_cx; cx++) {
                int h = grid_hash(cx, cy);
                if (entry_count >= GRID_MAX_ENTRIES) {
                    broadphase_build_bruteforce(engine, aabbs);
                    return;
                }

                entries[entry_count].body_index = i;
                entries[entry_count].next = bucket_heads[h];
                bucket_heads[h] = entry_count;
                entry_count++;
            }
        }
    }

    for (int h = 0; h < GRID_BUCKETS; h++) {
        for (int ei = bucket_heads[h]; ei != -1; ei = entries[ei].next) {
            for (int ej = entries[ei].next; ej != -1; ej = entries[ej].next) {
                int ia = entries[ei].body_index;
                int ib = entries[ej].body_index;
                broadphase_add_pair(engine, ia, ib, aabbs);
            }
        }
    }
}

static int intersect_segments(Vec2 p, Vec2 r, Vec2 q, Vec2 s, float* out_t) {
    float denom = vec2_cross(r, s);
    if (fabsf(denom) < 1e-6f) {
        return 0;
    }

    Vec2 qp = vec2_sub(q, p);
    float t = vec2_cross(qp, s) / denom;
    float u = vec2_cross(qp, r) / denom;

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
    if (a < 1e-6f) {
        return 0;
    }

    if (vec2_dot(f, f) <= radius * radius) {
        *out_t = 0.0f;
        return 1;
    }

    float b = 2.0f * vec2_dot(f, d);
    float c = vec2_dot(f, f) - radius * radius;
    float disc = b * b - 4.0f * a * c;

    if (disc < 0.0f) {
        return 0;
    }

    float sqrt_disc = sqrtf(disc);
    float t1 = (-b - sqrt_disc) / (2.0f * a);
    float t2 = (-b + sqrt_disc) / (2.0f * a);

    float hit_t = FLT_MAX;
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

    if (point_in_polygon_world(body, start)) {
        *out_t = 0.0f;
        return 1;
    }

    Vec2 r = vec2_sub(end, start);
    float best_t = FLT_MAX;

    for (int i = 0; i < poly->vertex_count; i++) {
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

PhysicsEngine* physics_engine_create(void) {
    PhysicsEngine* engine = (PhysicsEngine*)malloc(sizeof(PhysicsEngine));
    if (engine == NULL) {
        return NULL;
    }

    engine->gravity = vec2(0, 9.8f);
    engine->damping = 0.99f;
    engine->time_step = 1.0f / 60.0f;
    engine->iterations = 5;
    engine->body_count = 0;
    engine->contact_count = 0;
    engine->constraint_count = 0;
    engine->broadphase_pair_count = 0;
    engine->broadphase_cell_size = 10.0f;
    engine->broadphase_use_grid = 1;

    return engine;
}

void physics_engine_free(PhysicsEngine* engine) {
    if (engine == NULL) {
        return;
    }

    for (int i = 0; i < engine->body_count; i++) {
        body_free(engine->bodies[i]);
        engine->bodies[i] = NULL;
    }

    free(engine);
}

void physics_engine_set_gravity(PhysicsEngine* engine, Vec2 gravity) {
    if (engine == NULL) {
        return;
    }

    engine->gravity = gravity;
}

void physics_engine_set_time_step(PhysicsEngine* engine, float dt) {
    if (engine == NULL || dt <= 0.0f) {
        return;
    }

    engine->time_step = dt;
}

void physics_engine_set_iterations(PhysicsEngine* engine, int iterations) {
    if (engine == NULL || iterations < 1) {
        return;
    }

    engine->iterations = iterations;
}

void physics_engine_set_damping(PhysicsEngine* engine, float damping) {
    if (engine == NULL) {
        return;
    }

    engine->damping = clamp(damping, 0.0f, 1.0f);
}

void physics_engine_set_broadphase_cell_size(PhysicsEngine* engine, float cell_size) {
    if (engine == NULL || cell_size <= 0.0f) {
        return;
    }

    engine->broadphase_cell_size = cell_size;
}

void physics_engine_set_broadphase_use_grid(PhysicsEngine* engine, int enable) {
    if (engine == NULL) {
        return;
    }

    engine->broadphase_use_grid = enable ? 1 : 0;
}

Vec2 physics_engine_get_gravity(const PhysicsEngine* engine) {
    if (engine == NULL) {
        return vec2(0.0f, 0.0f);
    }
    return engine->gravity;
}

float physics_engine_get_time_step(const PhysicsEngine* engine) {
    if (engine == NULL) {
        return 0.0f;
    }
    return engine->time_step;
}

int physics_engine_get_iterations(const PhysicsEngine* engine) {
    if (engine == NULL) {
        return 0;
    }
    return engine->iterations;
}

float physics_engine_get_damping(const PhysicsEngine* engine) {
    if (engine == NULL) {
        return 0.0f;
    }
    return engine->damping;
}

float physics_engine_get_broadphase_cell_size(const PhysicsEngine* engine) {
    if (engine == NULL) {
        return 0.0f;
    }
    return engine->broadphase_cell_size;
}

int physics_engine_get_broadphase_use_grid(const PhysicsEngine* engine) {
    if (engine == NULL) {
        return 0;
    }
    return engine->broadphase_use_grid;
}

int physics_engine_get_body_count(const PhysicsEngine* engine) {
    if (engine == NULL) {
        return 0;
    }
    return engine->body_count;
}

RigidBody* physics_engine_get_body(const PhysicsEngine* engine, int index) {
    if (engine == NULL || index < 0 || index >= engine->body_count) {
        return NULL;
    }
    return engine->bodies[index];
}

int physics_engine_get_contact_count(const PhysicsEngine* engine) {
    if (engine == NULL) {
        return 0;
    }
    return engine->contact_count;
}

const CollisionManifold* physics_engine_get_contact(const PhysicsEngine* engine, int index) {
    if (engine == NULL || index < 0 || index >= engine->contact_count) {
        return NULL;
    }
    return &engine->contacts[index];
}

int physics_engine_get_constraint_count(const PhysicsEngine* engine) {
    if (engine == NULL) {
        return 0;
    }
    return engine->constraint_count;
}

const Constraint* physics_engine_get_constraint(const PhysicsEngine* engine, int index) {
    if (engine == NULL || index < 0 || index >= engine->constraint_count) {
        return NULL;
    }
    return &engine->constraints[index];
}

int physics_engine_find_constraint_index(const PhysicsEngine* engine, const Constraint* constraint) {
    int i;
    if (engine == NULL || constraint == NULL) {
        return -1;
    }
    for (i = 0; i < engine->constraint_count; i++) {
        if (&engine->constraints[i] == constraint) {
            return i;
        }
    }
    return -1;
}

int physics_engine_constraint_is_active(const PhysicsEngine* engine, int index) {
    const Constraint* c = physics_engine_get_constraint(engine, index);
    if (c == NULL) {
        return 0;
    }
    return c->active;
}

ConstraintType physics_engine_constraint_get_type(const PhysicsEngine* engine, int index) {
    const Constraint* c = physics_engine_get_constraint(engine, index);
    if (c == NULL) {
        return CONSTRAINT_DISTANCE;
    }
    return c->type;
}

float physics_engine_constraint_get_rest_length(const PhysicsEngine* engine, int index) {
    const Constraint* c = physics_engine_get_constraint(engine, index);
    if (c == NULL) {
        return 0.0f;
    }
    return c->rest_length;
}

float physics_engine_constraint_get_stiffness(const PhysicsEngine* engine, int index) {
    const Constraint* c = physics_engine_get_constraint(engine, index);
    if (c == NULL) {
        return 0.0f;
    }
    return c->stiffness;
}

float physics_engine_constraint_get_damping(const PhysicsEngine* engine, int index) {
    const Constraint* c = physics_engine_get_constraint(engine, index);
    if (c == NULL) {
        return 0.0f;
    }
    return c->damping;
}

float physics_engine_constraint_get_break_force(const PhysicsEngine* engine, int index) {
    const Constraint* c = physics_engine_get_constraint(engine, index);
    if (c == NULL) {
        return 0.0f;
    }
    return c->break_force;
}

float physics_engine_constraint_get_last_force(const PhysicsEngine* engine, int index) {
    const Constraint* c = physics_engine_get_constraint(engine, index);
    if (c == NULL) {
        return 0.0f;
    }
    return c->last_force;
}

int physics_engine_constraint_get_collide_connected(const PhysicsEngine* engine, int index) {
    const Constraint* c = physics_engine_get_constraint(engine, index);
    if (c == NULL) {
        return 0;
    }
    return c->collide_connected;
}

void physics_engine_constraint_set_active(PhysicsEngine* engine, int index, int active) {
    Constraint* c;
    if (engine == NULL || index < 0 || index >= engine->constraint_count) {
        return;
    }
    c = &engine->constraints[index];
    c->active = active ? 1 : 0;
}

void physics_engine_constraint_set_rest_length(PhysicsEngine* engine, int index, float rest_length) {
    Constraint* c;
    if (engine == NULL || index < 0 || index >= engine->constraint_count) {
        return;
    }
    c = &engine->constraints[index];
    c->rest_length = max_f(rest_length, 0.0f);
}

void physics_engine_constraint_set_stiffness(PhysicsEngine* engine, int index, float stiffness) {
    Constraint* c;
    if (engine == NULL || index < 0 || index >= engine->constraint_count) {
        return;
    }
    c = &engine->constraints[index];
    c->stiffness = max_f(stiffness, 0.0f);
}

void physics_engine_constraint_set_damping(PhysicsEngine* engine, int index, float damping) {
    Constraint* c;
    if (engine == NULL || index < 0 || index >= engine->constraint_count) {
        return;
    }
    c = &engine->constraints[index];
    c->damping = max_f(damping, 0.0f);
}

void physics_engine_constraint_set_break_force(PhysicsEngine* engine, int index, float break_force) {
    Constraint* c;
    if (engine == NULL || index < 0 || index >= engine->constraint_count) {
        return;
    }
    c = &engine->constraints[index];
    c->break_force = max_f(break_force, 0.0f);
}

void physics_engine_constraint_set_collide_connected(PhysicsEngine* engine, int index, int collide_connected) {
    Constraint* c;
    if (engine == NULL || index < 0 || index >= engine->constraint_count) {
        return;
    }
    c = &engine->constraints[index];
    c->collide_connected = collide_connected ? 1 : 0;
}

int physics_engine_get_broadphase_pair_count(const PhysicsEngine* engine) {
    if (engine == NULL) {
        return 0;
    }
    return engine->broadphase_pair_count;
}

void physics_engine_add_body(PhysicsEngine* engine, RigidBody* body) {
    if (engine == NULL || body == NULL) {
        return;
    }

    for (int i = 0; i < engine->body_count; i++) {
        if (engine->bodies[i] == body) {
            return;
        }
    }

    if (engine->body_count >= MAX_BODIES) {
        return;
    }

    engine->bodies[engine->body_count++] = body;
}

RigidBody* physics_engine_detach_body(PhysicsEngine* engine, RigidBody* body) {
    if (engine == NULL || body == NULL) {
        return NULL;
    }

    for (int i = 0; i < engine->body_count; i++) {
        if (engine->bodies[i] == body) {
            for (int ci = 0; ci < engine->constraint_count;) {
                Constraint* c = &engine->constraints[ci];
                if (c->body_a == body || c->body_b == body) {
                    for (int cj = ci; cj < engine->constraint_count - 1; cj++) {
                        engine->constraints[cj] = engine->constraints[cj + 1];
                    }
                    engine->constraint_count--;
                    continue;
                }
                ci++;
            }

            RigidBody* detached = engine->bodies[i];
            for (int j = i; j < engine->body_count - 1; j++) {
                engine->bodies[j] = engine->bodies[j + 1];
            }
            engine->bodies[engine->body_count - 1] = NULL;
            engine->body_count--;
            return detached;
        }
    }

    return NULL;
}

Constraint* physics_engine_add_distance_constraint(PhysicsEngine* engine, RigidBody* a, RigidBody* b,
                                                   Vec2 world_anchor_a, Vec2 world_anchor_b,
                                                   float stiffness, int collide_connected) {
    if (engine == NULL || a == NULL || b == NULL) {
        return NULL;
    }
    if (engine->constraint_count >= MAX_CONSTRAINTS) {
        return NULL;
    }

    Constraint* c = &engine->constraints[engine->constraint_count++];
    constraint_init_distance(c, a, b, world_anchor_a, world_anchor_b, stiffness, collide_connected);
    return c;
}

Constraint* physics_engine_add_spring_constraint(PhysicsEngine* engine, RigidBody* a, RigidBody* b,
                                                 Vec2 world_anchor_a, Vec2 world_anchor_b,
                                                 float rest_length, float stiffness, float damping,
                                                 int collide_connected) {
    if (engine == NULL || a == NULL || b == NULL) {
        return NULL;
    }
    if (engine->constraint_count >= MAX_CONSTRAINTS) {
        return NULL;
    }

    Constraint* c = &engine->constraints[engine->constraint_count++];
    constraint_init_spring(c, a, b, world_anchor_a, world_anchor_b, rest_length, stiffness, damping, collide_connected);
    return c;
}

void physics_engine_clear_constraints(PhysicsEngine* engine) {
    if (engine == NULL) {
        return;
    }

    engine->constraint_count = 0;
}

void physics_engine_remove_body(PhysicsEngine* engine, RigidBody* body) {
    RigidBody* detached = physics_engine_detach_body(engine, body);
    if (detached != NULL) {
        body_free(detached);
    }
}

void physics_engine_update_velocities(PhysicsEngine* engine) {
    if (engine == NULL) {
        return;
    }

    for (int i = 0; i < engine->body_count; i++) {
        RigidBody* body = engine->bodies[i];
        if (body == NULL || body->type != BODY_DYNAMIC) {
            continue;
        }
        if (!body_has_finite_state(body)) {
            body->active = 0;
            continue;
        }

        Vec2 gravity = body->use_custom_gravity ? body->gravity : engine->gravity;
        Vec2 gravity_force = vec2_scale(gravity, body->mass);
        body->force = vec2_add(body->force, gravity_force);

        body->acceleration = vec2_scale(body->force, body->inv_mass);
        body->angular_acceleration = body->torque * body->inv_inertia;

        body->velocity = vec2_add(body->velocity, vec2_scale(body->acceleration, engine->time_step));
        body->angular_velocity += body->angular_acceleration * engine->time_step;

        body->velocity = vec2_scale(body->velocity, engine->damping * body->damping);
        body->angular_velocity *= (engine->damping * body->damping);

        // Kill tiny residual motion to prevent endless rolling/jitter.
        if (vec2_length_sq(body->velocity) < 1e-4f) {
            body->velocity = vec2(0.0f, 0.0f);
        }
        if (fabsf(body->angular_velocity) < 0.01f) {
            body->angular_velocity = 0.0f;
        }
    }
}

void physics_engine_update_positions(PhysicsEngine* engine) {
    if (engine == NULL) {
        return;
    }

    for (int i = 0; i < engine->body_count; i++) {
        RigidBody* body = engine->bodies[i];
        if (body == NULL || body->type == BODY_STATIC) {
            continue;
        }
        if (!body_has_finite_state(body)) {
            body->active = 0;
            continue;
        }

        body->position = vec2_add(body->position, vec2_scale(body->velocity, engine->time_step));
        body->angle += body->angular_velocity * engine->time_step;
    }
}

void physics_engine_detect_collisions(PhysicsEngine* engine) {
    if (engine == NULL) {
        return;
    }

    engine->contact_count = 0;
    engine->broadphase_pair_count = 0;
    memset(g_pair_flags, 0, sizeof(g_pair_flags));

    AABB aabbs[MAX_BODIES];
    for (int i = 0; i < engine->body_count; i++) {
        RigidBody* body = engine->bodies[i];
        if (body == NULL || !body->active || body->shape == NULL || !body_has_finite_state(body)) {
            if (body != NULL) body->active = 0;
            aabbs[i].min = vec2(0.0f, 0.0f);
            aabbs[i].max = vec2(0.0f, 0.0f);
            continue;
        }
        aabbs[i] = body_compute_aabb(body);
    }

    if (engine->broadphase_use_grid && engine->broadphase_cell_size > 1e-6f) {
        broadphase_build_grid(engine, aabbs, engine->broadphase_cell_size);
    } else {
        broadphase_build_bruteforce(engine, aabbs);
    }

    for (int i = 0; i < engine->broadphase_pair_count; i++) {
        RigidBody* a = engine->broadphase_pairs[i].a;
        RigidBody* b = engine->broadphase_pairs[i].b;
        CollisionInfo info = {0};
        if (!collision_detect(a, b, &info)) {
            continue;
        }
        if (engine->contact_count >= MAX_CONTACTS) {
            break;
        }
        engine->contacts[engine->contact_count].bodyA = a;
        engine->contacts[engine->contact_count].bodyB = b;
        engine->contacts[engine->contact_count].info = info;
        engine->contact_count++;
    }
}

void physics_engine_resolve_collisions(PhysicsEngine* engine) {
    if (engine == NULL) {
        return;
    }
    int position_iters = engine->iterations + 3;

    for (int i = 0; i < engine->constraint_count; i++) {
        constraint_warm_start(&engine->constraints[i]);
    }

    for (int iter = 0; iter < engine->iterations; iter++) {
        for (int i = 0; i < engine->contact_count; i++) {
            collision_resolve_velocity(&engine->contacts[i]);
        }
        for (int i = 0; i < engine->constraint_count; i++) {
            constraint_solve_velocity(&engine->constraints[i], engine->time_step);
        }
    }

    for (int iter = 0; iter < position_iters; iter++) {
        for (int i = 0; i < engine->contact_count; i++) {
            CollisionManifold* m = &engine->contacts[i];
            if (m->bodyA == NULL || m->bodyB == NULL) {
                continue;
            }

            CollisionInfo refreshed = {0};
            if (!collision_detect(m->bodyA, m->bodyB, &refreshed)) {
                continue;
            }

            m->info = refreshed;
            collision_resolve_position(m);
        }
        for (int i = 0; i < engine->constraint_count; i++) {
            constraint_solve_position(&engine->constraints[i]);
        }
    }
}

void physics_engine_clear_forces(PhysicsEngine* engine) {
    if (engine == NULL) {
        return;
    }

    for (int i = 0; i < engine->body_count; i++) {
        RigidBody* body = engine->bodies[i];
        if (body == NULL) {
            continue;
        }

        body->force = vec2(0, 0);
        body->torque = 0.0f;
    }
}

void physics_engine_step(PhysicsEngine* engine) {
    enum { PHYSICS_SUBSTEPS = 3 };
    Vec2 saved_force[MAX_BODIES];
    Vec2 prev_pos[MAX_BODIES];
    float prev_angle[MAX_BODIES];
    float saved_torque[MAX_BODIES];
    float full_dt;
    int i;
    int sub;
    if (engine == NULL) {
        return;
    }

    full_dt = engine->time_step;
    for (i = 0; i < engine->body_count; i++) {
        RigidBody* body = engine->bodies[i];
        if (body == NULL) {
            saved_force[i] = vec2(0.0f, 0.0f);
            saved_torque[i] = 0.0f;
            prev_pos[i] = vec2(0.0f, 0.0f);
            prev_angle[i] = 0.0f;
            continue;
        }
        saved_force[i] = body->force;
        saved_torque[i] = body->torque;
        prev_pos[i] = body->position;
        prev_angle[i] = body->angle;
    }

    engine->time_step = full_dt / (float)PHYSICS_SUBSTEPS;
    for (sub = 0; sub < PHYSICS_SUBSTEPS; sub++) {
        for (i = 0; i < engine->body_count; i++) {
            RigidBody* body = engine->bodies[i];
            if (body == NULL) continue;
            body->force = saved_force[i];
            body->torque = saved_torque[i];
        }
        physics_engine_update_velocities(engine);
        physics_engine_update_positions(engine);
        physics_engine_detect_collisions(engine);
        physics_engine_resolve_collisions(engine);
        for (i = 0; i < engine->body_count; i++) {
            sanitize_dynamic_body_after_step(engine->bodies[i], prev_pos[i], prev_angle[i]);
            if (engine->bodies[i] != NULL) {
                prev_pos[i] = engine->bodies[i]->position;
                prev_angle[i] = engine->bodies[i]->angle;
            }
        }
    }
    engine->time_step = full_dt;
    physics_engine_clear_forces(engine);
}

RigidBody* physics_engine_raycast(PhysicsEngine* engine, Vec2 start, Vec2 end) {
    if (engine == NULL) {
        return NULL;
    }

    RigidBody* closest = NULL;
    float closest_t = FLT_MAX;

    for (int i = 0; i < engine->body_count; i++) {
        RigidBody* body = engine->bodies[i];
        if (body == NULL || body->shape == NULL || !body->active) {
            continue;
        }

        float t = 0.0f;
        int hit = 0;

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

int physics_engine_get_bodies_in_area(PhysicsEngine* engine, Vec2 center, float radius,
                                      RigidBody** out_bodies, int max_bodies) {
    if (engine == NULL || out_bodies == NULL || max_bodies <= 0) {
        return 0;
    }

    int count = 0;

    for (int i = 0; i < engine->body_count && count < max_bodies; i++) {
        RigidBody* body = engine->bodies[i];
        if (body == NULL || body->shape == NULL) {
            continue;
        }

        Vec2 delta = vec2_sub(body->position, center);
        float dist = vec2_length(delta);

        if (body->shape->type == SHAPE_CIRCLE) {
            dist -= body->shape->data.circle.radius;
        }

        if (dist < radius) {
            out_bodies[count++] = body;
        }
    }

    return count;
}
