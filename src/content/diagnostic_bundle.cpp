#include "physics_content/diagnostic_bundle.hpp"

#include <stdio.h>
#include <string.h>

#include <filesystem>
#include <fstream>

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

}  // namespace

void diagnostic_bundle_request_init(DiagnosticBundleRequest* request) {
    if (request == nullptr) return;
    memset(request, 0, sizeof(*request));
    copy_text(request->output_root, DIAGNOSTIC_BUNDLE_MAX_PATH, "artifacts/diagnostics");
    copy_text(request->label, DIAGNOSTIC_BUNDLE_MAX_LABEL, "manual_export");
}

int diagnostic_bundle_export_v1(const DiagnosticBundleRequest* request, DiagnosticBundleResult* out_result) {
    std::filesystem::path root;
    std::filesystem::path bundle_dir;
    std::ofstream summary;
    DiagnosticBundleResult result{};
    int file_index;
    if (request == nullptr || out_result == nullptr) return 0;
    root = std::filesystem::path(request->output_root[0] != '\0' ? request->output_root : "artifacts/diagnostics");
    std::filesystem::create_directories(root);
    bundle_dir = root / request->label;
    std::filesystem::create_directories(bundle_dir);

    if (request->build_metadata_path[0] != '\0' && std::filesystem::exists(request->build_metadata_path)) {
        std::filesystem::copy_file(request->build_metadata_path,
                                   bundle_dir / "build_metadata.json",
                                   std::filesystem::copy_options::overwrite_existing);
        result.copied_file_count++;
    }
    for (file_index = 0; file_index < request->file_count && file_index < DIAGNOSTIC_BUNDLE_MAX_FILES; ++file_index) {
        const DiagnosticBundleFile* file = &request->files[file_index];
        const std::filesystem::path source = file->source_path;
        const std::filesystem::path target = bundle_dir / (file->bundle_name[0] != '\0' ? file->bundle_name : source.filename().string().c_str());
        if (file->source_path[0] == '\0' || !std::filesystem::exists(source)) continue;
        std::filesystem::copy_file(source, target, std::filesystem::copy_options::overwrite_existing);
        result.copied_file_count++;
    }

    summary.open((bundle_dir / "summary.txt").string().c_str(), std::ios::out | std::ios::trunc);
    if (!summary.is_open()) return 0;
    summary << "label=" << request->label << "\n";
    summary << "copied_files=" << result.copied_file_count << "\n";
    summary << "logs=" << request->log_line_count << "\n";
    for (file_index = 0; file_index < request->log_line_count && file_index < DIAGNOSTIC_BUNDLE_MAX_LOG_LINES; ++file_index) {
        summary << "log[" << file_index << "]=" << request->log_lines[file_index] << "\n";
        result.exported_log_line_count++;
    }
    summary.close();

    copy_text(result.bundle_dir, DIAGNOSTIC_BUNDLE_MAX_PATH, bundle_dir.string().c_str());
    *out_result = result;
    return 1;
}
