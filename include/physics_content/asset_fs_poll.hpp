#ifndef ASSET_FS_POLL_H
#define ASSET_FS_POLL_H

#include "physics_content/asset_hot_reload.hpp"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    ASSET_FS_POLL_MAX_WATCHED = 512
};

typedef struct {
    char source_path[ASSET_WATCH_MAX_PATH];
    int exists;
    long long mtime_sec;
    long long size_bytes;
} AssetFsPollEntry;

typedef struct {
    int count;
    AssetFsPollEntry entries[ASSET_FS_POLL_MAX_WATCHED];
} AssetFsPollState;

void asset_fs_poll_init(AssetFsPollState* state);
int asset_fs_poll_watch_path(AssetFsPollState* state, const char* source_path);
int asset_fs_poll_scan(
    AssetFsPollState* state,
    AssetHotReloadService* service,
    long long timestamp_ms,
    int* out_change_count);

#ifdef __cplusplus
}
#endif

#endif
