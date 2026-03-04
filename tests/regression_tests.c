#include <math.h>
#include <stdio.h>
#include <string.h>
#include "../include/physics.h"

#define EPS 1e-3f
#define HS_STEPS 240
#define HS_TUNNEL_X 62.0f
#define HS_TUNNEL_Y_MIN 14.0f
#define HS_TUNNEL_Y_MAX 42.0f

#define STACK_COLUMNS 3
#define STACK_ROWS 12
#define STACK_STEPS 720
#define STACK_MAX_SPEED 260.0f
#define STACK_MIN_X -10.0f
#define STACK_MAX_X 110.0f
#define STACK_MIN_Y -20.0f
#define STACK_MAX_Y 70.0f

typedef struct {
    int broadphase_called;
    int narrowphase_called;
} PipelineHookCtx;

typedef struct {
    PipelineHookCtx hook;
    int init_called;
    int shutdown_called;
} PluginHookCtx;

typedef struct {
    int dispatch_count;
} JobSystemHookCtx;

typedef struct {
    int step_begin_count;
    int step_end_count;
    int error_count;
} EventSinkCtx;

static int custom_broadphase_builder(PhysicsEngine* engine, void* user) {
    PipelineHookCtx* ctx = (PipelineHookCtx*)user;
    if (ctx != NULL) ctx->broadphase_called++;
    physics_engine_clear_broadphase_pairs(engine);
    return physics_engine_add_broadphase_pair(engine, 0, 1);
}

static int custom_narrowphase_builder(PhysicsEngine* engine, void* user) {
    PipelineHookCtx* ctx = (PipelineHookCtx*)user;
    RigidBody* a;
    RigidBody* b;
    CollisionInfo info = {0};
    if (ctx != NULL) ctx->narrowphase_called++;
    physics_engine_clear_contacts(engine);
    a = physics_engine_get_body(engine, 0);
    b = physics_engine_get_body(engine, 1);
    if (a == NULL || b == NULL) return 0;
    if (!collision_detect(a, b, &info)) return 0;
    return physics_engine_add_contact(engine, a, b, &info);
}

static int plugin_init_ok(PhysicsEngine* engine, void* user) {
    PluginHookCtx* ctx = (PluginHookCtx*)user;
    (void)engine;
    if (ctx != NULL) ctx->init_called++;
    return 1;
}

static void plugin_shutdown_mark(PhysicsEngine* engine, void* user) {
    PluginHookCtx* ctx = (PluginHookCtx*)user;
    (void)engine;
    if (ctx != NULL) ctx->shutdown_called++;
}

static void job_system_parallel_for(int count, int grain, PhysicsJobRangeFn fn, void* fn_user, void* user) {
    JobSystemHookCtx* ctx = (JobSystemHookCtx*)user;
    (void)grain;
    if (ctx != NULL) ctx->dispatch_count++;
    if (fn != NULL && count > 0) {
        fn(0, count, fn_user);
    }
}

static void event_sink_collect(const PhysicsTraceEvent* event, void* user) {
    EventSinkCtx* ctx = (EventSinkCtx*)user;
    if (ctx == NULL || event == NULL) return;
    if (event->type == PHYSICS_EVENT_STEP_BEGIN) ctx->step_begin_count++;
    if (event->type == PHYSICS_EVENT_STEP_END) ctx->step_end_count++;
    if (event->type == PHYSICS_EVENT_ERROR) ctx->error_count++;
}
static int test_circle_circle_detection(void) {
    Shape* s1 = shape_create_circle(2.0f);
    Shape* s2 = shape_create_circle(2.0f);
    RigidBody* a = body_create(0.0f, 0.0f, 1.0f, s1);
    RigidBody* b = body_create(3.0f, 0.0f, 1.0f, s2);

    CollisionInfo info = {0};
    int hit = collision_detect(a, b, &info);

    body_free(a);
    body_free(b);

    if (!hit) {
        printf("[FAIL] circle-circle should collide\n");
        return 0;
    }

    if (fabsf(info.penetration - 1.0f) > EPS) {
        printf("[FAIL] circle-circle penetration expected 1.0 got %.4f\n", info.penetration);
        return 0;
    }

    printf("[PASS] circle-circle detection\n");
    return 1;
}

