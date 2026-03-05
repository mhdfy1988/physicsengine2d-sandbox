#include <string.h>
#include "physics_internal.h"
#include "internal/physics_defaults.h"

void physics_internal_sanitize_config(PhysicsConfig* config) {
    if (config == NULL) return;
    if (config->time_step <= 0.0f) config->time_step = PHYSICS_DEFAULT_TIME_STEP;
    if (config->substeps < 1) config->substeps = PHYSICS_DEFAULT_SUBSTEPS;
    if (config->iterations < 1) config->iterations = PHYSICS_DEFAULT_ITERATIONS;
    if (config->max_position_iterations_bias < 0) config->max_position_iterations_bias = 0;
    config->damping = clamp(config->damping, 0.0f, 1.0f);
    if (config->broadphase_cell_size <= 0.0f) {
        config->broadphase_cell_size = PHYSICS_DEFAULT_BROADPHASE_CELL_SIZE;
    }
    if (config->broadphase_type != PHYSICS_BROADPHASE_GRID &&
        config->broadphase_type != PHYSICS_BROADPHASE_SAP &&
        config->broadphase_type != PHYSICS_BROADPHASE_BVH) {
        config->broadphase_type = PHYSICS_BROADPHASE_BRUTE_FORCE;
    }
}

void physics_internal_sanitize_experimental_config(PhysicsExperimentalConfig* config) {
    if (config == NULL) return;
    config->ccd_enabled = config->ccd_enabled ? 1 : 0;
    config->sleep_enabled = config->sleep_enabled ? 1 : 0;
    config->threading_enabled = config->threading_enabled ? 1 : 0;
    if (config->worker_count < 1) config->worker_count = PHYSICS_DEFAULT_EXPERIMENTAL_WORKERS;
}

int physics_engine_install_pipeline_plugin(PhysicsEngine* engine, const PhysicsPipelinePluginV1* plugin) {
    int init_ok = 1;
    if (engine == NULL || plugin == NULL) {
        physics_internal_set_error_code(engine, PHYSICS_ERROR_INVALID_ARGUMENT);
        return 0;
    }
    if (plugin->api_version != PHYSICS_PIPELINE_PLUGIN_API_VERSION) {
        physics_internal_set_error_code(engine, PHYSICS_ERROR_API_VERSION_MISMATCH);
        return 0;
    }

    physics_engine_uninstall_pipeline_plugin(engine);
    engine->plugin = *plugin;
    engine->plugin_installed = 1;

    if ((plugin->capabilities & PHYSICS_PIPELINE_PLUGIN_CAP_BROADPHASE) != 0u && plugin->broadphase_builder != NULL) {
        engine->broadphase_ops.build_pairs = plugin->broadphase_builder;
        engine->broadphase_ops.user = plugin->user;
    }
    if ((plugin->capabilities & PHYSICS_PIPELINE_PLUGIN_CAP_NARROWPHASE) != 0u && plugin->narrowphase_builder != NULL) {
        engine->narrowphase_ops.build_contacts = plugin->narrowphase_builder;
        engine->narrowphase_ops.user = plugin->user;
    }
    if (plugin->init != NULL) {
        init_ok = plugin->init(engine, plugin->user);
    }
    if (!init_ok) {
        physics_engine_uninstall_pipeline_plugin(engine);
        physics_internal_set_error_code(engine, PHYSICS_ERROR_PLUGIN_INIT_FAILED);
        return 0;
    }
    physics_internal_set_error_code(engine, PHYSICS_ERROR_NONE);
    return 1;
}

void physics_engine_uninstall_pipeline_plugin(PhysicsEngine* engine) {
    if (engine == NULL) return;
    if (engine->plugin_installed && engine->plugin.shutdown != NULL) {
        engine->plugin.shutdown(engine, engine->plugin.user);
    }
    memset(&engine->plugin, 0, sizeof(engine->plugin));
    engine->plugin_installed = 0;
    physics_internal_bind_default_pipeline(engine);
    physics_internal_set_error_code(engine, PHYSICS_ERROR_NONE);
}

PhysicsErrorCode physics_engine_get_last_error(const PhysicsEngine* engine) {
    if (engine == NULL) return PHYSICS_ERROR_INVALID_ARGUMENT;
    return engine->last_error;
}

void physics_engine_clear_error(PhysicsEngine* engine) {
    physics_internal_set_error_code(engine, PHYSICS_ERROR_NONE);
}

const char* physics_error_code_string(PhysicsErrorCode code) {
    switch (code) {
        case PHYSICS_ERROR_NONE: return "none";
        case PHYSICS_ERROR_INVALID_ARGUMENT: return "invalid_argument";
        case PHYSICS_ERROR_API_VERSION_MISMATCH: return "api_version_mismatch";
        case PHYSICS_ERROR_PLUGIN_INIT_FAILED: return "plugin_init_failed";
        case PHYSICS_ERROR_CAPACITY_EXCEEDED: return "capacity_exceeded";
        default: return "unknown_error";
    }
}

