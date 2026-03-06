#ifndef ASSET_IMPORTER_H
#define ASSET_IMPORTER_H

#include "asset_database.hpp"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    ASSET_IMPORTER_MAX_PATH = 512,
    ASSET_IMPORTER_MAX_ERROR = 192
};

typedef enum {
    ASSET_IMPORT_KIND_UNKNOWN = 0,
    ASSET_IMPORT_KIND_TEXTURE = 1,
    ASSET_IMPORT_KIND_FONT = 2,
    ASSET_IMPORT_KIND_AUDIO = 3
} AssetImportKind;

typedef struct {
    int texture_srgb;
    int texture_max_size;
    int texture_generate_mips;
    int font_pixel_height;
    int font_hinting;
    int audio_bitrate_kbps;
    int audio_normalize;
} AssetImportParameters;

typedef struct {
    const char* source_path;
    const char* cache_root;
    const char* settings_fingerprint;
    const AssetImportParameters* parameters;
} AssetImportRequest;

typedef struct {
    int success;
    AssetImportKind kind;
    AssetMeta meta;
    char cache_key[ASSET_DB_MAX_HASH];
    char artifact_path[ASSET_IMPORTER_MAX_PATH];
    char error_message[ASSET_IMPORTER_MAX_ERROR];
} AssetImportResult;

AssetImportKind asset_importer_kind_from_path(const char* source_path);
void asset_import_parameters_default(AssetImportKind kind, AssetImportParameters* out_parameters);
int asset_import_parameters_fingerprint(
    AssetImportKind kind,
    const AssetImportParameters* parameters,
    char out_hash[ASSET_DB_MAX_HASH]);
void asset_import_parameters_to_meta(
    AssetImportKind kind,
    const AssetImportParameters* parameters,
    AssetMeta* meta);
int asset_import_parameters_from_meta(
    AssetImportKind kind,
    const AssetMeta* meta,
    AssetImportParameters* out_parameters);
int asset_importer_run(const AssetImportRequest* request, AssetImportResult* out_result);

#ifdef __cplusplus
}
#endif

#endif
