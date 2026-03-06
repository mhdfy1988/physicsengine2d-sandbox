#include "asset_pipeline.hpp"

#include "asset_importer.hpp"
#include "asset_invalidation.hpp"

#include <stdio.h>
#include <string.h>

static void asset_pipeline_copy_text(char* out, int out_cap, const char* src) {
    if (out == NULL || out_cap <= 0) return;
    if (src == NULL) {
        out[0] = '\0';
        return;
    }
    strncpy(out, src, (size_t)(out_cap - 1));
    out[out_cap - 1] = '\0';
}

static int asset_pipeline_find_source_index_by_guid(const AssetPipelineState* state, const char* guid) {
    int i;
    if (state == NULL || guid == NULL) return -1;
    for (i = 0; i < state->source_count; i++) {
        if (strcmp(state->sources[i].guid, guid) == 0) return i;
    }
    return -1;
}

static int asset_pipeline_guid_exists(char guids[ASSET_DB_MAX_ASSETS][ASSET_DB_MAX_GUID], int count, const char* guid) {
    int i;
    if (guid == NULL) return 0;
    for (i = 0; i < count; i++) {
        if (strcmp(guids[i], guid) == 0) return 1;
    }
    return 0;
}

static void asset_pipeline_report_init(AssetPipelineRunReport* report) {
    if (report == NULL) return;
    memset(report, 0, sizeof(*report));
}

void asset_pipeline_init(AssetPipelineState* state) {
    if (state == NULL) return;
    memset(state, 0, sizeof(*state));
    asset_dependency_graph_init(&state->dependency_graph);
}

int asset_pipeline_register_source(
    AssetPipelineState* state,
    const char* source_path,
    const char* settings_fingerprint,
    char out_guid[ASSET_DB_MAX_GUID]) {
    return asset_pipeline_register_source_ex(state, source_path, settings_fingerprint, NULL, out_guid);
}

int asset_pipeline_register_source_ex(
    AssetPipelineState* state,
    const char* source_path,
    const char* settings_fingerprint,
    const AssetImportParameters* parameters,
    char out_guid[ASSET_DB_MAX_GUID]) {
    int idx;
    char guid[ASSET_DB_MAX_GUID];
    char meta_path[ASSET_PIPELINE_MAX_PATH];
    AssetMeta meta;
    AssetImportParameters from_meta;
    AssetImportKind kind;
    if (state == NULL || source_path == NULL) return 0;
    if (!asset_meta_make_guid_from_path(source_path, guid)) return 0;
    idx = asset_pipeline_find_source_index_by_guid(state, guid);
    if (idx < 0) {
        if (state->source_count >= ASSET_DB_MAX_ASSETS) return 0;
        idx = state->source_count;
        state->source_count++;
    }
    asset_pipeline_copy_text(state->sources[idx].guid, ASSET_DB_MAX_GUID, guid);
    asset_pipeline_copy_text(state->sources[idx].source_path, ASSET_PIPELINE_MAX_PATH, source_path);
    asset_pipeline_copy_text(state->sources[idx].settings_fingerprint, ASSET_PIPELINE_MAX_SETTING,
                             (settings_fingerprint != NULL) ? settings_fingerprint : "default");
    state->sources[idx].use_structured_parameters = 0;
    memset(&state->sources[idx].parameters, 0, sizeof(state->sources[idx].parameters));
    kind = asset_importer_kind_from_path(source_path);
    if (parameters != NULL) {
        state->sources[idx].parameters = *parameters;
        state->sources[idx].use_structured_parameters = 1;
    } else {
        snprintf(meta_path, sizeof(meta_path), "%s.meta", source_path);
        if (asset_meta_load(meta_path, &meta) && asset_import_parameters_from_meta(kind, &meta, &from_meta)) {
            state->sources[idx].parameters = from_meta;
            state->sources[idx].use_structured_parameters = 1;
        }
    }
    if (!asset_dependency_graph_add_asset(&state->dependency_graph, guid)) return 0;
    if (out_guid != NULL) asset_pipeline_copy_text(out_guid, ASSET_DB_MAX_GUID, guid);
    return 1;
}

