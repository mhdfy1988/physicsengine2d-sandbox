#include <stdio.h>
#include <string.h>

#include "scene_schema.h"

static void print_usage(void) {
    printf("Usage: scene_migrate --in <legacy_snapshot.txt> [--out <scene_v1.txt>] [--dry-run]\n");
}

int main(int argc, char** argv) {
    const char* input_path = NULL;
    const char* output_path = NULL;
    char default_out[512];
    int dry_run = 0;
    int i;
    SceneSchemaMigrationOptions options;
    SceneSchemaMigrationReport report;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--in") == 0) {
            if (i + 1 >= argc) {
                print_usage();
                return 2;
            }
            input_path = argv[++i];
        } else if (strcmp(argv[i], "--out") == 0) {
            if (i + 1 >= argc) {
                print_usage();
                return 2;
            }
            output_path = argv[++i];
        } else if (strcmp(argv[i], "--dry-run") == 0) {
            dry_run = 1;
        } else {
            print_usage();
            return 2;
        }
    }

    if (input_path == NULL) {
        print_usage();
        return 2;
    }

    if (!dry_run && output_path == NULL) {
        int n = snprintf(default_out, sizeof(default_out), "%s.v1.scene", input_path);
        if (n <= 0 || n >= (int)sizeof(default_out)) {
            printf("[FAIL] output path too long\n");
            return 2;
        }
        output_path = default_out;
    }

    options.dry_run = dry_run ? 1 : 0;
    if (!scene_schema_migrate_legacy_snapshot_to_v1(input_path, output_path, &options, &report)) {
        printf("[FAIL] migration failed: in=%s out=%s dry_run=%d\n",
               input_path,
               (output_path != NULL) ? output_path : "(null)",
               options.dry_run);
        return 1;
    }

    printf("[OK] source_v=%d target_v=%d entities=%d constraints=%d warnings=%d failures=%d dry_run=%d\n",
           report.source_version,
           report.target_version,
           report.entity_count,
           report.constraint_count,
           report.warning_count,
           report.failure_count,
           options.dry_run);
    if (!dry_run) {
        printf("[OUT] %s\n", output_path);
    }
    return 0;
}
