#include "scene_schema.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char kind;
    float mass;
    float px;
    float py;
    float vx;
    float vy;
    float ang;
    float rest;
    float fric;
    float size;
    float damp;
} LegacyBodyRow;

typedef struct {
    int type;
    int ai;
    int bi;
    float rest_length;
    float stiffness;
    float damping;
    float break_force;
    int collide_connected;
} LegacyConstraintRow;

static void scene_schema_copy_text(char* out, int out_cap, const char* src) {
    if (out == NULL || out_cap <= 0) return;
    if (src == NULL) {
        out[0] = '\0';
        return;
    }
    strncpy(out, src, (size_t)(out_cap - 1));
    out[out_cap - 1] = '\0';
}

static int scene_schema_parse_int(const char* text, int* out_value) {
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

static int scene_schema_parse_float(const char* text, float* out_value) {
    char* endptr = NULL;
    float v;
    if (text == NULL || out_value == NULL) return 0;
    errno = 0;
    v = strtof(text, &endptr);
    if (errno != 0 || endptr == text || (endptr != NULL && *endptr != '\0')) return 0;
    *out_value = v;
    return 1;
}

static void scene_schema_trim_line(char* line) {
    size_t n;
    if (line == NULL) return;
    n = strlen(line);
    while (n > 0 && (line[n - 1] == '\n' || line[n - 1] == '\r')) {
        line[n - 1] = '\0';
        n--;
    }
}

static int scene_schema_split_pipe(char* line, char* tokens[], int max_tokens) {
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

static int scene_schema_entity_cmp(const void* lhs, const void* rhs) {
    const SceneSchemaEntity* a = (const SceneSchemaEntity*)lhs;
    const SceneSchemaEntity* b = (const SceneSchemaEntity*)rhs;
    if (a->entity_id < b->entity_id) return -1;
    if (a->entity_id > b->entity_id) return 1;
    return 0;
}

static int scene_schema_constraint_cmp(const void* lhs, const void* rhs) {
    const SceneSchemaConstraint* a = (const SceneSchemaConstraint*)lhs;
    const SceneSchemaConstraint* b = (const SceneSchemaConstraint*)rhs;
    if (a->constraint_id < b->constraint_id) return -1;
    if (a->constraint_id > b->constraint_id) return 1;
    return 0;
}

static const char* scene_schema_shape_to_text(SceneSchemaEntityShape shape) {
    return (shape == SCENE_SCHEMA_ENTITY_CIRCLE) ? "circle" : "box";
}

static SceneSchemaEntityShape scene_schema_shape_from_text(const char* text, int* ok) {
    if (ok != NULL) *ok = 1;
    if (text == NULL) {
        if (ok != NULL) *ok = 0;
        return SCENE_SCHEMA_ENTITY_BOX;
    }
    if (strcmp(text, "circle") == 0) return SCENE_SCHEMA_ENTITY_CIRCLE;
    if (strcmp(text, "box") == 0) return SCENE_SCHEMA_ENTITY_BOX;
    if (ok != NULL) *ok = 0;
    return SCENE_SCHEMA_ENTITY_BOX;
}

static void scene_schema_copy_report(SceneSchemaMigrationReport* out_report, const SceneSchemaMigrationReport* report) {
    if (out_report == NULL || report == NULL) return;
    *out_report = *report;
}

static int scene_schema_extract_basename(const char* path, char* out_name, int out_cap) {
    const char* base = path;
    size_t i;
    size_t n = 0;
    if (path == NULL || out_name == NULL || out_cap <= 0) return 0;
    for (i = 0; path[i] != '\0'; i++) {
        if (path[i] == '/' || path[i] == '\\') {
            base = path + i + 1;
        }
    }
    for (i = 0; base[i] != '\0' && base[i] != '.' && n < (size_t)(out_cap - 1); i++) {
        unsigned char ch = (unsigned char)base[i];
        if (isalnum(ch)) {
            out_name[n++] = (char)tolower(ch);
        } else {
            out_name[n++] = '_';
        }
    }
    if (n == 0) {
        scene_schema_copy_text(out_name, out_cap, "legacy_scene");
        return 1;
    }
    out_name[n] = '\0';
    return 1;
}

static int scene_schema_save_atomic(const SceneSchemaDocument* doc, const char* out_path) {
    char temp_path[512];
    int n;
    if (doc == NULL || out_path == NULL) return 0;
    n = snprintf(temp_path, sizeof(temp_path), "%s.tmp", out_path);
    if (n <= 0 || n >= (int)sizeof(temp_path)) return 0;
    if (!scene_schema_save_v1(doc, temp_path)) {
        remove(temp_path);
        return 0;
    }
    remove(out_path);
    if (rename(temp_path, out_path) != 0) {
        remove(temp_path);
        return 0;
    }
    return 1;
}

void scene_schema_document_init(SceneSchemaDocument* doc) {
    int i;
    if (doc == NULL) return;
    memset(doc, 0, sizeof(*doc));
    doc->schema_version = SCENE_SCHEMA_VERSION_V1;
    scene_schema_copy_text(doc->scene_guid, SCENE_SCHEMA_MAX_GUID, "scene://unnamed");
    for (i = 0; i < SCENE_SCHEMA_MAX_ENTITIES; i++) {
        doc->entities[i].shape = SCENE_SCHEMA_ENTITY_BOX;
        doc->entities[i].damping = 1.0f;
    }
    for (i = 0; i < SCENE_SCHEMA_MAX_CONSTRAINTS; i++) {
        doc->constraints[i].collide_connected = 1;
    }
}

int scene_schema_load_v1(const char* path, SceneSchemaDocument* out_doc) {
    FILE* fp;
    char line[512];
    int expected_entities = -1;
    int expected_constraints = -1;
    int got_version = 0;
    int got_guid = 0;
    SceneSchemaDocument doc;

    if (path == NULL || out_doc == NULL) return 0;
    fp = fopen(path, "r");
    if (fp == NULL) return 0;

    scene_schema_document_init(&doc);

    while (fgets(line, sizeof(line), fp) != NULL) {
        char* tokens[16];
        int token_count;
        scene_schema_trim_line(line);
        if (line[0] == '\0' || line[0] == '#') continue;
        token_count = scene_schema_split_pipe(line, tokens, (int)(sizeof(tokens) / sizeof(tokens[0])));
        if (token_count <= 0) {
            fclose(fp);
            return 0;
        }
        if (strcmp(tokens[0], "schema_version") == 0) {
            int version = 0;
            if (token_count != 2 || !scene_schema_parse_int(tokens[1], &version) || version != SCENE_SCHEMA_VERSION_V1) {
                fclose(fp);
                return 0;
            }
            doc.schema_version = version;
            got_version = 1;
        } else if (strcmp(tokens[0], "scene_guid") == 0) {
            if (token_count != 2 || tokens[1][0] == '\0') {
                fclose(fp);
                return 0;
            }
            scene_schema_copy_text(doc.scene_guid, SCENE_SCHEMA_MAX_GUID, tokens[1]);
            got_guid = 1;
        } else if (strcmp(tokens[0], "entity_count") == 0) {
            if (token_count != 2 || !scene_schema_parse_int(tokens[1], &expected_entities) || expected_entities < 0) {
                fclose(fp);
                return 0;
            }
        } else if (strcmp(tokens[0], "entity") == 0) {
            SceneSchemaEntity* e;
            int shape_ok = 0;
            if (token_count != 14 || doc.entity_count >= SCENE_SCHEMA_MAX_ENTITIES) {
                fclose(fp);
                return 0;
            }
            e = &doc.entities[doc.entity_count];
            if (!scene_schema_parse_int(tokens[1], &e->entity_id)) {
                fclose(fp);
                return 0;
            }
            scene_schema_copy_text(e->name, SCENE_SCHEMA_MAX_NAME, tokens[2]);
            e->shape = scene_schema_shape_from_text(tokens[3], &shape_ok);
            if (!shape_ok) {
                fclose(fp);
                return 0;
            }
            if (!scene_schema_parse_float(tokens[4], &e->mass) ||
                !scene_schema_parse_float(tokens[5], &e->position_x) ||
                !scene_schema_parse_float(tokens[6], &e->position_y) ||
                !scene_schema_parse_float(tokens[7], &e->velocity_x) ||
                !scene_schema_parse_float(tokens[8], &e->velocity_y) ||
                !scene_schema_parse_float(tokens[9], &e->angular_velocity) ||
                !scene_schema_parse_float(tokens[10], &e->restitution) ||
                !scene_schema_parse_float(tokens[11], &e->friction) ||
                !scene_schema_parse_float(tokens[12], &e->size) ||
                !scene_schema_parse_float(tokens[13], &e->damping)) {
                fclose(fp);
                return 0;
            }
            doc.entity_count++;
        } else if (strcmp(tokens[0], "constraint_count") == 0) {
            if (token_count != 2 || !scene_schema_parse_int(tokens[1], &expected_constraints) || expected_constraints < 0) {
                fclose(fp);
                return 0;
            }
        } else if (strcmp(tokens[0], "constraint") == 0) {
            SceneSchemaConstraint* c;
            if (token_count != 10 || doc.constraint_count >= SCENE_SCHEMA_MAX_CONSTRAINTS) {
                fclose(fp);
                return 0;
            }
            c = &doc.constraints[doc.constraint_count];
            if (!scene_schema_parse_int(tokens[1], &c->constraint_id) ||
                !scene_schema_parse_int(tokens[2], &c->type) ||
                !scene_schema_parse_int(tokens[3], &c->entity_id_a) ||
                !scene_schema_parse_int(tokens[4], &c->entity_id_b) ||
                !scene_schema_parse_float(tokens[5], &c->rest_length) ||
                !scene_schema_parse_float(tokens[6], &c->stiffness) ||
                !scene_schema_parse_float(tokens[7], &c->damping) ||
                !scene_schema_parse_float(tokens[8], &c->break_force) ||
                !scene_schema_parse_int(tokens[9], &c->collide_connected)) {
                fclose(fp);
                return 0;
            }
            doc.constraint_count++;
        } else {
            fclose(fp);
            return 0;
        }
    }
    fclose(fp);

    if (!got_version || !got_guid) return 0;
    if (expected_entities >= 0 && expected_entities != doc.entity_count) return 0;
    if (expected_constraints >= 0 && expected_constraints != doc.constraint_count) return 0;

    qsort(doc.entities, (size_t)doc.entity_count, sizeof(doc.entities[0]), scene_schema_entity_cmp);
    qsort(doc.constraints, (size_t)doc.constraint_count, sizeof(doc.constraints[0]), scene_schema_constraint_cmp);
    *out_doc = doc;
    return 1;
}

int scene_schema_save_v1(const SceneSchemaDocument* doc, const char* path) {
    FILE* fp;
    SceneSchemaEntity entities[SCENE_SCHEMA_MAX_ENTITIES];
    SceneSchemaConstraint constraints[SCENE_SCHEMA_MAX_CONSTRAINTS];
    int i;
    int entity_count;
    int constraint_count;
    if (doc == NULL || path == NULL) return 0;
    fp = fopen(path, "w");
    if (fp == NULL) return 0;

    entity_count = doc->entity_count;
    constraint_count = doc->constraint_count;
    if (entity_count < 0) entity_count = 0;
    if (constraint_count < 0) constraint_count = 0;
    if (entity_count > SCENE_SCHEMA_MAX_ENTITIES) entity_count = SCENE_SCHEMA_MAX_ENTITIES;
    if (constraint_count > SCENE_SCHEMA_MAX_CONSTRAINTS) constraint_count = SCENE_SCHEMA_MAX_CONSTRAINTS;
    if (entity_count > 0) memcpy(entities, doc->entities, (size_t)entity_count * sizeof(entities[0]));
    if (constraint_count > 0) memcpy(constraints, doc->constraints, (size_t)constraint_count * sizeof(constraints[0]));
    qsort(entities, (size_t)entity_count, sizeof(entities[0]), scene_schema_entity_cmp);
    qsort(constraints, (size_t)constraint_count, sizeof(constraints[0]), scene_schema_constraint_cmp);

    fprintf(fp, "schema_version|%d\n", SCENE_SCHEMA_VERSION_V1);
    fprintf(fp, "scene_guid|%s\n", (doc->scene_guid[0] != '\0') ? doc->scene_guid : "scene://unnamed");
    fprintf(fp, "entity_count|%d\n", entity_count);
    for (i = 0; i < entity_count; i++) {
        const SceneSchemaEntity* e = &entities[i];
        fprintf(fp, "entity|%d|%s|%s|%.6f|%.6f|%.6f|%.6f|%.6f|%.6f|%.6f|%.6f|%.6f|%.6f\n",
                e->entity_id,
                (e->name[0] != '\0') ? e->name : "entity",
                scene_schema_shape_to_text(e->shape),
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
    fprintf(fp, "constraint_count|%d\n", constraint_count);
    for (i = 0; i < constraint_count; i++) {
        const SceneSchemaConstraint* c = &constraints[i];
        fprintf(fp, "constraint|%d|%d|%d|%d|%.6f|%.6f|%.6f|%.6f|%d\n",
                c->constraint_id,
                c->type,
                c->entity_id_a,
                c->entity_id_b,
                c->rest_length,
                c->stiffness,
                c->damping,
                c->break_force,
                c->collide_connected ? 1 : 0);
    }
    if (fclose(fp) != 0) return 0;
    return 1;
}

int scene_schema_migrate_legacy_snapshot_to_v1(
    const char* legacy_path,
    const char* out_path,
    const SceneSchemaMigrationOptions* options,
    SceneSchemaMigrationReport* out_report) {
    FILE* fp;
    int body_n = 0;
    int con_n = 0;
    int i;
    int dry_run = 0;
    SceneSchemaDocument doc;
    SceneSchemaMigrationReport report;
    LegacyBodyRow legacy_bodies[SCENE_SCHEMA_MAX_ENTITIES];
    int legacy_body_count = 0;

    report.source_version = 0;
    report.target_version = SCENE_SCHEMA_VERSION_V1;
    report.entity_count = 0;
    report.constraint_count = 0;
    report.warning_count = 0;
    report.failure_count = 0;

    if (legacy_path == NULL) {
        report.failure_count++;
        scene_schema_copy_report(out_report, &report);
        return 0;
    }
    if (options != NULL && options->dry_run) dry_run = 1;

    fp = fopen(legacy_path, "r");
    if (fp == NULL) {
        report.failure_count++;
        scene_schema_copy_report(out_report, &report);
        return 0;
    }

    scene_schema_document_init(&doc);
    {
        char base_name[64];
        char guid_buf[SCENE_SCHEMA_MAX_GUID];
        scene_schema_extract_basename(legacy_path, base_name, (int)sizeof(base_name));
        snprintf(guid_buf, sizeof(guid_buf), "scene://%s", base_name);
        scene_schema_copy_text(doc.scene_guid, SCENE_SCHEMA_MAX_GUID, guid_buf);
    }

    if (fscanf(fp, "BODIES %d\n", &body_n) != 1 || body_n < 0) {
        fclose(fp);
        report.failure_count++;
        scene_schema_copy_report(out_report, &report);
        return 0;
    }
    for (i = 0; i < body_n; i++) {
        char linebuf[256];
        char kind = 0;
        LegacyBodyRow row;
        int n = 0;
        if (fgets(linebuf, sizeof(linebuf), fp) == NULL) {
            fclose(fp);
            report.failure_count++;
            scene_schema_copy_report(out_report, &report);
            return 0;
        }
        n = sscanf(linebuf, "b %c %f %f %f %f %f %f %f %f %f %f",
                   &kind, &row.mass, &row.px, &row.py, &row.vx, &row.vy, &row.ang,
                   &row.rest, &row.fric, &row.size, &row.damp);
        if (n != 10 && n != 11) {
            fclose(fp);
            report.failure_count++;
            scene_schema_copy_report(out_report, &report);
            return 0;
        }
        if (n == 10) row.damp = 1.0f;
        row.kind = kind;
        if (legacy_body_count >= SCENE_SCHEMA_MAX_ENTITIES) {
            report.warning_count++;
            continue;
        }
        legacy_bodies[legacy_body_count++] = row;
    }

    if (fscanf(fp, "CONSTRAINTS %d\n", &con_n) == 1 && con_n >= 0) {
        for (i = 0; i < con_n; i++) {
            char linebuf[256];
            LegacyConstraintRow row;
            int n;
            if (fgets(linebuf, sizeof(linebuf), fp) == NULL) {
                fclose(fp);
                report.failure_count++;
                scene_schema_copy_report(out_report, &report);
                return 0;
            }
            n = sscanf(linebuf, "c %d %d %d %f %f %f %f %d",
                       &row.type, &row.ai, &row.bi, &row.rest_length, &row.stiffness,
                       &row.damping, &row.break_force, &row.collide_connected);
            if (n != 8) {
                fclose(fp);
                report.failure_count++;
                scene_schema_copy_report(out_report, &report);
                return 0;
            }
            if (row.ai < 0 || row.ai >= legacy_body_count || row.bi < 0 || row.bi >= legacy_body_count) {
                report.warning_count++;
                continue;
            }
            if (doc.constraint_count >= SCENE_SCHEMA_MAX_CONSTRAINTS) {
                report.warning_count++;
                continue;
            }
            doc.constraints[doc.constraint_count].constraint_id = doc.constraint_count + 1;
            doc.constraints[doc.constraint_count].type = row.type;
            doc.constraints[doc.constraint_count].entity_id_a = row.ai + 1;
            doc.constraints[doc.constraint_count].entity_id_b = row.bi + 1;
            doc.constraints[doc.constraint_count].rest_length = row.rest_length;
            doc.constraints[doc.constraint_count].stiffness = row.stiffness;
            doc.constraints[doc.constraint_count].damping = row.damping;
            doc.constraints[doc.constraint_count].break_force = row.break_force;
            doc.constraints[doc.constraint_count].collide_connected = row.collide_connected ? 1 : 0;
            doc.constraint_count++;
        }
    }
    fclose(fp);

    for (i = 0; i < legacy_body_count; i++) {
        SceneSchemaEntity* e = &doc.entities[doc.entity_count];
        if (doc.entity_count >= SCENE_SCHEMA_MAX_ENTITIES) {
            report.warning_count++;
            break;
        }
        e->entity_id = doc.entity_count + 1;
        snprintf(e->name, sizeof(e->name), "entity_%03d", e->entity_id);
        e->shape = (legacy_bodies[i].kind == 'C') ? SCENE_SCHEMA_ENTITY_CIRCLE : SCENE_SCHEMA_ENTITY_BOX;
        e->mass = legacy_bodies[i].mass;
        e->position_x = legacy_bodies[i].px;
        e->position_y = legacy_bodies[i].py;
        e->velocity_x = legacy_bodies[i].vx;
        e->velocity_y = legacy_bodies[i].vy;
        e->angular_velocity = legacy_bodies[i].ang;
        e->restitution = legacy_bodies[i].rest;
        e->friction = legacy_bodies[i].fric;
        e->size = legacy_bodies[i].size;
        e->damping = legacy_bodies[i].damp;
        doc.entity_count++;
    }

    report.entity_count = doc.entity_count;
    report.constraint_count = doc.constraint_count;

    if (dry_run) {
        scene_schema_copy_report(out_report, &report);
        return 1;
    }

    if (out_path == NULL || !scene_schema_save_atomic(&doc, out_path)) {
        report.failure_count++;
        scene_schema_copy_report(out_report, &report);
        return 0;
    }

    scene_schema_copy_report(out_report, &report);
    return 1;
}
