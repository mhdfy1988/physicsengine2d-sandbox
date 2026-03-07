#ifndef ASSET_FS_WATCH_H
#define ASSET_FS_WATCH_H

#include "physics_content/asset_fs_poll.hpp"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    ASSET_FS_WATCH_BACKEND_POLL = 0,
    ASSET_FS_WATCH_BACKEND_WIN32_NOTIFY = 1,
    ASSET_FS_WATCH_MAX_DIRECTORIES = 16
};

typedef struct {
    char root_dir[ASSET_WATCH_MAX_PATH];
    void* handle;
} AssetFsWatchDirectory;

typedef struct {
    AssetFsPollState poll;
    int backend;
    int native_ready;
    int directory_count;
    AssetFsWatchDirectory directories[ASSET_FS_WATCH_MAX_DIRECTORIES];
} AssetFsWatchState;

void asset_fs_watch_init(AssetFsWatchState* state);
void asset_fs_watch_shutdown(AssetFsWatchState* state);
int asset_fs_watch_watch_path(AssetFsWatchState* state, const char* source_path);
int asset_fs_watch_watch_directory(AssetFsWatchState* state, const char* root_dir);
int asset_fs_watch_scan(
    AssetFsWatchState* state,
    AssetHotReloadService* service,
    long long timestamp_ms,
    int* out_change_count);
int asset_fs_watch_get_watch_count(const AssetFsWatchState* state);
int asset_fs_watch_is_native_active(const AssetFsWatchState* state);

#ifdef __cplusplus
}
#endif

#endif
