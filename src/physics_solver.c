#include <stddef.h>
#include "physics_internal.h"

typedef struct {
    int valid;
    Vec2 pos_a;
    Vec2 pos_b;
    float angle_a;
    float angle_b;
    CollisionInfo info;
} ContactSolveCache;

typedef struct {
    CollisionManifold* contacts;
    int count;
    ContactSolveCache* cache;
} ContactBatch;

typedef struct {
    Constraint* constraints;
    int count;
    float dt;
} ConstraintBatch;

static void solve_contact_velocity_batch(int begin, int end, void* user) {
    ContactBatch* b = (ContactBatch*)user;
    int i;
    if (b == NULL || b->contacts == NULL) return;
    if (begin < 0) begin = 0;
    if (end > b->count) end = b->count;
    for (i = begin; i < end; i++) {
        collision_resolve_velocity(&b->contacts[i]);
    }
}

static void solve_constraint_velocity_batch(int begin, int end, void* user) {
    ConstraintBatch* b = (ConstraintBatch*)user;
    int i;
    if (b == NULL || b->constraints == NULL) return;
    if (begin < 0) begin = 0;
    if (end > b->count) end = b->count;
    for (i = begin; i < end; i++) {
        constraint_solve_velocity(&b->constraints[i], b->dt);
    }
}

static void solve_contact_position_batch(int begin, int end, void* user) {
    ContactBatch* b = (ContactBatch*)user;
    int i;
    if (b == NULL || b->contacts == NULL) return;
    if (begin < 0) begin = 0;
    if (end > b->count) end = b->count;
    for (i = begin; i < end; i++) {
        CollisionManifold* m = &b->contacts[i];
        ContactSolveCache* c = (b->cache != NULL) ? &b->cache[i] : NULL;
        if (m->bodyA == NULL || m->bodyB == NULL) continue;
        if (c != NULL && c->valid &&
            c->pos_a.x == m->bodyA->position.x && c->pos_a.y == m->bodyA->position.y &&
            c->pos_b.x == m->bodyB->position.x && c->pos_b.y == m->bodyB->position.y &&
            c->angle_a == m->bodyA->angle && c->angle_b == m->bodyB->angle) {
            m->info = c->info;
        } else {
            CollisionInfo refreshed = {0};
            if (!collision_detect(m->bodyA, m->bodyB, &refreshed)) {
                m->info.penetration = 0.0f;
                if (c != NULL) c->valid = 0;
                continue;
            }
            m->info = refreshed;
            if (c != NULL) {
                c->valid = 1;
                c->pos_a = m->bodyA->position;
                c->pos_b = m->bodyB->position;
                c->angle_a = m->bodyA->angle;
                c->angle_b = m->bodyB->angle;
                c->info = m->info;
            }
        }
        if (m->info.penetration <= 0.0f) continue;
        collision_resolve_position(m);
    }
}

static void solve_constraint_position_batch(int begin, int end, void* user) {
    ConstraintBatch* b = (ConstraintBatch*)user;
    int i;
    if (b == NULL || b->constraints == NULL) return;
    if (begin < 0) begin = 0;
    if (end > b->count) end = b->count;
    for (i = begin; i < end; i++) {
        constraint_solve_position(&b->constraints[i]);
    }
}

void physics_internal_solve_world_view(PhysicsSolverWorldView* view, const PhysicsSolverContext* ctx) {
    int velocity_iters;
    int position_iters;
    float dt;
    int i;
    int iter;
    ContactBatch contact_batch;
    ConstraintBatch constraint_batch;
    ContactSolveCache* contact_cache = NULL;
    PhysicsEngine* engine_hint = NULL;
    if (view == NULL || view->contacts == NULL || view->constraints == NULL) {
        return;
    }

    velocity_iters = (ctx != NULL && ctx->velocity_iterations > 0) ? ctx->velocity_iterations : 1;
    position_iters = (ctx != NULL && ctx->position_iterations > 0) ? ctx->position_iterations : 1;
    dt = (ctx != NULL && ctx->dt > 0.0f) ? ctx->dt : (1.0f / 60.0f);
    contact_batch.contacts = view->contacts;
    contact_batch.count = view->contact_count;
    contact_batch.cache = NULL;
    constraint_batch.constraints = view->constraints;
    constraint_batch.count = view->constraint_count;
    constraint_batch.dt = dt;
    engine_hint = view->engine_hint;
    if (engine_hint != NULL && view->contact_count > 0) {
        contact_cache = (ContactSolveCache*)physics_internal_scratch_alloc(engine_hint,
            (int)(sizeof(ContactSolveCache) * (size_t)view->contact_count), 16);
        if (contact_cache != NULL) {
            int ci;
            for (ci = 0; ci < view->contact_count; ci++) {
                contact_cache[ci].valid = 0;
            }
            contact_batch.cache = contact_cache;
        }
    }

    for (i = 0; i < view->constraint_count; i++) {
        constraint_warm_start(&view->constraints[i]);
    }

    for (iter = 0; iter < velocity_iters; iter++) {
        physics_internal_parallel_for(engine_hint, view->contact_count, 64, solve_contact_velocity_batch, &contact_batch);
        physics_internal_parallel_for(engine_hint, view->constraint_count, 64, solve_constraint_velocity_batch, &constraint_batch);
    }

    for (iter = 0; iter < position_iters; iter++) {
        physics_internal_parallel_for(engine_hint, view->contact_count, 64, solve_contact_position_batch, &contact_batch);
        physics_internal_parallel_for(engine_hint, view->constraint_count, 64, solve_constraint_position_batch, &constraint_batch);
    }
}
