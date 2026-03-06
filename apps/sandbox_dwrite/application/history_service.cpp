#include <stddef.h>
#include <stdio.h>
#include <windows.h>
#include "history_service.hpp"

static int history_file_exists(const char* path) {
    DWORD attr;
    if (path == NULL) return 0;
    attr = GetFileAttributesA(path);
    if (attr == INVALID_FILE_ATTRIBUTES) return 0;
    return ((attr & FILE_ATTRIBUTE_DIRECTORY) == 0);
}

static void history_slot_path(int idx, char* out_path, int cap) {
    if (out_path == NULL || cap <= 0) return;
    snprintf(out_path, (size_t)cap, "history_%02d.txt", idx);
}

static void history_shift_left(HistoryState* state, int max_slots) {
    int i;
    char src[64];
    char dst[64];
    if (state == NULL || max_slots <= 0) return;
    for (i = 1; i < max_slots; i++) {
        history_slot_path(i, src, 64);
        history_slot_path(i - 1, dst, 64);
        if (history_file_exists(src)) {
            CopyFileA(src, dst, FALSE);
        }
    }
    history_slot_path(max_slots - 1, src, 64);
    DeleteFileA(src);
    if (state->cursor > 0) state->cursor--;
    if (state->top > 0) state->top--;
}

static void history_truncate_after_cursor(HistoryState* state) {
    int i;
    char p[64];
    if (state == NULL) return;
    for (i = state->cursor + 1; i <= state->top; i++) {
        history_slot_path(i, p, 64);
        DeleteFileA(p);
    }
    state->top = state->cursor;
    state->count = state->top + 1;
}

void history_service_push_snapshot(HistoryState* state, const HistoryServiceOps* ops) {
    char p[64];
    if (state == NULL || ops == NULL || ops->save_snapshot == NULL) return;
    if (state->replaying) return;
    history_truncate_after_cursor(state);
    if (state->top >= ops->max_slots - 1) {
        history_shift_left(state, ops->max_slots);
    }
    state->cursor++;
    state->top = state->cursor;
    state->count = state->top + 1;
    history_slot_path(state->cursor, p, 64);
    ops->save_snapshot(p, ops->user);
}

void history_service_reset_and_capture(HistoryState* state, const HistoryServiceOps* ops) {
    int i;
    char p[64];
    if (state == NULL || ops == NULL || ops->save_snapshot == NULL) return;
    for (i = 0; i < ops->max_slots; i++) {
        history_slot_path(i, p, 64);
        DeleteFileA(p);
    }
    state->cursor = 0;
    state->top = 0;
    state->count = 1;
    history_slot_path(0, p, 64);
    ops->save_snapshot(p, ops->user);
}

void history_service_undo(HistoryState* state, const HistoryServiceOps* ops) {
    char p[64];
    if (state == NULL || ops == NULL || ops->load_snapshot == NULL) return;
    if (state->cursor <= 0) {
        if (ops->log_text != NULL) ops->log_text(L"[历史] 没有可撤销操作", ops->user);
        return;
    }
    state->cursor--;
    history_slot_path(state->cursor, p, 64);
    state->replaying = 1;
    if (ops->load_snapshot(p, ops->user)) {
        if (ops->log_text != NULL) ops->log_text(L"[历史] 已撤销", ops->user);
    } else {
        if (ops->log_text != NULL) ops->log_text(L"[错误] 撤销失败", ops->user);
    }
    state->replaying = 0;
}

void history_service_redo(HistoryState* state, const HistoryServiceOps* ops) {
    char p[64];
    if (state == NULL || ops == NULL || ops->load_snapshot == NULL) return;
    if (state->cursor >= state->top) {
        if (ops->log_text != NULL) ops->log_text(L"[历史] 没有可重做操作", ops->user);
        return;
    }
    state->cursor++;
    history_slot_path(state->cursor, p, 64);
    state->replaying = 1;
    if (ops->load_snapshot(p, ops->user)) {
        if (ops->log_text != NULL) ops->log_text(L"[历史] 已重做", ops->user);
    } else {
        if (ops->log_text != NULL) ops->log_text(L"[错误] 重做失败", ops->user);
    }
    state->replaying = 0;
}
