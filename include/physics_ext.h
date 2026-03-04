#ifndef PHYSICS_EXT_H
#define PHYSICS_EXT_H

#ifndef PHYSICS_ENGINE_H
#include "physics.h"
#endif

typedef int (*PhysicsBroadphaseBuilder)(PhysicsEngine* engine, void* user);
typedef int (*PhysicsNarrowphaseBuilder)(PhysicsEngine* engine, void* user);

#define PHYSICS_PIPELINE_PLUGIN_API_VERSION 1u
#define PHYSICS_PIPELINE_PLUGIN_CAP_BROADPHASE 0x1u
#define PHYSICS_PIPELINE_PLUGIN_CAP_NARROWPHASE 0x2u
#define PHYSICS_JOB_SYSTEM_API_VERSION 1u
#ifndef PHYSICS_ENABLE_TRACE
#define PHYSICS_ENABLE_TRACE 1
#endif

#if defined(__GNUC__) || defined(__clang__)
#define PHYSICS_DEPRECATED(msg) __attribute__((deprecated(msg)))
#else
#define PHYSICS_DEPRECATED(msg)
#endif

typedef enum {
    PHYSICS_ERROR_NONE = 0,
    PHYSICS_ERROR_INVALID_ARGUMENT = 1,
    PHYSICS_ERROR_API_VERSION_MISMATCH = 2,
    PHYSICS_ERROR_PLUGIN_INIT_FAILED = 3,
    PHYSICS_ERROR_CAPACITY_EXCEEDED = 4
} PhysicsErrorCode;

typedef enum {
    PHYSICS_BROADPHASE_BRUTE_FORCE = 0,
    PHYSICS_BROADPHASE_GRID = 1
} PhysicsBroadphaseType;

typedef struct {
    float time_step;
    int substeps;
    int iterations;
    float damping;
    float broadphase_cell_size;
    PhysicsBroadphaseType broadphase_type;
    int max_position_iterations_bias;
} PhysicsConfig;

typedef struct {
    float time_step;
    int substeps;
    float damping;
} PhysicsRuntimeConfig;

typedef struct {
    int iterations;
    int max_position_iterations_bias;
} PhysicsSolverConfig;

typedef struct {
    float broadphase_cell_size;
    PhysicsBroadphaseType broadphase_type;
} PhysicsPipelineConfig;

typedef struct {
    int ccd_enabled;
    int sleep_enabled;
    int threading_enabled;
    int worker_count;
} PhysicsExperimentalConfig;

typedef struct {
    PhysicsRuntimeConfig runtime;
    PhysicsSolverConfig solver;
    PhysicsPipelineConfig pipeline;
    PhysicsExperimentalConfig experimental;
} PhysicsConfigSnapshot;

typedef struct {
    int index;
    unsigned int epoch;
} PhysicsBodyHandle;

typedef struct {
    int index;
    unsigned int epoch;
} PhysicsConstraintHandle;

typedef struct {
    double integrate_ms;
    double broadphase_ms;
    double narrowphase_ms;
    double solve_ms;
    double clear_forces_ms;
    double total_ms;
    int step_index;
    int pair_count;
    int contact_count;
} PhysicsStepProfile;

typedef struct {
    void (*on_pre_step)(PhysicsEngine* engine, void* user);
    void (*on_post_broadphase)(PhysicsEngine* engine, int pair_count, void* user);
    void (*on_post_narrowphase)(PhysicsEngine* engine, int contact_count, void* user);
    void (*on_contact_created)(PhysicsEngine* engine, const CollisionManifold* contact, void* user);
    void (*on_post_step)(PhysicsEngine* engine, const PhysicsStepProfile* profile, void* user);
    void* user;
} PhysicsCallbacks;

typedef enum {
    PHYSICS_EVENT_STEP_BEGIN = 1,
    PHYSICS_EVENT_POST_BROADPHASE = 2,
    PHYSICS_EVENT_POST_NARROWPHASE = 3,
    PHYSICS_EVENT_STEP_END = 4,
    PHYSICS_EVENT_ERROR = 5,
    PHYSICS_EVENT_CONTACT_CREATED = 6
} PhysicsEventType;

typedef struct {
    PhysicsEventType type;
    int step_index;
    union {
        struct {
            int count;
            double elapsed_ms;
        } stage;
        struct {
            int contact_count;
            double total_ms;
        } step_end;
        struct {
            PhysicsErrorCode code;
            const char* message;
        } error;
        struct {
            const CollisionManifold* contact;
        } contact;
    } payload;
} PhysicsTraceEvent;

