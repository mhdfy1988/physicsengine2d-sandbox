#ifndef PROJECT_WORKSPACE_H
#define PROJECT_WORKSPACE_H

enum {
    PROJECT_WORKSPACE_SCHEMA_VERSION_V1 = 1,
    PROJECT_WORKSPACE_MAX_NAME = 64,
    PROJECT_WORKSPACE_MAX_GUID = 96,
    PROJECT_WORKSPACE_MAX_PATH = 260,
    PROJECT_WORKSPACE_MAX_VERSION = 32,
    PROJECT_WORKSPACE_MAX_PROJECTS = 16,
    PROJECT_WORKSPACE_MAX_PACKAGES = 16,
    PROJECT_WORKSPACE_MAX_PLUGINS = 16
};

typedef struct {
    char project_guid[PROJECT_WORKSPACE_MAX_GUID];
    char project_name[PROJECT_WORKSPACE_MAX_NAME];
    char project_path[PROJECT_WORKSPACE_MAX_PATH];
} WorkspaceProjectRef;

typedef struct {
    int schema_version;
    char workspace_name[PROJECT_WORKSPACE_MAX_NAME];
    char active_project_guid[PROJECT_WORKSPACE_MAX_GUID];
    int project_count;
    WorkspaceProjectRef projects[PROJECT_WORKSPACE_MAX_PROJECTS];
} WorkspaceDocument;

typedef struct {
    char package_id[PROJECT_WORKSPACE_MAX_NAME];
    char package_path[PROJECT_WORKSPACE_MAX_PATH];
    int enabled;
} ProjectPackageRef;

typedef struct {
    int schema_version;
    char project_guid[PROJECT_WORKSPACE_MAX_GUID];
    char project_name[PROJECT_WORKSPACE_MAX_NAME];
    char engine_version[PROJECT_WORKSPACE_MAX_VERSION];
    char startup_scene[PROJECT_WORKSPACE_MAX_GUID];
    int package_count;
    ProjectPackageRef packages[PROJECT_WORKSPACE_MAX_PACKAGES];
} ProjectDocument;

typedef struct {
    char plugin_id[PROJECT_WORKSPACE_MAX_NAME];
    char plugin_manifest[PROJECT_WORKSPACE_MAX_PATH];
} PackagePluginRef;

typedef struct {
    int schema_version;
    char package_id[PROJECT_WORKSPACE_MAX_NAME];
    char display_name[PROJECT_WORKSPACE_MAX_NAME];
    char package_version[PROJECT_WORKSPACE_MAX_VERSION];
    char engine_version_range[PROJECT_WORKSPACE_MAX_VERSION];
    int plugin_count;
    PackagePluginRef plugins[PROJECT_WORKSPACE_MAX_PLUGINS];
} PackageDocument;

typedef struct {
    int schema_version;
    int auto_reload_assets;
    int hot_reload_in_pie;
    int reopen_last_workspace;
    char last_open_scene[PROJECT_WORKSPACE_MAX_GUID];
} ProjectSettingsDocument;

typedef struct {
    int created_directory_count;
    int created_file_count;
} ProjectWorkspaceBootstrapReport;

void workspace_document_init(WorkspaceDocument* doc);
void project_document_init(ProjectDocument* doc);
void package_document_init(PackageDocument* doc);
void project_settings_document_init(ProjectSettingsDocument* doc);

int workspace_document_load_v1(const char* path, WorkspaceDocument* out_doc);
int workspace_document_save_v1(const WorkspaceDocument* doc, const char* path);
int project_document_load_v1(const char* path, ProjectDocument* out_doc);
int project_document_save_v1(const ProjectDocument* doc, const char* path);
int package_document_load_v1(const char* path, PackageDocument* out_doc);
int package_document_save_v1(const PackageDocument* doc, const char* path);
int project_settings_document_load_v1(const char* path, ProjectSettingsDocument* out_doc);
int project_settings_document_save_v1(const ProjectSettingsDocument* doc, const char* path);

int project_workspace_validate_layout_v1(const char* root_path);
int project_workspace_bootstrap_v1(const char* root_path,
                                   const WorkspaceDocument* workspace,
                                   const ProjectDocument* project,
                                   const PackageDocument* package,
                                   const ProjectSettingsDocument* settings,
                                   ProjectWorkspaceBootstrapReport* out_report);

#endif
