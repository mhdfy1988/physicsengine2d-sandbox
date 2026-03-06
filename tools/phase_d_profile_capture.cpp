#include <direct.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include "../include/asset_importer.hpp"
#include "../include/physics.hpp"
#include "../include/subsystem_render_audio_animation.hpp"
#include "internal/physics_world_state.hpp"

static double now_ms(void) {
#ifdef _WIN32
    static LARGE_INTEGER freq{};
    LARGE_INTEGER now;
    if (freq.QuadPart == 0) {
        QueryPerformanceFrequency(&freq);
    }
    QueryPerformanceCounter(&now);
    return (double)now.QuadPart * 1000.0 / (double)freq.QuadPart;
#elif defined(CLOCK_MONOTONIC)
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
        return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1000000.0;
    }
    return (double)clock() * 1000.0 / (double)CLOCKS_PER_SEC;
#else
    return (double)clock() * 1000.0 / (double)CLOCKS_PER_SEC;
#endif
}

static int env_int_or_default(const char* name, int fallback) {
    const char* text = getenv(name);
    int value;
    if (text == NULL || text[0] == '\0') return fallback;
    value = atoi(text);
    return (value > 0) ? value : fallback;
}

static int write_text_file(const char* path, const char* text) {
    FILE* fp;
    size_t len;
    if (path == NULL || text == NULL) return 0;
    fp = fopen(path, "wb");
    if (fp == NULL) return 0;
    len = strlen(text);
    if (fwrite(text, 1, len, fp) != len) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

static long long file_size_bytes(const char* path) {
    FILE* fp;
    long long size;
    if (path == NULL) return -1;
    fp = fopen(path, "rb");
    if (fp == NULL) return -1;
    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return -1;
    }
    size = (long long)ftell(fp);
    fclose(fp);
    return size;
}

static int logical_cpu_count(void) {
#ifdef _WIN32
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return (int)info.dwNumberOfProcessors;
#else
    return 1;
#endif
}

static PhysicsEngine* build_engine(int threading_enabled, int worker_count, int body_count) {
    PhysicsEngine* engine = physics_engine_create();
    PhysicsExperimentalConfig ecfg;
    int i;
    if (engine == NULL) return NULL;
    physics_engine_set_gravity(engine, vec2(0.0f, 9.8f));
    physics_engine_set_time_step(engine, 1.0f / 120.0f);
    physics_engine_set_iterations(engine, 12);
    physics_engine_set_broadphase_use_grid(engine, 1);
    ecfg.ccd_enabled = 1;
    ecfg.sleep_enabled = 0;
    ecfg.threading_enabled = threading_enabled ? 1 : 0;
    ecfg.worker_count = worker_count;
    physics_engine_set_experimental_config(engine, &ecfg);
    for (i = 0; i < body_count; i++) {
        float x = (float)(i % 20) * 1.6f;
        float y = (float)(i / 20) * 1.6f;
        RigidBody* body = body_create(x, y, 1.0f, shape_create_circle(0.55f));
        if (body != NULL) {
            physics_engine_add_body(engine, body);
        }
    }
    {
        RigidBody* floor = body_create(16.0f, 32.0f, 1.0f, shape_create_box(72.0f, 2.0f));
        if (floor != NULL) {
            body_set_type(floor, BODY_STATIC);
            physics_engine_add_body(engine, floor);
        }
    }
    return engine;
}

static double run_average_step_ms(PhysicsEngine* engine, int warmup_steps, int measure_steps) {
    double start_ms;
    int i;
    if (engine == NULL || measure_steps <= 0) return 0.0;
    for (i = 0; i < warmup_steps; i++) {
        physics_engine_step(engine);
    }
    start_ms = now_ms();
    for (i = 0; i < measure_steps; i++) {
        physics_engine_step(engine);
    }
    return (now_ms() - start_ms) / (double)measure_steps;
}

static int capture_step_profile(
    PhysicsEngine* engine,
    int warmup_steps,
    int measure_steps,
    PhysicsStepProfile* out_average) {
    PhysicsStepProfile avg;
    int i;
    if (engine == NULL || out_average == NULL || measure_steps <= 0) return 0;
    memset(&avg, 0, sizeof(avg));
    for (i = 0; i < warmup_steps; i++) {
        physics_engine_step(engine);
    }
    for (i = 0; i < measure_steps; i++) {
        PhysicsStepProfile last;
        physics_engine_step(engine);
        physics_engine_get_last_step_profile(engine, &last);
        avg.integrate_ms += last.integrate_ms;
        avg.broadphase_ms += last.broadphase_ms;
        avg.narrowphase_ms += last.narrowphase_ms;
        avg.solve_ms += last.solve_ms;
        avg.clear_forces_ms += last.clear_forces_ms;
        avg.total_ms += last.total_ms;
        avg.pair_count += last.pair_count;
        avg.contact_count += last.contact_count;
    }
    avg.integrate_ms /= (double)measure_steps;
    avg.broadphase_ms /= (double)measure_steps;
    avg.narrowphase_ms /= (double)measure_steps;
    avg.solve_ms /= (double)measure_steps;
    avg.clear_forces_ms /= (double)measure_steps;
    avg.total_ms /= (double)measure_steps;
    avg.pair_count /= measure_steps;
    avg.contact_count /= measure_steps;
    *out_average = avg;
    return 1;
}

