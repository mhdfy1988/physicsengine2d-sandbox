#include <direct.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/asset_importer.hpp"
#include "../include/asset_invalidation.hpp"

static int file_exists(const char* path) {
    FILE* fp;
    if (path == NULL) return 0;
    fp = fopen(path, "rb");
    if (fp == NULL) return 0;
    fclose(fp);
    return 1;
}

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

static int batch_contains(const AssetInvalidationBatch* batch, const char* guid) {
    int i;
    if (batch == NULL || guid == NULL) return 0;
    for (i = 0; i < batch->count; i++) {
        if (strcmp(batch->guids[i], guid) == 0) return 1;
    }
    return 0;
}

int regression_test_asset_importer_smoke(void) {
    const char* assets_dir = "_tmp_import_assets";
    const char* cache_dir = "_tmp_import_cache";
    const char* texture_path = "_tmp_import_assets/demo_texture.png";
    const char* font_path = "_tmp_import_assets/demo_font.ttf";
    const char* audio_path = "_tmp_import_assets/demo_audio.wav";
    AssetImportRequest req;
    AssetImportResult tex_a;
    AssetImportResult tex_b;
    AssetImportResult font_r;
    AssetImportResult audio_r;
    char texture_meta_path[ASSET_IMPORTER_MAX_PATH];
    char font_meta_path[ASSET_IMPORTER_MAX_PATH];
    char audio_meta_path[ASSET_IMPORTER_MAX_PATH];

    _mkdir(assets_dir);
    _mkdir(cache_dir);
    if (!write_text_file(texture_path, "texture-bytes-demo") ||
        !write_text_file(font_path, "font-bytes-demo") ||
        !write_text_file(audio_path, "audio-bytes-demo")) {
        printf("[FAIL] failed to create importer smoke inputs\n");
        return 0;
    }

    req.source_path = texture_path;
    req.cache_root = cache_dir;
    req.settings_fingerprint = "profile=default";
    req.parameters = NULL;
    if (!asset_importer_run(&req, &tex_a) || tex_a.kind != ASSET_IMPORT_KIND_TEXTURE || !tex_a.success) {
        printf("[FAIL] texture importer run failed\n");
        return 0;
    }
    if (!asset_importer_run(&req, &tex_b) || strcmp(tex_a.cache_key, tex_b.cache_key) != 0) {
        printf("[FAIL] texture importer determinism failed\n");
        return 0;
    }

    req.source_path = font_path;
    req.parameters = NULL;
    if (!asset_importer_run(&req, &font_r) || font_r.kind != ASSET_IMPORT_KIND_FONT || !font_r.success) {
        printf("[FAIL] font importer run failed\n");
        return 0;
    }

    req.source_path = audio_path;
    req.parameters = NULL;
    if (!asset_importer_run(&req, &audio_r) || audio_r.kind != ASSET_IMPORT_KIND_AUDIO || !audio_r.success) {
        printf("[FAIL] audio importer run failed\n");
        return 0;
    }

    snprintf(texture_meta_path, sizeof(texture_meta_path), "%s.meta", texture_path);
    snprintf(font_meta_path, sizeof(font_meta_path), "%s.meta", font_path);
    snprintf(audio_meta_path, sizeof(audio_meta_path), "%s.meta", audio_path);

    if (!file_exists(texture_meta_path) || !file_exists(tex_a.artifact_path) ||
        !file_exists(font_meta_path) || !file_exists(font_r.artifact_path) ||
        !file_exists(audio_meta_path) || !file_exists(audio_r.artifact_path)) {
        printf("[FAIL] importer outputs missing\n");
        return 0;
    }

    remove(texture_meta_path);
    remove(font_meta_path);
    remove(audio_meta_path);
    remove(tex_a.artifact_path);
    remove(font_r.artifact_path);
    remove(audio_r.artifact_path);
    remove(texture_path);
    remove(font_path);
    remove(audio_path);
    _rmdir(cache_dir);
    _rmdir(assets_dir);

    printf("[PASS] asset importer smoke\n");
    return 1;
}

int regression_test_asset_importer_parameterized_settings(void) {
    const char* assets_dir = "_tmp_import_params_assets";
    const char* cache_dir = "_tmp_import_params_cache";
    const char* texture_path = "_tmp_import_params_assets/demo_texture.png";
    AssetImportRequest req;
    AssetImportResult r_default_a;
    AssetImportResult r_default_b;
    AssetImportResult r_variant;
    AssetImportParameters default_params;
    AssetImportParameters variant_params;
    char meta_path[ASSET_IMPORTER_MAX_PATH];

    _mkdir(assets_dir);
    _mkdir(cache_dir);
    if (!write_text_file(texture_path, "texture-bytes-param-demo")) {
        printf("[FAIL] failed to create parameterized importer input\n");
        return 0;
    }

    req.source_path = texture_path;
    req.cache_root = cache_dir;
    req.settings_fingerprint = NULL;
    req.parameters = &default_params;

    asset_import_parameters_default(ASSET_IMPORT_KIND_TEXTURE, &default_params);
    if (!asset_importer_run(&req, &r_default_a) || !r_default_a.success) {
        printf("[FAIL] default parameterized importer run failed\n");
        return 0;
    }
    if (!asset_importer_run(&req, &r_default_b) || strcmp(r_default_a.cache_key, r_default_b.cache_key) != 0) {
        printf("[FAIL] same importer parameters should produce identical cache key\n");
        return 0;
    }

    variant_params = default_params;
    variant_params.texture_max_size = 1024;
    req.parameters = &variant_params;
    if (!asset_importer_run(&req, &r_variant) || !r_variant.success) {
        printf("[FAIL] variant parameterized importer run failed\n");
        return 0;
    }
    if (strcmp(r_default_a.cache_key, r_variant.cache_key) == 0) {
        printf("[FAIL] different importer parameters should change cache key\n");
        return 0;
    }

    snprintf(meta_path, sizeof(meta_path), "%s.meta", texture_path);
    remove(meta_path);
    remove(r_default_a.artifact_path);
    remove(r_variant.artifact_path);
    remove(texture_path);
    _rmdir(cache_dir);
    _rmdir(assets_dir);

    printf("[PASS] asset importer parameterized settings\n");
    return 1;
}

int regression_test_asset_invalidation_propagation(void) {
    AssetDependencyGraph graph;
    AssetInvalidationBatch batch;
    asset_dependency_graph_init(&graph);
    asset_invalidation_batch_init(&batch);

    if (!asset_dependency_graph_add_dependency(&graph, "asset://scene", "asset://prefab") ||
        !asset_dependency_graph_add_dependency(&graph, "asset://prefab", "asset://texture") ||
        !asset_dependency_graph_add_dependency(&graph, "asset://texture", "asset://shader")) {
        printf("[FAIL] failed to build dependency chain\n");
        return 0;
    }
    if (!asset_invalidation_collect(&graph, "asset://shader", &batch)) {
        printf("[FAIL] invalidation collect failed\n");
        return 0;
    }
    if (batch.count != 4 ||
        !batch_contains(&batch, "asset://shader") ||
        !batch_contains(&batch, "asset://texture") ||
        !batch_contains(&batch, "asset://prefab") ||
        !batch_contains(&batch, "asset://scene")) {
        printf("[FAIL] invalidation propagation mismatch\n");
        return 0;
    }
    if (strcmp(batch.guids[0], "asset://shader") != 0) {
        printf("[FAIL] invalidation root should be first item\n");
        return 0;
    }
    printf("[PASS] asset invalidation propagation\n");
    return 1;
}
