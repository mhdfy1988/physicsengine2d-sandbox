#ifndef EDITOR_PLUGIN_H
#define EDITOR_PLUGIN_H

enum {
    EDITOR_PLUGIN_API_VERSION_1 = 1,
    EDITOR_PLUGIN_MAX_ID = 64,
    EDITOR_PLUGIN_MAX_NAME = 64,
    EDITOR_PLUGIN_MAX_VERSION = 32,
    EDITOR_PLUGIN_MAX_REASON = 128,
    EDITOR_PLUGIN_MAX_PATH = 260,
    EDITOR_PLUGIN_MAX_COUNT = 32
};

typedef enum {
    EDITOR_PLUGIN_STATE_DISCOVERED = 0,
    EDITOR_PLUGIN_STATE_INITIALIZED = 1,
    EDITOR_PLUGIN_STATE_DISABLED = 2,
    EDITOR_PLUGIN_STATE_FAILED = 3
} EditorPluginState;

typedef enum {
    EDITOR_PLUGIN_CAP_CUSTOM_INSPECTOR = 1 << 0,
    EDITOR_PLUGIN_CAP_ASSET_IMPORTER = 1 << 1,
    EDITOR_PLUGIN_CAP_MENU = 1 << 2,
    EDITOR_PLUGIN_CAP_TOOL_WINDOW = 1 << 3,
    EDITOR_PLUGIN_CAP_BUILD_STEP = 1 << 4
} EditorPluginCapability;

typedef struct {
    unsigned int api_version;
    char plugin_id[EDITOR_PLUGIN_MAX_ID];
    char display_name[EDITOR_PLUGIN_MAX_NAME];
    char plugin_version[EDITOR_PLUGIN_MAX_VERSION];
    char engine_version_range[EDITOR_PLUGIN_MAX_VERSION];
    unsigned int capabilities;
} EditorPluginManifest;

typedef struct {
    int (*init)(void* user);
    void (*shutdown)(void* user);
    void* user;
} EditorPluginV1;

typedef struct {
    EditorPluginManifest manifest;
    EditorPluginV1 implementation;
    EditorPluginState state;
    char manifest_path[EDITOR_PLUGIN_MAX_PATH];
    char disable_reason[EDITOR_PLUGIN_MAX_REASON];
} EditorPluginRecord;

typedef struct {
    int plugin_count;
    EditorPluginRecord plugins[EDITOR_PLUGIN_MAX_COUNT];
} EditorPluginRegistry;

void editor_plugin_registry_init(EditorPluginRegistry* registry);
int editor_plugin_manifest_load_v1(const char* path, EditorPluginManifest* out_manifest);
int editor_plugin_manifest_save_v1(const EditorPluginManifest* manifest, const char* path);
int editor_plugin_registry_scan_v1(const char* directory, EditorPluginRegistry* out_registry);
int editor_plugin_registry_attach(EditorPluginRegistry* registry, const char* plugin_id, const EditorPluginV1* implementation);
int editor_plugin_registry_initialize_all(EditorPluginRegistry* registry);
void editor_plugin_registry_disable(EditorPluginRegistry* registry, const char* plugin_id, const char* reason);
void editor_plugin_registry_shutdown(EditorPluginRegistry* registry);

#endif
