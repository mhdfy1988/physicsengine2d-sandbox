# Asset Pipeline v1 (Incremental Reimport Queue)

This document defines the first queue-based reimport baseline for Phase C2.

## 1. Core Flow

1. Register importable sources (`source_path`, settings fingerprint, computed stable GUID).
   - If `<source>.meta` exists and contains structured parameters, pipeline auto-deserializes them.
2. Build dependency graph (`A depends on B`).
3. Mark changed assets (by GUID or source path).
4. Run queue:
   - collect impacted assets via reverse dependency propagation
   - deduplicate impacted set
   - import assets that have registered source entries
   - emit run report (requested / affected / imported / failed)
5. Clear pending change queue.

## 2. Determinism/Batch Guarantees

1. Duplicate change marks are deduplicated.
2. Reimport execution order is stable by first discovery in invalidation batches.
3. Empty queue run is a no-op and returns zero counters.
4. Batch mark API deduplicates repeated source paths in the same queue window.

## 3. Failure Contract

1. Missing or invalid source causes per-item failure increment.
2. Queue run still completes and returns full report.
3. Function return value is `0` when `failed_count > 0`, otherwise `1`.
