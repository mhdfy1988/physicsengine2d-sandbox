#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include "../apps/sandbox_dwrite/application/editor_command_bus.h"
#include "../apps/sandbox_dwrite/application/history_service.h"

typedef struct {
    int value;
    int order_cursor;
    wchar_t scene_name[EDITOR_COMMAND_TEXT_CAP];
} EditorSmokeSnapshot;

typedef struct {
    int value;
    int order_cursor;
    wchar_t scene_name[EDITOR_COMMAND_TEXT_CAP];
    EditorSmokeSnapshot slots[32];
    int slot_valid[32];
    HistoryState history;
    HistoryServiceOps history_ops;
    int rename_count;
    int move_count;
    int reset_count;
    int inspector_set_count;
    int last_move_direction;
    wchar_t last_scene_name[EDITOR_COMMAND_TEXT_CAP];
} EditorSmokeState;

static int slot_index_from_path(const char* path) {
    int idx = -1;
    if (path == NULL) return -1;
    if (sscanf(path, "history_%02d.txt", &idx) != 1) return -1;
    if (idx < 0 || idx >= 32) return -1;
    return idx;
}

static int cb_save_snapshot(const char* path, void* user) {
    EditorSmokeState* s = (EditorSmokeState*)user;
    int idx = slot_index_from_path(path);
    if (s == NULL || idx < 0) return 0;
    s->slots[idx].value = s->value;
    s->slots[idx].order_cursor = s->order_cursor;
    wcsncpy(s->slots[idx].scene_name, s->scene_name, EDITOR_COMMAND_TEXT_CAP - 1);
    s->slots[idx].scene_name[EDITOR_COMMAND_TEXT_CAP - 1] = L'\0';
    s->slot_valid[idx] = 1;
    return 1;
}

static int cb_load_snapshot(const char* path, void* user) {
    EditorSmokeState* s = (EditorSmokeState*)user;
    int idx = slot_index_from_path(path);
    if (s == NULL || idx < 0 || !s->slot_valid[idx]) return 0;
    s->value = s->slots[idx].value;
    s->order_cursor = s->slots[idx].order_cursor;
    wcsncpy(s->scene_name, s->slots[idx].scene_name, EDITOR_COMMAND_TEXT_CAP - 1);
    s->scene_name[EDITOR_COMMAND_TEXT_CAP - 1] = L'\0';
    return 1;
}

static void cb_log_text(const wchar_t* text, void* user) {
    (void)text;
    (void)user;
}

static int cb_scene_rename(int scene_index, const wchar_t* name, void* user) {
    EditorSmokeState* s = (EditorSmokeState*)user;
    (void)scene_index;
    if (s == NULL || name == NULL) return 0;
    s->rename_count++;
    wcsncpy(s->last_scene_name, name, EDITOR_COMMAND_TEXT_CAP - 1);
    s->last_scene_name[EDITOR_COMMAND_TEXT_CAP - 1] = L'\0';
    wcsncpy(s->scene_name, name, EDITOR_COMMAND_TEXT_CAP - 1);
    s->scene_name[EDITOR_COMMAND_TEXT_CAP - 1] = L'\0';
    history_service_push_snapshot(&s->history, &s->history_ops);
    return 1;
}

static int cb_scene_order_move(int direction, void* user) {
    EditorSmokeState* s = (EditorSmokeState*)user;
    if (s == NULL) return 0;
    s->move_count++;
    s->last_move_direction = direction;
    s->order_cursor += direction;
    if (s->order_cursor < -8) s->order_cursor = -8;
    if (s->order_cursor > 8) s->order_cursor = 8;
    history_service_push_snapshot(&s->history, &s->history_ops);
    return 1;
}

static int cb_scene_order_reset(void* user) {
    EditorSmokeState* s = (EditorSmokeState*)user;
    if (s == NULL) return 0;
    s->reset_count++;
    s->order_cursor = 0;
    history_service_push_snapshot(&s->history, &s->history_ops);
    return 1;
}

static int cb_inspector_set_value(int row, double value, int emit_log, void* user) {
    EditorSmokeState* s = (EditorSmokeState*)user;
    (void)emit_log;
    if (s == NULL || row != 0) return 0;
    s->value = (int)value;
    s->inspector_set_count++;
    history_service_push_snapshot(&s->history, &s->history_ops);
    return 1;
}

