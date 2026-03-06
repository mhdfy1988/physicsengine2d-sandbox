#ifndef ASSET_INVALIDATION_H
#define ASSET_INVALIDATION_H

#include "asset_database.hpp"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int count;
    char guids[ASSET_DB_MAX_ASSETS][ASSET_DB_MAX_GUID];
} AssetInvalidationBatch;

void asset_invalidation_batch_init(AssetInvalidationBatch* batch);
int asset_invalidation_collect(
    const AssetDependencyGraph* graph,
    const char* changed_guid,
    AssetInvalidationBatch* out_batch);

#ifdef __cplusplus
}
#endif

#endif
