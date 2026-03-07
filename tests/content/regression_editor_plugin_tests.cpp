#include <stdio.h>
#include <string.h>

#include <filesystem>

#include "../../include/physics_content/editor_plugin.hpp"

static int g_plugin_init_calls = 0;
static int g_plugin_shutdown_calls = 0;

static int good_plugin_init(void* user) {
    int* state = (int*)user;
    g_plugin_init_calls++;
    if (state != NULL) *state = 1;
    return 1;
}

static void good_plugin_shutdown(void* user) {
    int* state = (int*)user;
    g_plugin_shutdown_calls++;
    if (state != NULL) *state = 0;
}

static int bad_plugin_init(void* user) {
    (void)user;
    return 0;
}

static void write_manifest(const std::filesystem::path& path, const char* plugin_id, unsigned int caps) {
    EditorPluginManifest manifest{};
    manifest.api_version = EDITOR_PLUGIN_API_VERSION_1;
    strcpy(manifest.plugin_id, plugin_id);
    strcpy(manifest.display_name, plugin_id);
    strcpy(manifest.plugin_version, "1.0.0");
    strcpy(manifest.engine_version_range, ">=phase-g");
    manifest.capabilities = caps;
    editor_plugin_manifest_save_v1(&manifest, path.string().c_str());
}

int regression_test_editor_plugin_registry_lifecycle(void) {
    const std::filesystem::path root = "_tmp_phase_g_plugins";
    EditorPluginRegistry registry;
    EditorPluginV1 good_impl{};
    EditorPluginV1 bad_impl{};
    int plugin_state = 0;

    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root);
    write_manifest(root / "builtin.scene_inspector.physicsplugin", "builtin.scene_inspector", EDITOR_PLUGIN_CAP_CUSTOM_INSPECTOR);
    write_manifest(root / "builtin.bad.physicsplugin", "builtin.bad", EDITOR_PLUGIN_CAP_MENU);

    if (!editor_plugin_registry_scan_v1(root.string().c_str(), &registry) || registry.plugin_count != 2) {
        printf("[FAIL] plugin registry scan failed\n");
        std::filesystem::remove_all(root);
        return 0;
    }

    good_impl.init = good_plugin_init;
    good_impl.shutdown = good_plugin_shutdown;
    good_impl.user = &plugin_state;
    bad_impl.init = bad_plugin_init;
    bad_impl.shutdown = NULL;

    if (!editor_plugin_registry_attach(&registry, "builtin.scene_inspector", &good_impl) ||
        !editor_plugin_registry_attach(&registry, "builtin.bad", &bad_impl) ||
        !editor_plugin_registry_initialize_all(&registry)) {
        printf("[FAIL] plugin registry init failed\n");
        std::filesystem::remove_all(root);
        return 0;
    }

    if (registry.plugins[0].state == EDITOR_PLUGIN_STATE_DISCOVERED ||
        registry.plugins[1].state == EDITOR_PLUGIN_STATE_DISCOVERED ||
        g_plugin_init_calls != 1 ||
        plugin_state != 1) {
        printf("[FAIL] plugin registry lifecycle state mismatch\n");
        std::filesystem::remove_all(root);
        return 0;
    }

    editor_plugin_registry_disable(&registry, "builtin.scene_inspector", "manual disable");
    if (plugin_state != 0 || g_plugin_shutdown_calls != 1) {
        printf("[FAIL] plugin registry disable did not shutdown cleanly\n");
        std::filesystem::remove_all(root);
        return 0;
    }

    editor_plugin_registry_shutdown(&registry);
    std::filesystem::remove_all(root);
    printf("[PASS] editor plugin registry lifecycle\n");
    return 1;
}

int regression_test_editor_plugin_registry_duplicate_rejected(void) {
    const std::filesystem::path root = "_tmp_phase_g_plugins_dupe";
    EditorPluginRegistry registry;

    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root);
    write_manifest(root / "one.physicsplugin", "builtin.scene_inspector", EDITOR_PLUGIN_CAP_CUSTOM_INSPECTOR);
    write_manifest(root / "two.physicsplugin", "builtin.scene_inspector", EDITOR_PLUGIN_CAP_MENU);

    if (editor_plugin_registry_scan_v1(root.string().c_str(), &registry)) {
        printf("[FAIL] duplicate plugin id should be rejected\n");
        std::filesystem::remove_all(root);
        return 0;
    }

    std::filesystem::remove_all(root);
    printf("[PASS] editor plugin registry duplicate rejection\n");
    return 1;
}
