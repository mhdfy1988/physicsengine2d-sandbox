#ifndef DIAGNOSTIC_BUNDLE_H
#define DIAGNOSTIC_BUNDLE_H

enum {
    DIAGNOSTIC_BUNDLE_MAX_PATH = 260,
    DIAGNOSTIC_BUNDLE_MAX_FILES = 16,
    DIAGNOSTIC_BUNDLE_MAX_LOG_LINES = 32,
    DIAGNOSTIC_BUNDLE_MAX_LOG_CHARS = 160,
    DIAGNOSTIC_BUNDLE_MAX_LABEL = 64
};

typedef struct {
    char source_path[DIAGNOSTIC_BUNDLE_MAX_PATH];
    char bundle_name[DIAGNOSTIC_BUNDLE_MAX_LABEL];
} DiagnosticBundleFile;

typedef struct {
    char output_root[DIAGNOSTIC_BUNDLE_MAX_PATH];
    char build_metadata_path[DIAGNOSTIC_BUNDLE_MAX_PATH];
    char label[DIAGNOSTIC_BUNDLE_MAX_LABEL];
    int file_count;
    DiagnosticBundleFile files[DIAGNOSTIC_BUNDLE_MAX_FILES];
    int log_line_count;
    char log_lines[DIAGNOSTIC_BUNDLE_MAX_LOG_LINES][DIAGNOSTIC_BUNDLE_MAX_LOG_CHARS];
} DiagnosticBundleRequest;

typedef struct {
    char bundle_dir[DIAGNOSTIC_BUNDLE_MAX_PATH];
    int copied_file_count;
    int exported_log_line_count;
} DiagnosticBundleResult;

void diagnostic_bundle_request_init(DiagnosticBundleRequest* request);
int diagnostic_bundle_export_v1(const DiagnosticBundleRequest* request, DiagnosticBundleResult* out_result);

#endif