int physics_engine_set_job_system(PhysicsEngine* engine, const PhysicsJobSystemV1* job_system) {
    if (engine == NULL || job_system == NULL || job_system->parallel_for == NULL) {
        physics_internal_set_error_code(engine, PHYSICS_ERROR_INVALID_ARGUMENT);
        return 0;
    }
    if (job_system->api_version != PHYSICS_JOB_SYSTEM_API_VERSION) {
        physics_internal_set_error_code(engine, PHYSICS_ERROR_API_VERSION_MISMATCH);
        return 0;
    }
    engine->job_system = *job_system;
    engine->job_system_installed = 1;
    physics_internal_set_error_code(engine, PHYSICS_ERROR_NONE);
    return 1;
}

void physics_engine_reset_job_system(PhysicsEngine* engine) {
    if (engine == NULL) return;
    memset(&engine->job_system, 0, sizeof(engine->job_system));
    engine->job_system_installed = 0;
    physics_internal_set_error_code(engine, PHYSICS_ERROR_NONE);
}

void physics_engine_set_runtime_config(PhysicsEngine* engine, const PhysicsRuntimeConfig* config) {
    if (engine == NULL || config == NULL) {
        physics_internal_set_error_code(engine, PHYSICS_ERROR_INVALID_ARGUMENT);
        return;
    }
    engine->config.time_step = config->time_step;
    engine->config.substeps = config->substeps;
    engine->config.damping = config->damping;
    physics_internal_sanitize_config(&engine->config);
    physics_internal_set_error_code(engine, PHYSICS_ERROR_NONE);
}

void physics_engine_get_runtime_config(const PhysicsEngine* engine, PhysicsRuntimeConfig* out_config) {
    if (engine == NULL || out_config == NULL) return;
    out_config->time_step = engine->config.time_step;
    out_config->substeps = engine->config.substeps;
    out_config->damping = engine->config.damping;
}

void physics_engine_set_solver_config(PhysicsEngine* engine, const PhysicsSolverConfig* config) {
    if (engine == NULL || config == NULL) {
        physics_internal_set_error_code(engine, PHYSICS_ERROR_INVALID_ARGUMENT);
        return;
    }
    engine->config.iterations = config->iterations;
    engine->config.max_position_iterations_bias = config->max_position_iterations_bias;
    physics_internal_sanitize_config(&engine->config);
    physics_internal_set_error_code(engine, PHYSICS_ERROR_NONE);
}

void physics_engine_get_solver_config(const PhysicsEngine* engine, PhysicsSolverConfig* out_config) {
    if (engine == NULL || out_config == NULL) return;
    out_config->iterations = engine->config.iterations;
    out_config->max_position_iterations_bias = engine->config.max_position_iterations_bias;
}

void physics_engine_set_pipeline_config(PhysicsEngine* engine, const PhysicsPipelineConfig* config) {
    PhysicsBroadphaseType requested_type;
    if (engine == NULL || config == NULL) {
        physics_internal_set_error_code(engine, PHYSICS_ERROR_INVALID_ARGUMENT);
        return;
    }
    requested_type = config->broadphase_type;
    engine->config.broadphase_cell_size = config->broadphase_cell_size;
    engine->config.broadphase_type = requested_type;
    physics_internal_sanitize_config(&engine->config);
    if (requested_type == PHYSICS_BROADPHASE_BVH) {
        engine->config.broadphase_type = PHYSICS_BROADPHASE_BRUTE_FORCE;
        physics_internal_set_error_code(engine, PHYSICS_ERROR_INVALID_ARGUMENT);
        return;
    }
    physics_internal_set_error_code(engine, PHYSICS_ERROR_NONE);
}

void physics_engine_get_pipeline_config(const PhysicsEngine* engine, PhysicsPipelineConfig* out_config) {
    if (engine == NULL || out_config == NULL) return;
    out_config->broadphase_cell_size = engine->config.broadphase_cell_size;
    out_config->broadphase_type = engine->config.broadphase_type;
}

void physics_engine_set_experimental_config(PhysicsEngine* engine, const PhysicsExperimentalConfig* config) {
    if (engine == NULL || config == NULL) {
        physics_internal_set_error_code(engine, PHYSICS_ERROR_INVALID_ARGUMENT);
        return;
    }
    engine->experimental = *config;
    physics_internal_sanitize_experimental_config(&engine->experimental);
    physics_internal_set_error_code(engine, PHYSICS_ERROR_NONE);
}

void physics_engine_get_experimental_config(const PhysicsEngine* engine, PhysicsExperimentalConfig* out_config) {
    if (engine == NULL || out_config == NULL) return;
    *out_config = engine->experimental;
}

void physics_engine_get_step_config_snapshot(const PhysicsEngine* engine, PhysicsConfigSnapshot* out_snapshot) {
    if (engine == NULL || out_snapshot == NULL) return;
    *out_snapshot = engine->last_step_config_snapshot;
}
