#include <direct.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "../../include/physics_content/asset_importer.hpp"
#include "../../include/physics_core/physics.hpp"
#include "../../include/physics_content/subsystem_render_audio_animation.hpp"

static int file_exists(const char* path) {
    FILE* fp;
    if (path == NULL) return 0;
    fp = fopen(path, "rb");
    if (fp == NULL) return 0;
    fclose(fp);
    return 1;
}

static int write_text_file(const char* path, const char* text) {
    FILE* fp;
    if (path == NULL || text == NULL) return 0;
    fp = fopen(path, "wb");
    if (fp == NULL) return 0;
    if (fwrite(text, 1, strlen(text), fp) != strlen(text)) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

static int nearly_equal(float a, float b) {
    float delta = a - b;
    if (delta < 0.0f) delta = -delta;
    return delta <= 0.001f;
}

int main(void) {
    const char* assets_dir = "_tmp_subsystem_assets";
    const char* cache_dir = "_tmp_subsystem_cache";
    const char* texture_path = "_tmp_subsystem_assets/hero.png";
    const char* audio_path = "_tmp_subsystem_assets/jump.wav";
    char texture_meta_path[ASSET_IMPORTER_MAX_PATH];
    char audio_meta_path[ASSET_IMPORTER_MAX_PATH];
    AssetImportRequest req;
    AssetImportResult texture_r;
    AssetImportResult audio_r;
    SubsystemWorkflowWorld world;
    AnimationKeyframe keyframes[2];
    Render2DDrawCommand draw_cmds[4];
    AudioPlayCommand audio_cmds[4];
    RigidBody* actor;
    int draw_count;
    int audio_count;

    _mkdir(assets_dir);
    _mkdir(cache_dir);
    if (!write_text_file(texture_path, "fake-png-bytes") ||
        !write_text_file(audio_path, "fake-wav-bytes")) {
        printf("[FAIL] failed to create subsystem workflow inputs\n");
        return 1;
    }

    memset(&req, 0, sizeof(req));
    req.cache_root = cache_dir;
    req.settings_fingerprint = "workflow=smoke";

    req.source_path = texture_path;
    if (!asset_importer_run(&req, &texture_r) ||
        !texture_r.success ||
        texture_r.kind != ASSET_IMPORT_KIND_TEXTURE ||
        !asset_meta_is_valid_guid(texture_r.meta.guid)) {
        printf("[FAIL] texture workflow import failed\n");
        return 2;
    }
    req.source_path = audio_path;
    if (!asset_importer_run(&req, &audio_r) ||
        !audio_r.success ||
        audio_r.kind != ASSET_IMPORT_KIND_AUDIO ||
        !asset_meta_is_valid_guid(audio_r.meta.guid)) {
        printf("[FAIL] audio workflow import failed\n");
        return 3;
    }
    if (!file_exists(texture_r.artifact_path) || !file_exists(audio_r.artifact_path)) {
        printf("[FAIL] workflow artifacts missing\n");
        return 4;
    }

    subsystem_workflow_init(&world);
    actor = body_create(0.0f, 0.0f, 1.0f, shape_create_box(1.0f, 1.0f));
    if (actor == NULL) {
        printf("[FAIL] failed to create workflow actor body\n");
        return 5;
    }
    body_set_type(actor, BODY_KINEMATIC);

    if (!subsystem_workflow_bind_sprite(&world, actor, texture_r.meta.guid, 2.0f, 1.0f, 3) ||
        !subsystem_workflow_bind_audio(&world, actor, audio_r.meta.guid, 0.8f, 0)) {
        printf("[FAIL] failed to bind render/audio workflow components\n");
        body_free(actor);
        return 6;
    }

    keyframes[0].time_s = 0.0f;
    keyframes[0].position = vec2(0.0f, 0.0f);
    keyframes[0].angle = 0.0f;
    keyframes[1].time_s = 1.0f;
    keyframes[1].position = vec2(4.0f, 2.0f);
    keyframes[1].angle = 1.0f;
    if (!subsystem_workflow_bind_animation(&world, actor, keyframes, 2, 1)) {
        printf("[FAIL] failed to bind animation workflow component\n");
        body_free(actor);
        return 7;
    }

    subsystem_workflow_tick_animation(&world, 0.25f);
    if (!nearly_equal(actor->position.x, 1.0f) ||
        !nearly_equal(actor->position.y, 0.5f) ||
        !nearly_equal(actor->angle, 0.25f)) {
        printf("[FAIL] animation workflow tick mismatch\n");
        body_free(actor);
        return 8;
    }

    draw_count = subsystem_workflow_build_draw_commands(&world, draw_cmds, 4);
    if (draw_count != 1 ||
        strcmp(draw_cmds[0].texture_guid, texture_r.meta.guid) != 0 ||
        !nearly_equal(draw_cmds[0].position.x, actor->position.x) ||
        !nearly_equal(draw_cmds[0].position.y, actor->position.y) ||
        !nearly_equal(draw_cmds[0].width, 2.0f) ||
        !nearly_equal(draw_cmds[0].height, 1.0f) ||
        draw_cmds[0].layer != 3) {
        printf("[FAIL] render workflow draw command mismatch\n");
        body_free(actor);
        return 9;
    }

    if (!subsystem_workflow_request_audio_play(&world, actor)) {
        printf("[FAIL] failed to queue audio play request\n");
        body_free(actor);
        return 10;
    }
    audio_count = subsystem_workflow_collect_audio_commands(&world, audio_cmds, 4);
    if (audio_count != 1 ||
        strcmp(audio_cmds[0].clip_guid, audio_r.meta.guid) != 0 ||
        !nearly_equal(audio_cmds[0].position.x, actor->position.x) ||
        !nearly_equal(audio_cmds[0].position.y, actor->position.y) ||
        !nearly_equal(audio_cmds[0].gain, 0.8f) ||
        audio_cmds[0].loop != 0) {
        printf("[FAIL] audio workflow command mismatch\n");
        body_free(actor);
        return 11;
    }
    if (subsystem_workflow_collect_audio_commands(&world, audio_cmds, 4) != 0) {
        printf("[FAIL] audio queue should be drained after collection\n");
        body_free(actor);
        return 12;
    }

    subsystem_workflow_tick_animation(&world, 1.0f);
    if (!nearly_equal(actor->position.x, 1.0f) ||
        !nearly_equal(actor->position.y, 0.5f) ||
        !nearly_equal(actor->angle, 0.25f)) {
        printf("[FAIL] looping animation workflow mismatch\n");
        body_free(actor);
        return 13;
    }

    body_free(actor);

    snprintf(texture_meta_path, sizeof(texture_meta_path), "%s.meta", texture_path);
    snprintf(audio_meta_path, sizeof(audio_meta_path), "%s.meta", audio_path);
    remove(texture_meta_path);
    remove(audio_meta_path);
    remove(texture_r.artifact_path);
    remove(audio_r.artifact_path);
    remove(texture_path);
    remove(audio_path);
    _rmdir(cache_dir);
    _rmdir(assets_dir);

    printf("[PASS] subsystem render/audio/animation smoke\n");
    return 0;
}
