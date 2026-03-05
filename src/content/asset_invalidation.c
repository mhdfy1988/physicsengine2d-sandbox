#include "asset_invalidation.h"

#include <string.h>

static int asset_invalidation_find_asset_index(const AssetDependencyGraph* graph, const char* guid) {
    int i;
    if (graph == NULL || guid == NULL) return -1;
    for (i = 0; i < graph->asset_count; i++) {
        if (strcmp(graph->assets[i], guid) == 0) return i;
    }
    return -1;
}

void asset_invalidation_batch_init(AssetInvalidationBatch* batch) {
    if (batch == NULL) return;
    memset(batch, 0, sizeof(*batch));
}

int asset_invalidation_collect(
    const AssetDependencyGraph* graph,
    const char* changed_guid,
    AssetInvalidationBatch* out_batch) {
    int root;
    int queue[ASSET_DB_MAX_ASSETS];
    int visited[ASSET_DB_MAX_ASSETS];
    int head = 0;
    int tail = 0;
    int i;
    if (graph == NULL || changed_guid == NULL || out_batch == NULL) return 0;
    root = asset_invalidation_find_asset_index(graph, changed_guid);
    if (root < 0) return 0;

    asset_invalidation_batch_init(out_batch);
    memset(visited, 0, sizeof(visited));
    visited[root] = 1;
    queue[tail++] = root;

    while (head < tail) {
        int cur = queue[head++];
        if (out_batch->count < ASSET_DB_MAX_ASSETS) {
            strcpy(out_batch->guids[out_batch->count], graph->assets[cur]);
            out_batch->count++;
        }
        for (i = 0; i < graph->edge_count; i++) {
            int dep_from = graph->edge_from[i];
            int dep_to = graph->edge_to[i];
            if (dep_to == cur && !visited[dep_from]) {
                visited[dep_from] = 1;
                if (tail < ASSET_DB_MAX_ASSETS) queue[tail++] = dep_from;
            }
        }
    }
    return 1;
}
