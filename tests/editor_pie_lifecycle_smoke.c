#include <stdio.h>
#include <string.h>
#include "../apps/sandbox_dwrite/application/pie_lifecycle.h"

typedef struct {
    int save_calls;
    int load_calls;
    int log_calls;
    int fail_save;
    int fail_load;
    char saved_path[PIE_SNAPSHOT_PATH_CAP];
    char loaded_path[PIE_SNAPSHOT_PATH_CAP];
} PieSmokeState;

static int cb_save_snapshot(const char* path, void* user) {
    PieSmokeState* state = (PieSmokeState*)user;
    if (state == NULL || path == NULL) return 0;
    state->save_calls++;
    strncpy(state->saved_path, path, PIE_SNAPSHOT_PATH_CAP - 1);
    state->saved_path[PIE_SNAPSHOT_PATH_CAP - 1] = '\0';
    return state->fail_save ? 0 : 1;
}

static int cb_load_snapshot(const char* path, void* user) {
    PieSmokeState* state = (PieSmokeState*)user;
    if (state == NULL || path == NULL) return 0;
    state->load_calls++;
    strncpy(state->loaded_path, path, PIE_SNAPSHOT_PATH_CAP - 1);
    state->loaded_path[PIE_SNAPSHOT_PATH_CAP - 1] = '\0';
    return state->fail_load ? 0 : 1;
}

static void cb_log_text(const wchar_t* text, void* user) {
    PieSmokeState* state = (PieSmokeState*)user;
    (void)text;
    if (state == NULL) return;
    state->log_calls++;
}

static PieLifecycleOps make_ops(PieSmokeState* state) {
    PieLifecycleOps ops;
    ops.save_snapshot = cb_save_snapshot;
    ops.load_snapshot = cb_load_snapshot;
    ops.log_text = cb_log_text;
    ops.user = state;
    return ops;
}

int main(void) {
    PieLifecycle lifecycle;
    PieSmokeState state;
    PieLifecycleOps ops;
    memset(&state, 0, sizeof(state));
    pie_lifecycle_init(&lifecycle, "tmp_pie_snapshot.txt");
    ops = make_ops(&state);

    if (!pie_lifecycle_enter(&lifecycle, &ops)) {
        printf("[FAIL] pie enter failed\n");
        return 1;
    }
    if (!lifecycle.active || state.save_calls != 1 || strcmp(state.saved_path, "tmp_pie_snapshot.txt") != 0) {
        printf("[FAIL] pie enter side effects mismatch\n");
        return 2;
    }

    if (!pie_lifecycle_enter(&lifecycle, &ops)) {
        printf("[FAIL] pie re-enter failed\n");
        return 3;
    }
    if (state.save_calls != 1) {
        printf("[FAIL] pie re-enter should be idempotent\n");
        return 4;
    }

    if (!pie_lifecycle_exit(&lifecycle, &ops)) {
        printf("[FAIL] pie exit failed\n");
        return 5;
    }
    if (lifecycle.active || state.load_calls != 1 || strcmp(state.loaded_path, "tmp_pie_snapshot.txt") != 0) {
        printf("[FAIL] pie exit side effects mismatch\n");
        return 6;
    }

    if (!pie_lifecycle_exit(&lifecycle, &ops)) {
        printf("[FAIL] pie double-exit failed\n");
        return 7;
    }
    if (state.load_calls != 1) {
        printf("[FAIL] pie exit should be idempotent\n");
        return 8;
    }

    memset(&state, 0, sizeof(state));
    state.fail_save = 1;
    pie_lifecycle_init(&lifecycle, "tmp_fail_save.txt");
    ops = make_ops(&state);
    if (pie_lifecycle_enter(&lifecycle, &ops)) {
        printf("[FAIL] pie enter should fail when save fails\n");
        return 9;
    }
    if (lifecycle.active) {
        printf("[FAIL] lifecycle should stay inactive after save failure\n");
        return 10;
    }

    memset(&state, 0, sizeof(state));
    pie_lifecycle_init(&lifecycle, "tmp_fail_load.txt");
    ops = make_ops(&state);
    if (!pie_lifecycle_enter(&lifecycle, &ops)) {
        printf("[FAIL] setup enter failed before load-failure test\n");
        return 11;
    }
    state.fail_load = 1;
    if (pie_lifecycle_exit(&lifecycle, &ops)) {
        printf("[FAIL] pie exit should fail when load fails\n");
        return 12;
    }
    if (!lifecycle.active) {
        printf("[FAIL] lifecycle should stay active after load failure\n");
        return 13;
    }

    printf("[PASS] editor pie lifecycle smoke\n");
    return 0;
}