int asset_pipeline_set_dependency(AssetPipelineState* state, const char* guid, const char* depends_on_guid) {
    if (state == NULL || guid == NULL || depends_on_guid == NULL) return 0;
    return asset_dependency_graph_add_dependency(&state->dependency_graph, guid, depends_on_guid);
}

int asset_pipeline_mark_changed(AssetPipelineState* state, const char* guid) {
    if (state == NULL || guid == NULL || guid[0] == '\0') return 0;
    if (!asset_dependency_graph_add_asset(&state->dependency_graph, guid)) return 0;
    if (asset_pipeline_guid_exists(state->changed_guids, state->changed_count, guid)) return 1;
    if (state->changed_count >= ASSET_DB_MAX_ASSETS) return 0;
    asset_pipeline_copy_text(state->changed_guids[state->changed_count], ASSET_DB_MAX_GUID, guid);
    state->changed_count++;
    return 1;
}

int asset_pipeline_mark_source_changed(AssetPipelineState* state, const char* source_path) {
    char guid[ASSET_DB_MAX_GUID];
    if (state == NULL || source_path == NULL) return 0;
    if (!asset_meta_make_guid_from_path(source_path, guid)) return 0;
    return asset_pipeline_mark_changed(state, guid);
}

int asset_pipeline_mark_source_changed_batch(
    AssetPipelineState* state,
    const char* const* source_paths,
    int source_count) {
    int i;
    int all_ok = 1;
    if (state == NULL || source_paths == NULL || source_count < 0) return 0;
    for (i = 0; i < source_count; i++) {
        if (source_paths[i] == NULL) {
            all_ok = 0;
            continue;
        }
        if (!asset_pipeline_mark_source_changed(state, source_paths[i])) {
            all_ok = 0;
        }
    }
    return all_ok;
}

int asset_pipeline_run_reimport(
    AssetPipelineState* state,
    const char* cache_root,
    AssetPipelineRunReport* out_report) {
    AssetPipelineRunReport report;
    int i;
    if (state == NULL) return 0;
    asset_pipeline_report_init(&report);
    report.requested_change_count = state->changed_count;
    for (i = 0; i < state->changed_count; i++) {
        AssetInvalidationBatch batch;
        int j;
        asset_invalidation_batch_init(&batch);
        if (!asset_invalidation_collect(&state->dependency_graph, state->changed_guids[i], &batch)) {
            report.failed_count++;
            continue;
        }
        for (j = 0; j < batch.count; j++) {
            if (!asset_pipeline_guid_exists(report.affected_guids, report.affected_count, batch.guids[j])) {
                if (report.affected_count < ASSET_DB_MAX_ASSETS) {
                    asset_pipeline_copy_text(report.affected_guids[report.affected_count], ASSET_DB_MAX_GUID, batch.guids[j]);
                    report.affected_count++;
                }
            }
        }
    }

    for (i = 0; i < report.affected_count; i++) {
        int source_idx = asset_pipeline_find_source_index_by_guid(state, report.affected_guids[i]);
        AssetImportRequest request;
        AssetImportResult result;
        if (source_idx < 0) {
            continue;
        }
        request.source_path = state->sources[source_idx].source_path;
        request.cache_root = (cache_root != NULL) ? cache_root : "Cache/imported";
        request.settings_fingerprint = state->sources[source_idx].settings_fingerprint;
        request.parameters = state->sources[source_idx].use_structured_parameters ? &state->sources[source_idx].parameters : NULL;
        if (asset_importer_run(&request, &result)) {
            if (report.imported_count < ASSET_DB_MAX_ASSETS) {
                asset_pipeline_copy_text(report.imported_guids[report.imported_count], ASSET_DB_MAX_GUID, report.affected_guids[i]);
                report.imported_count++;
            }
        } else {
            report.failed_count++;
        }
    }

    state->changed_count = 0;
    if (out_report != NULL) *out_report = report;
    return report.failed_count == 0 ? 1 : 0;
}
