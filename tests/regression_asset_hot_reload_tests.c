#include <direct.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/asset_hot_reload.h"
#include "../include/asset_watch.h"
#include "../include/asset_database.h"
#include "../include/asset_fs_poll.h"

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

static int read_text_file(const char* path, char* out_text, size_t out_cap) {
    FILE* fp;
    size_t n;
    if (path == NULL || out_text == NULL || out_cap == 0) return 0;
    fp = fopen(path, "rb");
    if (fp == NULL) return 0;
    n = fread(out_text, 1, out_cap - 1, fp);
    if (ferror(fp)) {
        fclose(fp);
        return 0;
    }
    out_text[n] = '\0';
    fclose(fp);
    return 1;
}

static int batch_contains(const AssetWatchBatch* batch, const char* source_path) {
    int i;
    if (batch == NULL || source_path == NULL) return 0;
    for (i = 0; i < batch->count; i++) {
        if (strcmp(batch->source_paths[i], source_path) == 0) return 1;
    }
    return 0;
}

static int report_contains_guid(const AssetPipelineRunReport* report, const char* guid) {
    int i;
    if (report == NULL || guid == NULL) return 0;
    for (i = 0; i < report->affected_count; i++) {
        if (strcmp(report->affected_guids[i], guid) == 0) return 1;
    }
    return 0;
}

int regression_test_asset_watch_debounce_batch(void) {
    AssetWatchState watch;
    AssetWatchBatch batch;
    asset_watch_init(&watch, 100);
    asset_watch_batch_init(&batch);

    if (!asset_watch_push_change(&watch, "Assets/a.png", 1000) ||
        !asset_watch_push_change(&watch, "Assets/a.png", 1040) ||
        !asset_watch_push_change(&watch, "Assets/b.wav", 1120)) {
        printf("[FAIL] watch push change failed\n");
        return 0;
    }
    if (!asset_watch_collect_ready(&watch, 1135, &batch) || batch.count != 0) {
        printf("[FAIL] watch debounce early flush mismatch\n");
        return 0;
    }
    if (!asset_watch_collect_ready(&watch, 1141, &batch) || batch.count != 1 || !batch_contains(&batch, "Assets/a.png")) {
        printf("[FAIL] watch debounce first flush mismatch\n");
        return 0;
    }
    if (!asset_watch_push_change(&watch, "Assets/a.png", 1150)) {
        printf("[FAIL] watch re-push failed\n");
        return 0;
    }
    if (!asset_watch_collect_ready(&watch, 1200, &batch) || batch.count != 0) {
        printf("[FAIL] watch debounce mid flush mismatch\n");
        return 0;
    }
    if (!asset_watch_collect_ready(&watch, 1251, &batch) || batch.count != 2 ||
        !batch_contains(&batch, "Assets/a.png") || !batch_contains(&batch, "Assets/b.wav")) {
        printf("[FAIL] watch debounce final flush mismatch\n");
        return 0;
    }
    printf("[PASS] asset watch debounce batch\n");
    return 1;
}

