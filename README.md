# PhysicsEngine2D

> AI Attribution: this repository was generated with AI assistance across architecture, code, tests, and documentation.

PhysicsEngine2D is a C++ 2D rigid-body physics engine with a Windows Direct2D/DirectWrite sandbox app, regression tests, and content/runtime workflow experiments.

## Overview

- Engine core lives under `src/` and `include/`.
- Sandbox/editor shell lives under `apps/sandbox_dwrite/`.
- Regression and smoke coverage lives under `tests/`.
- Tools and migration helpers live under `tools/`.

Build outputs are split into:

- `lib/libphysics2d.a`: reusable static library
- `bin/physics_sandbox(.exe)`: sandbox application
- `bin/physics_tests(.exe)`: regression test runner

## Project Structure

```text
.
|-- apps/
|   `-- sandbox_dwrite/
|       |-- application/
|       |-- infrastructure/
|       `-- main.cpp
|-- include/
|   |-- physics.hpp
|   |-- physics.h
|   |-- physics_ext.hpp
|   |-- physics_ext.h
|   |-- physics_world.hpp
|   `-- physics_world.h
|-- src/
|   |-- core/
|   |-- content/
|   |-- c_api/
|   `-- *.cpp
|-- tests/
|   `-- *.cpp
|-- tools/
|   `-- *.cpp
|-- CMakeLists.txt
`-- Makefile
```

## Public Header Policy

Preferred public headers:

- `physics.hpp`
- `physics_ext.hpp`
- `physics_world.hpp`

Legacy compatibility headers remain available:

- `physics.h`
- `physics_ext.h`
- `physics_world.h`

The `.h` files are forwarding shims only. They exist to keep older downstream integrations building while the public surface transitions to C++ naming.

## Build

### CMake

Sandbox only:

```bash
cmake -S . -B build -G "MinGW Makefiles" -DBUILD_SANDBOX=ON -DBUILD_TESTS=OFF
cmake --build build
./build/bin/physics_sandbox
```

Sandbox plus tests:

```bash
cmake -S . -B build -G "MinGW Makefiles" -DBUILD_SANDBOX=ON -DBUILD_TESTS=ON
cmake --build build
./build/bin/physics_tests
```

### Make

```bash
mingw32-make core
mingw32-make sandbox
mingw32-make test
mingw32-make run
```

## Dependencies

- C++ compiler with C++17 support
- Windows SDK libraries for Direct2D/DirectWrite: `d2d1`, `dwrite`, `ole32`, `uuid`, `shcore`, `gdi32`, `user32`

For MSYS2 UCRT64:

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-make mingw-w64-ucrt-x86_64-cmake
```

## Reusing The Core

1. Build the static library:

```bash
mingw32-make core
```

2. Include a public header and link the library:

```bash
g++ your_app.cpp -I path/to/physicsengine2d-sandbox/include path/to/physicsengine2d-sandbox/lib/libphysics2d.a -lm
```

Preferred include form:

```cpp
#include "physics.hpp"
```

Legacy include form remains valid:

```cpp
#include "physics.h"
```

## Current Sandbox Focus

`apps/sandbox_dwrite/main.cpp` is the main sandbox entry point. It owns the runtime/editor shell, PIE lifecycle, hot reload instrumentation, diagnostics UI, and workflow/demo integration used by the current roadmap.

## Documentation

- [CORE_DEV_GUIDE.md](CORE_DEV_GUIDE.md)
- [CHANGELOG.md](CHANGELOG.md)
- [docs/API_CN.md](docs/API_CN.md)
- [docs/API_EN.md](docs/API_EN.md)
- [docs/ENGINE_ARCHITECTURE.md](docs/ENGINE_ARCHITECTURE.md)

## Packaging

Portable zip:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\package.ps1 -Build -Version 1.0.0
```

Portable zip plus installer:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\package.ps1 -Build -WithInstaller -Version 1.0.0
```

Installer script:

`installer/physics_sandbox.iss`
