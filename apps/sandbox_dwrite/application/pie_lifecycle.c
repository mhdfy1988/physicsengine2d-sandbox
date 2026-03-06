#include <stddef.h>
#include <string.h>
#include "pie_lifecycle.h"

static const char* PIE_DEFAULT_SNAPSHOT_PATH = "pie_editor_state_snapshot.txt";

static void pie_log(const PieLifecycleOps* ops, const wchar_t* text) {
    if (ops == NULL || text == NULL) return;
    if (ops->log_text == NULL) return;
    ops->log_text(text, ops->user);
}

void pie_lifecycle_init(PieLifecycle* lifecycle, const char* snapshot_path) {
    const char* source_path = snapshot_path;
    if (lifecycle == NULL) return;
    lifecycle->active = 0;
    if (source_path == NULL || source_path[0] == '\0') {
        source_path = PIE_DEFAULT_SNAPSHOT_PATH;
    }
    strncpy(lifecycle->snapshot_path, source_path, PIE_SNAPSHOT_PATH_CAP - 1);
    lifecycle->snapshot_path[PIE_SNAPSHOT_PATH_CAP - 1] = '\0';
}

int pie_lifecycle_enter(PieLifecycle* lifecycle, const PieLifecycleOps* ops) {
    if (lifecycle == NULL || ops == NULL || ops->save_snapshot == NULL) return 0;
    if (lifecycle->active) return 1;
    if (!ops->save_snapshot(lifecycle->snapshot_path, ops->user)) {
        pie_log(ops, L"[PIE] Enter failed: unable to save editor snapshot");
        return 0;
    }
    lifecycle->active = 1;
    pie_log(ops, L"[PIE] Entered (editor snapshot frozen)");
    return 1;
}

int pie_lifecycle_exit(PieLifecycle* lifecycle, const PieLifecycleOps* ops) {
    if (lifecycle == NULL || ops == NULL || ops->load_snapshot == NULL) return 0;
    if (!lifecycle->active) return 1;
    if (!ops->load_snapshot(lifecycle->snapshot_path, ops->user)) {
        pie_log(ops, L"[PIE] Exit failed: unable to restore editor snapshot");
        return 0;
    }
    lifecycle->active = 0;
    pie_log(ops, L"[PIE] Exited and restored editor snapshot");
    return 1;
}
