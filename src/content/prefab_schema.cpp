#include "physics_content/prefab_schema.hpp"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void prefab_schema_copy_text(char* out, int out_cap, const char* src) {
    if (out == NULL || out_cap <= 0) return;
    if (src == NULL) {
        out[0] = '\0';
        return;
    }
    strncpy(out, src, (size_t)(out_cap - 1));
    out[out_cap - 1] = '\0';
}

static int prefab_schema_parse_int(const char* text, int* out_value) {
    char* endptr = NULL;
    long v;
    if (text == NULL || out_value == NULL) return 0;
    errno = 0;
    v = strtol(text, &endptr, 10);
    if (errno != 0 || endptr == text || (endptr != NULL && *endptr != '\0')) return 0;
    if (v < -2147483647L - 1L || v > 2147483647L) return 0;
    *out_value = (int)v;
    return 1;
}

static int prefab_schema_parse_float(const char* text, float* out_value) {
    char* endptr = NULL;
    float v;
    if (text == NULL || out_value == NULL) return 0;
    errno = 0;
    v = strtof(text, &endptr);
    if (errno != 0 || endptr == text || (endptr != NULL && *endptr != '\0')) return 0;
    *out_value = v;
    return 1;
}

static void prefab_schema_trim_line(char* line) {
    size_t n;
    if (line == NULL) return;
    n = strlen(line);
    while (n > 0 && (line[n - 1] == '\n' || line[n - 1] == '\r')) {
        line[n - 1] = '\0';
        n--;
    }
}

static int prefab_schema_split_pipe(char* line, char* tokens[], int max_tokens) {
    int count = 0;
    char* p = line;
    if (line == NULL || tokens == NULL || max_tokens <= 0) return 0;
    tokens[count++] = p;
    while (*p != '\0') {
        if (*p == '|') {
            *p = '\0';
            if (count < max_tokens) tokens[count++] = p + 1;
        }
        p++;
    }
    return count;
}

static int prefab_schema_entity_cmp(const void* lhs, const void* rhs) {
    const PrefabSchemaEntity* a = (const PrefabSchemaEntity*)lhs;
    const PrefabSchemaEntity* b = (const PrefabSchemaEntity*)rhs;
    if (a->entity_id < b->entity_id) return -1;
    if (a->entity_id > b->entity_id) return 1;
    return 0;
}

static int prefab_schema_override_cmp(const void* lhs, const void* rhs) {
    const PrefabSchemaOverride* a = (const PrefabSchemaOverride*)lhs;
    const PrefabSchemaOverride* b = (const PrefabSchemaOverride*)rhs;
    if (a->override_id < b->override_id) return -1;
    if (a->override_id > b->override_id) return 1;
    return 0;
}

static const char* prefab_schema_shape_to_text(PrefabSchemaEntityShape shape) {
    return (shape == PREFAB_SCHEMA_ENTITY_CIRCLE) ? "circle" : "box";
}

static PrefabSchemaEntityShape prefab_schema_shape_from_text(const char* text, int* ok) {
    if (ok != NULL) *ok = 1;
    if (text == NULL) {
        if (ok != NULL) *ok = 0;
        return PREFAB_SCHEMA_ENTITY_BOX;
    }
    if (strcmp(text, "circle") == 0) return PREFAB_SCHEMA_ENTITY_CIRCLE;
    if (strcmp(text, "box") == 0) return PREFAB_SCHEMA_ENTITY_BOX;
    if (ok != NULL) *ok = 0;
    return PREFAB_SCHEMA_ENTITY_BOX;
}

static PrefabSchemaDocument* prefab_schema_alloc_document(void) {
    return (PrefabSchemaDocument*)malloc(sizeof(PrefabSchemaDocument));
}

static PrefabSchemaEntity* prefab_schema_alloc_entities(int count) {
    if (count <= 0) return NULL;
    return (PrefabSchemaEntity*)malloc((size_t)count * sizeof(PrefabSchemaEntity));
}

static PrefabSchemaOverride* prefab_schema_alloc_overrides(int count) {
    if (count <= 0) return NULL;
    return (PrefabSchemaOverride*)malloc((size_t)count * sizeof(PrefabSchemaOverride));
}

void prefab_schema_document_init(PrefabSchemaDocument* doc) {
    int i;
    if (doc == NULL) return;
    memset(doc, 0, sizeof(*doc));
    doc->schema_version = PREFAB_SCHEMA_VERSION_V1;
    prefab_schema_copy_text(doc->prefab_guid, PREFAB_SCHEMA_MAX_GUID, "prefab://unnamed");
    prefab_schema_copy_text(doc->base_prefab_guid, PREFAB_SCHEMA_MAX_GUID, "none");
    for (i = 0; i < PREFAB_SCHEMA_MAX_ENTITIES; i++) {
        doc->entities[i].shape = PREFAB_SCHEMA_ENTITY_BOX;
        doc->entities[i].damping = 1.0f;
    }
}

