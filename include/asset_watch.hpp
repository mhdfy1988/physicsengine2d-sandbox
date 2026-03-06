#ifndef ASSET_WATCH_H
#define ASSET_WATCH_H

#ifdef __cplusplus
extern "C" {
#endif

enum {
    ASSET_WATCH_MAX_PATH = 512,
    ASSET_WATCH_MAX_PENDING = 512
};

typedef struct {
    char source_path[ASSET_WATCH_MAX_PATH];
    long long last_change_ms;
} AssetWatchPendingItem;

typedef struct {
    int count;
    char source_paths[ASSET_WATCH_MAX_PENDING][ASSET_WATCH_MAX_PATH];
} AssetWatchBatch;

typedef struct {
    int debounce_ms;
    int pending_count;
    AssetWatchPendingItem pending[ASSET_WATCH_MAX_PENDING];
} AssetWatchState;

void asset_watch_batch_init(AssetWatchBatch* batch);
void asset_watch_init(AssetWatchState* state, int debounce_ms);
int asset_watch_push_change(AssetWatchState* state, const char* source_path, long long timestamp_ms);
int asset_watch_collect_ready(AssetWatchState* state, long long now_ms, AssetWatchBatch* out_batch);

#ifdef __cplusplus
}
#endif

#endif
