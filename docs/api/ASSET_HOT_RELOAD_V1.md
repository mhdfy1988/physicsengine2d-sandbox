# Asset Hot-Reload v1 (C3 Kickoff)

This module bridges debounced file-change events to incremental reimport pipeline execution.

## 1. Components

1. `asset_watch`: collects file-change events, applies debounce, emits ready batch.
2. `asset_hot_reload`: consumes ready batch, marks pipeline source changes, triggers reimport run.
3. `asset_pipeline`: handles dependency invalidation and importer execution.
4. `asset_fs_poll`: polls watched file metadata (`exists/mtime/size`) and feeds change events into `asset_hot_reload`.
   - Windows backend uses `GetFileAttributesExA` high-resolution write timestamps.
   - Re-watching an already tracked path is idempotent and does not reset baseline metadata.

## 2. Tick Contract

1. `asset_hot_reload_on_file_changed(path, ts)` records events.
2. `asset_hot_reload_tick(now, cache_root, report)`:
   - flushes ready changes (`now - last_change >= debounce`)
   - pushes batch to pipeline change queue
   - executes one pipeline reimport run
   - returns structured report (ready batch + pipeline report)

## 3. Behavior Guarantees

1. Duplicate events within debounce window collapse to one path.
2. Tick before debounce threshold is no-op.
3. Tick after threshold produces deterministic batch and reimport report.
4. `asset_fs_poll_scan` can be used as the concrete file-change producer when native watcher integration is unavailable.
5. Failed reimport keeps last successful artifact on disk (rollback-safe fallback baseline).
6. Re-watching an already watched path must not reset poll baseline (prevents dropped changes during periodic rediscovery).

## 4. Sandbox Runtime Integration

1. `apps/sandbox_dwrite/main.cpp` initializes one hot-reload service at bootstrap.
2. Startup recursively discovers supported sources under `assets/` and registers them into:
   - pipeline source registry
   - fs-poll watch list
3. Runtime tick flow:
   - periodic source discovery (to include newly added files)
   - poll delta scan
   - debounce + incremental reimport
   - publish `APP_EVENT_HOT_RELOAD_BATCH`
4. Failed hot-reload batches are bridged into runtime error channel (`APP_RUNTIME_ERROR_CODE_PIPELINE_MAPPING_ERRORS`) on next tick.

## 5. Smoke Script

Run `scripts/dev/hot_reload_smoke.ps1` to execute a local sandbox smoke:

1. launches `bin/physics_sandbox.exe`
2. mutates `assets/hot_reload_smoke/smoke_texture.png` twice
3. verifies corresponding `Cache/imported/<guid>.asset` is updated for both mutations

Notes:
1. Script includes retry strategy for the second mutation window to tolerate poll-tick race timing.
2. Use `-SkipLaunch` for preflight validation without launching UI process.
3. Script defaults to cleanup/rollback of temporary smoke files; pass `-KeepArtifacts` to preserve generated artifacts/log traces.
