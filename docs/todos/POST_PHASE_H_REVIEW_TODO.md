# Post Phase H Review TODO

Date: 2026-03-06
Scope: follow-up structural issues found during code review after the `cpp/` directory removal and sandbox startup fix

## 1. Prefab Schema Reentrancy

Status: pending
Severity: high

### Problem

`prefab_schema_load_v1()` and `prefab_schema_save_v1()` currently use static working buffers to avoid large stack frames:

- `src/content/prefab_schema.cpp`

This avoids startup stack overflow, but it also makes the API non-reentrant and unsafe for concurrent use. Any future parallel load/save path, nested call, or tool/editor shared usage can corrupt data through shared static storage.

### Why It Matters

- breaks thread-safety assumptions in content code
- introduces hidden shared mutable state
- makes future asset/prefab pipeline parallelism risky

### Follow-up Direction

Replace static working buffers with one of:

1. heap-backed temporary buffers
2. caller-provided scratch storage
3. streamed parsing / sorting that avoids full duplicate copies

### Minimum Acceptance

- `prefab_schema_load_v1()` is reentrant
- `prefab_schema_save_v1()` is reentrant
- no large fixed-size local stack frame regression
- regression coverage added for repeated or concurrent-style calls

## 2. Sandbox Startup Layering

Status: pending
Severity: medium

### Problem

`phase_g_refresh_editor_extension_state()` in:

- `apps/sandbox_dwrite/main.cpp`

still mixes too many responsibilities:

- project/workspace bootstrap
- project/package/settings disk I/O
- plugin scan and attach
- prefab semantic analysis
- global editor state mutation

This keeps engineering-system orchestration inside the UI executable entry path instead of a dedicated application/content service.

### Why It Matters

- sandbox main remains oversized and hard to reason about
- reuse from headless/editor tooling is poor
- future project-system changes will keep coupling UI code to content/bootstrap code

### Follow-up Direction

Extract a dedicated startup service, for example:

- `editor_extension_state_service.*`
- or `project_bootstrap_service.*`

That service should own:

- bootstrap-if-missing
- load current workspace/project/package/settings
- scan/attach/initialize built-in plugins
- run prefab analysis
- return an explicit result object

### Minimum Acceptance

- `main.cpp` no longer directly orchestrates all startup substeps
- startup service returns structured success/failure result
- plugin/prefab/project bootstrap logic can be reused outside sandbox UI

## 3. Startup Error Handling and Diagnostics

Status: pending
Severity: medium

### Problem

Several startup operations currently ignore return values, including bootstrap and load calls in:

- `apps/sandbox_dwrite/main.cpp`

When files are missing, malformed, or inaccessible, startup silently degrades instead of surfacing a clear diagnostic path.

### Why It Matters

- broken project settings can look like random editor behavior
- plugin state can silently disappear
- prefab analysis can fail without an actionable report

### Follow-up Direction

Introduce explicit startup diagnostics:

1. check all bootstrap/load return values
2. log failures with source path and operation name
3. return aggregated startup status to UI
4. surface degraded mode in debug/status panel if needed

### Minimum Acceptance

- no ignored critical startup return values
- startup failures are logged with concrete path + stage
- degraded startup is observable from UI/logs

## 4. Bootstrap / Load Redundancy

Status: pending
Severity: low

### Problem

The current startup path hardcodes bootstrap defaults and then immediately reloads the same project/package/settings data from disk. This duplicates configuration knowledge in UI code and increases drift risk.

### Why It Matters

- default paths and IDs are duplicated
- future manifest/path changes can drift between code and checked-in sample files
- maintenance cost is higher than necessary

### Follow-up Direction

Unify bootstrap and load into one service contract:

1. ensure defaults exist if missing
2. load canonical on-disk state
3. return one normalized result

Move default templates out of `main.cpp` where practical.

### Minimum Acceptance

- bootstrap defaults are not open-coded inside sandbox UI startup
- one canonical path owns default generation
- sample files and bootstrap defaults are easier to keep aligned

## Suggested Order

1. Fix prefab schema reentrancy
2. Add startup error handling
3. Extract startup orchestration service
4. Remove bootstrap/load duplication
