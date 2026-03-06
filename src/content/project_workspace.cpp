#include "project_workspace.hpp"

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

bool ensure_directory_counted(const std::filesystem::path& path, int* out_created_directory_count) {
    if (std::filesystem::exists(path)) return true;
    if (!std::filesystem::create_directories(path)) return false;
    if (out_created_directory_count != nullptr) (*out_created_directory_count)++;
    return true;
}

}  // namespace

void workspace_document_init(WorkspaceDocument* doc) {
    if (doc == nullptr) return;
    memset(doc, 0, sizeof(*doc));
    doc->schema_version = PROJECT_WORKSPACE_SCHEMA_VERSION_V1;
    copy_text(doc->workspace_name, PROJECT_WORKSPACE_MAX_NAME, "PhysicsWorkspace");
}

void project_document_init(ProjectDocument* doc) {
    if (doc == nullptr) return;
    memset(doc, 0, sizeof(*doc));
    doc->schema_version = PROJECT_WORKSPACE_SCHEMA_VERSION_V1;
    copy_text(doc->project_guid, PROJECT_WORKSPACE_MAX_GUID, "project://physics-sandbox");
    copy_text(doc->project_name, PROJECT_WORKSPACE_MAX_NAME, "PhysicsSandbox");
    copy_text(doc->engine_version, PROJECT_WORKSPACE_MAX_VERSION, "phase-g");
    copy_text(doc->startup_scene, PROJECT_WORKSPACE_MAX_GUID, "scene://default");
}

void package_document_init(PackageDocument* doc) {
    if (doc == nullptr) return;
    memset(doc, 0, sizeof(*doc));
    doc->schema_version = PROJECT_WORKSPACE_SCHEMA_VERSION_V1;
    copy_text(doc->package_id, PROJECT_WORKSPACE_MAX_NAME, "package.core.tools");
    copy_text(doc->display_name, PROJECT_WORKSPACE_MAX_NAME, "Core Tools");
    copy_text(doc->package_version, PROJECT_WORKSPACE_MAX_VERSION, "1.0.0");
    copy_text(doc->engine_version_range, PROJECT_WORKSPACE_MAX_VERSION, ">=phase-g");
}

void project_settings_document_init(ProjectSettingsDocument* doc) {
    if (doc == nullptr) return;
    memset(doc, 0, sizeof(*doc));
    doc->schema_version = PROJECT_WORKSPACE_SCHEMA_VERSION_V1;
    doc->auto_reload_assets = 1;
    doc->hot_reload_in_pie = 1;
    doc->reopen_last_workspace = 1;
    copy_text(doc->last_open_scene, PROJECT_WORKSPACE_MAX_GUID, "scene://default");
}

int workspace_document_load_v1(const char* path, WorkspaceDocument* out_doc) {
    FILE* fp;
    char line[512];
    WorkspaceDocument doc;
    if (path == nullptr || out_doc == nullptr) return 0;
    fp = fopen(path, "r");
    if (fp == nullptr) return 0;
    workspace_document_init(&doc);
    while (fgets(line, sizeof(line), fp) != nullptr) {
        char* tokens[8];
        int token_count;
        trim_line(line);
        if (line[0] == '\0' || line[0] == '#') continue;
        token_count = split_pipe(line, tokens, 8);
        if (token_count <= 0) {
            fclose(fp);
            return 0;
        }
        if (strcmp(tokens[0], "schema_version") == 0) {
            if (token_count != 2 || !parse_int(tokens[1], &doc.schema_version) || doc.schema_version != PROJECT_WORKSPACE_SCHEMA_VERSION_V1) {
                fclose(fp);
                return 0;
            }
        } else if (strcmp(tokens[0], "workspace_name") == 0) {
            if (token_count != 2) {
                fclose(fp);
                return 0;
            }
            copy_text(doc.workspace_name, PROJECT_WORKSPACE_MAX_NAME, tokens[1]);
        } else if (strcmp(tokens[0], "active_project_guid") == 0) {
            if (token_count != 2) {
                fclose(fp);
                return 0;
            }
            copy_text(doc.active_project_guid, PROJECT_WORKSPACE_MAX_GUID, tokens[1]);
        } else if (strcmp(tokens[0], "project_count") == 0) {
            int ignored_count = 0;
            if (token_count != 2 || !parse_int(tokens[1], &ignored_count) ||
                ignored_count < 0 || ignored_count > PROJECT_WORKSPACE_MAX_PROJECTS) {
                fclose(fp);
                return 0;
            }
            doc.project_count = 0;
        } else if (strcmp(tokens[0], "project") == 0) {
            WorkspaceProjectRef* project;
            if (token_count != 4 || doc.project_count >= PROJECT_WORKSPACE_MAX_PROJECTS) {
                fclose(fp);
                return 0;
            }
            project = &doc.projects[doc.project_count++];
            copy_text(project->project_guid, PROJECT_WORKSPACE_MAX_GUID, tokens[1]);
            copy_text(project->project_name, PROJECT_WORKSPACE_MAX_NAME, tokens[2]);
            copy_text(project->project_path, PROJECT_WORKSPACE_MAX_PATH, tokens[3]);
        } else {
            fclose(fp);
            return 0;
        }
    }
    fclose(fp);
    *out_doc = doc;
    return 1;
}