int regression_test_asset_hot_reload_service_pipeline_bridge(void) {
    const char* assets_dir = "_tmp_hot_assets";
    const char* cache_dir = "_tmp_hot_cache";
    const char* texture_path = "_tmp_hot_assets/hot_texture.png";
    const char* audio_path = "_tmp_hot_assets/hot_audio.wav";
    char texture_guid[ASSET_DB_MAX_GUID];
    char audio_guid[ASSET_DB_MAX_GUID];
    char texture_meta_path[ASSET_WATCH_MAX_PATH];
    char audio_meta_path[ASSET_WATCH_MAX_PATH];
    char texture_artifact[ASSET_WATCH_MAX_PATH];
    char audio_artifact[ASSET_WATCH_MAX_PATH];
    const char* tex_suffix;
    const char* aud_suffix;
    AssetHotReloadService service;
    AssetHotReloadTickReport report;

    _mkdir(assets_dir);
    _mkdir(cache_dir);
    if (!write_text_file(texture_path, "hot-texture-data") ||
        !write_text_file(audio_path, "hot-audio-data")) {
        printf("[FAIL] failed to prepare hot-reload source files\n");
        return 0;
    }

    asset_hot_reload_service_init(&service, 100);
    if (!asset_hot_reload_register_source(&service, texture_path, "texture:default", texture_guid) ||
        !asset_hot_reload_register_source(&service, audio_path, "audio:default", audio_guid)) {
        printf("[FAIL] failed to register hot-reload sources\n");
        return 0;
    }
    if (!asset_hot_reload_set_dependency(&service, "asset://scene_main", texture_guid)) {
        printf("[FAIL] failed to set hot-reload dependency\n");
        return 0;
    }

    if (!asset_hot_reload_on_file_changed(&service, texture_path, 1000) ||
        !asset_hot_reload_on_file_changed(&service, audio_path, 1010)) {
        printf("[FAIL] failed to push hot-reload fs events\n");
        return 0;
    }
    if (!asset_hot_reload_tick(&service, 1050, cache_dir, &report) ||
        report.pipeline_ran != 0 ||
        report.ready_batch.count != 0) {
        printf("[FAIL] hot-reload early tick mismatch\n");
        return 0;
    }
    if (!asset_hot_reload_tick(&service, 1120, cache_dir, &report) ||
        report.pipeline_ran != 1 ||
        report.ready_batch.count != 2 ||
        report.pipeline_report.imported_count != 2 ||
        report.pipeline_report.failed_count != 0 ||
        !report_contains_guid(&report.pipeline_report, texture_guid) ||
        !report_contains_guid(&report.pipeline_report, audio_guid) ||
        !report_contains_guid(&report.pipeline_report, "asset://scene_main")) {
        printf("[FAIL] hot-reload main tick mismatch\n");
        return 0;
    }

    if (!asset_hot_reload_on_file_changed(&service, texture_path, 1130) ||
        !asset_hot_reload_on_file_changed(&service, texture_path, 1150)) {
        printf("[FAIL] failed to push second hot-reload events\n");
        return 0;
    }
    if (!asset_hot_reload_tick(&service, 1240, cache_dir, &report) ||
        report.pipeline_ran != 0 ||
        report.ready_batch.count != 0) {
        printf("[FAIL] hot-reload second early tick mismatch\n");
        return 0;
    }
    if (!asset_hot_reload_tick(&service, 1251, cache_dir, &report) ||
        report.pipeline_ran != 1 ||
        report.ready_batch.count != 1 ||
        report.pipeline_report.imported_count != 1 ||
        report.pipeline_report.failed_count != 0) {
        printf("[FAIL] hot-reload second flush tick mismatch\n");
        return 0;
    }

    snprintf(texture_meta_path, sizeof(texture_meta_path), "%s.meta", texture_path);
    snprintf(audio_meta_path, sizeof(audio_meta_path), "%s.meta", audio_path);
    tex_suffix = (strncmp(texture_guid, "asset://", 8) == 0) ? texture_guid + 8 : texture_guid;
    aud_suffix = (strncmp(audio_guid, "asset://", 8) == 0) ? audio_guid + 8 : audio_guid;
    snprintf(texture_artifact, sizeof(texture_artifact), "%s/%s.asset", cache_dir, tex_suffix);
    snprintf(audio_artifact, sizeof(audio_artifact), "%s/%s.asset", cache_dir, aud_suffix);

    remove(texture_meta_path);
    remove(audio_meta_path);
    remove(texture_artifact);
    remove(audio_artifact);
    remove(texture_path);
    remove(audio_path);
    _rmdir(cache_dir);
    _rmdir(assets_dir);

    printf("[PASS] asset hot-reload service pipeline bridge\n");
    return 1;
}

