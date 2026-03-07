#include <stdio.h>
#include <string.h>
#include "../../include/physics_core/physics.hpp"
#include "../regression_tests.hpp"

typedef struct {
    int step_begin_count;
    int step_end_count;
    int error_count;
} EventSinkCtx;

typedef struct {
    PhysicsEngine* engine;
    int contact_count;
    int pair_a[32];
    int pair_b[32];
} ContactEventOrderCtx;

static void event_sink_collect(const PhysicsTraceEvent* event, void* user) {
    EventSinkCtx* ctx = (EventSinkCtx*)user;
    if (ctx == NULL || event == NULL) return;
    if (event->type == PHYSICS_EVENT_STEP_BEGIN) ctx->step_begin_count++;
    if (event->type == PHYSICS_EVENT_STEP_END) ctx->step_end_count++;
    if (event->type == PHYSICS_EVENT_ERROR) ctx->error_count++;
}

static int test_body_index_of(const PhysicsEngine* engine, const RigidBody* body) {
    int i;
    if (engine == NULL || body == NULL) return -1;
    for (i = 0; i < physics_engine_get_body_count(engine); i++) {
        if (physics_engine_get_body(engine, i) == body) return i;
    }
    return -1;
}

static int unsorted_broadphase_builder(PhysicsEngine* engine, void* user) {
    (void)user;
    physics_engine_clear_broadphase_pairs(engine);
    (void)physics_engine_add_broadphase_pair(engine, 2, 3);
    (void)physics_engine_add_broadphase_pair(engine, 0, 1);
    return 2;
}

static void event_sink_collect_contact_order(const PhysicsTraceEvent* event, void* user) {
    ContactEventOrderCtx* ctx = (ContactEventOrderCtx*)user;
    const CollisionManifold* c;
    int ia;
    int ib;
    if (ctx == NULL || event == NULL) return;
    if (event->type != PHYSICS_EVENT_CONTACT_CREATED) return;
    if (ctx->contact_count >= (int)(sizeof(ctx->pair_a) / sizeof(ctx->pair_a[0]))) return;
    c = event->payload.contact.contact;
    if (c == NULL) return;
    ia = test_body_index_of(ctx->engine, c->bodyA);
    ib = test_body_index_of(ctx->engine, c->bodyB);
    if (ia > ib) {
        int t = ia;
        ia = ib;
        ib = t;
    }
    ctx->pair_a[ctx->contact_count] = ia;
    ctx->pair_b[ctx->contact_count] = ib;
    ctx->contact_count++;
}

int regression_test_broadphase_grid_filters_pairs(void) {
    PhysicsEngine* engine = physics_engine_create();
    if (engine == NULL) {
        printf("[FAIL] failed to create engine for broadphase test\n");
        return 0;
    }

    physics_engine_set_gravity(engine, vec2(0.0f, 0.0f));
    physics_engine_set_broadphase_use_grid(engine, 1);
    physics_engine_set_broadphase_cell_size(engine, 8.0f);

    for (int i = 0; i < 60; i++) {
        RigidBody* b = body_create(100.0f + i * 20.0f, 0.0f, 1.0f, shape_create_circle(1.0f));
        body_set_type(b, BODY_STATIC);
        physics_engine_add_body(engine, b);
    }

    RigidBody* a = body_create(0.0f, 0.0f, 1.0f, shape_create_circle(2.0f));
    RigidBody* b = body_create(3.0f, 0.0f, 1.0f, shape_create_circle(2.0f));
    physics_engine_add_body(engine, a);
    physics_engine_add_body(engine, b);

    physics_engine_detect_collisions(engine);

    int pass = 1;
    if (physics_engine_get_contact_count(engine) <= 0) {
        printf("[FAIL] broadphase grid missed obvious overlap\n");
        pass = 0;
    } else if (physics_engine_get_broadphase_pair_count(engine) >= 200) {
        printf("[FAIL] broadphase grid candidate pairs too high: %d\n", physics_engine_get_broadphase_pair_count(engine));
        pass = 0;
    }

    physics_engine_free(engine);
    if (!pass) {
        return 0;
    }

    printf("[PASS] broadphase grid filtering\n");
    return 1;
}

