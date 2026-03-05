# Asset Importer v1 Skeleton (Phase C / C2)

This is the first executable importer baseline.

## 1. Supported Source Extensions

1. Texture: `png`, `jpg`, `jpeg`, `bmp`
2. Font: `ttf`, `otf`
3. Audio: `wav`, `ogg`

## 2. Output Contract

For each source asset:

1. Write `<source>.meta` using `AssetMeta` contract.
2. Write cache artifact under `Cache/imported/<guid>.asset`.
3. Fill deterministic fields:
   - `guid` from source path
   - `source_hash` from file bytes
   - `import_settings_hash` from settings fingerprint
   - `cache_key` from source/importer/settings tuple

## 2.1 Structured Import Parameters

Importer accepts typed parameters and computes deterministic settings fingerprint:

1. Texture: `texture_srgb`, `texture_max_size`, `texture_generate_mips`
2. Font: `font_pixel_height`, `font_hinting`
3. Audio: `audio_bitrate_kbps`, `audio_normalize`

Different parameter sets produce different cache keys; same sets produce identical keys.

## 3. Invalidation

`asset_invalidation_collect()` takes changed GUID and dependency graph, then returns all impacted assets (changed asset + reverse dependents).
