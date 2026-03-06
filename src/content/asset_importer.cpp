#include "asset_importer.hpp"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include "physics2d/physics_filesystem.hpp"

static void asset_importer_copy_text(char* out, int out_cap, const char* src) {
    if (out == NULL || out_cap <= 0) return;
    if (src == NULL) {
        out[0] = '\0';
        return;
    }
    strncpy(out, src, (size_t)(out_cap - 1));
    out[out_cap - 1] = '\0';
}

static const char* asset_importer_file_ext(const char* path) {
    const char* ext = NULL;
    size_t i;
    if (path == NULL) return NULL;
    for (i = 0; path[i] != '\0'; i++) {
        if (path[i] == '.') ext = path + i + 1;
    }
    return ext;
}

static int asset_importer_ext_eq(const char* ext, const char* rhs) {
    size_t i = 0;
    if (ext == NULL || rhs == NULL) return 0;
    while (ext[i] != '\0' && rhs[i] != '\0') {
        if (tolower((unsigned char)ext[i]) != tolower((unsigned char)rhs[i])) return 0;
        i++;
    }
    return ext[i] == '\0' && rhs[i] == '\0';
}

static int asset_importer_file_exists(const char* path) {
    if (path == NULL) return 0;
    {
        const physics2d::foundation::FileInfo info =
            physics2d::foundation::stat_file(physics2d::foundation::Path(path));
        return info.exists && !info.is_directory;
    }
}

static int asset_importer_ensure_dir(const char* path) {
    if (path == NULL || path[0] == '\0') return 0;
    return physics2d::foundation::ensure_directory(physics2d::foundation::Path(path)) ? 1 : 0;
}

static int asset_importer_compute_source_hash(const char* source_path, char out_hash[ASSET_DB_MAX_HASH]) {
    FILE* fp;
    unsigned long long hash = 1469598103934665603ull;
    int ch;
    if (source_path == NULL || out_hash == NULL) return 0;
    fp = fopen(source_path, "rb");
    if (fp == NULL) return 0;
    for (;;) {
        ch = fgetc(fp);
        if (ch == EOF) break;
        hash ^= (unsigned long long)(unsigned char)ch;
        hash *= 1099511628211ull;
    }
    fclose(fp);
    snprintf(out_hash, ASSET_DB_MAX_HASH, "fnv64:%016llx", (unsigned long long)hash);
    return 1;
}

static int asset_importer_compute_settings_hash(const char* settings_fingerprint, char out_hash[ASSET_DB_MAX_HASH]) {
    const char* text = (settings_fingerprint != NULL && settings_fingerprint[0] != '\0') ? settings_fingerprint : "default";
    unsigned long long hash = 1469598103934665603ull;
    size_t i;
    if (out_hash == NULL) return 0;
    for (i = 0; text[i] != '\0'; i++) {
        hash ^= (unsigned long long)(unsigned char)text[i];
        hash *= 1099511628211ull;
    }
    snprintf(out_hash, ASSET_DB_MAX_HASH, "fnv64:%016llx", (unsigned long long)hash);
    return 1;
}

void asset_import_parameters_default(AssetImportKind kind, AssetImportParameters* out_parameters) {
    if (out_parameters == NULL) return;
    memset(out_parameters, 0, sizeof(*out_parameters));
    if (kind == ASSET_IMPORT_KIND_TEXTURE) {
        out_parameters->texture_srgb = 1;
        out_parameters->texture_max_size = 4096;
        out_parameters->texture_generate_mips = 1;
    } else if (kind == ASSET_IMPORT_KIND_FONT) {
        out_parameters->font_pixel_height = 32;
        out_parameters->font_hinting = 1;
    } else if (kind == ASSET_IMPORT_KIND_AUDIO) {
        out_parameters->audio_bitrate_kbps = 192;
        out_parameters->audio_normalize = 0;
    }
}

int asset_import_parameters_fingerprint(
    AssetImportKind kind,
    const AssetImportParameters* parameters,
    char out_hash[ASSET_DB_MAX_HASH]) {
    AssetImportParameters p;
    char text[256];
    unsigned long long hash = 1469598103934665603ull;
    size_t i;
    if (out_hash == NULL) return 0;
    asset_import_parameters_default(kind, &p);
    if (parameters != NULL) p = *parameters;
    snprintf(text, sizeof(text), "k=%d|tx_srgb=%d|tx_max=%d|tx_mips=%d|font_px=%d|font_hint=%d|aud_kbps=%d|aud_norm=%d",
             (int)kind,
             p.texture_srgb,
             p.texture_max_size,
             p.texture_generate_mips,
             p.font_pixel_height,
             p.font_hinting,
             p.audio_bitrate_kbps,
             p.audio_normalize);
    for (i = 0; text[i] != '\0'; i++) {
        hash ^= (unsigned long long)(unsigned char)text[i];
        hash *= 1099511628211ull;
    }
    snprintf(out_hash, ASSET_DB_MAX_HASH, "fnv64:%016llx", (unsigned long long)hash);
    return 1;
}

