#include "asset_database.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void asset_db_copy_text(char* out, int out_cap, const char* src) {
    if (out == NULL || out_cap <= 0) return;
    if (src == NULL) {
        out[0] = '\0';
        return;
    }
    strncpy(out, src, (size_t)(out_cap - 1));
    out[out_cap - 1] = '\0';
}

static int asset_db_parse_int(const char* text, int* out_value) {
    char* endptr = NULL;
    long v;
    if (text == NULL || out_value == NULL) return 0;
    errno = 0;
    v = strtol(text, &endptr, 10);
    if (errno != 0 || endptr == text || (endptr != NULL && *endptr != '\0')) return 0;
    if (v < -2147483647L - 1L || v > 2147483647L) return 0;
    *out_value = (int)v;
    return 1;
}

static void asset_db_trim_line(char* line) {
    size_t n;
    if (line == NULL) return;
    n = strlen(line);
    while (n > 0 && (line[n - 1] == '\n' || line[n - 1] == '\r')) {
        line[n - 1] = '\0';
        n--;
    }
}

static int asset_db_split_pipe(char* line, char* tokens[], int max_tokens) {
    int count = 0;
    char* p = line;
    if (line == NULL || tokens == NULL || max_tokens <= 0) return 0;
    tokens[count++] = p;
    while (*p != '\0') {
        if (*p == '|') {
            *p = '\0';
            if (count < max_tokens) tokens[count++] = p + 1;
        }
        p++;
    }
    return count;
}

static int asset_db_str_cmp(const void* lhs, const void* rhs) {
    const char* const* a = (const char* const*)lhs;
    const char* const* b = (const char* const*)rhs;
    return strcmp(*a, *b);
}

static unsigned long long asset_db_fnv1a64_append(unsigned long long hash, const char* text) {
    size_t i;
    if (text == NULL) return hash;
    for (i = 0; text[i] != '\0'; i++) {
        hash ^= (unsigned long long)(unsigned char)text[i];
        hash *= 1099511628211ull;
    }
    return hash;
}

static int asset_db_is_guid_char(char c) {
    if (c >= 'a' && c <= 'z') return 1;
    if (c >= 'A' && c <= 'Z') return 1;
    if (c >= '0' && c <= '9') return 1;
    if (c == '_' || c == '-' || c == '.' || c == ':') return 1;
    return 0;
}

static int asset_db_find_asset_index(const AssetDependencyGraph* graph, const char* guid) {
    int i;
    if (graph == NULL || guid == NULL) return -1;
    for (i = 0; i < graph->asset_count; i++) {
        if (strcmp(graph->assets[i], guid) == 0) return i;
    }
    return -1;
}

void asset_meta_init(AssetMeta* meta) {
    int i;
    if (meta == NULL) return;
    memset(meta, 0, sizeof(*meta));
    asset_db_copy_text(meta->guid, ASSET_DB_MAX_GUID, "asset://unknown");
    asset_db_copy_text(meta->asset_type, ASSET_DB_MAX_TYPE, "unknown");
    asset_db_copy_text(meta->importer_id, ASSET_DB_MAX_IMPORTER, "none");
    meta->importer_version = 1;
    asset_db_copy_text(meta->source_hash, ASSET_DB_MAX_HASH, "none");
    asset_db_copy_text(meta->import_settings_hash, ASSET_DB_MAX_HASH, "none");
    meta->has_parameters = 0;
    meta->texture_srgb = 1;
    meta->texture_max_size = 4096;
    meta->texture_generate_mips = 1;
    meta->font_pixel_height = 32;
    meta->font_hinting = 1;
    meta->audio_bitrate_kbps = 192;
    meta->audio_normalize = 0;
    for (i = 0; i < ASSET_DB_MAX_DEPENDENCIES; i++) {
        meta->dependencies[i][0] = '\0';
    }
}

int asset_meta_make_guid_from_path(const char* path, char out_guid[ASSET_DB_MAX_GUID]) {
    unsigned long long hash = 1469598103934665603ull;
    if (path == NULL || out_guid == NULL) return 0;
    hash = asset_db_fnv1a64_append(hash, path);
    snprintf(out_guid, ASSET_DB_MAX_GUID, "asset://%016llx", (unsigned long long)hash);
    return 1;
}

