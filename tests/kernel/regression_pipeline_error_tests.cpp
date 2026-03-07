#include <stdio.h>
#include <string.h>
#include "../../include/physics_core/physics.hpp"
#include "../regression_tests.hpp"

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
    CollisionInfo info{};
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

int regression_test_custom_pipeline_builders(void) {
    PhysicsEngine* engine = physics_engine_create();
    PipelineHookCtx ctx{};
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

int regression_test_pipeline_plugin_abi(void) {
    PhysicsEngine* engine = physics_engine_create();
    PluginHookCtx ctx{};
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

int regression_test_error_channel_basics(void) {
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

int regression_test_job_system_hook(void) {
    PhysicsEngine* engine = physics_engine_create();
    JobSystemHookCtx ctx{};
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
