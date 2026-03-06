#include <math.h>
#include <stdio.h>
#include "../include/physics.hpp"
#include "regression_tests.hpp"

#define EVENT_SNAPSHOT_EPS 1e-3f

typedef struct {
    int step_begin_count;
    int step_end_count;
    int error_count;
} EventSinkCtx;

static void event_sink_collect(const PhysicsTraceEvent* event, void* user) {
    EventSinkCtx* ctx = (EventSinkCtx*)user;
    if (ctx == NULL || event == NULL) return;
    if (event->type == PHYSICS_EVENT_STEP_BEGIN) ctx->step_begin_count++;
    if (event->type == PHYSICS_EVENT_STEP_END) ctx->step_end_count++;
    if (event->type == PHYSICS_EVENT_ERROR) ctx->error_count++;
}

int regression_test_event_sink_trace(void) {
    PhysicsEngine* engine = physics_engine_create();
    EventSinkCtx ctx{};
    int pass = 1;
    if (engine == NULL) {
        printf("[FAIL] failed to create engine for event-sink test\n");
        return 0;
    }

    physics_engine_set_event_sink(engine, event_sink_collect, &ctx);
    physics_engine_step(engine);
    (void)physics_engine_add_broadphase_pair(engine, -1, 0);

    if (ctx.step_begin_count <= 0 || ctx.step_end_count <= 0) {
        printf("[FAIL] event sink did not receive step begin/end\n");
        pass = 0;
    }
    if (ctx.error_count <= 0) {
        printf("[FAIL] event sink did not receive error event\n");
        pass = 0;
    }

    physics_engine_free(engine);
    if (!pass) return 0;
    printf("[PASS] event sink trace\n");
    return 1;
}

int regression_test_snapshot_replay_determinism(void) {
    PhysicsEngine* a = physics_engine_create();
    PhysicsEngine* b = physics_engine_create();
    PhysicsWorldSnapshot snap;
    RigidBody* a0;
    RigidBody* b0;
    Vec2 posa;
    Vec2 posb;
    int i;
    int pass = 1;
    if (a == NULL || b == NULL) {
        if (a != NULL) physics_engine_free(a);
        if (b != NULL) physics_engine_free(b);
        printf("[FAIL] failed to create engines for snapshot replay test\n");
        return 0;
    }
    physics_engine_set_gravity(a, vec2(0.0f, 0.0f));
    physics_engine_set_gravity(b, vec2(0.0f, 0.0f));
    a0 = body_create(0.0f, 0.0f, 1.0f, shape_create_circle(1.0f));
    b0 = body_create(0.0f, 0.0f, 1.0f, shape_create_circle(1.0f));
    a0->velocity = vec2(2.0f, 0.5f);
    b0->velocity = vec2(2.0f, 0.5f);
    physics_engine_add_body(a, a0);
    physics_engine_add_body(b, b0);
    if (!physics_engine_capture_snapshot(a, &snap) || !physics_engine_apply_snapshot(b, &snap)) {
        physics_engine_free(a);
        physics_engine_free(b);
        printf("[FAIL] snapshot capture/apply failed\n");
        return 0;
    }
    for (i = 0; i < 90; i++) {
        physics_engine_step(a);
        physics_engine_step(b);
    }
    posa = a0->position;
    posb = b0->position;
    if (fabsf(posa.x - posb.x) > 1e-4f || fabsf(posa.y - posb.y) > 1e-4f) {
        printf("[FAIL] replay mismatch: A(%.5f,%.5f) B(%.5f,%.5f)\n", posa.x, posa.y, posb.x, posb.y);
        pass = 0;
    }
    physics_engine_free(a);
    physics_engine_free(b);
    if (!pass) return 0;
    printf("[PASS] snapshot replay determinism\n");
    return 1;
}

int regression_test_snapshot_applies_mass_inertia(void) {
    PhysicsEngine* a = physics_engine_create();
    PhysicsEngine* b = physics_engine_create();
    PhysicsWorldSnapshot snap;
    RigidBody* a0;
    RigidBody* b0;
    int pass = 1;
    if (a == NULL || b == NULL) {
        if (a != NULL) physics_engine_free(a);
        if (b != NULL) physics_engine_free(b);
        printf("[FAIL] failed to create engines for snapshot mass/inertia test\n");
        return 0;
    }

    a0 = body_create(0.0f, 0.0f, 3.0f, shape_create_circle(2.0f));
    b0 = body_create(0.0f, 0.0f, 1.0f, shape_create_circle(1.0f));
    physics_engine_add_body(a, a0);
    physics_engine_add_body(b, b0);
    if (!physics_engine_capture_snapshot(a, &snap)) {
        physics_engine_free(a);
        physics_engine_free(b);
        printf("[FAIL] failed to capture snapshot for mass/inertia test\n");
        return 0;
    }

    b0->mass = 99.0f;
    b0->inertia = 77.0f;
    b0->inv_mass = 1.0f / b0->mass;
    b0->inv_inertia = 1.0f / b0->inertia;
    if (!physics_engine_apply_snapshot(b, &snap)) {
        physics_engine_free(a);
        physics_engine_free(b);
        printf("[FAIL] failed to apply snapshot for mass/inertia test\n");
        return 0;
    }

    if (fabsf(b0->mass - a0->mass) > EVENT_SNAPSHOT_EPS ||
        fabsf(b0->inertia - a0->inertia) > EVENT_SNAPSHOT_EPS) {
        printf("[FAIL] snapshot did not apply mass/inertia values\n");
        pass = 0;
    } else if (fabsf(b0->inv_mass - a0->inv_mass) > EVENT_SNAPSHOT_EPS ||
               fabsf(b0->inv_inertia - a0->inv_inertia) > EVENT_SNAPSHOT_EPS) {
        printf("[FAIL] snapshot did not recompute inverse mass/inertia consistently\n");
        pass = 0;
    }

    physics_engine_free(a);
    physics_engine_free(b);
    if (!pass) return 0;
    printf("[PASS] snapshot mass/inertia application\n");
    return 1;
}