int asset_meta_is_valid_guid(const char* guid) {
    const char* p;
    size_t len;
    if (guid == NULL) return 0;
    len = strlen(guid);
    if (len <= 8 || len >= ASSET_DB_MAX_GUID) return 0;
    if (strncmp(guid, "asset://", 8) != 0) return 0;
    p = guid + 8;
    while (*p != '\0') {
        if (!asset_db_is_guid_char(*p)) return 0;
        p++;
    }
    return 1;
}

int asset_meta_make_cache_key(const AssetMeta* meta, char out_key[ASSET_DB_MAX_HASH]) {
    unsigned long long hash = 1469598103934665603ull;
    char version_buf[32];
    if (meta == NULL || out_key == NULL) return 0;
    hash = asset_db_fnv1a64_append(hash, meta->source_hash);
    hash = asset_db_fnv1a64_append(hash, "|");
    hash = asset_db_fnv1a64_append(hash, meta->importer_id);
    hash = asset_db_fnv1a64_append(hash, "|");
    snprintf(version_buf, sizeof(version_buf), "%d", meta->importer_version);
    hash = asset_db_fnv1a64_append(hash, version_buf);
    hash = asset_db_fnv1a64_append(hash, "|");
    hash = asset_db_fnv1a64_append(hash, meta->import_settings_hash);
    snprintf(out_key, ASSET_DB_MAX_HASH, "cache://%016llx", (unsigned long long)hash);
    return 1;
}

int asset_meta_save(const AssetMeta* meta, const char* path) {
    FILE* fp;
    int dep_count;
    int i;
    const char* dep_ptrs[ASSET_DB_MAX_DEPENDENCIES];
    if (meta == NULL || path == NULL) return 0;
    fp = fopen(path, "w");
    if (fp == NULL) return 0;

    dep_count = meta->dependency_count;
    if (dep_count < 0) dep_count = 0;
    if (dep_count > ASSET_DB_MAX_DEPENDENCIES) dep_count = ASSET_DB_MAX_DEPENDENCIES;
    for (i = 0; i < dep_count; i++) dep_ptrs[i] = meta->dependencies[i];
    qsort(dep_ptrs, (size_t)dep_count, sizeof(dep_ptrs[0]), asset_db_str_cmp);

    fprintf(fp, "guid|%s\n", (meta->guid[0] != '\0') ? meta->guid : "asset://unknown");
    fprintf(fp, "asset_type|%s\n", (meta->asset_type[0] != '\0') ? meta->asset_type : "unknown");
    fprintf(fp, "importer_id|%s\n", (meta->importer_id[0] != '\0') ? meta->importer_id : "none");
    fprintf(fp, "importer_version|%d\n", meta->importer_version);
    fprintf(fp, "source_hash|%s\n", (meta->source_hash[0] != '\0') ? meta->source_hash : "none");
    fprintf(fp, "import_settings_hash|%s\n", (meta->import_settings_hash[0] != '\0') ? meta->import_settings_hash : "none");
    fprintf(fp, "has_parameters|%d\n", meta->has_parameters ? 1 : 0);
    fprintf(fp, "param_texture_srgb|%d\n", meta->texture_srgb);
    fprintf(fp, "param_texture_max_size|%d\n", meta->texture_max_size);
    fprintf(fp, "param_texture_generate_mips|%d\n", meta->texture_generate_mips);
    fprintf(fp, "param_font_pixel_height|%d\n", meta->font_pixel_height);
    fprintf(fp, "param_font_hinting|%d\n", meta->font_hinting);
    fprintf(fp, "param_audio_bitrate_kbps|%d\n", meta->audio_bitrate_kbps);
    fprintf(fp, "param_audio_normalize|%d\n", meta->audio_normalize);
    fprintf(fp, "dependency_count|%d\n", dep_count);
    for (i = 0; i < dep_count; i++) {
        fprintf(fp, "dependency|%s\n", dep_ptrs[i]);
    }
    if (fclose(fp) != 0) return 0;
    return 1;
}

