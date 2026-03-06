#ifndef ASSET_DATABASE_H
#define ASSET_DATABASE_H

#ifdef __cplusplus
extern "C" {
#endif

enum {
    ASSET_DB_MAX_GUID = 64,
    ASSET_DB_MAX_TYPE = 32,
    ASSET_DB_MAX_IMPORTER = 32,
    ASSET_DB_MAX_HASH = 128,
    ASSET_DB_MAX_DEPENDENCIES = 64,
    ASSET_DB_MAX_ASSETS = 512,
    ASSET_DB_MAX_EDGES = 2048
};

typedef struct {
    char guid[ASSET_DB_MAX_GUID];
    char asset_type[ASSET_DB_MAX_TYPE];
    char importer_id[ASSET_DB_MAX_IMPORTER];
    int importer_version;
    char source_hash[ASSET_DB_MAX_HASH];
    char import_settings_hash[ASSET_DB_MAX_HASH];
    int has_parameters;
    int texture_srgb;
    int texture_max_size;
    int texture_generate_mips;
    int font_pixel_height;
    int font_hinting;
    int audio_bitrate_kbps;
    int audio_normalize;
    int dependency_count;
    char dependencies[ASSET_DB_MAX_DEPENDENCIES][ASSET_DB_MAX_GUID];
} AssetMeta;

typedef struct {
    int asset_count;
    char assets[ASSET_DB_MAX_ASSETS][ASSET_DB_MAX_GUID];
    int edge_count;
    int edge_from[ASSET_DB_MAX_EDGES];
    int edge_to[ASSET_DB_MAX_EDGES];
} AssetDependencyGraph;

void asset_meta_init(AssetMeta* meta);
int asset_meta_save(const AssetMeta* meta, const char* path);
int asset_meta_load(const char* path, AssetMeta* out_meta);
int asset_meta_make_guid_from_path(const char* path, char out_guid[ASSET_DB_MAX_GUID]);
int asset_meta_is_valid_guid(const char* guid);
int asset_meta_make_cache_key(const AssetMeta* meta, char out_key[ASSET_DB_MAX_HASH]);

void asset_dependency_graph_init(AssetDependencyGraph* graph);
int asset_dependency_graph_add_asset(AssetDependencyGraph* graph, const char* guid);
int asset_dependency_graph_add_dependency(AssetDependencyGraph* graph, const char* guid, const char* depends_on_guid);
int asset_dependency_graph_depends_on(const AssetDependencyGraph* graph, const char* guid, const char* depends_on_guid);

#ifdef __cplusplus
}
#endif

#endif
