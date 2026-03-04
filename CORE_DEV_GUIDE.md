# Core Dev Guide

## Goal

Evolve the physics core (`src/`, `include/`) without breaking editor or external consumers.

## Compatibility Rules

1. Keep `PhysicsEngine` opaque.
- Do not expose `struct PhysicsEngine` in public headers.
- New read/write access should be API functions in `include/physics.h`.

2. Prefer additive API changes.
- Add new functions first.
- Mark old behavior as deprecated in docs before removal.

3. Do not let app/editor depend on internal arrays/layout.
- No `engine->...` access in app/test code.
- Use `physics_engine_get_*` and constraint index APIs.

4. Preserve deterministic behavior when possible.
- If solver behavior changes, add/update regression tests and note impact in `CHANGELOG.md`.

## Change Workflow

1. Implement in `src/` and `include/`.
2. Update callers:
- `apps/sandbox_dwrite/main.c`
- `tests/regression_tests.c`
3. Add or update regression test(s).
4. Run:
```bash
make test
make sandbox
```
5. Update `CHANGELOG.md`.

## API Change Checklist

- Header updated: `include/physics.h`
- Null/invalid input guarded
- Backward compatibility considered
- Tests added/updated
- Changelog updated