static int measure_import_average_ms(
    const char* source_path,
    const char* cache_root,
    int runs,
    AssetImportResult* out_last_result,
    double* out_avg_ms) {
    AssetImportRequest req;
    AssetImportResult last_result;
    double total_ms = 0.0;
    int i;
    if (source_path == NULL || cache_root == NULL || runs <= 0 || out_last_result == NULL || out_avg_ms == NULL) {
        return 0;
    }
    memset(&req, 0, sizeof(req));
    memset(&last_result, 0, sizeof(last_result));
    req.cache_root = cache_root;
    req.settings_fingerprint = "phase-d-profile";
    req.source_path = source_path;
    for (i = 0; i < runs; i++) {
        double start_ms = now_ms();
        if (!asset_importer_run(&req, &last_result) || !last_result.success) {
            return 0;
        }
        total_ms += (now_ms() - start_ms);
    }
    *out_last_result = last_result;
    *out_avg_ms = total_ms / (double)runs;
    return 1;
}

static void cleanup_import_artifacts(const char* source_path, const AssetImportResult* result) {
    char meta_path[ASSET_IMPORTER_MAX_PATH];
    if (source_path != NULL) {
        snprintf(meta_path, sizeof(meta_path), "%s.meta", source_path);
        remove(meta_path);
        remove(source_path);
    }
    if (result != NULL && result->artifact_path[0] != '\0') {
        remove(result->artifact_path);
    }
}

