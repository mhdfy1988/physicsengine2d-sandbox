#include "physics_content/asset_fs_watch.hpp"

#include <string.h>
#include <windows.h>

static void asset_fs_watch_copy_text(char* out, int out_cap, const char* src) {
    if (out == NULL || out_cap <= 0) return;
    if (src == NULL) {
        out[0] = '\0';
        return;
    }
    strncpy(out, src, (size_t)(out_cap - 1));
    out[out_cap - 1] = '\0';
}

static int asset_fs_watch_find_directory(const AssetFsWatchState* state, const char* root_dir) {
    int i;
    if (state == NULL || root_dir == NULL) return -1;
    for (i = 0; i < state->directory_count; i++) {
        if (strcmp(state->directories[i].root_dir, root_dir) == 0) return i;
    }
    return -1;
}

static HANDLE asset_fs_watch_handle_from_slot(AssetFsWatchDirectory* directory) {
    if (directory == NULL) return NULL;
    return (HANDLE)directory->handle;
}

static void asset_fs_watch_close_slot(AssetFsWatchDirectory* directory) {
    HANDLE handle;
    if (directory == NULL) return;
    handle = asset_fs_watch_handle_from_slot(directory);
    if (handle != NULL && handle != INVALID_HANDLE_VALUE) {
        FindCloseChangeNotification(handle);
    }
    directory->handle = NULL;
}

static void asset_fs_watch_update_native_ready(AssetFsWatchState* state) {
    int i;
    int active = 0;
    if (state == NULL) return;
    for (i = 0; i < state->directory_count; i++) {
        if (state->directories[i].handle != NULL) {
            active = 1;
            break;
        }
    }
    state->native_ready = active;
}

static int asset_fs_watch_native_should_scan(AssetFsWatchState* state) {
    int i;
    int saw_signal = 0;
    if (state == NULL || !state->native_ready) return 1;
    for (i = 0; i < state->directory_count; i++) {
        HANDLE handle = asset_fs_watch_handle_from_slot(&state->directories[i]);
        DWORD wait_result;
        if (handle == NULL || handle == INVALID_HANDLE_VALUE) continue;
        wait_result = WaitForSingleObject(handle, 0);
        if (wait_result == WAIT_OBJECT_0) {
            saw_signal = 1;
            if (!FindNextChangeNotification(handle)) {
                asset_fs_watch_close_slot(&state->directories[i]);
            }
        } else if (wait_result == WAIT_FAILED) {
            asset_fs_watch_close_slot(&state->directories[i]);
        }
    }
    asset_fs_watch_update_native_ready(state);
    if (!state->native_ready) {
        state->backend = ASSET_FS_WATCH_BACKEND_POLL;
        return 1;
    }
    return saw_signal;
}

void asset_fs_watch_init(AssetFsWatchState* state) {
    if (state == NULL) return;
    memset(state, 0, sizeof(*state));
    asset_fs_poll_init(&state->poll);
    state->backend = ASSET_FS_WATCH_BACKEND_POLL;
}

void asset_fs_watch_shutdown(AssetFsWatchState* state) {
    int i;
    if (state == NULL) return;
    for (i = 0; i < state->directory_count; i++) {
        asset_fs_watch_close_slot(&state->directories[i]);
    }
    state->directory_count = 0;
    state->native_ready = 0;
    state->backend = ASSET_FS_WATCH_BACKEND_POLL;
}

int asset_fs_watch_watch_path(AssetFsWatchState* state, const char* source_path) {
    if (state == NULL) return 0;
    return asset_fs_poll_watch_path(&state->poll, source_path);
}

int asset_fs_watch_watch_directory(AssetFsWatchState* state, const char* root_dir) {
    int idx;
    HANDLE handle;
    if (state == NULL || root_dir == NULL || root_dir[0] == '\0') return 0;
    idx = asset_fs_watch_find_directory(state, root_dir);
    if (idx >= 0) return 1;
    if (state->directory_count >= ASSET_FS_WATCH_MAX_DIRECTORIES) return 0;
    handle = FindFirstChangeNotificationA(root_dir, TRUE,
                                          FILE_NOTIFY_CHANGE_FILE_NAME |
                                          FILE_NOTIFY_CHANGE_DIR_NAME |
                                          FILE_NOTIFY_CHANGE_SIZE |
                                          FILE_NOTIFY_CHANGE_LAST_WRITE);
    if (handle == INVALID_HANDLE_VALUE || handle == NULL) {
        return 0;
    }
    idx = state->directory_count;
    state->directory_count++;
    asset_fs_watch_copy_text(state->directories[idx].root_dir, ASSET_WATCH_MAX_PATH, root_dir);
    state->directories[idx].handle = (void*)handle;
    state->native_ready = 1;
    state->backend = ASSET_FS_WATCH_BACKEND_WIN32_NOTIFY;
    return 1;
}

int asset_fs_watch_scan(
    AssetFsWatchState* state,
    AssetHotReloadService* service,
    long long timestamp_ms,
    int* out_change_count) {
    if (state == NULL || service == NULL) return 0;
    if (out_change_count != NULL) *out_change_count = 0;
    if (state->backend == ASSET_FS_WATCH_BACKEND_WIN32_NOTIFY && state->native_ready) {
        if (!asset_fs_watch_native_should_scan(state)) {
            return 1;
        }
    }
    return asset_fs_poll_scan(&state->poll, service, timestamp_ms, out_change_count);
}

int asset_fs_watch_get_watch_count(const AssetFsWatchState* state) {
    if (state == NULL) return 0;
    return state->poll.count;
}

int asset_fs_watch_is_native_active(const AssetFsWatchState* state) {
    if (state == NULL) return 0;
    return (state->backend == ASSET_FS_WATCH_BACKEND_WIN32_NOTIFY && state->native_ready) ? 1 : 0;
}
