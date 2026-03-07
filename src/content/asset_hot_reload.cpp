#include "physics_content/asset_hot_reload.hpp"

#include <string.h>

void asset_hot_reload_tick_report_init(AssetHotReloadTickReport* report) {
    if (report == NULL) return;
    memset(report, 0, sizeof(*report));
    asset_watch_batch_init(&report->ready_batch);
}

void asset_hot_reload_service_init(AssetHotReloadService* service, int debounce_ms) {
    if (service == NULL) return;
    memset(service, 0, sizeof(*service));
    asset_watch_init(&service->watch, debounce_ms);
    asset_pipeline_init(&service->pipeline);
}

int asset_hot_reload_register_source(
    AssetHotReloadService* service,
    const char* source_path,
    const char* settings_fingerprint,
    char out_guid[ASSET_DB_MAX_GUID]) {
    return asset_hot_reload_register_source_ex(service, source_path, settings_fingerprint, NULL, out_guid);
}

int asset_hot_reload_register_source_ex(
    AssetHotReloadService* service,
    const char* source_path,
    const char* settings_fingerprint,
    const AssetImportParameters* parameters,
    char out_guid[ASSET_DB_MAX_GUID]) {
    if (service == NULL) return 0;
    return asset_pipeline_register_source_ex(&service->pipeline, source_path, settings_fingerprint, parameters, out_guid);
}

int asset_hot_reload_set_dependency(AssetHotReloadService* service, const char* guid, const char* depends_on_guid) {
    if (service == NULL) return 0;
    return asset_pipeline_set_dependency(&service->pipeline, guid, depends_on_guid);
}

int asset_hot_reload_on_file_changed(AssetHotReloadService* service, const char* source_path, long long timestamp_ms) {
    if (service == NULL) return 0;
    return asset_watch_push_change(&service->watch, source_path, timestamp_ms);
}

int asset_hot_reload_tick(
    AssetHotReloadService* service,
    long long now_ms,
    const char* cache_root,
    AssetHotReloadTickReport* out_report) {
    AssetHotReloadTickReport report;
    int ok = 1;
    int i;
    const char* changed_sources[ASSET_WATCH_MAX_PENDING];
    if (service == NULL || out_report == NULL) return 0;
    asset_hot_reload_tick_report_init(&report);
    if (!asset_watch_collect_ready(&service->watch, now_ms, &report.ready_batch)) {
        return 0;
    }
    if (report.ready_batch.count <= 0) {
        *out_report = report;
        return 1;
    }
    for (i = 0; i < report.ready_batch.count; i++) {
        changed_sources[i] = report.ready_batch.source_paths[i];
    }
    if (!asset_pipeline_mark_source_changed_batch(&service->pipeline, changed_sources, report.ready_batch.count)) {
        ok = 0;
    }
    if (!asset_pipeline_run_reimport(&service->pipeline, cache_root, &report.pipeline_report)) {
        ok = 0;
    }
    report.pipeline_ran = 1;
    *out_report = report;
    return ok;
}
