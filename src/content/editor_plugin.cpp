#include "physics_content/editor_plugin.hpp"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <filesystem>

namespace {

void copy_text(char* out, int out_cap, const char* src) {
    if (out == nullptr || out_cap <= 0) return;
    if (src == nullptr) {
        out[0] = '\0';
        return;
    }
    strncpy(out, src, (size_t)(out_cap - 1));
    out[out_cap - 1] = '\0';
}

void trim_line(char* line) {
    size_t n;
    if (line == nullptr) return;
    n = strlen(line);
    while (n > 0 && (line[n - 1] == '\n' || line[n - 1] == '\r')) {
        line[n - 1] = '\0';
        n--;
    }
}

int split_pipe(char* line, char* tokens[], int max_tokens) {
    int count = 0;
    char* p = line;
    if (line == nullptr || tokens == nullptr || max_tokens <= 0) return 0;
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

int parse_int(const char* text, int* out_value) {
    char* endptr = nullptr;
    long value;
    if (text == nullptr || out_value == nullptr) return 0;
    errno = 0;
    value = strtol(text, &endptr, 10);
    if (errno != 0 || endptr == text || (endptr != nullptr && *endptr != '\0')) return 0;
    if (value < -2147483647L - 1L || value > 2147483647L) return 0;
    *out_value = (int)value;
    return 1;
}

EditorPluginRecord* find_plugin(EditorPluginRegistry* registry, const char* plugin_id) {
    int index;
    if (registry == nullptr || plugin_id == nullptr) return nullptr;
    for (index = 0; index < registry->plugin_count; ++index) {
        if (strcmp(registry->plugins[index].manifest.plugin_id, plugin_id) == 0) {
            return &registry->plugins[index];
        }
    }
    return nullptr;
}

const EditorPluginRecord* find_plugin_const(const EditorPluginRegistry* registry, const char* plugin_id) {
    int index;
    if (registry == nullptr || plugin_id == nullptr) return nullptr;
    for (index = 0; index < registry->plugin_count; ++index) {
        if (strcmp(registry->plugins[index].manifest.plugin_id, plugin_id) == 0) {
            return &registry->plugins[index];
        }
    }
    return nullptr;
}

}  // namespace

void editor_plugin_registry_init(EditorPluginRegistry* registry) {
    if (registry == nullptr) return;
    memset(registry, 0, sizeof(*registry));
}

int editor_plugin_manifest_load_v1(const char* path, EditorPluginManifest* out_manifest) {
    FILE* fp;
    char line[512];
    EditorPluginManifest manifest{};
    if (path == nullptr || out_manifest == nullptr) return 0;
    fp = fopen(path, "r");
    if (fp == nullptr) return 0;
    manifest.api_version = EDITOR_PLUGIN_API_VERSION_1;
    while (fgets(line, sizeof(line), fp) != nullptr) {
        char* tokens[8];
        int token_count;
        int parsed = 0;
        trim_line(line);
        if (line[0] == '\0' || line[0] == '#') continue;
        token_count = split_pipe(line, tokens, 8);
        if (token_count <= 0) {
            fclose(fp);
            return 0;
        }
        if (strcmp(tokens[0], "api_version") == 0) {
            if (token_count != 2 || !parse_int(tokens[1], &parsed)) {
                fclose(fp);
                return 0;
            }
            manifest.api_version = (unsigned int)parsed;
        } else if (strcmp(tokens[0], "plugin_id") == 0) {
            if (token_count != 2) {
                fclose(fp);
                return 0;
            }
            copy_text(manifest.plugin_id, EDITOR_PLUGIN_MAX_ID, tokens[1]);
        } else if (strcmp(tokens[0], "display_name") == 0) {
            if (token_count != 2) {
                fclose(fp);
                return 0;
            }
            copy_text(manifest.display_name, EDITOR_PLUGIN_MAX_NAME, tokens[1]);
        } else if (strcmp(tokens[0], "plugin_version") == 0) {
            if (token_count != 2) {
                fclose(fp);
                return 0;
            }
            copy_text(manifest.plugin_version, EDITOR_PLUGIN_MAX_VERSION, tokens[1]);
        } else if (strcmp(tokens[0], "engine_version_range") == 0) {
            if (token_count != 2) {
                fclose(fp);
                return 0;
            }
            copy_text(manifest.engine_version_range, EDITOR_PLUGIN_MAX_VERSION, tokens[1]);
        } else if (strcmp(tokens[0], "capabilities") == 0) {
            if (token_count != 2 || !parse_int(tokens[1], &parsed)) {
                fclose(fp);
                return 0;
            }
            manifest.capabilities = (unsigned int)parsed;
        } else {
            fclose(fp);
            return 0;
        }
    }
    fclose(fp);
    if (manifest.api_version != EDITOR_PLUGIN_API_VERSION_1 || manifest.plugin_id[0] == '\0') return 0;
    *out_manifest = manifest;
    return 1;
}

int editor_plugin_manifest_save_v1(const EditorPluginManifest* manifest, const char* path) {
    FILE* fp;
    if (manifest == nullptr || path == nullptr) return 0;
    fp = fopen(path, "w");
    if (fp == nullptr) return 0;
    fprintf(fp, "api_version|%u\n", manifest->api_version);
    fprintf(fp, "plugin_id|%s\n", manifest->plugin_id);
    fprintf(fp, "display_name|%s\n", manifest->display_name);
    fprintf(fp, "plugin_version|%s\n", manifest->plugin_version);
    fprintf(fp, "engine_version_range|%s\n", manifest->engine_version_range);
    fprintf(fp, "capabilities|%u\n", manifest->capabilities);
    return fclose(fp) == 0;
}

int editor_plugin_registry_scan_v1(const char* directory, EditorPluginRegistry* out_registry) {
    EditorPluginRegistry registry;
    if (directory == nullptr || out_registry == nullptr) return 0;
    editor_plugin_registry_init(&registry);
    if (!std::filesystem::exists(directory)) {
        *out_registry = registry;
        return 1;
    }
    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        const std::filesystem::path path = entry.path();
        EditorPluginManifest manifest;
        EditorPluginRecord* record;
        if (!entry.is_regular_file()) continue;
        if (path.extension().string() != ".physicsplugin") continue;
        if (registry.plugin_count >= EDITOR_PLUGIN_MAX_COUNT) return 0;
        if (!editor_plugin_manifest_load_v1(path.string().c_str(), &manifest)) return 0;
        if (find_plugin_const(&registry, manifest.plugin_id) != nullptr) return 0;
        record = &registry.plugins[registry.plugin_count++];
        memset(record, 0, sizeof(*record));
        record->manifest = manifest;
        record->state = EDITOR_PLUGIN_STATE_DISCOVERED;
        copy_text(record->manifest_path, EDITOR_PLUGIN_MAX_PATH, path.string().c_str());
    }
    *out_registry = registry;
    return 1;
}