int prefab_schema_load_v1(const char* path, PrefabSchemaDocument* out_doc) {
    FILE* fp;
    char line[512];
    int expected_entities = -1;
    int expected_overrides = -1;
    int got_version = 0;
    int got_guid = 0;
    PrefabSchemaDocument* doc;

    if (path == NULL || out_doc == NULL) return 0;
    fp = fopen(path, "r");
    if (fp == NULL) return 0;
    doc = prefab_schema_alloc_document();
    if (doc == NULL) {
        fclose(fp);
        return 0;
    }

    prefab_schema_document_init(doc);

    while (fgets(line, sizeof(line), fp) != NULL) {
        char* tokens[16];
        int token_count;
        prefab_schema_trim_line(line);
        if (line[0] == '\0' || line[0] == '#') continue;
        token_count = prefab_schema_split_pipe(line, tokens, (int)(sizeof(tokens) / sizeof(tokens[0])));
        if (token_count <= 0) {
            fclose(fp);
            free(doc);
            return 0;
        }
        if (strcmp(tokens[0], "schema_version") == 0) {
            int version = 0;
            if (token_count != 2 || !prefab_schema_parse_int(tokens[1], &version) || version != PREFAB_SCHEMA_VERSION_V1) {
                fclose(fp);
                free(doc);
                return 0;
            }
            doc->schema_version = version;
            got_version = 1;
        } else if (strcmp(tokens[0], "prefab_guid") == 0) {
            if (token_count != 2 || tokens[1][0] == '\0') {
                fclose(fp);
                free(doc);
                return 0;
            }
            prefab_schema_copy_text(doc->prefab_guid, PREFAB_SCHEMA_MAX_GUID, tokens[1]);
            got_guid = 1;
        } else if (strcmp(tokens[0], "base_prefab_guid") == 0) {
            if (token_count != 2) {
                fclose(fp);
                free(doc);
                return 0;
            }
            prefab_schema_copy_text(doc->base_prefab_guid, PREFAB_SCHEMA_MAX_GUID, tokens[1]);
        } else if (strcmp(tokens[0], "is_variant") == 0) {
            if (token_count != 2 || !prefab_schema_parse_int(tokens[1], &doc->is_variant)) {
                fclose(fp);
                free(doc);
                return 0;
            }
            doc->is_variant = doc->is_variant ? 1 : 0;
        } else if (strcmp(tokens[0], "entity_count") == 0) {
            if (token_count != 2 || !prefab_schema_parse_int(tokens[1], &expected_entities) || expected_entities < 0) {
                fclose(fp);
                free(doc);
                return 0;
            }
        } else if (strcmp(tokens[0], "entity") == 0) {
            PrefabSchemaEntity* e;
            int shape_ok = 0;
            if (token_count != 14 || doc->entity_count >= PREFAB_SCHEMA_MAX_ENTITIES) {
                fclose(fp);
                free(doc);
                return 0;
            }
            e = &doc->entities[doc->entity_count];
            if (!prefab_schema_parse_int(tokens[1], &e->entity_id)) {
                fclose(fp);
                free(doc);
                return 0;
            }
            prefab_schema_copy_text(e->name, PREFAB_SCHEMA_MAX_NAME, tokens[2]);
            e->shape = prefab_schema_shape_from_text(tokens[3], &shape_ok);
            if (!shape_ok) {
                fclose(fp);
                free(doc);
                return 0;
            }
            if (!prefab_schema_parse_float(tokens[4], &e->mass) ||
                !prefab_schema_parse_float(tokens[5], &e->position_x) ||
                !prefab_schema_parse_float(tokens[6], &e->position_y) ||
                !prefab_schema_parse_float(tokens[7], &e->velocity_x) ||
                !prefab_schema_parse_float(tokens[8], &e->velocity_y) ||
                !prefab_schema_parse_float(tokens[9], &e->angular_velocity) ||
                !prefab_schema_parse_float(tokens[10], &e->restitution) ||
                !prefab_schema_parse_float(tokens[11], &e->friction) ||
                !prefab_schema_parse_float(tokens[12], &e->size) ||
                !prefab_schema_parse_float(tokens[13], &e->damping)) {
                fclose(fp);
                free(doc);
                return 0;
            }
            doc->entity_count++;
        } else if (strcmp(tokens[0], "override_count") == 0) {
            if (token_count != 2 || !prefab_schema_parse_int(tokens[1], &expected_overrides) || expected_overrides < 0) {
                fclose(fp);
                free(doc);
                return 0;
            }
        } else if (strcmp(tokens[0], "override") == 0) {
            PrefabSchemaOverride* o;
            if (token_count != 5 || doc->override_count >= PREFAB_SCHEMA_MAX_OVERRIDES) {
                fclose(fp);
                free(doc);
                return 0;
            }
            o = &doc->overrides[doc->override_count];
            if (!prefab_schema_parse_int(tokens[1], &o->override_id) ||
                !prefab_schema_parse_int(tokens[2], &o->entity_id)) {
                fclose(fp);
                free(doc);
                return 0;
            }
            prefab_schema_copy_text(o->field, PREFAB_SCHEMA_MAX_FIELD, tokens[3]);
            prefab_schema_copy_text(o->value, PREFAB_SCHEMA_MAX_VALUE, tokens[4]);
            doc->override_count++;
        } else {
            fclose(fp);
            free(doc);
            return 0;
        }
    }
    fclose(fp);

    if (!got_version || !got_guid) {
        free(doc);
        return 0;
    }
    if (expected_entities >= 0 && expected_entities != doc->entity_count) {
        free(doc);
        return 0;
    }
    if (expected_overrides >= 0 && expected_overrides != doc->override_count) {
        free(doc);
        return 0;
    }

    qsort(doc->entities, (size_t)doc->entity_count, sizeof(doc->entities[0]), prefab_schema_entity_cmp);
    qsort(doc->overrides, (size_t)doc->override_count, sizeof(doc->overrides[0]), prefab_schema_override_cmp);
    *out_doc = *doc;
    free(doc);
    return 1;
}

