#ifndef SUBSYSTEM_RENDER_AUDIO_ANIMATION_H
#define SUBSYSTEM_RENDER_AUDIO_ANIMATION_H

#include "physics_content/asset_database.hpp"
#include "physics_core/body.hpp"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    SUBSYSTEM_WORKFLOW_MAX_SPRITES = 32,
    SUBSYSTEM_WORKFLOW_MAX_AUDIO_SOURCES = 32,
    SUBSYSTEM_WORKFLOW_MAX_ANIMATIONS = 16,
    SUBSYSTEM_WORKFLOW_MAX_KEYFRAMES = 8
};

typedef struct {
    RigidBody* body;
    char texture_guid[ASSET_DB_MAX_GUID];
    float width;
    float height;
    int layer;
    int visible;
} Render2DSpriteBinding;

typedef struct {
    RigidBody* body;
    char clip_guid[ASSET_DB_MAX_GUID];
    float gain;
    int loop;
    int pending_play_count;
} AudioSourceBinding;

typedef struct {
    float time_s;
    Vec2 position;
    float angle;
} AnimationKeyframe;

typedef struct {
    RigidBody* body;
    int keyframe_count;
    AnimationKeyframe keyframes[SUBSYSTEM_WORKFLOW_MAX_KEYFRAMES];
    float current_time_s;
    int loop;
    int active;
} AnimationBinding;

typedef struct {
    char texture_guid[ASSET_DB_MAX_GUID];
    Vec2 position;
    float angle;
    float width;
    float height;
    int layer;
} Render2DDrawCommand;

typedef struct {
    char clip_guid[ASSET_DB_MAX_GUID];
    Vec2 position;
    float gain;
    int loop;
} AudioPlayCommand;

typedef struct {
    Render2DSpriteBinding sprites[SUBSYSTEM_WORKFLOW_MAX_SPRITES];
    int sprite_count;
    AudioSourceBinding audio_sources[SUBSYSTEM_WORKFLOW_MAX_AUDIO_SOURCES];
    int audio_source_count;
    AnimationBinding animations[SUBSYSTEM_WORKFLOW_MAX_ANIMATIONS];
    int animation_count;
} SubsystemWorkflowWorld;

void subsystem_workflow_init(SubsystemWorkflowWorld* world);
int subsystem_workflow_bind_sprite(
    SubsystemWorkflowWorld* world,
    RigidBody* body,
    const char* texture_guid,
    float width,
    float height,
    int layer);
int subsystem_workflow_bind_audio(
    SubsystemWorkflowWorld* world,
    RigidBody* body,
    const char* clip_guid,
    float gain,
    int loop);
int subsystem_workflow_bind_animation(
    SubsystemWorkflowWorld* world,
    RigidBody* body,
    const AnimationKeyframe* keyframes,
    int keyframe_count,
    int loop);
void subsystem_workflow_tick_animation(SubsystemWorkflowWorld* world, float dt_s);
int subsystem_workflow_request_audio_play(SubsystemWorkflowWorld* world, RigidBody* body);
int subsystem_workflow_build_draw_commands(
    const SubsystemWorkflowWorld* world,
    Render2DDrawCommand* out_commands,
    int max_commands);
int subsystem_workflow_collect_audio_commands(
    SubsystemWorkflowWorld* world,
    AudioPlayCommand* out_commands,
    int max_commands);

#ifdef __cplusplus
}
#endif

#endif