int asset_meta_load(const char* path, AssetMeta* out_meta) {
    FILE* fp;
    char line[512];
    int expected_deps = -1;
    int got_guid = 0;
    int got_type = 0;
    int got_importer = 0;
    int got_version = 0;
    int got_source_hash = 0;
    int got_settings_hash = 0;
    AssetMeta meta;

    if (path == NULL || out_meta == NULL) return 0;
    fp = fopen(path, "r");
    if (fp == NULL) return 0;

    asset_meta_init(&meta);

    while (fgets(line, sizeof(line), fp) != NULL) {
        char* tokens[4];
        int token_count;
        asset_db_trim_line(line);
        if (line[0] == '\0' || line[0] == '#') continue;
        token_count = asset_db_split_pipe(line, tokens, (int)(sizeof(tokens) / sizeof(tokens[0])));
        if (token_count <= 0) {
            fclose(fp);
            return 0;
        }
        if (strcmp(tokens[0], "guid") == 0) {
            if (token_count != 2 || tokens[1][0] == '\0') {
                fclose(fp);
                return 0;
            }
            asset_db_copy_text(meta.guid, ASSET_DB_MAX_GUID, tokens[1]);
            got_guid = 1;
        } else if (strcmp(tokens[0], "asset_type") == 0) {
            if (token_count != 2 || tokens[1][0] == '\0') {
                fclose(fp);
                return 0;
            }
            asset_db_copy_text(meta.asset_type, ASSET_DB_MAX_TYPE, tokens[1]);
            got_type = 1;
        } else if (strcmp(tokens[0], "importer_id") == 0) {
            if (token_count != 2 || tokens[1][0] == '\0') {
                fclose(fp);
                return 0;
            }
            asset_db_copy_text(meta.importer_id, ASSET_DB_MAX_IMPORTER, tokens[1]);
            got_importer = 1;
        } else if (strcmp(tokens[0], "importer_version") == 0) {
            if (token_count != 2 || !asset_db_parse_int(tokens[1], &meta.importer_version)) {
                fclose(fp);
                return 0;
            }
            got_version = 1;
        } else if (strcmp(tokens[0], "source_hash") == 0) {
            if (token_count != 2) {
                fclose(fp);
                return 0;
            }
            asset_db_copy_text(meta.source_hash, ASSET_DB_MAX_HASH, tokens[1]);
            got_source_hash = 1;
        } else if (strcmp(tokens[0], "import_settings_hash") == 0) {
            if (token_count != 2) {
                fclose(fp);
                return 0;
            }
            asset_db_copy_text(meta.import_settings_hash, ASSET_DB_MAX_HASH, tokens[1]);
            got_settings_hash = 1;
        } else if (strcmp(tokens[0], "has_parameters") == 0) {
            if (token_count != 2 || !asset_db_parse_int(tokens[1], &meta.has_parameters)) {
                fclose(fp);
                return 0;
            }
            meta.has_parameters = meta.has_parameters ? 1 : 0;
        } else if (strcmp(tokens[0], "param_texture_srgb") == 0) {
            if (token_count != 2 || !asset_db_parse_int(tokens[1], &meta.texture_srgb)) {
                fclose(fp);
                return 0;
            }
        } else if (strcmp(tokens[0], "param_texture_max_size") == 0) {
            if (token_count != 2 || !asset_db_parse_int(tokens[1], &meta.texture_max_size)) {
                fclose(fp);
                return 0;
            }
        } else if (strcmp(tokens[0], "param_texture_generate_mips") == 0) {
            if (token_count != 2 || !asset_db_parse_int(tokens[1], &meta.texture_generate_mips)) {
                fclose(fp);
                return 0;
            }
        } else if (strcmp(tokens[0], "param_font_pixel_height") == 0) {
            if (token_count != 2 || !asset_db_parse_int(tokens[1], &meta.font_pixel_height)) {
                fclose(fp);
                return 0;
            }
        } else if (strcmp(tokens[0], "param_font_hinting") == 0) {
            if (token_count != 2 || !asset_db_parse_int(tokens[1], &meta.font_hinting)) {
                fclose(fp);
                return 0;
            }
        } else if (strcmp(tokens[0], "param_audio_bitrate_kbps") == 0) {
            if (token_count != 2 || !asset_db_parse_int(tokens[1], &meta.audio_bitrate_kbps)) {
                fclose(fp);
                return 0;
            }
        } else if (strcmp(tokens[0], "param_audio_normalize") == 0) {
            if (token_count != 2 || !asset_db_parse_int(tokens[1], &meta.audio_normalize)) {
                fclose(fp);
                return 0;
            }
        } else if (strcmp(tokens[0], "dependency_count") == 0) {
            if (token_count != 2 || !asset_db_parse_int(tokens[1], &expected_deps) || expected_deps < 0) {
                fclose(fp);
                return 0;
            }
        } else if (strcmp(tokens[0], "dependency") == 0) {
            if (token_count != 2 || meta.dependency_count >= ASSET_DB_MAX_DEPENDENCIES) {
                fclose(fp);
                return 0;
            }
            asset_db_copy_text(meta.dependencies[meta.dependency_count], ASSET_DB_MAX_GUID, tokens[1]);
            meta.dependency_count++;
        } else {
            fclose(fp);
            return 0;
        }
    }
    fclose(fp);

    if (!got_guid || !got_type || !got_importer || !got_version || !got_source_hash || !got_settings_hash) return 0;
    if (expected_deps >= 0 && expected_deps != meta.dependency_count) return 0;
    {
        int i;
        const char* dep_ptrs[ASSET_DB_MAX_DEPENDENCIES];
        for (i = 0; i < meta.dependency_count; i++) dep_ptrs[i] = meta.dependencies[i];
        qsort(dep_ptrs, (size_t)meta.dependency_count, sizeof(dep_ptrs[0]), asset_db_str_cmp);
        for (i = 0; i < meta.dependency_count; i++) {
            asset_db_copy_text(meta.dependencies[i], ASSET_DB_MAX_GUID, dep_ptrs[i]);
        }
    }
    *out_meta = meta;
    return 1;
}

