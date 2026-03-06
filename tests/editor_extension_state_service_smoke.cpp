#include <cstdio>
#include <cstring>

#include "../apps/sandbox_dwrite/application/editor_extension_state_service.hpp"

static int smoke_plugin_init(void* user) {
    int* flag = (int*)user;
    if (flag != nullptr) *flag = 1;
    return 1;
}

static void smoke_plugin_shutdown(void* user) {
    int* flag = (int*)user;
    if (flag != nullptr) *flag = 0;
}

static int failing_plugin_init(void* user) {
    (void)user;
    return 0;
}

int main() {
    EditorExtensionStartupConfig config;
    EditorExtensionStartupResult result;
    EditorExtensionBuiltinPlugin builtin_plugins[2]{};
    int enabled_flag = 0;

    editor_extension_startup_config_init(&config);
    builtin_plugins[0].implementation.init = smoke_plugin_init;
    builtin_plugins[0].implementation.shutdown = smoke_plugin_shutdown;
    builtin_plugins[0].implementation.user = &enabled_flag;
    std::strcpy(builtin_plugins[0].plugin_id, "builtin.scene_inspector");
    std::strcpy(builtin_plugins[0].manifest_path, "Packages/builtin.scene_inspector.physicsplugin");

    builtin_plugins[1].implementation.init = failing_plugin_init;
    builtin_plugins[1].implementation.shutdown = nullptr;
    builtin_plugins[1].implementation.user = nullptr;
    std::strcpy(builtin_plugins[1].plugin_id, "builtin.failing_menu");
    std::strcpy(builtin_plugins[1].manifest_path, "Packages/builtin.failing_menu.physicsplugin");

    config.builtin_plugins = builtin_plugins;
    config.builtin_plugin_count = 2;

    if (!editor_extension_startup_run(&config, &result)) {
        std::printf("[FAIL] editor extension startup returned fatal failure\n");
        return 1;
    }
    if (result.workspace_doc.project_count < 1 ||
        result.project_doc.package_count < 1 ||
        result.package_doc.plugin_count < 2 ||
        !result.prefab_analysis_valid) {
        std::printf("[FAIL] editor extension startup state incomplete\n");
        editor_extension_startup_shutdown(&result);
        return 2;
    }
    if (!result.degraded || result.diagnostic_count < 1) {
        std::printf("[FAIL] editor extension startup should report degraded startup\n");
        editor_extension_startup_shutdown(&result);
        return 3;
    }
    if (enabled_flag != 1) {
        std::printf("[FAIL] builtin startup plugin was not initialized\n");
        editor_extension_startup_shutdown(&result);
        return 4;
    }
    editor_extension_startup_shutdown(&result);
    if (enabled_flag != 0) {
        std::printf("[FAIL] builtin startup plugin was not shutdown\n");
        return 5;
    }
    std::printf("[PASS] editor extension startup service smoke\n");
    return 0;
}