int workspace_document_save_v1(const WorkspaceDocument* doc, const char* path) {
    FILE* fp;
    int i;
    if (doc == nullptr || path == nullptr) return 0;
    fp = fopen(path, "w");
    if (fp == nullptr) return 0;
    fprintf(fp, "schema_version|%d\n", PROJECT_WORKSPACE_SCHEMA_VERSION_V1);
    fprintf(fp, "workspace_name|%s\n", doc->workspace_name);
    fprintf(fp, "active_project_guid|%s\n", doc->active_project_guid);
    fprintf(fp, "project_count|%d\n", doc->project_count);
    for (i = 0; i < doc->project_count && i < PROJECT_WORKSPACE_MAX_PROJECTS; ++i) {
        const WorkspaceProjectRef* project = &doc->projects[i];
        fprintf(fp, "project|%s|%s|%s\n", project->project_guid, project->project_name, project->project_path);
    }
    return fclose(fp) == 0;
}

int project_document_load_v1(const char* path, ProjectDocument* out_doc) {
    FILE* fp;
    char line[512];
    ProjectDocument doc;
    if (path == nullptr || out_doc == nullptr) return 0;
    fp = fopen(path, "r");
    if (fp == nullptr) return 0;
    project_document_init(&doc);
    while (fgets(line, sizeof(line), fp) != nullptr) {
        char* tokens[8];
        int token_count;
        trim_line(line);
        if (line[0] == '\0' || line[0] == '#') continue;
        token_count = split_pipe(line, tokens, 8);
        if (token_count <= 0) {
            fclose(fp);
            return 0;
        }
        if (strcmp(tokens[0], "schema_version") == 0) {
            if (token_count != 2 || !parse_int(tokens[1], &doc.schema_version) || doc.schema_version != PROJECT_WORKSPACE_SCHEMA_VERSION_V1) {
                fclose(fp);
                return 0;
            }
        } else if (strcmp(tokens[0], "project_guid") == 0) {
            if (token_count != 2) {
                fclose(fp);
                return 0;
            }
            copy_text(doc.project_guid, PROJECT_WORKSPACE_MAX_GUID, tokens[1]);
        } else if (strcmp(tokens[0], "project_name") == 0) {
            if (token_count != 2) {
                fclose(fp);
                return 0;
            }
            copy_text(doc.project_name, PROJECT_WORKSPACE_MAX_NAME, tokens[1]);
        } else if (strcmp(tokens[0], "engine_version") == 0) {
            if (token_count != 2) {
                fclose(fp);
                return 0;
            }
            copy_text(doc.engine_version, PROJECT_WORKSPACE_MAX_VERSION, tokens[1]);
        } else if (strcmp(tokens[0], "startup_scene") == 0) {
            if (token_count != 2) {
                fclose(fp);
                return 0;
            }
            copy_text(doc.startup_scene, PROJECT_WORKSPACE_MAX_GUID, tokens[1]);
        } else if (strcmp(tokens[0], "package_count") == 0) {
            int ignored_count = 0;
            if (token_count != 2 || !parse_int(tokens[1], &ignored_count) ||
                ignored_count < 0 || ignored_count > PROJECT_WORKSPACE_MAX_PACKAGES) {
                fclose(fp);
                return 0;
            }
            doc.package_count = 0;
        } else if (strcmp(tokens[0], "package") == 0) {
            ProjectPackageRef* package;
            int enabled = 0;
            if (token_count != 4 || doc.package_count >= PROJECT_WORKSPACE_MAX_PACKAGES || !parse_int(tokens[3], &enabled)) {
                fclose(fp);
                return 0;
            }
            package = &doc.packages[doc.package_count++];
            copy_text(package->package_id, PROJECT_WORKSPACE_MAX_NAME, tokens[1]);
            copy_text(package->package_path, PROJECT_WORKSPACE_MAX_PATH, tokens[2]);
            package->enabled = enabled ? 1 : 0;
        } else {
            fclose(fp);
            return 0;
        }
    }
    fclose(fp);
    *out_doc = doc;
    return 1;
}