int regression_test_asset_fs_poll_source_bridge(void) {
    const char* assets_dir = "_tmp_poll_assets";
    const char* cache_dir = "_tmp_poll_cache";
    const char* texture_path = "_tmp_poll_assets/poll_texture.png";
    char texture_guid[ASSET_DB_MAX_GUID];
    char meta_path[ASSET_WATCH_MAX_PATH];
    char artifact_path[ASSET_WATCH_MAX_PATH];
    const char* guid_suffix;
    AssetHotReloadService service;
    AssetHotReloadTickReport report;
    AssetFsPollState poll;
    int change_count = 0;

    _mkdir(assets_dir);
    _mkdir(cache_dir);
    if (!write_text_file(texture_path, "poll-data-v1")) {
        printf("[FAIL] failed to create poll source file\n");
        return 0;
    }

    asset_hot_reload_service_init(&service, 0);
    if (!asset_hot_reload_register_source(&service, texture_path, "poll:default", texture_guid)) {
        printf("[FAIL] failed to register poll source\n");
        return 0;
    }
    asset_fs_poll_init(&poll);
    if (!asset_fs_poll_watch_path(&poll, texture_path)) {
        printf("[FAIL] failed to watch poll path\n");
        return 0;
    }
    if (!asset_fs_poll_scan(&poll, &service, 1000, &change_count) || change_count != 0) {
        printf("[FAIL] poll baseline scan mismatch\n");
        return 0;
    }

    if (!write_text_file(texture_path, "poll-data-v2-expanded")) {
        printf("[FAIL] failed to update poll source file\n");
        return 0;
    }
    if (!asset_fs_poll_scan(&poll, &service, 1010, &change_count) || change_count != 1) {
        printf("[FAIL] poll scan did not detect changed file\n");
        return 0;
    }
    if (!asset_hot_reload_tick(&service, 1010, cache_dir, &report) ||
        report.pipeline_ran != 1 ||
        report.ready_batch.count != 1 ||
        report.pipeline_report.imported_count != 1 ||
        report.pipeline_report.failed_count != 0) {
        printf("[FAIL] poll->hot-reload tick mismatch\n");
        return 0;
    }

    snprintf(meta_path, sizeof(meta_path), "%s.meta", texture_path);
    guid_suffix = (strncmp(texture_guid, "asset://", 8) == 0) ? texture_guid + 8 : texture_guid;
    snprintf(artifact_path, sizeof(artifact_path), "%s/%s.asset", cache_dir, guid_suffix);
    remove(meta_path);
    remove(artifact_path);
    remove(texture_path);
    _rmdir(cache_dir);
    _rmdir(assets_dir);

    printf("[PASS] asset fs poll source bridge\n");
    return 1;
}

int regression_test_asset_hot_reload_missing_source_failure(void) {
    const char* cache_dir = "_tmp_hot_missing_cache";
    const char* missing_source = "_tmp_hot_missing_assets/missing_texture.png";
    AssetHotReloadService service;
    AssetHotReloadTickReport report;
    char guid[ASSET_DB_MAX_GUID];

    _mkdir(cache_dir);
    asset_hot_reload_service_init(&service, 0);
    if (!asset_hot_reload_register_source(&service, missing_source, "missing:default", guid)) {
        printf("[FAIL] failed to register missing-source hot-reload entry\n");
        _rmdir(cache_dir);
        return 0;
    }
    if (!asset_hot_reload_on_file_changed(&service, missing_source, 1000)) {
        printf("[FAIL] failed to queue missing-source hot-reload change\n");
        _rmdir(cache_dir);
        return 0;
    }
    if (asset_hot_reload_tick(&service, 1000, cache_dir, &report) != 0 ||
        report.pipeline_ran != 1 ||
        report.ready_batch.count != 1 ||
        report.pipeline_report.imported_count != 0 ||
        report.pipeline_report.failed_count != 1 ||
        !report_contains_guid(&report.pipeline_report, guid)) {
        printf("[FAIL] missing-source hot-reload failure contract mismatch\n");
        _rmdir(cache_dir);
        return 0;
    }

    _rmdir(cache_dir);
    printf("[PASS] asset hot-reload missing-source failure\n");
    return 1;
}

