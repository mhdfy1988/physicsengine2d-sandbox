#include "asset_fs_poll.h"

#include <string.h>
#include <windows.h>

static void asset_fs_poll_copy_text(char* out, int out_cap, const char* src) {
    if (out == NULL || out_cap <= 0) return;
    if (src == NULL) {
        out[0] = '\0';
        return;
    }
    strncpy(out, src, (size_t)(out_cap - 1));
    out[out_cap - 1] = '\0';
}

static int asset_fs_poll_find_index(const AssetFsPollState* state, const char* source_path) {
    int i;
    if (state == NULL || source_path == NULL) return -1;
    for (i = 0; i < state->count; i++) {
        if (strcmp(state->entries[i].source_path, source_path) == 0) return i;
    }
    return -1;
}

static void asset_fs_poll_query_file(
    const char* source_path,
    int* out_exists,
    long long* out_mtime_sec,
    long long* out_size_bytes) {
    WIN32_FILE_ATTRIBUTE_DATA data;
    ULARGE_INTEGER last_write;
    ULARGE_INTEGER file_size;
    if (out_exists != NULL) *out_exists = 0;
    if (out_mtime_sec != NULL) *out_mtime_sec = 0;
    if (out_size_bytes != NULL) *out_size_bytes = 0;
    if (source_path == NULL) return;
    if (GetFileAttributesExA(source_path, GetFileExInfoStandard, &data)) {
        last_write.LowPart = data.ftLastWriteTime.dwLowDateTime;
        last_write.HighPart = data.ftLastWriteTime.dwHighDateTime;
        file_size.LowPart = data.nFileSizeLow;
        file_size.HighPart = data.nFileSizeHigh;
        if (out_exists != NULL) *out_exists = 1;
        if (out_mtime_sec != NULL) *out_mtime_sec = (long long)last_write.QuadPart;
        if (out_size_bytes != NULL) *out_size_bytes = (long long)file_size.QuadPart;
    }
}

void asset_fs_poll_init(AssetFsPollState* state) {
    if (state == NULL) return;
    memset(state, 0, sizeof(*state));
}

int asset_fs_poll_watch_path(AssetFsPollState* state, const char* source_path) {
    int idx;
    int exists = 0;
    long long mtime_sec = 0;
    long long size_bytes = 0;
    if (state == NULL || source_path == NULL || source_path[0] == '\0') return 0;
    idx = asset_fs_poll_find_index(state, source_path);
    if (idx >= 0) {
        return 1;
    }
    if (state->count >= ASSET_FS_POLL_MAX_WATCHED) return 0;
    idx = state->count;
    state->count++;
    asset_fs_poll_copy_text(state->entries[idx].source_path, ASSET_WATCH_MAX_PATH, source_path);
    asset_fs_poll_query_file(source_path, &exists, &mtime_sec, &size_bytes);
    state->entries[idx].exists = exists;
    state->entries[idx].mtime_sec = mtime_sec;
    state->entries[idx].size_bytes = size_bytes;
    return 1;
}

int asset_fs_poll_scan(
    AssetFsPollState* state,
    AssetHotReloadService* service,
    long long timestamp_ms,
    int* out_change_count) {
    int i;
    int change_count = 0;
    if (state == NULL || service == NULL) return 0;
    for (i = 0; i < state->count; i++) {
        int exists = 0;
        long long mtime_sec = 0;
        long long size_bytes = 0;
        int changed = 0;
        asset_fs_poll_query_file(state->entries[i].source_path, &exists, &mtime_sec, &size_bytes);
        if (exists != state->entries[i].exists) {
            changed = 1;
        } else if (exists && (mtime_sec != state->entries[i].mtime_sec || size_bytes != state->entries[i].size_bytes)) {
            changed = 1;
        }
        state->entries[i].exists = exists;
        state->entries[i].mtime_sec = mtime_sec;
        state->entries[i].size_bytes = size_bytes;
        if (changed) {
            if (!asset_hot_reload_on_file_changed(service, state->entries[i].source_path, timestamp_ms)) {
                return 0;
            }
            change_count++;
        }
    }
    if (out_change_count != NULL) *out_change_count = change_count;
    return 1;
}