int project_document_save_v1(const ProjectDocument* doc, const char* path) {
    FILE* fp;
    int i;
    if (doc == nullptr || path == nullptr) return 0;
    fp = fopen(path, "w");
    if (fp == nullptr) return 0;
    fprintf(fp, "schema_version|%d\n", PROJECT_WORKSPACE_SCHEMA_VERSION_V1);
    fprintf(fp, "project_guid|%s\n", doc->project_guid);
    fprintf(fp, "project_name|%s\n", doc->project_name);
    fprintf(fp, "engine_version|%s\n", doc->engine_version);
    fprintf(fp, "startup_scene|%s\n", doc->startup_scene);
    fprintf(fp, "package_count|%d\n", doc->package_count);
    for (i = 0; i < doc->package_count && i < PROJECT_WORKSPACE_MAX_PACKAGES; ++i) {
        const ProjectPackageRef* package = &doc->packages[i];
        fprintf(fp, "package|%s|%s|%d\n", package->package_id, package->package_path, package->enabled ? 1 : 0);
    }
    return fclose(fp) == 0;
}

int package_document_load_v1(const char* path, PackageDocument* out_doc) {
    FILE* fp;
    char line[512];
    PackageDocument doc;
    if (path == nullptr || out_doc == nullptr) return 0;
    fp = fopen(path, "r");
    if (fp == nullptr) return 0;
    package_document_init(&doc);
    while (fgets(line, sizeof(line), fp) != nullptr) {
        char* tokens[8];
        int token_count;
        trim_line(line);
        if (line[0] == '\0' || line[0] == '#') continue;
        token_count = split_pipe(line, tokens, 8);
        if (token_count <= 0) {
            fclose(fp);
            return 0;
        }
        if (strcmp(tokens[0], "schema_version") == 0) {
            if (token_count != 2 || !parse_int(tokens[1], &doc.schema_version) || doc.schema_version != PROJECT_WORKSPACE_SCHEMA_VERSION_V1) {
                fclose(fp);
                return 0;
            }
        } else if (strcmp(tokens[0], "package_id") == 0) {
            if (token_count != 2) {
                fclose(fp);
                return 0;
            }
            copy_text(doc.package_id, PROJECT_WORKSPACE_MAX_NAME, tokens[1]);
        } else if (strcmp(tokens[0], "display_name") == 0) {
            if (token_count != 2) {
                fclose(fp);
                return 0;
            }
            copy_text(doc.display_name, PROJECT_WORKSPACE_MAX_NAME, tokens[1]);
        } else if (strcmp(tokens[0], "package_version") == 0) {
            if (token_count != 2) {
                fclose(fp);
                return 0;
            }
            copy_text(doc.package_version, PROJECT_WORKSPACE_MAX_VERSION, tokens[1]);
        } else if (strcmp(tokens[0], "engine_version_range") == 0) {
            if (token_count != 2) {
                fclose(fp);
                return 0;
            }
            copy_text(doc.engine_version_range, PROJECT_WORKSPACE_MAX_VERSION, tokens[1]);
        } else if (strcmp(tokens[0], "plugin_count") == 0) {
            int ignored_count = 0;
            if (token_count != 2 || !parse_int(tokens[1], &ignored_count) ||
                ignored_count < 0 || ignored_count > PROJECT_WORKSPACE_MAX_PLUGINS) {
                fclose(fp);
                return 0;
            }
            doc.plugin_count = 0;
        } else if (strcmp(tokens[0], "plugin") == 0) {
            PackagePluginRef* plugin;
            if (token_count != 3 || doc.plugin_count >= PROJECT_WORKSPACE_MAX_PLUGINS) {
                fclose(fp);
                return 0;
            }
            plugin = &doc.plugins[doc.plugin_count++];
            copy_text(plugin->plugin_id, PROJECT_WORKSPACE_MAX_NAME, tokens[1]);
            copy_text(plugin->plugin_manifest, PROJECT_WORKSPACE_MAX_PATH, tokens[2]);
        } else {
            fclose(fp);
            return 0;
        }
    }
    fclose(fp);
    *out_doc = doc;
    return 1;
}

