#include <math.h>
#include <string.h>
#include "physics_internal.h"

#define GRID_BUCKETS 2048
#define GRID_MAX_ENTRIES 32768
#define GRID_MAX_SPAN 24

typedef struct {
    int body_index;
    int next;
} GridEntry;

static unsigned char g_pair_flags[MAX_BODIES][MAX_BODIES];

static int aabb_overlaps(AABB a, AABB b) {
    if (a.max.x < b.min.x || b.max.x < a.min.x) return 0;
    if (a.max.y < b.min.y || b.max.y < a.min.y) return 0;
    return 1;
}

static int physics_engine_pair_blocked_by_constraint(const PhysicsEngine* engine, const RigidBody* a, const RigidBody* b) {
    int i;
    for (i = 0; i < engine->constraint_count; i++) {
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
    RigidBody* a;
    RigidBody* b;
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

    a = engine->bodies[ia];
    b = engine->bodies[ib];
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
    int i;
    int j;
    for (i = 0; i < engine->body_count; i++) {
        for (j = i + 1; j < engine->body_count; j++) {
            broadphase_add_pair(engine, i, j, aabbs);
        }
    }
}

static void broadphase_build_grid(PhysicsEngine* engine, AABB* aabbs, float cell_size) {
    int bucket_heads[GRID_BUCKETS];
    GridEntry entries[GRID_MAX_ENTRIES];
    int entry_count = 0;
    int i;
    memset(bucket_heads, -1, sizeof(bucket_heads));

    for (i = 0; i < engine->body_count; i++) {
        RigidBody* body = engine->bodies[i];
        int min_cx;
        int max_cx;
        int min_cy;
        int max_cy;
        int cx;
        int cy;
        if (body == NULL || !body->active || body->shape == NULL) {
            continue;
        }

        min_cx = (int)floorf(aabbs[i].min.x / cell_size);
        max_cx = (int)floorf(aabbs[i].max.x / cell_size);
        min_cy = (int)floorf(aabbs[i].min.y / cell_size);
        max_cy = (int)floorf(aabbs[i].max.y / cell_size);

        if ((max_cx - min_cx) > GRID_MAX_SPAN || (max_cy - min_cy) > GRID_MAX_SPAN) {
            broadphase_build_bruteforce(engine, aabbs);
            return;
        }

        for (cy = min_cy; cy <= max_cy; cy++) {
            for (cx = min_cx; cx <= max_cx; cx++) {
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

    for (i = 0; i < GRID_BUCKETS; i++) {
        int ei;
        for (ei = bucket_heads[i]; ei != -1; ei = entries[ei].next) {
            int ej;
            for (ej = entries[ei].next; ej != -1; ej = entries[ej].next) {
                int ia = entries[ei].body_index;
                int ib = entries[ej].body_index;
                broadphase_add_pair(engine, ia, ib, aabbs);
            }
        }
    }
}

void physics_internal_build_broadphase_pairs(PhysicsEngine* engine, AABB* aabbs) {
    if (engine == NULL || aabbs == NULL) {
        return;
    }
    memset(g_pair_flags, 0, sizeof(g_pair_flags));
    if (engine->broadphase_use_grid && engine->broadphase_cell_size > 1e-6f) {
        broadphase_build_grid(engine, aabbs, engine->broadphase_cell_size);
    } else {
        broadphase_build_bruteforce(engine, aabbs);
    }
}
