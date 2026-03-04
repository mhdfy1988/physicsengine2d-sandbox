# API Compatibility Policy

## Versioning

- Follow SemVer for public headers (`include/physics*.h`).
- Patch (`x.y.Z`): bug fixes only, no public API removal.
- Minor (`x.Y.z`): additive API only, deprecations allowed.
- Major (`X.y.z`): breaking API allowed with migration notes.

## Deprecation Window

- Deprecated APIs must remain for at least one minor release.
- Each deprecation must include:
  - replacement API
  - planned removal version

## Release Gates

- `mingw32-make test` must pass.
- `mingw32-make check-arch` must pass.
- `mingw32-make check-api` must pass.

## Baseline Management

- Public API signatures are tracked in `docs/api_surface_baseline.txt`.
- If intentional public API changes are made, update baseline with:
  - `powershell -ExecutionPolicy Bypass -File .\scripts\check_api_surface.ps1 -Update`
- Include migration notes in changelog for any baseline change.
