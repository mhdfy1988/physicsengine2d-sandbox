#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../include/physics_content/asset_database.hpp"

static int file_content_equals(const char* a_path, const char* b_path) {
    FILE* fa = fopen(a_path, "r");
    FILE* fb = fopen(b_path, "r");
    char la[512];
    char lb[512];
    int ok = 1;
    if (fa == NULL || fb == NULL) {
        if (fa != NULL) fclose(fa);
        if (fb != NULL) fclose(fb);
        return 0;
    }
    for (;;) {
        char* pa = fgets(la, sizeof(la), fa);
        char* pb = fgets(lb, sizeof(lb), fb);
        size_t na;
        size_t nb;
        if (pa == NULL && pb == NULL) break;
        if (pa == NULL || pb == NULL) {
            ok = 0;
            break;
        }
        na = strlen(la);
        while (na > 0 && (la[na - 1] == '\n' || la[na - 1] == '\r')) {
            la[na - 1] = '\0';
            na--;
        }
        nb = strlen(lb);
        while (nb > 0 && (lb[nb - 1] == '\n' || lb[nb - 1] == '\r')) {
            lb[nb - 1] = '\0';
            nb--;
        }
        if (strcmp(la, lb) != 0) {
            ok = 0;
            break;
        }
    }
    fclose(fa);
    fclose(fb);
    return ok;
}

int regression_test_asset_meta_roundtrip(void) {
    const char* file_a = "_tmp_asset_meta_a.meta";
    const char* file_b = "_tmp_asset_meta_b.meta";
    AssetMeta input;
    AssetMeta loaded;
    char cache_key[ASSET_DB_MAX_HASH];
    char guid_a[ASSET_DB_MAX_GUID];
    char guid_b[ASSET_DB_MAX_GUID];

    asset_meta_init(&input);
    if (!asset_meta_make_guid_from_path("Assets/textures/hero.png", input.guid)) {
        printf("[FAIL] failed to generate asset guid\n");
        return 0;
    }
    strcpy(input.asset_type, "texture");
    strcpy(input.importer_id, "texture-importer");
    input.importer_version = 3;
    strcpy(input.source_hash, "sha256:source_hash_demo");
    strcpy(input.import_settings_hash, "sha256:settings_hash_demo");
    input.dependency_count = 3;
    strcpy(input.dependencies[0], "asset://bbbb");
    strcpy(input.dependencies[1], "asset://aaaa");
    strcpy(input.dependencies[2], "asset://cccc");

    if (!asset_meta_save(&input, file_a)) {
        printf("[FAIL] failed to save asset meta source\n");
        return 0;
    }
    if (!asset_meta_load(file_a, &loaded)) {
        printf("[FAIL] failed to load asset meta source\n");
        remove(file_a);
        return 0;
    }
    if (loaded.dependency_count != 3 ||
        strcmp(loaded.dependencies[0], "asset://aaaa") != 0 ||
        strcmp(loaded.dependencies[1], "asset://bbbb") != 0 ||
        strcmp(loaded.dependencies[2], "asset://cccc") != 0) {
        printf("[FAIL] asset dependencies not stable-sorted\n");
        remove(file_a);
        return 0;
    }
    if (!asset_meta_make_cache_key(&loaded, cache_key) || strncmp(cache_key, "cache://", 8) != 0) {
        printf("[FAIL] failed to create cache key\n");
        remove(file_a);
        return 0;
    }
    if (!asset_meta_save(&loaded, file_b)) {
        printf("[FAIL] failed to save asset meta target\n");
        remove(file_a);
        return 0;
    }
    if (!file_content_equals(file_a, file_b)) {
        printf("[FAIL] asset meta roundtrip not deterministic\n");
        remove(file_a);
        remove(file_b);
        return 0;
    }

    if (!asset_meta_make_guid_from_path("Assets/textures/hero.png", guid_a) ||
        !asset_meta_make_guid_from_path("Assets/textures/hero.png", guid_b) ||
        strcmp(guid_a, guid_b) != 0) {
        printf("[FAIL] asset guid generation is not deterministic\n");
        remove(file_a);
        remove(file_b);
        return 0;
    }
    if (!asset_meta_is_valid_guid(guid_a) ||
        !asset_meta_is_valid_guid("asset://scene_main") ||
        asset_meta_is_valid_guid("asset://") ||
        asset_meta_is_valid_guid("asset:///bad") ||
        asset_meta_is_valid_guid("asset://bad/path") ||
        asset_meta_is_valid_guid("Assets/textures/hero.png")) {
        printf("[FAIL] asset guid validation rules mismatch\n");
        remove(file_a);
        remove(file_b);
        return 0;
    }

    remove(file_a);
    remove(file_b);
    printf("[PASS] asset meta roundtrip determinism\n");
    return 1;
}

int regression_test_asset_dependency_graph(void) {
    AssetDependencyGraph graph;
    asset_dependency_graph_init(&graph);

    if (!asset_dependency_graph_add_asset(&graph, "asset://scene_a")) {
        printf("[FAIL] add asset scene_a failed\n");
        return 0;
    }
    if (!asset_dependency_graph_add_dependency(&graph, "asset://scene_a", "asset://prefab_enemy")) {
        printf("[FAIL] add dependency scene->prefab failed\n");
        return 0;
    }
    if (!asset_dependency_graph_add_dependency(&graph, "asset://prefab_enemy", "asset://texture_enemy")) {
        printf("[FAIL] add dependency prefab->texture failed\n");
        return 0;
    }
    if (!asset_dependency_graph_add_dependency(&graph, "asset://texture_enemy", "asset://shader_default")) {
        printf("[FAIL] add dependency texture->shader failed\n");
        return 0;
    }
    if (!asset_dependency_graph_depends_on(&graph, "asset://scene_a", "asset://shader_default")) {
        printf("[FAIL] transitive dependency query failed\n");
        return 0;
    }
    if (asset_dependency_graph_depends_on(&graph, "asset://shader_default", "asset://scene_a")) {
        printf("[FAIL] inverse dependency should not hold\n");
        return 0;
    }
    printf("[PASS] asset dependency graph query\n");
    return 1;
}
