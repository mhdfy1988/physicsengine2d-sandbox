#include "asset_watch.h"

#include <string.h>

static void asset_watch_copy_text(char* out, int out_cap, const char* src) {
    if (out == NULL || out_cap <= 0) return;
    if (src == NULL) {
        out[0] = '\0';
        return;
    }
    strncpy(out, src, (size_t)(out_cap - 1));
    out[out_cap - 1] = '\0';
}

static int asset_watch_find_pending_index(const AssetWatchState* state, const char* source_path) {
    int i;
    if (state == NULL || source_path == NULL) return -1;
    for (i = 0; i < state->pending_count; i++) {
        if (strcmp(state->pending[i].source_path, source_path) == 0) return i;
    }
    return -1;
}

void asset_watch_batch_init(AssetWatchBatch* batch) {
    if (batch == NULL) return;
    memset(batch, 0, sizeof(*batch));
}

void asset_watch_init(AssetWatchState* state, int debounce_ms) {
    if (state == NULL) return;
    memset(state, 0, sizeof(*state));
    state->debounce_ms = (debounce_ms >= 0) ? debounce_ms : 0;
}

int asset_watch_push_change(AssetWatchState* state, const char* source_path, long long timestamp_ms) {
    int idx;
    if (state == NULL || source_path == NULL || source_path[0] == '\0') return 0;
    idx = asset_watch_find_pending_index(state, source_path);
    if (idx >= 0) {
        state->pending[idx].last_change_ms = timestamp_ms;
        return 1;
    }
    if (state->pending_count >= ASSET_WATCH_MAX_PENDING) return 0;
    idx = state->pending_count;
    state->pending_count++;
    asset_watch_copy_text(state->pending[idx].source_path, ASSET_WATCH_MAX_PATH, source_path);
    state->pending[idx].last_change_ms = timestamp_ms;
    return 1;
}

int asset_watch_collect_ready(AssetWatchState* state, long long now_ms, AssetWatchBatch* out_batch) {
    int i = 0;
    if (state == NULL || out_batch == NULL) return 0;
    asset_watch_batch_init(out_batch);
    while (i < state->pending_count) {
        long long dt = now_ms - state->pending[i].last_change_ms;
        if (dt >= (long long)state->debounce_ms) {
            if (out_batch->count < ASSET_WATCH_MAX_PENDING) {
                asset_watch_copy_text(out_batch->source_paths[out_batch->count], ASSET_WATCH_MAX_PATH, state->pending[i].source_path);
                out_batch->count++;
            }
            state->pending[i] = state->pending[state->pending_count - 1];
            state->pending_count--;
        } else {
            i++;
        }
    }
    return 1;
}
