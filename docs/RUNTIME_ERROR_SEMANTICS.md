# Runtime Error Semantics (Phase B + Phase D Extensions)

This document defines a stable mapping for runtime bridge errors in C++ facade and editor-facing logs, including Phase D hot-reload taxonomy extensions.

## Source

- C++ runtime facade error channel: `cpp/physics_runtime_facade.hpp`
- Editor runtime snapshot fields: `apps/sandbox_dwrite/infrastructure/app_event_bus.hpp`
- Editor log/render sink: `apps/sandbox_dwrite/main.cpp`

## RuntimeErrorCode Mapping

| RuntimeErrorCode | Name | Severity | Notes |
|---|---|---|---|
| `BridgeMissingReverse` | `bridge_missing_reverse` | `warning` | Runtime body exists but reverse body->entity map is missing/mismatched. |
| `BridgeStaleEntity` | `bridge_stale_entity` | `warning` | Runtime reference points to non-alive entity. |
| `BridgeNullBody` | `bridge_null_body` | `error` | Runtime reference body pointer is null. |
| `BridgeDuplicateBody` | `bridge_duplicate_body` | `error` | Multiple entities map to the same body pointer. |
| `BridgeRefCountMismatch` | `bridge_ref_count_mismatch` | `warning` | Forward/reverse mapping counts do not match. |
| `PipelineMappingErrors` | `pipeline_mapping_errors` | `warning` | Pipeline reports bridge mapping inconsistencies. |

## App Runtime Hot-Reload Taxonomy (Phase D)

The following codes are app-runtime error channel extensions (editor/sandbox side), used to avoid collapsing all hot-reload failures into a single generic pipeline code:

| AppRuntimeErrorCode | Name | Severity | Trigger |
|---|---|---|---|
| `APP_RUNTIME_ERROR_CODE_HOT_RELOAD_SCAN_FAILED` | `hot_reload_scan_failed` | `warning` | Filesystem watch/scan stage failed. |
| `APP_RUNTIME_ERROR_CODE_HOT_RELOAD_IMPORT_FAILED` | `hot_reload_import_failed` | `error` | Hot-reload batch finished with one or more failed imports. |
| `APP_RUNTIME_ERROR_CODE_HOT_RELOAD_BATCH_FAILED` | `hot_reload_batch_failed` | `error` | Hot-reload batch pipeline failed before/without import-failure details. |

## Editor Snapshot Fields

- `runtime_error_count`: currently `0/1` in C runtime path (no error / has error).
- `runtime_error_code`: from `physics_engine_get_last_error(engine)`.
- Error text in logs/panel is normalized by `runtime_error_label(...)` to keep consistency.

## Logging Contract

- On error: `[警告] 运行时错误 code=<n> (<name>)`
- On recovery: `[状态] 运行时错误已恢复`

## Notes

- C++ facade emits detailed multi-item error list (`last_errors()`).
- C editor path supports pending multi-error injection via `app_runtime_set_runtime_errors(...)`.
- Hot-reload taxonomy codes are app-side and currently do not alter C++ facade `RuntimeErrorCode` enumeration.
