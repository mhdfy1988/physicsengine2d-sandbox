#ifndef PREFAB_SCHEMA_H
#define PREFAB_SCHEMA_H

#ifdef __cplusplus
extern "C" {
#endif

enum {
    PREFAB_SCHEMA_VERSION_V1 = 1,
    PREFAB_SCHEMA_MAX_GUID = 96,
    PREFAB_SCHEMA_MAX_NAME = 64,
    PREFAB_SCHEMA_MAX_FIELD = 64,
    PREFAB_SCHEMA_MAX_VALUE = 128,
    PREFAB_SCHEMA_MAX_ENTITIES = 1024,
    PREFAB_SCHEMA_MAX_OVERRIDES = 2048
};

typedef enum {
    PREFAB_SCHEMA_ENTITY_CIRCLE = 0,
    PREFAB_SCHEMA_ENTITY_BOX = 1
} PrefabSchemaEntityShape;

typedef struct {
    int entity_id;
    char name[PREFAB_SCHEMA_MAX_NAME];
    PrefabSchemaEntityShape shape;
    float mass;
    float position_x;
    float position_y;
    float velocity_x;
    float velocity_y;
    float angular_velocity;
    float restitution;
    float friction;
    float size;
    float damping;
} PrefabSchemaEntity;

typedef struct {
    int override_id;
    int entity_id;
    char field[PREFAB_SCHEMA_MAX_FIELD];
    char value[PREFAB_SCHEMA_MAX_VALUE];
} PrefabSchemaOverride;

typedef struct {
    int schema_version;
    char prefab_guid[PREFAB_SCHEMA_MAX_GUID];
    char base_prefab_guid[PREFAB_SCHEMA_MAX_GUID];
    int is_variant;
    int entity_count;
    PrefabSchemaEntity entities[PREFAB_SCHEMA_MAX_ENTITIES];
    int override_count;
    PrefabSchemaOverride overrides[PREFAB_SCHEMA_MAX_OVERRIDES];
} PrefabSchemaDocument;

void prefab_schema_document_init(PrefabSchemaDocument* doc);
int prefab_schema_load_v1(const char* path, PrefabSchemaDocument* out_doc);
int prefab_schema_save_v1(const PrefabSchemaDocument* doc, const char* path);

#ifdef __cplusplus
}
#endif

#endif