void asset_import_parameters_to_meta(
    AssetImportKind kind,
    const AssetImportParameters* parameters,
    AssetMeta* meta) {
    AssetImportParameters p;
    if (meta == NULL) return;
    asset_import_parameters_default(kind, &p);
    if (parameters != NULL) p = *parameters;
    meta->has_parameters = 1;
    meta->texture_srgb = p.texture_srgb;
    meta->texture_max_size = p.texture_max_size;
    meta->texture_generate_mips = p.texture_generate_mips;
    meta->font_pixel_height = p.font_pixel_height;
    meta->font_hinting = p.font_hinting;
    meta->audio_bitrate_kbps = p.audio_bitrate_kbps;
    meta->audio_normalize = p.audio_normalize;
}

int asset_import_parameters_from_meta(
    AssetImportKind kind,
    const AssetMeta* meta,
    AssetImportParameters* out_parameters) {
    AssetImportParameters p;
    if (meta == NULL || out_parameters == NULL) return 0;
    asset_import_parameters_default(kind, &p);
    if (!meta->has_parameters) return 0;
    p.texture_srgb = meta->texture_srgb;
    p.texture_max_size = meta->texture_max_size;
    p.texture_generate_mips = meta->texture_generate_mips;
    p.font_pixel_height = meta->font_pixel_height;
    p.font_hinting = meta->font_hinting;
    p.audio_bitrate_kbps = meta->audio_bitrate_kbps;
    p.audio_normalize = meta->audio_normalize;
    *out_parameters = p;
    return 1;
}

static int asset_importer_write_artifact(
    const char* artifact_path,
    AssetImportKind kind,
    const char* source_path,
    const AssetMeta* meta,
    const char* cache_key) {
    const char* kind_text = "unknown";
    std::string artifact_text;
    if (artifact_path == NULL || source_path == NULL || meta == NULL || cache_key == NULL) return 0;
    if (kind == ASSET_IMPORT_KIND_TEXTURE) kind_text = "texture";
    else if (kind == ASSET_IMPORT_KIND_FONT) kind_text = "font";
    else if (kind == ASSET_IMPORT_KIND_AUDIO) kind_text = "audio";
    artifact_text += "kind|";
    artifact_text += kind_text;
    artifact_text += "\nsource_path|";
    artifact_text += source_path;
    artifact_text += "\nguid|";
    artifact_text += meta->guid;
    artifact_text += "\nsource_hash|";
    artifact_text += meta->source_hash;
    artifact_text += "\ncache_key|";
    artifact_text += cache_key;
    artifact_text += "\n";
    return physics2d::foundation::write_text_file(
               physics2d::foundation::Path(artifact_path),
               artifact_text)
               ? 1
               : 0;
}

static void asset_importer_fill_importer_fields(AssetImportKind kind, AssetMeta* meta) {
    if (meta == NULL) return;
    if (kind == ASSET_IMPORT_KIND_TEXTURE) {
        asset_importer_copy_text(meta->asset_type, ASSET_DB_MAX_TYPE, "texture");
        asset_importer_copy_text(meta->importer_id, ASSET_DB_MAX_IMPORTER, "texture-importer");
        meta->importer_version = 1;
    } else if (kind == ASSET_IMPORT_KIND_FONT) {
        asset_importer_copy_text(meta->asset_type, ASSET_DB_MAX_TYPE, "font");
        asset_importer_copy_text(meta->importer_id, ASSET_DB_MAX_IMPORTER, "font-importer");
        meta->importer_version = 1;
    } else if (kind == ASSET_IMPORT_KIND_AUDIO) {
        asset_importer_copy_text(meta->asset_type, ASSET_DB_MAX_TYPE, "audio");
        asset_importer_copy_text(meta->importer_id, ASSET_DB_MAX_IMPORTER, "audio-importer");
        meta->importer_version = 1;
    } else {
        asset_importer_copy_text(meta->asset_type, ASSET_DB_MAX_TYPE, "unknown");
        asset_importer_copy_text(meta->importer_id, ASSET_DB_MAX_IMPORTER, "unknown-importer");
        meta->importer_version = 1;
    }
}

AssetImportKind asset_importer_kind_from_path(const char* source_path) {
    const char* ext = asset_importer_file_ext(source_path);
    if (asset_importer_ext_eq(ext, "png") || asset_importer_ext_eq(ext, "jpg") ||
        asset_importer_ext_eq(ext, "jpeg") || asset_importer_ext_eq(ext, "bmp")) {
        return ASSET_IMPORT_KIND_TEXTURE;
    }
    if (asset_importer_ext_eq(ext, "ttf") || asset_importer_ext_eq(ext, "otf")) {
        return ASSET_IMPORT_KIND_FONT;
    }
    if (asset_importer_ext_eq(ext, "wav") || asset_importer_ext_eq(ext, "ogg")) {
        return ASSET_IMPORT_KIND_AUDIO;
    }
    return ASSET_IMPORT_KIND_UNKNOWN;
}

