#include <direct.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/asset_pipeline.hpp"
#include "../include/asset_database.hpp"

static int write_text_file(const char* path, const char* text) {
    FILE* fp;
    if (path == NULL || text == NULL) return 0;
    fp = fopen(path, "wb");
    if (fp == NULL) return 0;
    if (fwrite(text, 1, strlen(text), fp) != strlen(text)) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

static int file_exists(const char* path) {
    FILE* fp;
    if (path == NULL) return 0;
    fp = fopen(path, "rb");
    if (fp == NULL) return 0;
    fclose(fp);
    return 1;
}

static int report_contains(const AssetPipelineRunReport* report, const char* guid) {
    int i;
    if (report == NULL || guid == NULL) return 0;
    for (i = 0; i < report->affected_count; i++) {
        if (strcmp(report->affected_guids[i], guid) == 0) return 1;
    }
    return 0;
}

static int imported_contains(const AssetPipelineRunReport* report, const char* guid) {
    int i;
    if (report == NULL || guid == NULL) return 0;
    for (i = 0; i < report->imported_count; i++) {
        if (strcmp(report->imported_guids[i], guid) == 0) return 1;
    }
    return 0;
}

int regression_test_asset_pipeline_incremental_reimport(void) {
    const char* assets_dir = "_tmp_pipeline_assets";
    const char* cache_dir = "_tmp_pipeline_cache";
    const char* texture_path = "_tmp_pipeline_assets/texture.png";
    const char* audio_path = "_tmp_pipeline_assets/sfx.wav";
    char texture_guid[ASSET_DB_MAX_GUID];
    char audio_guid[ASSET_DB_MAX_GUID];
    AssetPipelineState state;
    AssetPipelineRunReport report;
    char texture_meta_path[ASSET_PIPELINE_MAX_PATH];
    char audio_meta_path[ASSET_PIPELINE_MAX_PATH];
    char texture_artifact[ASSET_PIPELINE_MAX_PATH];
    char audio_artifact[ASSET_PIPELINE_MAX_PATH];
    const char* texture_suffix;
    const char* audio_suffix;

    _mkdir(assets_dir);
    _mkdir(cache_dir);
    if (!write_text_file(texture_path, "texture-data") ||
        !write_text_file(audio_path, "audio-data")) {
        printf("[FAIL] failed to prepare pipeline test files\n");
        return 0;
    }

    asset_pipeline_init(&state);
    if (!asset_pipeline_register_source(&state, texture_path, "tex:q=high", texture_guid) ||
        !asset_pipeline_register_source(&state, audio_path, "aud:q=medium", audio_guid)) {
        printf("[FAIL] failed to register pipeline sources\n");
        return 0;
    }
    if (!asset_pipeline_set_dependency(&state, "asset://scene_main", texture_guid) ||
        !asset_pipeline_set_dependency(&state, "asset://scene_main", audio_guid)) {
        printf("[FAIL] failed to set pipeline dependencies\n");
        return 0;
    }

    {
        const char* changed_sources[3];
        changed_sources[0] = texture_path;
        changed_sources[1] = audio_path;
        changed_sources[2] = texture_path;
        if (!asset_pipeline_mark_source_changed_batch(&state, changed_sources, 3) ||
            !asset_pipeline_mark_changed(&state, texture_guid)) {
            printf("[FAIL] failed to mark pipeline changes\n");
            return 0;
        }
    }

    if (state.changed_count != 2) {
        printf("[FAIL] pipeline changed queue dedupe mismatch\n");
        return 0;
    }

    if (!asset_pipeline_run_reimport(&state, cache_dir, &report)) {
        printf("[FAIL] pipeline reimport run failed\n");
        return 0;
    }
    if (report.requested_change_count != 2 || report.affected_count != 3 || report.imported_count != 2 || report.failed_count != 0) {
        printf("[FAIL] pipeline report counts mismatch\n");
        return 0;
    }
    if (!report_contains(&report, texture_guid) || !report_contains(&report, audio_guid) || !report_contains(&report, "asset://scene_main")) {
        printf("[FAIL] pipeline affected guid set mismatch\n");
        return 0;
    }
    if (!imported_contains(&report, texture_guid) || !imported_contains(&report, audio_guid)) {
        printf("[FAIL] pipeline imported guid set mismatch\n");
        return 0;
    }

    snprintf(texture_meta_path, sizeof(texture_meta_path), "%s.meta", texture_path);
    snprintf(audio_meta_path, sizeof(audio_meta_path), "%s.meta", audio_path);
    texture_suffix = (strncmp(texture_guid, "asset://", 8) == 0) ? texture_guid + 8 : texture_guid;
    audio_suffix = (strncmp(audio_guid, "asset://", 8) == 0) ? audio_guid + 8 : audio_guid;
    snprintf(texture_artifact, sizeof(texture_artifact), "%s/%s.asset", cache_dir, texture_suffix);
    snprintf(audio_artifact, sizeof(audio_artifact), "%s/%s.asset", cache_dir, audio_suffix);
    if (!file_exists(texture_meta_path) || !file_exists(audio_meta_path) ||
        !file_exists(texture_artifact) || !file_exists(audio_artifact)) {
        printf("[FAIL] pipeline did not produce expected outputs\n");
        return 0;
    }

    if (!asset_pipeline_run_reimport(&state, cache_dir, &report)) {
        printf("[FAIL] pipeline empty run failed\n");
        return 0;
    }
    if (report.requested_change_count != 0 || report.affected_count != 0 || report.imported_count != 0 || report.failed_count != 0) {
        printf("[FAIL] pipeline did not clear change queue\n");
        return 0;
    }

    remove(texture_meta_path);
    remove(audio_meta_path);
    remove(texture_artifact);
    remove(audio_artifact);
    remove(texture_path);
    remove(audio_path);
    _rmdir(cache_dir);
    _rmdir(assets_dir);

    printf("[PASS] asset pipeline incremental reimport\n");
    return 1;
}

int regression_test_asset_pipeline_meta_parameter_deserialize(void) {
    const char* assets_dir = "_tmp_pipeline_meta_assets";
    const char* cache_dir = "_tmp_pipeline_meta_cache";
    const char* texture_path = "_tmp_pipeline_meta_assets/texture.png";
    char texture_guid[ASSET_DB_MAX_GUID];
    char meta_path[ASSET_PIPELINE_MAX_PATH];
    AssetPipelineState state;
    AssetPipelineRunReport report;
    AssetMeta meta;
    AssetMeta out_meta;
    char artifact_path[ASSET_PIPELINE_MAX_PATH];
    const char* guid_suffix;

    _mkdir(assets_dir);
    _mkdir(cache_dir);
    if (!write_text_file(texture_path, "texture-meta-parameter-demo")) {
        printf("[FAIL] failed to create meta-parameter source file\n");
        return 0;
    }

    asset_meta_init(&meta);
    if (!asset_meta_make_guid_from_path(texture_path, meta.guid)) {
        printf("[FAIL] failed to create meta guid\n");
        return 0;
    }
    strcpy(meta.asset_type, "texture");
    strcpy(meta.importer_id, "texture-importer");
    meta.importer_version = 1;
    strcpy(meta.source_hash, "fnv64:seed");
    strcpy(meta.import_settings_hash, "fnv64:seed");
    meta.has_parameters = 1;
    meta.texture_srgb = 0;
    meta.texture_max_size = 1024;
    meta.texture_generate_mips = 0;
    snprintf(meta_path, sizeof(meta_path), "%s.meta", texture_path);
    if (!asset_meta_save(&meta, meta_path)) {
        printf("[FAIL] failed to write seed meta file\n");
        return 0;
    }

    asset_pipeline_init(&state);
    if (!asset_pipeline_register_source(&state, texture_path, "legacy-fallback", texture_guid)) {
        printf("[FAIL] failed to register source for meta-parameter path\n");
        return 0;
    }
    if (!state.sources[0].use_structured_parameters ||
        state.sources[0].parameters.texture_srgb != 0 ||
        state.sources[0].parameters.texture_max_size != 1024 ||
        state.sources[0].parameters.texture_generate_mips != 0) {
        printf("[FAIL] pipeline did not deserialize parameters from .meta\n");
        return 0;
    }

    if (!asset_pipeline_mark_source_changed(&state, texture_path) ||
        !asset_pipeline_run_reimport(&state, cache_dir, &report) ||
        report.imported_count != 1 ||
        report.failed_count != 0) {
        printf("[FAIL] pipeline reimport using meta parameters failed\n");
        return 0;
    }
    if (!asset_meta_load(meta_path, &out_meta) ||
        !out_meta.has_parameters ||
        out_meta.texture_srgb != 0 ||
        out_meta.texture_max_size != 1024 ||
        out_meta.texture_generate_mips != 0) {
        printf("[FAIL] importer did not persist structured parameters from pipeline source\n");
        return 0;
    }

    guid_suffix = (strncmp(texture_guid, "asset://", 8) == 0) ? texture_guid + 8 : texture_guid;
    snprintf(artifact_path, sizeof(artifact_path), "%s/%s.asset", cache_dir, guid_suffix);
    remove(meta_path);
    remove(artifact_path);
    remove(texture_path);
    _rmdir(cache_dir);
    _rmdir(assets_dir);

    printf("[PASS] asset pipeline meta parameter deserialize\n");
    return 1;
}

int regression_test_asset_pipeline_missing_source_failure(void) {
    const char* assets_dir = "_tmp_pipeline_missing";
    const char* cache_dir = "_tmp_pipeline_missing_cache";
    const char* texture_path = "_tmp_pipeline_missing/missing.png";
    char texture_guid[ASSET_DB_MAX_GUID];
    AssetPipelineState state;
    AssetPipelineRunReport report;

    _mkdir(assets_dir);
    _mkdir(cache_dir);
    if (!write_text_file(texture_path, "missing-source-before-delete")) {
        printf("[FAIL] failed to write pipeline missing-source input\n");
        return 0;
    }

    asset_pipeline_init(&state);
    if (!asset_pipeline_register_source(&state, texture_path, "tex:default", texture_guid)) {
        printf("[FAIL] failed to register missing-source pipeline entry\n");
        return 0;
    }
    remove(texture_path);
    if (!asset_pipeline_mark_changed(&state, texture_guid)) {
        printf("[FAIL] failed to mark missing-source change\n");
        return 0;
    }
    if (asset_pipeline_run_reimport(&state, cache_dir, &report)) {
        printf("[FAIL] pipeline should fail when source file is missing\n");
        return 0;
    }
    if (report.requested_change_count != 1 || report.affected_count != 1 || report.imported_count != 0 || report.failed_count != 1) {
        printf("[FAIL] missing-source pipeline report mismatch\n");
        return 0;
    }

    _rmdir(cache_dir);
    _rmdir(assets_dir);
    printf("[PASS] asset pipeline missing-source failure handling\n");
    return 1;
}