static int test_circle_inside_polygon(void) {
    Shape* cs = shape_create_circle(1.0f);
    Shape* ps = shape_create_box(10.0f, 10.0f);
    RigidBody* circle = body_create(0.0f, 0.0f, 1.0f, cs);
    RigidBody* box = body_create(0.0f, 0.0f, 1.0f, ps);
    body_set_type(box, BODY_STATIC);

    CollisionInfo info = {0};
    int hit = collision_detect(circle, box, &info);

    body_free(circle);
    body_free(box);

    if (!hit) {
        printf("[FAIL] circle-inside-polygon should collide\n");
        return 0;
    }

    if (info.penetration <= 0.0f) {
        printf("[FAIL] circle-inside-polygon penetration should be positive\n");
        return 0;
    }

    printf("[PASS] circle-inside-polygon detection\n");
    return 1;
}

static int test_collision_detect_null_safety(void) {
    CollisionInfo info = {0};
    Shape* shape = shape_create_circle(1.0f);
    RigidBody* body = body_create(0.0f, 0.0f, 1.0f, shape);
    int pass = 1;

    if (collision_detect(NULL, body, &info) != 0) {
        printf("[FAIL] collision_detect should reject NULL body A\n");
        pass = 0;
    }
    if (collision_detect(body, NULL, &info) != 0) {
        printf("[FAIL] collision_detect should reject NULL body B\n");
        pass = 0;
    }
    if (collision_detect(body, body, NULL) != 0) {
        printf("[FAIL] collision_detect should reject NULL info\n");
        pass = 0;
    }

    body_free(body);
    if (!pass) {
        return 0;
    }

    printf("[PASS] collision_detect null safety\n");
    return 1;
}

static int test_shape_helpers_null_safety(void) {
    Vec2 center = shape_get_center_of_mass(NULL);
    if (fabsf(shape_get_area(NULL)) > EPS ||
        fabsf(shape_get_moment_of_inertia(NULL, 1.0f)) > EPS ||
        fabsf(center.x) > EPS || fabsf(center.y) > EPS) {
        printf("[FAIL] shape helpers should be NULL-safe\n");
        return 0;
    }

    printf("[PASS] shape helper null safety\n");
    return 1;
}

static int test_gravity_override(void) {
    PhysicsEngine* engine = physics_engine_create();
    physics_engine_set_gravity(engine, vec2(0.0f, 20.0f));
    physics_engine_set_time_step(engine, 0.1f);
    physics_engine_set_damping(engine, 1.0f);

    RigidBody* g_default = body_create(0.0f, 0.0f, 1.0f, shape_create_circle(1.0f));
    RigidBody* g_custom = body_create(0.0f, 0.0f, 1.0f, shape_create_circle(1.0f));
    body_set_gravity(g_custom, vec2(0.0f, 0.0f));

    physics_engine_add_body(engine, g_default);
    physics_engine_add_body(engine, g_custom);

    physics_engine_step(engine);

    float vy_default = g_default->velocity.y;
    float vy_custom = g_custom->velocity.y;

    physics_engine_free(engine);

    if (vy_default <= vy_custom + EPS) {
        printf("[FAIL] custom gravity override not applied: default=%.4f custom=%.4f\n", vy_default, vy_custom);
        return 0;
    }

    printf("[PASS] gravity global + custom override\n");
    return 1;
}

static int test_raycast_geometry(void) {
    PhysicsEngine* engine = physics_engine_create();
    physics_engine_set_gravity(engine, vec2(0.0f, 0.0f));

    RigidBody* off_ray = body_create(5.0f, 5.0f, 1.0f, shape_create_circle(1.0f));
    RigidBody* on_ray = body_create(7.0f, 0.0f, 1.0f, shape_create_circle(1.0f));

    body_set_type(off_ray, BODY_STATIC);
    body_set_type(on_ray, BODY_STATIC);

    physics_engine_add_body(engine, off_ray);
    physics_engine_add_body(engine, on_ray);

    RigidBody* hit = physics_engine_raycast(engine, vec2(0.0f, 0.0f), vec2(10.0f, 0.0f));
    int pass = (hit == on_ray);

    physics_engine_free(engine);

    if (!pass) {
        printf("[FAIL] raycast should hit on-ray shape, not off-ray shape\n");
        return 0;
    }

    printf("[PASS] raycast geometric hit\n");
    return 1;
}