int regression_test_sleep_wakeup_system(void) {
    PhysicsEngine* engine = physics_engine_create();
    PhysicsExperimentalConfig ecfg;
    RigidBody* b;
    int pass = 1;
    if (engine == NULL) {
        printf("[FAIL] failed to create engine for sleep test\n");
        return 0;
    }
    physics_engine_set_gravity(engine, vec2(0.0f, 0.0f));
    physics_engine_set_time_step(engine, 1.0f / 60.0f);
    ecfg.ccd_enabled = 0;
    ecfg.sleep_enabled = 1;
    ecfg.threading_enabled = 0;
    ecfg.worker_count = 1;
    physics_engine_set_experimental_config(engine, &ecfg);
    b = body_create(0.0f, 0.0f, 1.0f, shape_create_circle(1.0f));
    b->velocity = vec2(0.02f, 0.0f);
    physics_engine_add_body(engine, b);
    {
        int i;
        for (i = 0; i < 120; i++) physics_engine_step(engine);
    }
    if (!b->sleeping) {
        printf("[FAIL] body should enter sleeping state\n");
        pass = 0;
    }
    body_apply_impulse(b, vec2(2.0f, 0.0f));
    physics_engine_step(engine);
    if (b->sleeping) {
        printf("[FAIL] body should wake up after impulse\n");
        pass = 0;
    }
    physics_engine_free(engine);
    if (!pass) return 0;
    printf("[PASS] sleep/wakeup system\n");
    return 1;
}

int regression_test_rope_constraint_limit(void) {
    PhysicsEngine* engine = physics_engine_create();
    RigidBody* a;
    RigidBody* b;
    float dist;
    int pass = 1;
    if (engine == NULL) {
        printf("[FAIL] failed to create engine for rope test\n");
        return 0;
    }
    physics_engine_set_gravity(engine, vec2(0.0f, 0.0f));
    physics_engine_set_time_step(engine, 1.0f / 60.0f);
    a = body_create(0.0f, 0.0f, 1.0f, shape_create_circle(0.5f));
    b = body_create(8.0f, 0.0f, 1.0f, shape_create_circle(0.5f));
    physics_engine_add_body(engine, a);
    physics_engine_add_body(engine, b);
    if (physics_engine_add_rope_constraint(engine, a, b, a->position, b->position, 4.0f, 1.0f, 1) == NULL) {
        physics_engine_free(engine);
        printf("[FAIL] failed to add rope constraint\n");
        return 0;
    }
    {
        int i;
        for (i = 0; i < 120; i++) physics_engine_step(engine);
    }
    dist = vec2_length(vec2_sub(a->position, b->position));
    if (dist > 4.5f) {
        printf("[FAIL] rope constraint exceeded max length: %.3f\n", dist);
        pass = 0;
    }
    physics_engine_free(engine);
    if (!pass) return 0;
    printf("[PASS] rope constraint limit\n");
    return 1;
}

int regression_test_broadphase_sap_mode(void) {
    PhysicsEngine* engine = physics_engine_create();
    PhysicsPipelineConfig pcfg;
    int pair_count;
    int i;
    int pass = 1;
    if (engine == NULL) {
        printf("[FAIL] failed to create engine for SAP test\n");
        return 0;
    }
    physics_engine_get_pipeline_config(engine, &pcfg);
    pcfg.broadphase_type = PHYSICS_BROADPHASE_SAP;
    physics_engine_set_pipeline_config(engine, &pcfg);
    for (i = 0; i < 40; i++) {
        RigidBody* b = body_create((float)i * 1.8f, 0.0f, 1.0f, shape_create_circle(1.0f));
        physics_engine_add_body(engine, b);
    }
    physics_engine_detect_collisions(engine);
    pair_count = physics_engine_get_broadphase_pair_count(engine);
    if (pair_count <= 0) {
        printf("[FAIL] SAP broadphase should produce candidate pairs\n");
        pass = 0;
    }
    physics_engine_free(engine);
    if (!pass) return 0;
    printf("[PASS] broadphase SAP mode\n");
    return 1;
}

