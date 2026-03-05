# Runtime Error Semantics (Phase B)

This document defines a stable mapping for runtime bridge errors in C++ facade and editor-facing logs.

## Source

- C++ runtime facade error channel: `cpp/physics_runtime_facade.hpp`
- Editor runtime snapshot fields: `apps/sandbox_dwrite/infrastructure/app_event_bus.h`
- Editor log/render sink: `apps/sandbox_dwrite/main.c`

## RuntimeErrorCode Mapping

| RuntimeErrorCode | Name | Severity | Notes |
|---|---|---|---|
| `BridgeMissingReverse` | `bridge_missing_reverse` | `warning` | Runtime body exists but reverse body->entity map is missing/mismatched. |
| `BridgeStaleEntity` | `bridge_stale_entity` | `warning` | Runtime reference points to non-alive entity. |
| `BridgeNullBody` | `bridge_null_body` | `error` | Runtime reference body pointer is null. |
| `BridgeDuplicateBody` | `bridge_duplicate_body` | `error` | Multiple entities map to the same body pointer. |
| `BridgeRefCountMismatch` | `bridge_ref_count_mismatch` | `warning` | Forward/reverse mapping counts do not match. |
| `PipelineMappingErrors` | `pipeline_mapping_errors` | `warning` | Pipeline reports bridge mapping inconsistencies. |

## Editor Snapshot Fields

- `runtime_error_count`: currently `0/1` in C runtime path (no error / has error).
- `runtime_error_code`: from `physics_engine_get_last_error(engine)`.
- Error text in logs/panel is normalized by `runtime_error_label(...)` to keep consistency.

## Logging Contract

- On error: `[警告] 运行时错误 code=<n> (<name>)`
- On recovery: `[状态] 运行时错误已恢复`

## Notes

- C++ facade emits detailed multi-item error list (`last_errors()`).
- C editor path currently consumes C engine last-error (`0/1 + code`); this is an intentional incremental bridge in Phase B.
- Next step in Phase C: unify editor event bus with facade-level multi-error payload.