static int test_polygon_polygon_sat_axes(void) {
    Shape* a_shape = shape_create_box(4.0f, 20.0f);
    Shape* b_shape = shape_create_box(20.0f, 4.0f);
    RigidBody* a = body_create(0.0f, 0.0f, 1.0f, a_shape);
    RigidBody* b = body_create(8.0f, 8.0f, 1.0f, b_shape);
    a->angle = 0.78539816339f;
    b->angle = -0.78539816339f;

    CollisionInfo info = {0};
    int hit = collision_detect(a, b, &info);

    body_free(a);
    body_free(b);

    if (hit) {
        printf("[FAIL] polygon SAT should separate these rotated boxes\n");
        return 0;
    }

    printf("[PASS] polygon SAT dual-axis separation\n");
    return 1;
}

static int test_high_speed_tunnel_regression(void) {
    PhysicsEngine* engine = physics_engine_create();
    Shape* thin_shape = shape_create_box(1.2f, 24.0f);
    RigidBody* thin_wall = body_create(58.0f, 28.0f, 1000.0f, thin_shape);
    Shape* fast_shape_a = shape_create_circle(1.0f);
    Shape* fast_shape_b = shape_create_circle(1.0f);
    RigidBody* ball_a = body_create(10.0f, 30.0f, 0.6f, fast_shape_a);
    RigidBody* ball_b = body_create(10.0f, 34.0f, 0.6f, fast_shape_b);
    int i;
    int peak_contacts = 0;
    int tunnel_signals = 0;

    if (engine == NULL) {
        printf("[FAIL] failed to create engine for high-speed regression\n");
        return 0;
    }

    physics_engine_set_gravity(engine, vec2(0.0f, 9.8f));
    physics_engine_set_time_step(engine, 1.0f / 120.0f);
    physics_engine_set_damping(engine, 0.997f);
    physics_engine_set_iterations(engine, 16);

    body_set_type(thin_wall, BODY_STATIC);
    fast_shape_a->friction = 0.05f;
    fast_shape_b->friction = 0.05f;
    fast_shape_a->restitution = 0.75f;
    fast_shape_b->restitution = 0.75f;
    ball_a->velocity = vec2(105.0f, -4.0f);
    ball_b->velocity = vec2(127.0f, -2.0f);

    physics_engine_add_body(engine, thin_wall);
    physics_engine_add_body(engine, ball_a);
    physics_engine_add_body(engine, ball_b);

    for (i = 0; i < HS_STEPS; i++) {
        physics_engine_step(engine);
        if (physics_engine_get_contact_count(engine) > peak_contacts) {
            peak_contacts = physics_engine_get_contact_count(engine);
        }
        if (ball_a->position.x > HS_TUNNEL_X &&
            ball_a->position.y > HS_TUNNEL_Y_MIN &&
            ball_a->position.y < HS_TUNNEL_Y_MAX) {
            tunnel_signals++;
        }
        if (ball_b->position.x > HS_TUNNEL_X &&
            ball_b->position.y > HS_TUNNEL_Y_MIN &&
            ball_b->position.y < HS_TUNNEL_Y_MAX) {
            tunnel_signals++;
        }
    }

    physics_engine_free(engine);

    if (peak_contacts <= 0) {
        printf("[FAIL] high-speed regression expected wall contact but got none\n");
        return 0;
    }
    if (tunnel_signals > 0) {
        printf("[FAIL] high-speed regression detected tunnel-through (%d samples)\n", tunnel_signals);
        return 0;
    }

    printf("[PASS] high-speed tunnel regression\n");
    return 1;
}

