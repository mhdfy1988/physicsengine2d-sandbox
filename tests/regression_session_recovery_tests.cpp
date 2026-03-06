#include <stdio.h>
#include <string.h>

#include <filesystem>

#include "../include/session_recovery.hpp"
#include "../include/diagnostic_bundle.hpp"

int regression_test_session_recovery_roundtrip(void) {
    const char* path = "_tmp_session_recovery.txt";
    SessionRecoveryState state;
    SessionRecoveryState loaded;

    session_recovery_state_init(&state);
    strcpy(state.last_snapshot_path, "autosave_snapshot.txt");
    strcpy(state.replay_capture_path, "session_replay.txt");
    strcpy(state.last_scene_guid, "scene://stress");
    strcpy(state.recent_action, "autosave");
    state.unclean_shutdown = 1;
    if (!session_recovery_state_save_v1(&state, path) || !session_recovery_state_load_v1(path, &loaded)) {
        printf("[FAIL] session recovery roundtrip io failed\n");
        return 0;
    }
    if (loaded.unclean_shutdown != 1 ||
        strcmp(loaded.last_snapshot_path, "autosave_snapshot.txt") != 0 ||
        strcmp(loaded.replay_capture_path, "session_replay.txt") != 0 ||
        strcmp(loaded.last_scene_guid, "scene://stress") != 0) {
        printf("[FAIL] session recovery roundtrip mismatch\n");
        remove(path);
        return 0;
    }
    remove(path);
    printf("[PASS] session recovery roundtrip\n");
    return 1;
}

int regression_test_session_recovery_pending_restore(void) {
    SessionRecoveryState state;
    session_recovery_state_init(&state);
    session_recovery_mark_unclean(&state, "autosave_snapshot.txt", "autosave_snapshot.txt", "scene://default", "tick autosave");
    if (!session_recovery_should_restore(&state)) {
        printf("[FAIL] session recovery should restore after unclean shutdown\n");
        return 0;
    }
    session_recovery_mark_clean(&state, "clean exit");
    if (session_recovery_should_restore(&state)) {
        printf("[FAIL] session recovery should not restore after clean exit\n");
        return 0;
    }
    printf("[PASS] session recovery restore gating\n");
    return 1;
}

int regression_test_diagnostic_bundle_export(void) {
    const std::filesystem::path root = "_tmp_diag_bundle";
    const std::filesystem::path source = root / "input.txt";
    DiagnosticBundleRequest request;
    DiagnosticBundleResult result{};

    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root);
    {
        FILE* fp = fopen(source.string().c_str(), "w");
        if (fp == NULL) {
            printf("[FAIL] unable to create diagnostic bundle input\n");
            return 0;
        }
        fprintf(fp, "bundle test\n");
        fclose(fp);
    }

    diagnostic_bundle_request_init(&request);
    strcpy(request.output_root, "_tmp_diag_bundle/out");
    strcpy(request.label, "bundle_case");
    request.file_count = 1;
    strcpy(request.files[0].source_path, source.string().c_str());
    strcpy(request.files[0].bundle_name, "copied_input.txt");
    request.log_line_count = 2;
    strcpy(request.log_lines[0], "line one");
    strcpy(request.log_lines[1], "line two");
    if (!diagnostic_bundle_export_v1(&request, &result)) {
        printf("[FAIL] diagnostic bundle export failed\n");
        std::filesystem::remove_all(root);
        return 0;
    }
    if (!std::filesystem::exists(std::filesystem::path(result.bundle_dir) / "copied_input.txt") ||
        !std::filesystem::exists(std::filesystem::path(result.bundle_dir) / "summary.txt") ||
        result.copied_file_count != 1 ||
        result.exported_log_line_count != 2) {
        printf("[FAIL] diagnostic bundle export output mismatch\n");
        std::filesystem::remove_all(root);
        return 0;
    }
    std::filesystem::remove_all(root);
    printf("[PASS] diagnostic bundle export\n");
    return 1;
}
