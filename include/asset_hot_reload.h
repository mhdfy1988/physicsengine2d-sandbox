#ifndef ASSET_HOT_RELOAD_H
#define ASSET_HOT_RELOAD_H

#include "asset_pipeline.h"
#include "asset_watch.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int pipeline_ran;
    AssetWatchBatch ready_batch;
    AssetPipelineRunReport pipeline_report;
} AssetHotReloadTickReport;

typedef struct {
    AssetWatchState watch;
    AssetPipelineState pipeline;
} AssetHotReloadService;

void asset_hot_reload_tick_report_init(AssetHotReloadTickReport* report);
void asset_hot_reload_service_init(AssetHotReloadService* service, int debounce_ms);
int asset_hot_reload_register_source(
    AssetHotReloadService* service,
    const char* source_path,
    const char* settings_fingerprint,
    char out_guid[ASSET_DB_MAX_GUID]);
int asset_hot_reload_register_source_ex(
    AssetHotReloadService* service,
    const char* source_path,
    const char* settings_fingerprint,
    const AssetImportParameters* parameters,
    char out_guid[ASSET_DB_MAX_GUID]);
int asset_hot_reload_set_dependency(AssetHotReloadService* service, const char* guid, const char* depends_on_guid);
int asset_hot_reload_on_file_changed(AssetHotReloadService* service, const char* source_path, long long timestamp_ms);
int asset_hot_reload_tick(
    AssetHotReloadService* service,
    long long now_ms,
    const char* cache_root,
    AssetHotReloadTickReport* out_report);

#ifdef __cplusplus
}
#endif

#endif