static int test_high_stack_stability_regression(void) {
    PhysicsEngine* engine = physics_engine_create();
    int c;
    int r;
    int i;
    int dynamic_count = 0;

    if (engine == NULL) {
        printf("[FAIL] failed to create engine for high-stack regression\n");
        return 0;
    }

    physics_engine_set_gravity(engine, vec2(0.0f, 9.8f));
    physics_engine_set_time_step(engine, 1.0f / 60.0f);
    physics_engine_set_damping(engine, 0.998f);
    physics_engine_set_iterations(engine, 22);

    {
        Shape* ground_shape = shape_create_box(120.0f, 2.0f);
        RigidBody* ground = body_create(50.0f, 45.0f, 1000.0f, ground_shape);
        body_set_type(ground, BODY_STATIC);
        physics_engine_add_body(engine, ground);
    }

    for (c = 0; c < STACK_COLUMNS; c++) {
        for (r = 0; r < STACK_ROWS; r++) {
            Shape* box_shape = shape_create_box(4.6f, 4.6f);
            RigidBody* box = body_create(38.0f + c * 12.0f, 43.0f - r * 4.8f, 4.0f, box_shape);
            box_shape->friction = 0.38f;
            box_shape->restitution = 0.05f;
            physics_engine_add_body(engine, box);
            dynamic_count++;
        }
    }

    for (i = 0; i < STACK_STEPS; i++) {
        int bi;
        physics_engine_step(engine);

        for (bi = 0; bi < physics_engine_get_body_count(engine); bi++) {
            RigidBody* b = physics_engine_get_body(engine, bi);
            float speed_sq;
            if (b == NULL || b->type != BODY_DYNAMIC) continue;

            if (!isfinite(b->position.x) || !isfinite(b->position.y) ||
                !isfinite(b->velocity.x) || !isfinite(b->velocity.y)) {
                physics_engine_free(engine);
                printf("[FAIL] high-stack regression produced NaN/Inf at step %d\n", i);
                return 0;
            }

            speed_sq = vec2_length_sq(b->velocity);
            if (speed_sq > STACK_MAX_SPEED * STACK_MAX_SPEED) {
                physics_engine_free(engine);
                printf("[FAIL] high-stack regression unstable speed %.3f at step %d\n", sqrtf(speed_sq), i);
                return 0;
            }

            if (b->position.x < STACK_MIN_X || b->position.x > STACK_MAX_X ||
                b->position.y < STACK_MIN_Y || b->position.y > STACK_MAX_Y) {
                physics_engine_free(engine);
                printf("[FAIL] high-stack regression body escaped bounds at step %d\n", i);
                return 0;
            }
        }
    }

    {
        int alive_dynamic = 0;
        for (i = 0; i < physics_engine_get_body_count(engine); i++) {
            RigidBody* b = physics_engine_get_body(engine, i);
            if (b != NULL && b->type == BODY_DYNAMIC) alive_dynamic++;
        }
        if (alive_dynamic != dynamic_count) {
            physics_engine_free(engine);
            printf("[FAIL] high-stack regression body-count drift: expected %d got %d\n", dynamic_count, alive_dynamic);
            return 0;
        }
    }

    physics_engine_free(engine);
    printf("[PASS] high-stack stability regression\n");
    return 1;
}

