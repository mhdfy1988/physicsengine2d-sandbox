# Asset Meta v1 (Phase C / C2 Kickoff)

This document defines the initial `.meta` file baseline used by the Asset Database prototype.

## 1. File Format

```
guid|asset://...
asset_type|texture|font|audio|...
importer_id|texture-importer|font-importer|audio-importer|...
importer_version|N
source_hash|sha256:...
import_settings_hash|sha256:...
has_parameters|0/1
param_texture_srgb|0/1
param_texture_max_size|N
param_texture_generate_mips|0/1
param_font_pixel_height|N
param_font_hinting|0/1
param_audio_bitrate_kbps|N
param_audio_normalize|0/1
dependency_count|M
dependency|asset://...
...
```

## 2. Determinism Rules

1. Keys are serialized in fixed order.
2. Dependencies are sorted before serialization.
3. Cache key is derived from:
   `source_hash + importer_id + importer_version + import_settings_hash`.

## 3. Scope

1. This increment provides baseline meta read/write, deterministic GUID generation from source path, and dependency graph query.
2. Actual importer execution (texture/font/audio conversion) will consume this meta contract in follow-up C2 increments.
