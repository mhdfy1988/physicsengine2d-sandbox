# PhysicsEngine2D

Interactive 2D physics engine project in C.

A C99 2D rigid-body physics engine with a Windows Direct2D sandbox app and regression tests.

## GitHub Metadata

- Suggested description: C99 2D rigid-body physics engine with an interactive Windows sandbox, scene presets, and regression tests.
- Suggested topics: `physics-engine`, `2d-physics`, `rigid-body`, `collision-detection`, `impulse-solver`, `c99`, `direct2d`, `directwrite`, `windows`, `sandbox`

The repository is organized around a visual sandbox workflow:
- engine core (`src/`, `include/`)
- interactive app (`apps/sandbox_dwrite/`)
- regression checks (`tests/`)

Build output is now explicitly split:
- `lib/libphysics2d.a` as reusable core library
- `bin/physics_sandbox(.exe)` as editor app

## Project Structure

```
.
|-- apps/
|   `-- sandbox_dwrite/
|       `-- main.c
|-- include/
|   |-- body.h
|   |-- collision.h
|   |-- physics.h
|   |-- physics_math.h
|   `-- shape.h
|-- src/
|   |-- body.c
|   |-- collision.c
|   |-- math.c
|   |-- physics.c
|   `-- shape.c
|-- tests/
|   `-- regression_tests.c
|-- CMakeLists.txt
`-- Makefile
```

## Current Focus

`apps/sandbox_dwrite/main.c` is the main entry point.

It provides:
- Play/Pause/Single-step simulation
- Mouse pick and drag for dynamic bodies
- Spawn circle/box at cursor
- Scene presets and hot switching (`F1-F9`, `[`, `]`)
- Chinese UI modals: `配置` and `使用说明`
- Per-scene config editing in config modal (`+/-` disabled at limit)
- Contact point and normal visualization
- Velocity vector visualization
- Runtime tuning for gravity/time step/damping/iterations
- Out-of-stage recycle counter and recycle protection at spawn
- Configurable UI font path via `sandbox_ui.ini`

## Build

### CMake (recommended)

Build sandbox only:

```bash
cmake -S . -B build -DBUILD_SANDBOX=ON -DBUILD_TESTS=OFF
cmake --build build
./build/bin/physics_sandbox
```

Build with regression tests:

```bash
cmake -S . -B build -DBUILD_SANDBOX=ON -DBUILD_TESTS=ON
cmake --build build
./build/bin/physics_tests
```

### Make

```bash
make core     # build reusable physics core static library
make          # same as: make sandbox
make run      # run sandbox
make test     # build + run regression tests
make check-core   # run core-maintenance checklist
```

## Dependencies

- C compiler with C99 support
- Windows SDK libraries for Direct2D/DirectWrite (`d2d1`, `dwrite`, `ole32`, `uuid`, `shcore`, `gdi32`, `user32`)

For MSYS2 UCRT64:

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-make
```

## Reuse Core In Other Apps

1. Build core library:

```bash
make core
```

2. In another app, include headers from `include/` and link:

```bash
gcc your_app.c -I path/to/physicsengine2d-sandbox/include path/to/physicsengine2d-sandbox/lib/libphysics2d.a -lm
```

## Core Maintenance Docs

- Core development rules: [CORE_DEV_GUIDE.md](CORE_DEV_GUIDE.md)
- Change history: [CHANGELOG.md](CHANGELOG.md)
- Regression test template: [tests/REGRESSION_TEST_TEMPLATE.md](tests/REGRESSION_TEST_TEMPLATE.md)
- Chinese API doc: [docs/API_CN.md](docs/API_CN.md)
- English API doc: [docs/API_EN.md](docs/API_EN.md)

## Packaging (Windows)

Portable zip:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\package.ps1 -Build -Version 1.0.0
```

Portable zip + installer (Inno Setup required):

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\package.ps1 -Build -WithInstaller -Version 1.0.0
```

Installer script path:

`installer/physics_sandbox.iss`