static int test_broadphase_grid_filters_pairs(void) {
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

static int test_add_body_duplicate_guard(void) {
    PhysicsEngine* engine = physics_engine_create();
    RigidBody* body;
    if (engine == NULL) {
        printf("[FAIL] failed to create engine for duplicate-body test\n");
        return 0;
    }

    body = body_create(0.0f, 0.0f, 1.0f, shape_create_circle(1.0f));
    if (body == NULL) {
        physics_engine_free(engine);
        printf("[FAIL] failed to create body for duplicate-body test\n");
        return 0;
    }

    physics_engine_add_body(engine, body);
    physics_engine_add_body(engine, body);

    if (physics_engine_get_body_count(engine) != 1) {
        physics_engine_free(engine);
        printf("[FAIL] duplicate body insertion should be ignored\n");
        return 0;
    }

    physics_engine_free(engine);
    printf("[PASS] duplicate body insertion guard\n");
    return 1;
}

static int test_distance_constraint_stability(void) {
    PhysicsEngine* engine = physics_engine_create();
    if (engine == NULL) {
        printf("[FAIL] failed to create engine for constraint test\n");
        return 0;
    }

    physics_engine_set_gravity(engine, vec2(0.0f, 0.0f));
    physics_engine_set_time_step(engine, 1.0f / 120.0f);
    physics_engine_set_damping(engine, 1.0f);
    physics_engine_set_iterations(engine, 14);

    RigidBody* a = body_create(-5.0f, 0.0f, 1.0f, shape_create_circle(0.8f));
    RigidBody* b = body_create(5.0f, 0.0f, 1.0f, shape_create_circle(0.8f));
    physics_engine_add_body(engine, a);
    physics_engine_add_body(engine, b);

    if (physics_engine_add_distance_constraint(engine, a, b, a->position, b->position, 0.95f, 0) == NULL) {
        physics_engine_free(engine);
        printf("[FAIL] failed to add distance constraint\n");
        return 0;
    }

    a->velocity = vec2(24.0f, 0.0f);
    b->velocity = vec2(-4.0f, 0.0f);

    for (int i = 0; i < 240; i++) {
        physics_engine_step(engine);
    }

    float final_dist = vec2_length(vec2_sub(a->position, b->position));
    physics_engine_free(engine);

    if (fabsf(final_dist - 10.0f) > 0.55f) {
        printf("[FAIL] distance constraint drifted: got %.4f expected near 10.0\n", final_dist);
        return 0;
    }

    printf("[PASS] distance constraint stability\n");
    return 1;
}

static int test_constraint_break_force(void) {
    PhysicsEngine* engine = physics_engine_create();
    if (engine == NULL) {
        printf("[FAIL] failed to create engine for break-force test\n");
        return 0;
    }

    physics_engine_set_gravity(engine, vec2(0.0f, 0.0f));
    physics_engine_set_time_step(engine, 1.0f / 120.0f);
    physics_engine_set_damping(engine, 1.0f);
    physics_engine_set_iterations(engine, 12);

    RigidBody* a = body_create(-4.0f, 0.0f, 1.0f, shape_create_circle(0.8f));
    RigidBody* b = body_create(4.0f, 0.0f, 1.0f, shape_create_circle(0.8f));
    physics_engine_add_body(engine, a);
    physics_engine_add_body(engine, b);

    Constraint* c = physics_engine_add_distance_constraint(engine, a, b, a->position, b->position, 1.0f, 0);
    if (c == NULL) {
        physics_engine_free(engine);
        printf("[FAIL] failed to add constraint for break-force test\n");
        return 0;
    }
    c->break_force = 40.0f;
    a->velocity = vec2(120.0f, 0.0f);
    b->velocity = vec2(-120.0f, 0.0f);

    for (int i = 0; i < 60; i++) {
        physics_engine_step(engine);
        if (!c->active) break;
    }

    int pass = !c->active;
    physics_engine_free(engine);
    if (!pass) {
        printf("[FAIL] break-force constraint should deactivate under high load\n");
        return 0;
    }

    printf("[PASS] constraint break-force deactivation\n");
    return 1;
}

static int test_custom_pipeline_builders(void) {
    PhysicsEngine* engine = physics_engine_create();
    PipelineHookCtx ctx = {0};
    PhysicsPipelinePluginV1 plugin;
    RigidBody* a;
    RigidBody* b;
    int pass = 1;
    if (engine == NULL) {
        printf("[FAIL] failed to create engine for custom pipeline test\n");
        return 0;
    }

    physics_engine_set_gravity(engine, vec2(0.0f, 0.0f));
    physics_engine_set_damping(engine, 1.0f);
    physics_engine_set_time_step(engine, 1.0f / 60.0f);

    a = body_create(0.0f, 0.0f, 1.0f, shape_create_circle(2.0f));
    b = body_create(3.0f, 0.0f, 1.0f, shape_create_circle(2.0f));
    body_set_type(a, BODY_STATIC);
    body_set_type(b, BODY_STATIC);
    physics_engine_add_body(engine, a);
    physics_engine_add_body(engine, b);

    memset(&plugin, 0, sizeof(plugin));
    plugin.api_version = PHYSICS_PIPELINE_PLUGIN_API_VERSION;
    plugin.capabilities = PHYSICS_PIPELINE_PLUGIN_CAP_BROADPHASE | PHYSICS_PIPELINE_PLUGIN_CAP_NARROWPHASE;
    plugin.broadphase_builder = custom_broadphase_builder;
    plugin.narrowphase_builder = custom_narrowphase_builder;
    plugin.user = &ctx;
    if (physics_engine_install_pipeline_plugin(engine, &plugin) == 0) {
        physics_engine_free(engine);
        printf("[FAIL] failed to install plugin for custom pipeline test\n");
        return 0;
    }
    physics_engine_detect_collisions(engine);

    if (ctx.broadphase_called <= 0 || ctx.narrowphase_called <= 0) {
        printf("[FAIL] custom pipeline callbacks were not called\n");
        pass = 0;
    } else if (physics_engine_get_contact_count(engine) <= 0) {
        printf("[FAIL] custom pipeline did not produce contact\n");
        pass = 0;
    }

    physics_engine_uninstall_pipeline_plugin(engine);
    physics_engine_free(engine);
    if (!pass) return 0;
    printf("[PASS] custom pipeline builders\n");
    return 1;
}

static int test_pipeline_plugin_abi(void) {
    PhysicsEngine* engine = physics_engine_create();
    PluginHookCtx ctx = {0};
    PhysicsPipelinePluginV1 plugin;
    PhysicsPipelinePluginV1 bad_plugin;
    RigidBody* a;
    RigidBody* b;
    int pass = 1;
    if (engine == NULL) {
        printf("[FAIL] failed to create engine for plugin ABI test\n");
        return 0;
    }

    a = body_create(0.0f, 0.0f, 1.0f, shape_create_circle(2.0f));
    b = body_create(3.0f, 0.0f, 1.0f, shape_create_circle(2.0f));
    body_set_type(a, BODY_STATIC);
    body_set_type(b, BODY_STATIC);
    physics_engine_add_body(engine, a);
    physics_engine_add_body(engine, b);

    memset(&bad_plugin, 0, sizeof(bad_plugin));
    bad_plugin.api_version = 999u;
    if (physics_engine_install_pipeline_plugin(engine, &bad_plugin) != 0) {
        printf("[FAIL] plugin ABI should reject wrong api version\n");
        pass = 0;
    } else if (physics_engine_get_last_error(engine) != PHYSICS_ERROR_API_VERSION_MISMATCH) {
        printf("[FAIL] expected API version mismatch error, got %s\n",
               physics_error_code_string(physics_engine_get_last_error(engine)));
        pass = 0;
    }

    memset(&plugin, 0, sizeof(plugin));
    plugin.api_version = PHYSICS_PIPELINE_PLUGIN_API_VERSION;
    plugin.capabilities = PHYSICS_PIPELINE_PLUGIN_CAP_BROADPHASE | PHYSICS_PIPELINE_PLUGIN_CAP_NARROWPHASE;
    plugin.init = plugin_init_ok;
    plugin.shutdown = plugin_shutdown_mark;
    plugin.broadphase_builder = custom_broadphase_builder;
    plugin.narrowphase_builder = custom_narrowphase_builder;
    plugin.user = &ctx;

    if (physics_engine_install_pipeline_plugin(engine, &plugin) == 0) {
        printf("[FAIL] plugin ABI install failed for valid plugin\n");
        pass = 0;
    } else {
        physics_engine_detect_collisions(engine);
        if (ctx.init_called <= 0) {
            printf("[FAIL] plugin init hook was not called\n");
            pass = 0;
        }
        if (ctx.hook.broadphase_called <= 0 || ctx.hook.narrowphase_called <= 0) {
            printf("[FAIL] plugin builders were not invoked\n");
            pass = 0;
        }
        physics_engine_uninstall_pipeline_plugin(engine);
        if (ctx.shutdown_called <= 0) {
            printf("[FAIL] plugin shutdown hook was not called\n");
            pass = 0;
        }
    }

    physics_engine_free(engine);
    if (!pass) return 0;
    printf("[PASS] pipeline plugin ABI\n");
    return 1;
}

static int test_error_channel_basics(void) {
    PhysicsEngine* engine = physics_engine_create();
    int pass = 1;
    if (engine == NULL) {
        printf("[FAIL] failed to create engine for error channel test\n");
        return 0;
    }

    if (physics_engine_add_broadphase_pair(engine, -1, 0) != 0) {
        printf("[FAIL] invalid broadphase pair should fail\n");
        pass = 0;
    } else if (physics_engine_get_last_error(engine) != PHYSICS_ERROR_INVALID_ARGUMENT) {
        printf("[FAIL] expected invalid-argument error, got %s\n",
               physics_error_code_string(physics_engine_get_last_error(engine)));
        pass = 0;
    }

    physics_engine_clear_error(engine);
    if (physics_engine_get_last_error(engine) != PHYSICS_ERROR_NONE) {
        printf("[FAIL] clear_error should reset last error\n");
        pass = 0;
    }

    physics_engine_free(engine);
    if (!pass) return 0;
    printf("[PASS] error channel basics\n");
    return 1;
}

static int test_job_system_hook(void) {
    PhysicsEngine* engine = physics_engine_create();
    JobSystemHookCtx ctx = {0};
    PhysicsJobSystemV1 js;
    RigidBody* a;
    RigidBody* b;
    int pass = 1;
    if (engine == NULL) {
        printf("[FAIL] failed to create engine for job-system test\n");
        return 0;
    }

    memset(&js, 0, sizeof(js));
    js.api_version = PHYSICS_JOB_SYSTEM_API_VERSION;
    js.parallel_for = job_system_parallel_for;
    js.user = &ctx;
    if (physics_engine_set_job_system(engine, &js) == 0) {
        physics_engine_free(engine);
        printf("[FAIL] failed to install job system hook\n");
        return 0;
    }

    physics_engine_set_gravity(engine, vec2(0.0f, 0.0f));
    a = body_create(0.0f, 0.0f, 1.0f, shape_create_circle(2.0f));
    b = body_create(3.0f, 0.0f, 1.0f, shape_create_circle(2.0f));
    physics_engine_add_body(engine, a);
    physics_engine_add_body(engine, b);
    physics_engine_detect_collisions(engine);
    physics_engine_resolve_collisions(engine);

    if (ctx.dispatch_count <= 0) {
        printf("[FAIL] job system hook was not used by solver path\n");
        pass = 0;
    }

    physics_engine_reset_job_system(engine);
    physics_engine_free(engine);
    if (!pass) return 0;
    printf("[PASS] job system hook\n");
    return 1;
}

static int test_layered_config_snapshot(void) {
    PhysicsEngine* engine = physics_engine_create();
    PhysicsRuntimeConfig rcfg;
    PhysicsSolverConfig scfg;
    PhysicsPipelineConfig pcfg;
    PhysicsExperimentalConfig ecfg;
    PhysicsConfigSnapshot snap;
    int pass = 1;
    if (engine == NULL) {
        printf("[FAIL] failed to create engine for layered-config test\n");
        return 0;
    }

    rcfg.time_step = 1.0f / 120.0f;
    rcfg.substeps = 2;
    rcfg.damping = 0.97f;
    scfg.iterations = 9;
    scfg.max_position_iterations_bias = 4;
    pcfg.broadphase_cell_size = 7.5f;
    pcfg.broadphase_type = PHYSICS_BROADPHASE_GRID;
    ecfg.ccd_enabled = 1;
    ecfg.sleep_enabled = 0;
    ecfg.threading_enabled = 1;
    ecfg.worker_count = 2;

    physics_engine_set_runtime_config(engine, &rcfg);
    physics_engine_set_solver_config(engine, &scfg);
    physics_engine_set_pipeline_config(engine, &pcfg);
    physics_engine_set_experimental_config(engine, &ecfg);
    physics_engine_step(engine);
    physics_engine_get_step_config_snapshot(engine, &snap);

    if (fabsf(snap.runtime.time_step - rcfg.time_step) > EPS ||
        snap.runtime.substeps != rcfg.substeps ||
        fabsf(snap.runtime.damping - rcfg.damping) > EPS ||
        snap.solver.iterations != scfg.iterations ||
        snap.solver.max_position_iterations_bias != scfg.max_position_iterations_bias ||
        fabsf(snap.pipeline.broadphase_cell_size - pcfg.broadphase_cell_size) > EPS ||
        snap.pipeline.broadphase_type != pcfg.broadphase_type ||
        snap.experimental.ccd_enabled != ecfg.ccd_enabled ||
        snap.experimental.threading_enabled != ecfg.threading_enabled ||
        snap.experimental.worker_count != ecfg.worker_count) {
        printf("[FAIL] layered config snapshot mismatch\n");
        pass = 0;
    }

    physics_engine_free(engine);
    if (!pass) return 0;
    printf("[PASS] layered config snapshot\n");
    return 1;
}

static int test_generation_handles(void) {
    PhysicsEngine* engine = physics_engine_create();
    RigidBody* a;
    RigidBody* b;
    Constraint* c;
    PhysicsBodyHandle bh;
    PhysicsConstraintHandle ch;
    int pass = 1;
    if (engine == NULL) {
        printf("[FAIL] failed to create engine for generation-handle test\n");
        return 0;
    }

    a = body_create(0.0f, 0.0f, 1.0f, shape_create_circle(1.0f));
    b = body_create(2.0f, 0.0f, 1.0f, shape_create_circle(1.0f));
    physics_engine_add_body(engine, a);
    physics_engine_add_body(engine, b);
    bh = physics_engine_get_body_handle(engine, a);
    if (physics_engine_resolve_body_handle(engine, bh) != a) {
        printf("[FAIL] body handle should resolve before mutation\n");
        pass = 0;
    }

    c = physics_engine_add_distance_constraint(engine, a, b, a->position, b->position, 0.8f, 1);
    ch = physics_engine_get_constraint_handle(engine, c);
    if (physics_engine_resolve_constraint_handle(engine, ch) == NULL) {
        printf("[FAIL] constraint handle should resolve before mutation\n");
        pass = 0;
    }

    physics_engine_detach_body(engine, a);
    body_free(a);
    if (physics_engine_resolve_body_handle(engine, bh) != NULL) {
        printf("[FAIL] stale body handle should be invalid after mutation\n");
        pass = 0;
    }
    if (physics_engine_resolve_constraint_handle(engine, ch) != NULL) {
        printf("[FAIL] stale constraint handle should be invalid after related mutation\n");
        pass = 0;
    }

    physics_engine_free(engine);
    if (!pass) return 0;
    printf("[PASS] generation handles\n");
    return 1;
}

static int test_event_sink_trace(void) {
    PhysicsEngine* engine = physics_engine_create();
    EventSinkCtx ctx = {0};
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

int main(void) {
    int passed = 0;
    int total = 0;
#define RUN_TEST_OR_FAIL(fn)                                          \
    do {                                                               \
        total++;                                                       \
        if (!(fn)()) {                                                 \
            printf("\nResult: FAIL (%d/%d)\n", passed, total);        \
            return 1;                                                  \
        }                                                              \
        passed++;                                                      \
    } while (0)

    RUN_TEST_OR_FAIL(test_circle_circle_detection);
    RUN_TEST_OR_FAIL(test_circle_inside_polygon);
    RUN_TEST_OR_FAIL(test_collision_detect_null_safety);
    RUN_TEST_OR_FAIL(test_shape_helpers_null_safety);
    RUN_TEST_OR_FAIL(test_gravity_override);
    RUN_TEST_OR_FAIL(test_raycast_geometry);
    RUN_TEST_OR_FAIL(test_polygon_polygon_sat_axes);
    RUN_TEST_OR_FAIL(test_high_speed_tunnel_regression);
    RUN_TEST_OR_FAIL(test_high_stack_stability_regression);
    RUN_TEST_OR_FAIL(test_broadphase_grid_filters_pairs);
    RUN_TEST_OR_FAIL(test_add_body_duplicate_guard);
    RUN_TEST_OR_FAIL(test_distance_constraint_stability);
    RUN_TEST_OR_FAIL(test_constraint_break_force);
    RUN_TEST_OR_FAIL(test_custom_pipeline_builders);
    RUN_TEST_OR_FAIL(test_pipeline_plugin_abi);
    RUN_TEST_OR_FAIL(test_error_channel_basics);
    RUN_TEST_OR_FAIL(test_job_system_hook);
    RUN_TEST_OR_FAIL(test_layered_config_snapshot);
    RUN_TEST_OR_FAIL(test_generation_handles);
    RUN_TEST_OR_FAIL(test_event_sink_trace);

    printf("\nResult: PASS (%d/%d)\n", passed, total);
    return 0;
}