int main(void) {
    const int body_count = env_int_or_default("PHYSICS_D_PROFILE_BODIES", 960);
    const int warmup_steps = env_int_or_default("PHYSICS_D_PROFILE_WARMUP", 120);
    const int measure_steps = env_int_or_default("PHYSICS_D_PROFILE_STEPS", 240);
    const int import_runs = env_int_or_default("PHYSICS_D_PROFILE_IMPORT_RUNS", 5);
    const char* assets_dir = "_tmp_phase_d_profile_assets";
    const char* cache_dir = "_tmp_phase_d_profile_cache";
    const char* texture_path = "_tmp_phase_d_profile_assets/profile_sprite.png";
    const char* audio_path = "_tmp_phase_d_profile_assets/profile_sfx.wav";
    PhysicsEngine* cpu_engine = NULL;
    PhysicsEngine* seq_engine = NULL;
    PhysicsEngine* par2_engine = NULL;
    PhysicsEngine* par4_engine = NULL;
    PhysicsStepProfile avg_profile;
    AssetImportResult texture_result;
    AssetImportResult audio_result;
    double texture_ms = 0.0;
    double audio_ms = 0.0;
    double seq_ms;
    double par2_ms;
    double par4_ms;
    size_t engine_struct_bytes;
    size_t pair_stamp_bytes;
    size_t scratch_bytes;
    size_t active_body_bytes;
    size_t total_runtime_bytes;
    long long texture_source_bytes;
    long long texture_artifact_bytes;
    long long audio_source_bytes;
    long long audio_artifact_bytes;

    memset(&avg_profile, 0, sizeof(avg_profile));
    memset(&texture_result, 0, sizeof(texture_result));
    memset(&audio_result, 0, sizeof(audio_result));

    _mkdir(assets_dir);
    _mkdir(cache_dir);
    if (!write_text_file(texture_path, "phase-d-profile-texture") ||
        !write_text_file(audio_path, "phase-d-profile-audio")) {
        printf("[FAIL] phase D profile capture: temp input generation failed\n");
        return 1;
    }

    cpu_engine = build_engine(0, 1, body_count);
    seq_engine = build_engine(0, 1, body_count);
    par2_engine = build_engine(1, 2, body_count);
    par4_engine = build_engine(1, 4, body_count);
    if (cpu_engine == NULL || seq_engine == NULL || par2_engine == NULL || par4_engine == NULL) {
        printf("[FAIL] phase D profile capture: engine create failed\n");
        physics_engine_free(cpu_engine);
        physics_engine_free(seq_engine);
        physics_engine_free(par2_engine);
        physics_engine_free(par4_engine);
        cleanup_import_artifacts(texture_path, &texture_result);
        cleanup_import_artifacts(audio_path, &audio_result);
        _rmdir(cache_dir);
        _rmdir(assets_dir);
        return 2;
    }

    if (!capture_step_profile(cpu_engine, warmup_steps, measure_steps, &avg_profile)) {
        printf("[FAIL] phase D profile capture: cpu profile capture failed\n");
        physics_engine_free(cpu_engine);
        physics_engine_free(seq_engine);
        physics_engine_free(par2_engine);
        physics_engine_free(par4_engine);
        cleanup_import_artifacts(texture_path, &texture_result);
        cleanup_import_artifacts(audio_path, &audio_result);
        _rmdir(cache_dir);
        _rmdir(assets_dir);
        return 3;
    }

    seq_ms = run_average_step_ms(seq_engine, warmup_steps, measure_steps);
    par2_ms = run_average_step_ms(par2_engine, warmup_steps, measure_steps);
    par4_ms = run_average_step_ms(par4_engine, warmup_steps, measure_steps);

    if (!measure_import_average_ms(texture_path, cache_dir, import_runs, &texture_result, &texture_ms) ||
        !measure_import_average_ms(audio_path, cache_dir, import_runs, &audio_result, &audio_ms)) {
        printf("[FAIL] phase D profile capture: asset import capture failed\n");
        physics_engine_free(cpu_engine);
        physics_engine_free(seq_engine);
        physics_engine_free(par2_engine);
        physics_engine_free(par4_engine);
        cleanup_import_artifacts(texture_path, &texture_result);
        cleanup_import_artifacts(audio_path, &audio_result);
        _rmdir(cache_dir);
        _rmdir(assets_dir);
        return 4;
    }

    engine_struct_bytes = sizeof(PhysicsEngine);
    pair_stamp_bytes = (size_t)(MAX_BODIES * MAX_BODIES) * sizeof(unsigned short);
    scratch_bytes = (size_t)cpu_engine->scratch.capacity;
    active_body_bytes = (size_t)physics_engine_get_body_count(cpu_engine) * sizeof(RigidBody);
    total_runtime_bytes = engine_struct_bytes + pair_stamp_bytes + scratch_bytes + active_body_bytes;
    texture_source_bytes = file_size_bytes(texture_path);
    texture_artifact_bytes = file_size_bytes(texture_result.artifact_path);
    audio_source_bytes = file_size_bytes(audio_path);
    audio_artifact_bytes = file_size_bytes(audio_result.artifact_path);

    printf("[INFO] phase_d_profile env logical_cpus=%d bodies=%d warmup=%d steps=%d import_runs=%d\n",
           logical_cpu_count(),
           body_count,
           warmup_steps,
           measure_steps,
           import_runs);
    printf("[INFO] phase_d_profile cpu_avg_step_ms total=%.4f integrate=%.4f broadphase=%.4f narrowphase=%.4f solve=%.4f clear=%.4f pairs=%d contacts=%d\n",
           avg_profile.total_ms,
           avg_profile.integrate_ms,
           avg_profile.broadphase_ms,
           avg_profile.narrowphase_ms,
           avg_profile.solve_ms,
           avg_profile.clear_forces_ms,
           avg_profile.pair_count,
           avg_profile.contact_count);
    printf("[INFO] phase_d_profile runtime_memory_bytes engine=%zu pair_stamp=%zu scratch=%zu active_bodies=%zu total_estimated=%zu workflow_world=%zu\n",
           engine_struct_bytes,
           pair_stamp_bytes,
           scratch_bytes,
           active_body_bytes,
           total_runtime_bytes,
           sizeof(SubsystemWorkflowWorld));
    printf("[INFO] phase_d_profile import_texture avg_ms=%.4f source_bytes=%lld artifact_bytes=%lld guid=%s\n",
           texture_ms,
           texture_source_bytes,
           texture_artifact_bytes,
           texture_result.meta.guid);
    printf("[INFO] phase_d_profile import_audio avg_ms=%.4f source_bytes=%lld artifact_bytes=%lld guid=%s\n",
           audio_ms,
           audio_source_bytes,
           audio_artifact_bytes,
           audio_result.meta.guid);
    printf("[INFO] phase_d_profile parallel_avg_step_ms sequential=%.4f workers=1\n", seq_ms);
    printf("[INFO] phase_d_profile parallel_avg_step_ms threaded=%.4f workers=2 speedup=%.3fx\n",
           par2_ms,
           (par2_ms > 0.0) ? (seq_ms / par2_ms) : 0.0);
    printf("[INFO] phase_d_profile parallel_avg_step_ms threaded=%.4f workers=4 speedup=%.3fx\n",
           par4_ms,
           (par4_ms > 0.0) ? (seq_ms / par4_ms) : 0.0);
    printf("[PASS] phase D profile capture\n");

    physics_engine_free(cpu_engine);
    physics_engine_free(seq_engine);
    physics_engine_free(par2_engine);
    physics_engine_free(par4_engine);
    cleanup_import_artifacts(texture_path, &texture_result);
    cleanup_import_artifacts(audio_path, &audio_result);
    _rmdir(cache_dir);
    _rmdir(assets_dir);
    return 0;
}