int package_document_save_v1(const PackageDocument* doc, const char* path) {
    FILE* fp;
    int i;
    if (doc == nullptr || path == nullptr) return 0;
    fp = fopen(path, "w");
    if (fp == nullptr) return 0;
    fprintf(fp, "schema_version|%d\n", PROJECT_WORKSPACE_SCHEMA_VERSION_V1);
    fprintf(fp, "package_id|%s\n", doc->package_id);
    fprintf(fp, "display_name|%s\n", doc->display_name);
    fprintf(fp, "package_version|%s\n", doc->package_version);
    fprintf(fp, "engine_version_range|%s\n", doc->engine_version_range);
    fprintf(fp, "plugin_count|%d\n", doc->plugin_count);
    for (i = 0; i < doc->plugin_count && i < PROJECT_WORKSPACE_MAX_PLUGINS; ++i) {
        const PackagePluginRef* plugin = &doc->plugins[i];
        fprintf(fp, "plugin|%s|%s\n", plugin->plugin_id, plugin->plugin_manifest);
    }
    return fclose(fp) == 0;
}

int project_settings_document_load_v1(const char* path, ProjectSettingsDocument* out_doc) {
    FILE* fp;
    char line[512];
    ProjectSettingsDocument doc;
    if (path == nullptr || out_doc == nullptr) return 0;
    fp = fopen(path, "r");
    if (fp == nullptr) return 0;
    project_settings_document_init(&doc);
    while (fgets(line, sizeof(line), fp) != nullptr) {
        char* tokens[8];
        int token_count;
        trim_line(line);
        if (line[0] == '\0' || line[0] == '#') continue;
        token_count = split_pipe(line, tokens, 8);
        if (token_count <= 0) {
            fclose(fp);
            return 0;
        }
        if (strcmp(tokens[0], "schema_version") == 0) {
            if (token_count != 2 || !parse_int(tokens[1], &doc.schema_version) || doc.schema_version != PROJECT_WORKSPACE_SCHEMA_VERSION_V1) {
                fclose(fp);
                return 0;
            }
        } else if (strcmp(tokens[0], "auto_reload_assets") == 0) {
            if (token_count != 2 || !parse_int(tokens[1], &doc.auto_reload_assets)) {
                fclose(fp);
                return 0;
            }
            doc.auto_reload_assets = doc.auto_reload_assets ? 1 : 0;
        } else if (strcmp(tokens[0], "hot_reload_in_pie") == 0) {
            if (token_count != 2 || !parse_int(tokens[1], &doc.hot_reload_in_pie)) {
                fclose(fp);
                return 0;
            }
            doc.hot_reload_in_pie = doc.hot_reload_in_pie ? 1 : 0;
        } else if (strcmp(tokens[0], "reopen_last_workspace") == 0) {
            if (token_count != 2 || !parse_int(tokens[1], &doc.reopen_last_workspace)) {
                fclose(fp);
                return 0;
            }
            doc.reopen_last_workspace = doc.reopen_last_workspace ? 1 : 0;
        } else if (strcmp(tokens[0], "last_open_scene") == 0) {
            if (token_count != 2) {
                fclose(fp);
                return 0;
            }
            copy_text(doc.last_open_scene, PROJECT_WORKSPACE_MAX_GUID, tokens[1]);
        } else {
            fclose(fp);
            return 0;
        }
    }
    fclose(fp);
    *out_doc = doc;
    return 1;
}