int regression_test_contact_event_order_stable_after_sort(void) {
    PhysicsEngine* engine = physics_engine_create();
    ContactEventOrderCtx ctx;
    PhysicsPipelinePluginV1 plugin;
    int i;
    int pass = 1;
    if (engine == NULL) {
        printf("[FAIL] failed to create engine for contact-event order test\n");
        return 0;
    }

    memset(&ctx, 0, sizeof(ctx));
    ctx.engine = engine;
    physics_engine_set_event_sink(engine, event_sink_collect_contact_order, &ctx);
    physics_engine_set_gravity(engine, vec2(0.0f, 0.0f));
    memset(&plugin, 0, sizeof(plugin));
    plugin.api_version = PHYSICS_PIPELINE_PLUGIN_API_VERSION;
    plugin.capabilities = PHYSICS_PIPELINE_PLUGIN_CAP_BROADPHASE;
    plugin.broadphase_builder = unsorted_broadphase_builder;
    plugin.user = NULL;
    if (!physics_engine_install_pipeline_plugin(engine, &plugin)) {
        physics_engine_free(engine);
        printf("[FAIL] failed to install plugin for contact-event order test\n");
        return 0;
    }

    physics_engine_add_body(engine, body_create(0.0f, 0.0f, 1.0f, shape_create_circle(2.0f)));
    physics_engine_add_body(engine, body_create(3.0f, 0.0f, 1.0f, shape_create_circle(2.0f)));
    physics_engine_add_body(engine, body_create(20.0f, 0.0f, 1.0f, shape_create_circle(2.0f)));
    physics_engine_add_body(engine, body_create(23.0f, 0.0f, 1.0f, shape_create_circle(2.0f)));

    physics_engine_detect_collisions(engine);
    if (ctx.contact_count != physics_engine_get_contact_count(engine)) {
        printf("[FAIL] contact-created event count mismatch (%d vs %d)\n",
               ctx.contact_count, physics_engine_get_contact_count(engine));
        pass = 0;
    } else {
        for (i = 0; i < ctx.contact_count; i++) {
            const CollisionManifold* c = physics_engine_get_contact(engine, i);
            int ia = test_body_index_of(engine, c->bodyA);
            int ib = test_body_index_of(engine, c->bodyB);
            if (ia > ib) {
                int t = ia;
                ia = ib;
                ib = t;
            }
            if (ctx.pair_a[i] != ia || ctx.pair_b[i] != ib) {
                printf("[FAIL] contact-created event order mismatch at %d: event(%d,%d) final(%d,%d)\n",
                       i, ctx.pair_a[i], ctx.pair_b[i], ia, ib);
                pass = 0;
                break;
            }
        }
    }

    physics_engine_free(engine);
    if (!pass) return 0;
    printf("[PASS] contact event order stability\n");
    return 1;
}

int regression_test_broadphase_bvh_reports_unsupported(void) {
    PhysicsEngine* engine = physics_engine_create();
    PhysicsPipelineConfig pcfg;
    EventSinkCtx evt{};
    int pass = 1;
    if (engine == NULL) {
        printf("[FAIL] failed to create engine for BVH unsupported test\n");
        return 0;
    }

    physics_engine_set_event_sink(engine, event_sink_collect, &evt);
    physics_engine_get_pipeline_config(engine, &pcfg);
    pcfg.broadphase_type = PHYSICS_BROADPHASE_BVH;
    physics_engine_set_pipeline_config(engine, &pcfg);
    physics_engine_get_pipeline_config(engine, &pcfg);
    if (physics_engine_get_last_error(engine) != PHYSICS_ERROR_INVALID_ARGUMENT) {
        printf("[FAIL] BVH config should report invalid argument error\n");
        pass = 0;
    }
    if (pcfg.broadphase_type != PHYSICS_BROADPHASE_BRUTE_FORCE) {
        printf("[FAIL] BVH config should downgrade to brute-force\n");
        pass = 0;
    }
    if (evt.error_count <= 0) {
        printf("[FAIL] BVH unsupported config should emit error event\n");
        pass = 0;
    }

    physics_engine_free(engine);
    if (!pass) return 0;
    printf("[PASS] broadphase BVH unsupported signaling\n");
    return 1;
}
