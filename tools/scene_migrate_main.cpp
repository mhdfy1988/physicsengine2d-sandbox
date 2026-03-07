#include <cstdio>
#include <string>
#include "physics_content/scene_schema.hpp"
#include "physics2d/physics_filesystem.hpp"

static void print_usage() {
    std::printf("Usage: scene_migrate --in <legacy_snapshot.txt> [--out <scene_v1.txt>] [--dry-run]\n");
}

int main(int argc, char** argv) {
    const char* input_path = nullptr;
    const char* output_path = nullptr;
    std::string default_out;
    int dry_run = 0;
    SceneSchemaMigrationOptions options{};
    SceneSchemaMigrationReport report{};

    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--in") {
            if (i + 1 >= argc) {
                print_usage();
                return 2;
            }
            input_path = argv[++i];
        } else if (std::string(argv[i]) == "--out") {
            if (i + 1 >= argc) {
                print_usage();
                return 2;
            }
            output_path = argv[++i];
        } else if (std::string(argv[i]) == "--dry-run") {
            dry_run = 1;
        } else {
            print_usage();
            return 2;
        }
    }

    if (input_path == nullptr) {
        print_usage();
        return 2;
    }

    const physics2d::foundation::Path input = physics2d::foundation::normalize_path(input_path);
    if (!dry_run && output_path == nullptr) {
        default_out = input.string() + ".v1.scene";
        output_path = default_out.c_str();
    }

    options.dry_run = dry_run ? 1 : 0;
    if (!scene_schema_migrate_legacy_snapshot_to_v1(input.string().c_str(), output_path, &options, &report)) {
        std::printf("[FAIL] migration failed: in=%s out=%s dry_run=%d\n",
                    input.string().c_str(),
                    (output_path != nullptr) ? output_path : "(null)",
                    options.dry_run);
        return 1;
    }

    std::printf("[OK] source_v=%d target_v=%d entities=%d constraints=%d warnings=%d failures=%d dry_run=%d\n",
                report.source_version,
                report.target_version,
                report.entity_count,
                report.constraint_count,
                report.warning_count,
                report.failure_count,
                options.dry_run);
    if (!dry_run) {
        std::printf("[OUT] %s\n", output_path);
    }
    return 0;
}
