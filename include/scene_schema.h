#ifndef SCENE_SCHEMA_H
#define SCENE_SCHEMA_H

#ifdef __cplusplus
extern "C" {
#endif

enum {
    SCENE_SCHEMA_VERSION_V1 = 1,
    SCENE_SCHEMA_MAX_GUID = 96,
    SCENE_SCHEMA_MAX_NAME = 64,
    SCENE_SCHEMA_MAX_ENTITIES = 1024,
    SCENE_SCHEMA_MAX_CONSTRAINTS = 1024
};

typedef enum {
    SCENE_SCHEMA_ENTITY_CIRCLE = 0,
    SCENE_SCHEMA_ENTITY_BOX = 1
} SceneSchemaEntityShape;

typedef struct {
    int entity_id;
    char name[SCENE_SCHEMA_MAX_NAME];
    SceneSchemaEntityShape shape;
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
} SceneSchemaEntity;

typedef struct {
    int constraint_id;
    int type;
    int entity_id_a;
    int entity_id_b;
    float rest_length;
    float stiffness;
    float damping;
    float break_force;
    int collide_connected;
} SceneSchemaConstraint;

typedef struct {
    int schema_version;
    char scene_guid[SCENE_SCHEMA_MAX_GUID];
    int entity_count;
    SceneSchemaEntity entities[SCENE_SCHEMA_MAX_ENTITIES];
    int constraint_count;
    SceneSchemaConstraint constraints[SCENE_SCHEMA_MAX_CONSTRAINTS];
} SceneSchemaDocument;

typedef struct {
    int dry_run;
} SceneSchemaMigrationOptions;

typedef struct {
    int source_version;
    int target_version;
    int entity_count;
    int constraint_count;
    int warning_count;
    int failure_count;
} SceneSchemaMigrationReport;

void scene_schema_document_init(SceneSchemaDocument* doc);
int scene_schema_load_v1(const char* path, SceneSchemaDocument* out_doc);
int scene_schema_save_v1(const SceneSchemaDocument* doc, const char* path);
int scene_schema_migrate_legacy_snapshot_to_v1(
    const char* legacy_path,
    const char* out_path,
    const SceneSchemaMigrationOptions* options,
    SceneSchemaMigrationReport* out_report);

#ifdef __cplusplus
}
#endif

#endif