typedef void (*PhysicsEventSinkFn)(const PhysicsTraceEvent* event, void* user);

typedef struct {
    unsigned int api_version;
    unsigned int capabilities;
    int (*init)(PhysicsEngine* engine, void* user);
    void (*shutdown)(PhysicsEngine* engine, void* user);
    PhysicsBroadphaseBuilder broadphase_builder;
    PhysicsNarrowphaseBuilder narrowphase_builder;
    void* user;
} PhysicsPipelinePluginV1;

typedef void (*PhysicsJobRangeFn)(int begin, int end, void* user);

typedef struct {
    unsigned int api_version;
    void (*parallel_for)(int count, int grain, PhysicsJobRangeFn fn, void* fn_user, void* user);
    void* user;
} PhysicsJobSystemV1;

void physics_engine_set_config(PhysicsEngine* engine, const PhysicsConfig* config);
void physics_engine_get_config(const PhysicsEngine* engine, PhysicsConfig* out_config);
PHYSICS_DEPRECATED("Use physics_engine_set_event_sink")
void physics_engine_set_callbacks(PhysicsEngine* engine, const PhysicsCallbacks* callbacks);
void physics_engine_get_last_step_profile(const PhysicsEngine* engine, PhysicsStepProfile* out_profile);
PHYSICS_DEPRECATED("Use physics_engine_install_pipeline_plugin")
void physics_engine_set_broadphase_builder(PhysicsEngine* engine, PhysicsBroadphaseBuilder builder, void* user);
PHYSICS_DEPRECATED("Use physics_engine_install_pipeline_plugin")
void physics_engine_set_narrowphase_builder(PhysicsEngine* engine, PhysicsNarrowphaseBuilder builder, void* user);
PHYSICS_DEPRECATED("Use physics_engine_uninstall_pipeline_plugin")
void physics_engine_reset_pipeline(PhysicsEngine* engine);

void physics_engine_clear_broadphase_pairs(PhysicsEngine* engine);
int physics_engine_add_broadphase_pair(PhysicsEngine* engine, int index_a, int index_b);
void physics_engine_clear_contacts(PhysicsEngine* engine);
int physics_engine_add_contact(PhysicsEngine* engine, RigidBody* a, RigidBody* b, const CollisionInfo* info);
int physics_engine_install_pipeline_plugin(PhysicsEngine* engine, const PhysicsPipelinePluginV1* plugin);
void physics_engine_uninstall_pipeline_plugin(PhysicsEngine* engine);
PhysicsErrorCode physics_engine_get_last_error(const PhysicsEngine* engine);
void physics_engine_clear_error(PhysicsEngine* engine);
const char* physics_error_code_string(PhysicsErrorCode code);
int physics_engine_set_job_system(PhysicsEngine* engine, const PhysicsJobSystemV1* job_system);
void physics_engine_reset_job_system(PhysicsEngine* engine);
void physics_engine_set_runtime_config(PhysicsEngine* engine, const PhysicsRuntimeConfig* config);
void physics_engine_get_runtime_config(const PhysicsEngine* engine, PhysicsRuntimeConfig* out_config);
void physics_engine_set_solver_config(PhysicsEngine* engine, const PhysicsSolverConfig* config);
void physics_engine_get_solver_config(const PhysicsEngine* engine, PhysicsSolverConfig* out_config);
void physics_engine_set_pipeline_config(PhysicsEngine* engine, const PhysicsPipelineConfig* config);
void physics_engine_get_pipeline_config(const PhysicsEngine* engine, PhysicsPipelineConfig* out_config);
void physics_engine_set_experimental_config(PhysicsEngine* engine, const PhysicsExperimentalConfig* config);
void physics_engine_get_experimental_config(const PhysicsEngine* engine, PhysicsExperimentalConfig* out_config);
void physics_engine_get_step_config_snapshot(const PhysicsEngine* engine, PhysicsConfigSnapshot* out_snapshot);
PhysicsBodyHandle physics_engine_get_body_handle(const PhysicsEngine* engine, const RigidBody* body);
RigidBody* physics_engine_resolve_body_handle(const PhysicsEngine* engine, PhysicsBodyHandle handle);
PhysicsConstraintHandle physics_engine_get_constraint_handle(const PhysicsEngine* engine, const Constraint* constraint);
const Constraint* physics_engine_resolve_constraint_handle(const PhysicsEngine* engine, PhysicsConstraintHandle handle);
void physics_engine_set_event_sink(PhysicsEngine* engine, PhysicsEventSinkFn sink, void* user);

#endif