int regression_test_asset_hot_reload_failure_keeps_previous_artifact(void) {
    const char* assets_dir = "_tmp_hot_fallback_assets";
    const char* cache_dir = "_tmp_hot_fallback_cache";
    const char* texture_path = "_tmp_hot_fallback_assets/fallback_texture.png";
    char guid[ASSET_DB_MAX_GUID];
    char meta_path[ASSET_WATCH_MAX_PATH];
    char artifact_path[ASSET_WATCH_MAX_PATH];
    const char* guid_suffix;
    char artifact_before[512];
    char artifact_after[512];
    AssetHotReloadService service;
    AssetHotReloadTickReport report;

    _mkdir(assets_dir);
    _mkdir(cache_dir);
    if (!write_text_file(texture_path, "fallback-v1")) {
        printf("[FAIL] failed to prepare fallback source file\n");
        return 0;
    }

    asset_hot_reload_service_init(&service, 0);
    if (!asset_hot_reload_register_source(&service, texture_path, "fallback:default", guid)) {
        printf("[FAIL] failed to register fallback source\n");
        remove(texture_path);
        _rmdir(cache_dir);
        _rmdir(assets_dir);
        return 0;
    }
    if (!asset_hot_reload_on_file_changed(&service, texture_path, 1000) ||
        !asset_hot_reload_tick(&service, 1000, cache_dir, &report) ||
        report.pipeline_ran != 1 ||
        report.pipeline_report.imported_count != 1 ||
        report.pipeline_report.failed_count != 0) {
        printf("[FAIL] fallback baseline import failed\n");
        remove(texture_path);
        _rmdir(cache_dir);
        _rmdir(assets_dir);
        return 0;
    }

    guid_suffix = (strncmp(guid, "asset://", 8) == 0) ? guid + 8 : guid;
    snprintf(meta_path, sizeof(meta_path), "%s.meta", texture_path);
    snprintf(artifact_path, sizeof(artifact_path), "%s/%s.asset", cache_dir, guid_suffix);
    if (!read_text_file(artifact_path, artifact_before, sizeof(artifact_before))) {
        printf("[FAIL] failed to read baseline artifact\n");
        remove(meta_path);
        remove(texture_path);
        _rmdir(cache_dir);
        _rmdir(assets_dir);
        return 0;
    }

    remove(texture_path);
    if (!asset_hot_reload_on_file_changed(&service, texture_path, 1010) ||
        asset_hot_reload_tick(&service, 1010, cache_dir, &report) != 0 ||
        report.pipeline_ran != 1 ||
        report.pipeline_report.imported_count != 0 ||
        report.pipeline_report.failed_count != 1) {
        printf("[FAIL] fallback missing-source reimport contract mismatch\n");
        remove(meta_path);
        remove(artifact_path);
        _rmdir(cache_dir);
        _rmdir(assets_dir);
        return 0;
    }
    if (!read_text_file(artifact_path, artifact_after, sizeof(artifact_after)) ||
        strcmp(artifact_before, artifact_after) != 0) {
        printf("[FAIL] fallback artifact was not preserved after failed reimport\n");
        remove(meta_path);
        remove(artifact_path);
        _rmdir(cache_dir);
        _rmdir(assets_dir);
        return 0;
    }

    remove(meta_path);
    remove(artifact_path);
    _rmdir(cache_dir);
    _rmdir(assets_dir);
    printf("[PASS] asset hot-reload failure keeps previous artifact\n");
    return 1;
}

int regression_test_asset_fs_poll_rewatch_does_not_swallow_change(void) {
    const char* assets_dir = "_tmp_poll_rewatch_assets";
    const char* texture_path = "_tmp_poll_rewatch_assets/rewatch_texture.png";
    AssetHotReloadService service;
    AssetFsPollState poll;
    AssetHotReloadTickReport report;
    int change_count = 0;

    _mkdir(assets_dir);
    if (!write_text_file(texture_path, "rewatch-v1")) {
        printf("[FAIL] failed to create rewatch source file\n");
        return 0;
    }

    asset_hot_reload_service_init(&service, 0);
    asset_fs_poll_init(&poll);
    if (!asset_fs_poll_watch_path(&poll, texture_path)) {
        printf("[FAIL] failed to register first watch\n");
        remove(texture_path);
        _rmdir(assets_dir);
        return 0;
    }
    if (!asset_fs_poll_scan(&poll, &service, 1000, &change_count) || change_count != 0) {
        printf("[FAIL] rewatch baseline scan mismatch\n");
        remove(texture_path);
        _rmdir(assets_dir);
        return 0;
    }

    if (!write_text_file(texture_path, "rewatch-v2-expanded")) {
        printf("[FAIL] failed to mutate rewatch source file\n");
        remove(texture_path);
        _rmdir(assets_dir);
        return 0;
    }
    if (!asset_fs_poll_watch_path(&poll, texture_path)) {
        printf("[FAIL] failed to re-register watch\n");
        remove(texture_path);
        _rmdir(assets_dir);
        return 0;
    }
    if (!asset_fs_poll_scan(&poll, &service, 1010, &change_count) || change_count != 1) {
        printf("[FAIL] rewatch swallowed changed file event\n");
        remove(texture_path);
        _rmdir(assets_dir);
        return 0;
    }
    if (!asset_hot_reload_tick(&service, 1010, "_tmp_poll_rewatch_cache", &report) ||
        report.pipeline_ran != 1 ||
        report.ready_batch.count != 1) {
        printf("[FAIL] rewatch hot-reload tick mismatch\n");
        remove(texture_path);
        _rmdir(assets_dir);
        return 0;
    }

    remove(texture_path);
    _rmdir(assets_dir);
    printf("[PASS] asset fs poll rewatch does not swallow change\n");
    return 1;
}