int project_settings_document_save_v1(const ProjectSettingsDocument* doc, const char* path) {
    FILE* fp;
    if (doc == nullptr || path == nullptr) return 0;
    fp = fopen(path, "w");
    if (fp == nullptr) return 0;
    fprintf(fp, "schema_version|%d\n", PROJECT_WORKSPACE_SCHEMA_VERSION_V1);
    fprintf(fp, "auto_reload_assets|%d\n", doc->auto_reload_assets ? 1 : 0);
    fprintf(fp, "hot_reload_in_pie|%d\n", doc->hot_reload_in_pie ? 1 : 0);
    fprintf(fp, "reopen_last_workspace|%d\n", doc->reopen_last_workspace ? 1 : 0);
    fprintf(fp, "last_open_scene|%s\n", doc->last_open_scene);
    return fclose(fp) == 0;
}

int project_workspace_validate_layout_v1(const char* root_path) {
    const std::filesystem::path root = root_path == nullptr ? "." : root_path;
    return std::filesystem::exists(root / "Assets") &&
           std::filesystem::exists(root / "Scenes") &&
           std::filesystem::exists(root / "Prefabs") &&
           std::filesystem::exists(root / "Packages") &&
           std::filesystem::exists(root / "ProjectSettings") &&
           std::filesystem::exists(root / "Build") &&
           std::filesystem::exists(root / "Cache");
}

int project_workspace_bootstrap_v1(const char* root_path,
                                   const WorkspaceDocument* workspace,
                                   const ProjectDocument* project,
                                   const PackageDocument* package,
                                   const ProjectSettingsDocument* settings,
                                   ProjectWorkspaceBootstrapReport* out_report) {
    std::filesystem::path root;
    ProjectWorkspaceBootstrapReport report{};
    if (root_path == nullptr || workspace == nullptr || project == nullptr || package == nullptr || settings == nullptr) return 0;
    root = std::filesystem::path(root_path);
    if (!ensure_directory_counted(root, &report.created_directory_count) ||
        !ensure_directory_counted(root / "Assets", &report.created_directory_count) ||
        !ensure_directory_counted(root / "Scenes", &report.created_directory_count) ||
        !ensure_directory_counted(root / "Prefabs", &report.created_directory_count) ||
        !ensure_directory_counted(root / "ProjectSettings", &report.created_directory_count) ||
        !ensure_directory_counted(root / "Packages", &report.created_directory_count) ||
        !ensure_directory_counted(root / "Build", &report.created_directory_count) ||
        !ensure_directory_counted(root / "Cache", &report.created_directory_count)) {
        return 0;
    }

    if (!std::filesystem::exists(root / "ProjectSettings" / "workspace.physicsworkspace")) {
        if (!workspace_document_save_v1(workspace, (root / "ProjectSettings" / "workspace.physicsworkspace").string().c_str())) return 0;
        report.created_file_count++;
    }
    if (!std::filesystem::exists(root / "ProjectSettings" / "project.physicsproject")) {
        if (!project_document_save_v1(project, (root / "ProjectSettings" / "project.physicsproject").string().c_str())) return 0;
        report.created_file_count++;
    }
    if (!std::filesystem::exists(root / "Packages" / "core_tools.physicspackage")) {
        if (!package_document_save_v1(package, (root / "Packages" / "core_tools.physicspackage").string().c_str())) return 0;
        report.created_file_count++;
    }
    if (!std::filesystem::exists(root / "ProjectSettings" / "editor.physicssettings")) {
        if (!project_settings_document_save_v1(settings, (root / "ProjectSettings" / "editor.physicssettings").string().c_str())) return 0;
        report.created_file_count++;
    }
    if (out_report != nullptr) *out_report = report;
    return 1;
}
