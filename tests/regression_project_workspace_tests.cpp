#include <stdio.h>
#include <string.h>

#include <filesystem>

#include "../include/project_workspace.hpp"

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
        if (pa == NULL && pb == NULL) break;
        if (pa == NULL || pb == NULL || strcmp(la, lb) != 0) {
            ok = 0;
            break;
        }
    }
    fclose(fa);
    fclose(fb);
    return ok;
}

int regression_test_project_workspace_roundtrip(void) {
    const char* workspace_a = "_tmp_workspace_a.txt";
    const char* workspace_b = "_tmp_workspace_b.txt";
    const char* project_a = "_tmp_project_a.txt";
    const char* project_b = "_tmp_project_b.txt";
    const char* package_a = "_tmp_package_a.txt";
    const char* package_b = "_tmp_package_b.txt";
    const char* settings_a = "_tmp_settings_a.txt";
    const char* settings_b = "_tmp_settings_b.txt";
    WorkspaceDocument workspace;
    WorkspaceDocument loaded_workspace;
    ProjectDocument project;
    ProjectDocument loaded_project;
    PackageDocument package;
    PackageDocument loaded_package;
    ProjectSettingsDocument settings;
    ProjectSettingsDocument loaded_settings;

    workspace_document_init(&workspace);
    strcpy(workspace.workspace_name, "SandboxWorkspace");
    strcpy(workspace.active_project_guid, "project://sandbox");
    workspace.project_count = 1;
    strcpy(workspace.projects[0].project_guid, "project://sandbox");
    strcpy(workspace.projects[0].project_name, "SandboxProject");
    strcpy(workspace.projects[0].project_path, "ProjectSettings/project.physicsproject");

    project_document_init(&project);
    strcpy(project.project_guid, "project://sandbox");
    strcpy(project.project_name, "SandboxProject");
    strcpy(project.engine_version, "phase-g");
    strcpy(project.startup_scene, "scene://default");
    project.package_count = 1;
    strcpy(project.packages[0].package_id, "package.core.tools");
    strcpy(project.packages[0].package_path, "Packages/core_tools.physicspackage");
    project.packages[0].enabled = 1;

    package_document_init(&package);
    strcpy(package.package_id, "package.core.tools");
    strcpy(package.display_name, "Core Tools");
    strcpy(package.package_version, "1.2.0");
    strcpy(package.engine_version_range, ">=phase-g");
    package.plugin_count = 1;
    strcpy(package.plugins[0].plugin_id, "builtin.scene_inspector");
    strcpy(package.plugins[0].plugin_manifest, "Packages/builtin.scene_inspector.physicsplugin");

    project_settings_document_init(&settings);
    settings.auto_reload_assets = 1;
    settings.hot_reload_in_pie = 0;
    settings.reopen_last_workspace = 1;
    strcpy(settings.last_open_scene, "scene://sandbox");

    if (!workspace_document_save_v1(&workspace, workspace_a) ||
        !project_document_save_v1(&project, project_a) ||
        !package_document_save_v1(&package, package_a) ||
        !project_settings_document_save_v1(&settings, settings_a)) {
        printf("[FAIL] failed to write project workspace sources\n");
        return 0;
    }
    if (!workspace_document_load_v1(workspace_a, &loaded_workspace) ||
        !project_document_load_v1(project_a, &loaded_project) ||
        !package_document_load_v1(package_a, &loaded_package) ||
        !project_settings_document_load_v1(settings_a, &loaded_settings)) {
        printf("[FAIL] failed to read project workspace sources\n");
        return 0;
    }
    if (!workspace_document_save_v1(&loaded_workspace, workspace_b) ||
        !project_document_save_v1(&loaded_project, project_b) ||
        !package_document_save_v1(&loaded_package, package_b) ||
        !project_settings_document_save_v1(&loaded_settings, settings_b)) {
        printf("[FAIL] failed to write project workspace targets\n");
        return 0;
    }
    if (!file_content_equals(workspace_a, workspace_b) ||
        !file_content_equals(project_a, project_b) ||
        !file_content_equals(package_a, package_b) ||
        !file_content_equals(settings_a, settings_b)) {
        printf("[FAIL] project workspace roundtrip is not deterministic\n");
        return 0;
    }

    remove(workspace_a);
    remove(workspace_b);
    remove(project_a);
    remove(project_b);
    remove(package_a);
    remove(package_b);
    remove(settings_a);
    remove(settings_b);
    printf("[PASS] project workspace roundtrip determinism\n");
    return 1;
}

int regression_test_project_workspace_bootstrap(void) {
    const std::filesystem::path root = "_tmp_phase_g_workspace";
    WorkspaceDocument workspace;
    ProjectDocument project;
    PackageDocument package;
    ProjectSettingsDocument settings;
    ProjectWorkspaceBootstrapReport report{};

    std::filesystem::remove_all(root);
    workspace_document_init(&workspace);
    project_document_init(&project);
    package_document_init(&package);
    project_settings_document_init(&settings);

    workspace.project_count = 1;
    strcpy(workspace.active_project_guid, project.project_guid);
    strcpy(workspace.projects[0].project_guid, project.project_guid);
    strcpy(workspace.projects[0].project_name, project.project_name);
    strcpy(workspace.projects[0].project_path, "ProjectSettings/project.physicsproject");
    package.plugin_count = 1;
    strcpy(package.plugins[0].plugin_id, "builtin.scene_inspector");
    strcpy(package.plugins[0].plugin_manifest, "Packages/builtin.scene_inspector.physicsplugin");

    if (!project_workspace_bootstrap_v1(root.string().c_str(), &workspace, &project, &package, &settings, &report)) {
        printf("[FAIL] project workspace bootstrap failed\n");
        return 0;
    }
    if (!project_workspace_validate_layout_v1(root.string().c_str()) ||
        !std::filesystem::exists(root / "ProjectSettings" / "workspace.physicsworkspace") ||
        !std::filesystem::exists(root / "ProjectSettings" / "project.physicsproject") ||
        !std::filesystem::exists(root / "ProjectSettings" / "editor.physicssettings") ||
        !std::filesystem::exists(root / "Packages" / "core_tools.physicspackage") ||
        !std::filesystem::exists(root / "Build") ||
        !std::filesystem::exists(root / "Cache") ||
        report.created_directory_count <= 0 ||
        report.created_file_count != 4) {
        printf("[FAIL] project workspace bootstrap output mismatch\n");
        std::filesystem::remove_all(root);
        return 0;
    }

    std::filesystem::remove_all(root);
    printf("[PASS] project workspace bootstrap layout\n");
    return 1;
}
