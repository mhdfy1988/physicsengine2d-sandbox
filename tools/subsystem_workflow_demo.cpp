#include <direct.h>
#include <stdio.h>
#include <string.h>
#include "../include/asset_importer.hpp"
#include "../include/subsystem_render_audio_animation.hpp"

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

int main(void) {
    const char* assets_dir = "_tmp_subsystem_demo_assets";
    const char* cache_dir = "_tmp_subsystem_demo_cache";
    const char* texture_path = "_tmp_subsystem_demo_assets/demo_sprite.png";
    const char* audio_path = "_tmp_subsystem_demo_assets/demo_sfx.wav";
    AssetImportRequest req;
    AssetImportResult texture_r;
    AssetImportResult audio_r;
    SubsystemWorkflowWorld world;
    AnimationKeyframe frames[2];
    Render2DDrawCommand draw_cmd;
    AudioPlayCommand audio_cmd;
    RigidBody* body;

    _mkdir(assets_dir);
    _mkdir(cache_dir);
    if (!write_text_file(texture_path, "demo-texture") || !write_text_file(audio_path, "demo-audio")) {
        printf("[FAIL] subsystem workflow demo input generation failed\n");
        return 1;
    }
    memset(&req, 0, sizeof(req));
    req.cache_root = cache_dir;
    req.settings_fingerprint = "demo";
    req.source_path = texture_path;
    if (!asset_importer_run(&req, &texture_r) || !texture_r.success) {
        printf("[FAIL] subsystem workflow demo texture import failed\n");
        return 2;
    }
    req.source_path = audio_path;
    if (!asset_importer_run(&req, &audio_r) || !audio_r.success) {
        printf("[FAIL] subsystem workflow demo audio import failed\n");
        return 3;
    }

    subsystem_workflow_init(&world);
    body = body_create(0.0f, 0.0f, 1.0f, shape_create_box(1.0f, 1.0f));
    if (body == NULL) {
        printf("[FAIL] subsystem workflow demo body creation failed\n");
        return 4;
    }
    body_set_type(body, BODY_KINEMATIC);
    subsystem_workflow_bind_sprite(&world, body, texture_r.meta.guid, 1.5f, 1.0f, 2);
    subsystem_workflow_bind_audio(&world, body, audio_r.meta.guid, 0.75f, 0);
    frames[0].time_s = 0.0f;
    frames[0].position = vec2(0.0f, 0.0f);
    frames[0].angle = 0.0f;
    frames[1].time_s = 1.0f;
    frames[1].position = vec2(3.0f, 1.5f);
    frames[1].angle = 0.5f;
    subsystem_workflow_bind_animation(&world, body, frames, 2, 1);
    subsystem_workflow_tick_animation(&world, 0.5f);
    subsystem_workflow_request_audio_play(&world, body);
    if (subsystem_workflow_build_draw_commands(&world, &draw_cmd, 1) != 1 ||
        subsystem_workflow_collect_audio_commands(&world, &audio_cmd, 1) != 1) {
        printf("[FAIL] subsystem workflow demo command emission failed\n");
        body_free(body);
        return 5;
    }

    printf("[DEMO] draw texture=%s pos=(%.2f,%.2f) angle=%.2f size=(%.2f,%.2f) layer=%d\n",
           draw_cmd.texture_guid,
           draw_cmd.position.x,
           draw_cmd.position.y,
           draw_cmd.angle,
           draw_cmd.width,
           draw_cmd.height,
           draw_cmd.layer);
    printf("[DEMO] audio clip=%s pos=(%.2f,%.2f) gain=%.2f loop=%d\n",
           audio_cmd.clip_guid,
           audio_cmd.position.x,
           audio_cmd.position.y,
           audio_cmd.gain,
           audio_cmd.loop);
    printf("[PASS] subsystem workflow demo\n");

    body_free(body);
    remove(texture_r.artifact_path);
    remove(audio_r.artifact_path);
    {
        char meta_path[ASSET_IMPORTER_MAX_PATH];
        snprintf(meta_path, sizeof(meta_path), "%s.meta", texture_path);
        remove(meta_path);
        snprintf(meta_path, sizeof(meta_path), "%s.meta", audio_path);
        remove(meta_path);
    }
    remove(texture_path);
    remove(audio_path);
    _rmdir(cache_dir);
    _rmdir(assets_dir);
    return 0;
}