int editor_plugin_registry_attach(EditorPluginRegistry* registry, const char* plugin_id, const EditorPluginV1* implementation) {
    EditorPluginRecord* record = find_plugin(registry, plugin_id);
    if (record == nullptr || implementation == nullptr) return 0;
    record->implementation = *implementation;
    return 1;
}

int editor_plugin_registry_initialize_all(EditorPluginRegistry* registry) {
    int index;
    if (registry == nullptr) return 0;
    for (index = 0; index < registry->plugin_count; ++index) {
        EditorPluginRecord* record = &registry->plugins[index];
        if (record->state == EDITOR_PLUGIN_STATE_DISABLED) continue;
        if (record->implementation.init == nullptr) {
            record->state = EDITOR_PLUGIN_STATE_DISABLED;
            copy_text(record->disable_reason, EDITOR_PLUGIN_MAX_REASON, "missing host binding");
            continue;
        }
        if (!record->implementation.init(record->implementation.user)) {
            record->state = EDITOR_PLUGIN_STATE_FAILED;
            copy_text(record->disable_reason, EDITOR_PLUGIN_MAX_REASON, "plugin init failed");
            continue;
        }
        record->state = EDITOR_PLUGIN_STATE_INITIALIZED;
        record->disable_reason[0] = '\0';
    }
    return 1;
}

void editor_plugin_registry_disable(EditorPluginRegistry* registry, const char* plugin_id, const char* reason) {
    EditorPluginRecord* record = find_plugin(registry, plugin_id);
    if (record == nullptr) return;
    if (record->state == EDITOR_PLUGIN_STATE_INITIALIZED && record->implementation.shutdown != nullptr) {
        record->implementation.shutdown(record->implementation.user);
    }
    record->state = EDITOR_PLUGIN_STATE_DISABLED;
    copy_text(record->disable_reason, EDITOR_PLUGIN_MAX_REASON, reason != nullptr ? reason : "disabled");
}

void editor_plugin_registry_shutdown(EditorPluginRegistry* registry) {
    int index;
    if (registry == nullptr) return;
    for (index = 0; index < registry->plugin_count; ++index) {
        EditorPluginRecord* record = &registry->plugins[index];
        if (record->state == EDITOR_PLUGIN_STATE_INITIALIZED && record->implementation.shutdown != nullptr) {
            record->implementation.shutdown(record->implementation.user);
        }
        if (record->state == EDITOR_PLUGIN_STATE_INITIALIZED) {
            record->state = EDITOR_PLUGIN_STATE_DISCOVERED;
        }
    }
}
