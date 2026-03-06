#ifndef ASSET_PIPELINE_H
#define ASSET_PIPELINE_H

#include "asset_importer.hpp"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    ASSET_PIPELINE_MAX_PATH = 512,
    ASSET_PIPELINE_MAX_SETTING = 128
};

typedef struct {
    char guid[ASSET_DB_MAX_GUID];
    char source_path[ASSET_PIPELINE_MAX_PATH];
    char settings_fingerprint[ASSET_PIPELINE_MAX_SETTING];
    int use_structured_parameters;
    AssetImportParameters parameters;
} AssetPipelineSourceEntry;

typedef struct {
    AssetDependencyGraph dependency_graph;
    int source_count;
    AssetPipelineSourceEntry sources[ASSET_DB_MAX_ASSETS];
    int changed_count;
    char changed_guids[ASSET_DB_MAX_ASSETS][ASSET_DB_MAX_GUID];
} AssetPipelineState;

typedef struct {
    int requested_change_count;
    int affected_count;
    char affected_guids[ASSET_DB_MAX_ASSETS][ASSET_DB_MAX_GUID];
    int imported_count;
    char imported_guids[ASSET_DB_MAX_ASSETS][ASSET_DB_MAX_GUID];
    int failed_count;
} AssetPipelineRunReport;

void asset_pipeline_init(AssetPipelineState* state);
int asset_pipeline_register_source(
    AssetPipelineState* state,
    const char* source_path,
    const char* settings_fingerprint,
    char out_guid[ASSET_DB_MAX_GUID]);
int asset_pipeline_register_source_ex(
    AssetPipelineState* state,
    const char* source_path,
    const char* settings_fingerprint,
    const AssetImportParameters* parameters,
    char out_guid[ASSET_DB_MAX_GUID]);
int asset_pipeline_set_dependency(AssetPipelineState* state, const char* guid, const char* depends_on_guid);
int asset_pipeline_mark_changed(AssetPipelineState* state, const char* guid);
int asset_pipeline_mark_source_changed(AssetPipelineState* state, const char* source_path);
int asset_pipeline_mark_source_changed_batch(
    AssetPipelineState* state,
    const char* const* source_paths,
    int source_count);
int asset_pipeline_run_reimport(
    AssetPipelineState* state,
    const char* cache_root,
    AssetPipelineRunReport* out_report);

#ifdef __cplusplus
}
#endif

#endif
