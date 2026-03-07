# Scene Schema v1 (Phase C / C1)

This document defines the first committed scene serialization baseline used by Phase C.

## 1. File Format

The format is line-based, UTF-8 text, deterministic by key order and stable IDs.

```
schema_version|1
scene_guid|scene://example_scene
entity_count|N
entity|<entity_id>|<name>|<shape>|<mass>|<px>|<py>|<vx>|<vy>|<ang>|<rest>|<fric>|<size>|<damping>
...
constraint_count|M
constraint|<constraint_id>|<type>|<entity_id_a>|<entity_id_b>|<rest_length>|<stiffness>|<damping>|<break_force>|<collide_connected>
...
```

## 2. Determinism Rules

1. Header keys are always written in fixed order.
2. `entity` rows are sorted by `entity_id`.
3. `constraint` rows are sorted by `constraint_id`.
4. Floats are serialized with fixed precision (`%.6f`).

## 3. Versioning

1. Current version: `schema_version=1`.
2. Migration path in this increment: legacy snapshot text (`BODIES/CONSTRAINTS`) -> schema v1.
3. Migration supports `dry-run` reporting and atomic output replace.

## 4. Migration Tool

Build:

```powershell
C:\msys64\ucrt64\bin\mingw32-make.exe scene-migrate
```

Dry run:

```powershell
.\bin\scene_migrate --in tests/data/scene_snapshot_v0.txt --dry-run
```

Write output:

```powershell
.\bin\scene_migrate --in tests/data/scene_snapshot_v0.txt --out Scenes/sample_scene_v1.scene
```

## 5. Current Scope Notes

1. This increment finalizes scene schema baseline and migration scaffold.
2. Prefab schema is reserved for the next C1 follow-up increment.