int asset_importer_run(const AssetImportRequest* request, AssetImportResult* out_result) {
    const char* cache_root;
    AssetImportResult result;
    char meta_path[ASSET_IMPORTER_MAX_PATH];
    char artifact_dir[ASSET_IMPORTER_MAX_PATH];
    const char* guid_suffix;

    if (request == NULL || out_result == NULL || request->source_path == NULL) return 0;
    memset(&result, 0, sizeof(result));
    asset_meta_init(&result.meta);
    result.kind = asset_importer_kind_from_path(request->source_path);
    if (result.kind == ASSET_IMPORT_KIND_UNKNOWN) {
        asset_importer_copy_text(result.error_message, ASSET_IMPORTER_MAX_ERROR, "unsupported extension");
        *out_result = result;
        return 0;
    }
    if (!asset_importer_file_exists(request->source_path)) {
        asset_importer_copy_text(result.error_message, ASSET_IMPORTER_MAX_ERROR, "source file missing");
        *out_result = result;
        return 0;
    }

    asset_importer_fill_importer_fields(result.kind, &result.meta);
    if (!asset_meta_make_guid_from_path(request->source_path, result.meta.guid) ||
        !asset_importer_compute_source_hash(request->source_path, result.meta.source_hash)) {
        asset_importer_copy_text(result.error_message, ASSET_IMPORTER_MAX_ERROR, "failed to compute metadata");
        *out_result = result;
        return 0;
    }
    if (request->parameters != NULL) {
        asset_import_parameters_to_meta(result.kind, request->parameters, &result.meta);
        if (!asset_import_parameters_fingerprint(result.kind, request->parameters, result.meta.import_settings_hash)) {
            asset_importer_copy_text(result.error_message, ASSET_IMPORTER_MAX_ERROR, "failed to compute metadata");
            *out_result = result;
            return 0;
        }
    } else {
        result.meta.has_parameters = 0;
        if (!asset_importer_compute_settings_hash(request->settings_fingerprint, result.meta.import_settings_hash)) {
            asset_importer_copy_text(result.error_message, ASSET_IMPORTER_MAX_ERROR, "failed to compute metadata");
            *out_result = result;
            return 0;
        }
    }
    if (!asset_meta_make_cache_key(&result.meta, result.cache_key)) {
        asset_importer_copy_text(result.error_message, ASSET_IMPORTER_MAX_ERROR, "failed to compute metadata");
        *out_result = result;
        return 0;
    }

    {
        int n = snprintf(meta_path, sizeof(meta_path), "%s.meta", request->source_path);
        if (n <= 0 || n >= (int)sizeof(meta_path)) {
            asset_importer_copy_text(result.error_message, ASSET_IMPORTER_MAX_ERROR, "meta path too long");
            *out_result = result;
            return 0;
        }
    }
    if (!asset_meta_save(&result.meta, meta_path)) {
        asset_importer_copy_text(result.error_message, ASSET_IMPORTER_MAX_ERROR, "failed to write meta");
        *out_result = result;
        return 0;
    }

    cache_root = (request->cache_root != NULL && request->cache_root[0] != '\0') ? request->cache_root : "Cache/imported";
    asset_importer_copy_text(artifact_dir, (int)sizeof(artifact_dir), cache_root);
    if (!asset_importer_ensure_dir(artifact_dir)) {
        asset_importer_copy_text(result.error_message, ASSET_IMPORTER_MAX_ERROR, "failed to create cache directory");
        *out_result = result;
        return 0;
    }
    guid_suffix = result.meta.guid;
    if (strncmp(guid_suffix, "asset://", 8) == 0) guid_suffix += 8;
    {
        int n = snprintf(result.artifact_path, sizeof(result.artifact_path), "%s/%s.asset", artifact_dir, guid_suffix);
        if (n <= 0 || n >= (int)sizeof(result.artifact_path)) {
            asset_importer_copy_text(result.error_message, ASSET_IMPORTER_MAX_ERROR, "artifact path too long");
            *out_result = result;
            return 0;
        }
    }
    if (!asset_importer_write_artifact(result.artifact_path, result.kind, request->source_path, &result.meta, result.cache_key)) {
        asset_importer_copy_text(result.error_message, ASSET_IMPORTER_MAX_ERROR, "failed to write artifact");
        *out_result = result;
        return 0;
    }

    result.success = 1;
    result.error_message[0] = '\0';
    *out_result = result;
    return 1;
}