int main(void) {
    EditorSmokeState state;
    EditorCommandCallbacks callbacks;
    EditorCommand cmd;

    memset(&state, 0, sizeof(state));
    state.value = 10;
    state.order_cursor = 0;
    wcscpy(state.scene_name, L"Default");
    state.history_ops.max_slots = 16;
    state.history_ops.save_snapshot = cb_save_snapshot;
    state.history_ops.load_snapshot = cb_load_snapshot;
    state.history_ops.log_text = cb_log_text;
    state.history_ops.user = &state;
    history_service_reset_and_capture(&state.history, &state.history_ops);

    editor_command_bus_init(&callbacks);
    callbacks.scene_rename = cb_scene_rename;
    callbacks.scene_order_move = cb_scene_order_move;
    callbacks.scene_order_reset = cb_scene_order_reset;
    callbacks.inspector_set_value = cb_inspector_set_value;
    callbacks.user = &state;

    memset(&cmd, 0, sizeof(cmd));
    cmd.type = EDITOR_CMD_SCENE_RENAME;
    cmd.arg_i0 = 2;
    wcscpy(cmd.text, L"PhaseD_Rename");
    if (!editor_command_bus_dispatch(&callbacks, &cmd) ||
        state.rename_count != 1 ||
        wcscmp(state.last_scene_name, L"PhaseD_Rename") != 0) {
        printf("[FAIL] editor command scene rename dispatch failed\n");
        return 1;
    }

    memset(&cmd, 0, sizeof(cmd));
    cmd.type = EDITOR_CMD_SCENE_ORDER_MOVE;
    cmd.arg_i0 = 1;
    if (!editor_command_bus_dispatch(&callbacks, &cmd) ||
        state.move_count != 1 ||
        state.last_move_direction != 1 ||
        state.order_cursor != 1) {
        printf("[FAIL] editor command scene order move dispatch failed\n");
        return 2;
    }

    memset(&cmd, 0, sizeof(cmd));
    cmd.type = EDITOR_CMD_SCENE_ORDER_RESET;
    if (!editor_command_bus_dispatch(&callbacks, &cmd) || state.reset_count != 1 || state.order_cursor != 0) {
        printf("[FAIL] editor command scene order reset dispatch failed\n");
        return 3;
    }

    memset(&cmd, 0, sizeof(cmd));
    cmd.type = EDITOR_CMD_INSPECTOR_SET_VALUE;
    cmd.arg_i0 = 0;
    cmd.arg_f0 = 20.0;
    cmd.arg_i1 = 1;
    if (!editor_command_bus_dispatch(&callbacks, &cmd) || state.value != 20) {
        printf("[FAIL] inspector command first apply failed\n");
        return 4;
    }

    memset(&cmd, 0, sizeof(cmd));
    cmd.type = EDITOR_CMD_INSPECTOR_SET_VALUE;
    cmd.arg_i0 = 0;
    cmd.arg_f0 = 30.0;
    cmd.arg_i1 = 1;
    if (!editor_command_bus_dispatch(&callbacks, &cmd) || state.value != 30) {
        printf("[FAIL] inspector command second apply failed\n");
        return 5;
    }
    if (state.inspector_set_count != 2) {
        printf("[FAIL] inspector callback invocation count mismatch\n");
        return 6;
    }

    history_service_undo(&state.history, &state.history_ops);
    if (state.value != 20 || state.order_cursor != 0 || wcscmp(state.scene_name, L"PhaseD_Rename") != 0) {
        printf("[FAIL] undo step #1 mismatch\n");
        return 7;
    }
    history_service_undo(&state.history, &state.history_ops);
    if (state.value != 10 || state.order_cursor != 0 || wcscmp(state.scene_name, L"PhaseD_Rename") != 0) {
        printf("[FAIL] undo step #2 mismatch\n");
        return 8;
    }
    history_service_undo(&state.history, &state.history_ops);
    if (state.value != 10 || state.order_cursor != 1 || wcscmp(state.scene_name, L"PhaseD_Rename") != 0) {
        printf("[FAIL] undo step #3 mismatch\n");
        return 9;
    }
    history_service_undo(&state.history, &state.history_ops);
    if (state.value != 10 || state.order_cursor != 0 || wcscmp(state.scene_name, L"PhaseD_Rename") != 0) {
        printf("[FAIL] undo step #4 mismatch\n");
        return 10;
    }
    history_service_undo(&state.history, &state.history_ops);
    if (state.value != 10 || state.order_cursor != 0 || wcscmp(state.scene_name, L"Default") != 0) {
        printf("[FAIL] undo step #5 mismatch\n");
        return 11;
    }

    history_service_redo(&state.history, &state.history_ops);
    if (state.value != 10 || state.order_cursor != 0 || wcscmp(state.scene_name, L"PhaseD_Rename") != 0) {
        printf("[FAIL] redo step #1 mismatch\n");
        return 12;
    }
    history_service_redo(&state.history, &state.history_ops);
    if (state.value != 10 || state.order_cursor != 1 || wcscmp(state.scene_name, L"PhaseD_Rename") != 0) {
        printf("[FAIL] redo step #2 mismatch\n");
        return 13;
    }
    history_service_redo(&state.history, &state.history_ops);
    if (state.value != 10 || state.order_cursor != 0 || wcscmp(state.scene_name, L"PhaseD_Rename") != 0) {
        printf("[FAIL] redo step #3 mismatch\n");
        return 14;
    }
    history_service_redo(&state.history, &state.history_ops);
    if (state.value != 20 || state.order_cursor != 0 || wcscmp(state.scene_name, L"PhaseD_Rename") != 0) {
        printf("[FAIL] redo step #4 mismatch\n");
        return 15;
    }
    history_service_redo(&state.history, &state.history_ops);
    if (state.value != 30 || state.order_cursor != 0 || wcscmp(state.scene_name, L"PhaseD_Rename") != 0) {
        printf("[FAIL] redo step #5 mismatch\n");
        return 16;
    }

    printf("[PASS] editor undo/redo smoke\n");
    return 0;
}