int prefab_schema_save_v1(const PrefabSchemaDocument* doc, const char* path) {
    FILE* fp;
    PrefabSchemaEntity* entities = NULL;
    PrefabSchemaOverride* overrides = NULL;
    int i;
    int entity_count;
    int override_count;
    if (doc == NULL || path == NULL) return 0;
    fp = fopen(path, "w");
    if (fp == NULL) return 0;

    entity_count = doc->entity_count;
    override_count = doc->override_count;
    if (entity_count < 0) entity_count = 0;
    if (override_count < 0) override_count = 0;
    if (entity_count > PREFAB_SCHEMA_MAX_ENTITIES) entity_count = PREFAB_SCHEMA_MAX_ENTITIES;
    if (override_count > PREFAB_SCHEMA_MAX_OVERRIDES) override_count = PREFAB_SCHEMA_MAX_OVERRIDES;
    entities = prefab_schema_alloc_entities(entity_count);
    overrides = prefab_schema_alloc_overrides(override_count);
    if ((entity_count > 0 && entities == NULL) || (override_count > 0 && overrides == NULL)) {
        free(entities);
        free(overrides);
        fclose(fp);
        return 0;
    }
    if (entity_count > 0) memcpy(entities, doc->entities, (size_t)entity_count * sizeof(entities[0]));
    if (override_count > 0) memcpy(overrides, doc->overrides, (size_t)override_count * sizeof(overrides[0]));
    qsort(entities, (size_t)entity_count, sizeof(entities[0]), prefab_schema_entity_cmp);
    qsort(overrides, (size_t)override_count, sizeof(overrides[0]), prefab_schema_override_cmp);

    fprintf(fp, "schema_version|%d\n", PREFAB_SCHEMA_VERSION_V1);
    fprintf(fp, "prefab_guid|%s\n", (doc->prefab_guid[0] != '\0') ? doc->prefab_guid : "prefab://unnamed");
    fprintf(fp, "base_prefab_guid|%s\n", (doc->base_prefab_guid[0] != '\0') ? doc->base_prefab_guid : "none");
    fprintf(fp, "is_variant|%d\n", doc->is_variant ? 1 : 0);
    fprintf(fp, "entity_count|%d\n", entity_count);
    for (i = 0; i < entity_count; i++) {
        const PrefabSchemaEntity* e = &entities[i];
        fprintf(fp, "entity|%d|%s|%s|%.6f|%.6f|%.6f|%.6f|%.6f|%.6f|%.6f|%.6f|%.6f|%.6f\n",
                e->entity_id,
                (e->name[0] != '\0') ? e->name : "entity",
                prefab_schema_shape_to_text(e->shape),
                e->mass,
                e->position_x,
                e->position_y,
                e->velocity_x,
                e->velocity_y,
                e->angular_velocity,
                e->restitution,
                e->friction,
                e->size,
                e->damping);
    }
    fprintf(fp, "override_count|%d\n", override_count);
    for (i = 0; i < override_count; i++) {
        const PrefabSchemaOverride* o = &overrides[i];
        fprintf(fp, "override|%d|%d|%s|%s\n",
                o->override_id,
                o->entity_id,
                (o->field[0] != '\0') ? o->field : "field",
                (o->value[0] != '\0') ? o->value : "value");
    }
    free(entities);
    free(overrides);
    if (fclose(fp) != 0) return 0;
    return 1;
}