void asset_dependency_graph_init(AssetDependencyGraph* graph) {
    if (graph == NULL) return;
    memset(graph, 0, sizeof(*graph));
}

int asset_dependency_graph_add_asset(AssetDependencyGraph* graph, const char* guid) {
    if (graph == NULL || guid == NULL || guid[0] == '\0') return 0;
    if (asset_db_find_asset_index(graph, guid) >= 0) return 1;
    if (graph->asset_count >= ASSET_DB_MAX_ASSETS) return 0;
    asset_db_copy_text(graph->assets[graph->asset_count], ASSET_DB_MAX_GUID, guid);
    graph->asset_count++;
    return 1;
}

int asset_dependency_graph_add_dependency(AssetDependencyGraph* graph, const char* guid, const char* depends_on_guid) {
    int from;
    int to;
    int i;
    if (graph == NULL || guid == NULL || depends_on_guid == NULL) return 0;
    if (!asset_dependency_graph_add_asset(graph, guid)) return 0;
    if (!asset_dependency_graph_add_asset(graph, depends_on_guid)) return 0;
    from = asset_db_find_asset_index(graph, guid);
    to = asset_db_find_asset_index(graph, depends_on_guid);
    if (from < 0 || to < 0) return 0;
    for (i = 0; i < graph->edge_count; i++) {
        if (graph->edge_from[i] == from && graph->edge_to[i] == to) return 1;
    }
    if (graph->edge_count >= ASSET_DB_MAX_EDGES) return 0;
    graph->edge_from[graph->edge_count] = from;
    graph->edge_to[graph->edge_count] = to;
    graph->edge_count++;
    return 1;
}

int asset_dependency_graph_depends_on(const AssetDependencyGraph* graph, const char* guid, const char* depends_on_guid) {
    int from;
    int to;
    int queue[ASSET_DB_MAX_ASSETS];
    int visited[ASSET_DB_MAX_ASSETS];
    int head = 0;
    int tail = 0;
    int i;
    if (graph == NULL || guid == NULL || depends_on_guid == NULL) return 0;
    from = asset_db_find_asset_index(graph, guid);
    to = asset_db_find_asset_index(graph, depends_on_guid);
    if (from < 0 || to < 0) return 0;
    memset(visited, 0, sizeof(visited));
    visited[from] = 1;
    queue[tail++] = from;
    while (head < tail) {
        int cur = queue[head++];
        if (cur == to) return 1;
        for (i = 0; i < graph->edge_count; i++) {
            if (graph->edge_from[i] == cur && !visited[graph->edge_to[i]]) {
                visited[graph->edge_to[i]] = 1;
                if (tail < ASSET_DB_MAX_ASSETS) {
                    queue[tail++] = graph->edge_to[i];
                }
            }
        }
    }
    return 0;
}
