# Prefab Schema v1 (Phase C / C1)

This document defines the first committed prefab serialization baseline used by Phase C.

## 1. File Format

```
schema_version|1
prefab_guid|prefab://example
base_prefab_guid|none or prefab://base
is_variant|0 or 1
entity_count|N
entity|<entity_id>|<name>|<shape>|<mass>|<px>|<py>|<vx>|<vy>|<ang>|<rest>|<fric>|<size>|<damping>
...
override_count|M
override|<override_id>|<entity_id>|<field>|<value>
...
```

## 2. Determinism Rules

1. Header keys are written in fixed order.
2. `entity` rows are sorted by `entity_id`.
3. `override` rows are sorted by `override_id`.
4. Numeric fields use fixed precision (`%.6f`) in serialized entities.

## 3. Variant Semantics (v1)

1. `is_variant=0`: base prefab.
2. `is_variant=1`: variant prefab with `base_prefab_guid`.
3. Override entries carry field-level diff only (`field` + `value`).
