#include "subsystem_render_audio_animation.hpp"

#include <math.h>
#include <stddef.h>
#include <string.h>

static void subsystem_copy_guid(char out_guid[ASSET_DB_MAX_GUID], const char* guid) {
    if (out_guid == NULL) return;
    if (guid == NULL) {
        out_guid[0] = '\0';
        return;
    }
    strncpy(out_guid, guid, ASSET_DB_MAX_GUID - 1);
    out_guid[ASSET_DB_MAX_GUID - 1] = '\0';
}

static int subsystem_valid_guid(const char* guid) {
    return asset_meta_is_valid_guid(guid);
}

static float subsystem_lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

static Vec2 subsystem_vec2_lerp(Vec2 a, Vec2 b, float t) {
    Vec2 out_v;
    out_v.x = subsystem_lerp(a.x, b.x, t);
    out_v.y = subsystem_lerp(a.y, b.y, t);
    return out_v;
}

static int subsystem_find_sprite_index(const SubsystemWorkflowWorld* world, const RigidBody* body) {
    int i;
    if (world == NULL || body == NULL) return -1;
    for (i = 0; i < world->sprite_count; i++) {
        if (world->sprites[i].body == body) return i;
    }
    return -1;
}

static int subsystem_find_audio_index(const SubsystemWorkflowWorld* world, const RigidBody* body) {
    int i;
    if (world == NULL || body == NULL) return -1;
    for (i = 0; i < world->audio_source_count; i++) {
        if (world->audio_sources[i].body == body) return i;
    }
    return -1;
}

static int subsystem_find_animation_index(const SubsystemWorkflowWorld* world, const RigidBody* body) {
    int i;
    if (world == NULL || body == NULL) return -1;
    for (i = 0; i < world->animation_count; i++) {
        if (world->animations[i].body == body) return i;
    }
    return -1;
}

static int subsystem_animation_sorted(const AnimationKeyframe* keyframes, int keyframe_count) {
    int i;
    if (keyframes == NULL || keyframe_count <= 0) return 0;
    for (i = 1; i < keyframe_count; i++) {
        if (keyframes[i].time_s < keyframes[i - 1].time_s) return 0;
    }
    return 1;
}

static void subsystem_animation_apply(AnimationBinding* binding) {
    float duration;
    int i;
    if (binding == NULL || binding->body == NULL || binding->keyframe_count <= 0) return;
    if (binding->keyframe_count == 1) {
        binding->body->position = binding->keyframes[0].position;
        binding->body->angle = binding->keyframes[0].angle;
        binding->body->sleeping = 0;
        binding->body->sleep_timer = 0.0f;
        return;
    }
    if (binding->current_time_s <= binding->keyframes[0].time_s) {
        binding->body->position = binding->keyframes[0].position;
        binding->body->angle = binding->keyframes[0].angle;
        binding->body->sleeping = 0;
        binding->body->sleep_timer = 0.0f;
        return;
    }
    duration = binding->keyframes[binding->keyframe_count - 1].time_s;
    if (binding->current_time_s >= duration) {
        binding->body->position = binding->keyframes[binding->keyframe_count - 1].position;
        binding->body->angle = binding->keyframes[binding->keyframe_count - 1].angle;
        binding->body->sleeping = 0;
        binding->body->sleep_timer = 0.0f;
        return;
    }
    for (i = 0; i < (binding->keyframe_count - 1); i++) {
        const AnimationKeyframe* a = &binding->keyframes[i];
        const AnimationKeyframe* b = &binding->keyframes[i + 1];
        if (binding->current_time_s >= a->time_s && binding->current_time_s <= b->time_s) {
            float span = b->time_s - a->time_s;
            float t = (span > 0.0f) ? ((binding->current_time_s - a->time_s) / span) : 0.0f;
            binding->body->position = subsystem_vec2_lerp(a->position, b->position, t);
            binding->body->angle = subsystem_lerp(a->angle, b->angle, t);
            binding->body->sleeping = 0;
            binding->body->sleep_timer = 0.0f;
            return;
        }
    }
}

void subsystem_workflow_init(SubsystemWorkflowWorld* world) {
    if (world == NULL) return;
    memset(world, 0, sizeof(*world));
}

int subsystem_workflow_bind_sprite(
    SubsystemWorkflowWorld* world,
    RigidBody* body,
    const char* texture_guid,
    float width,
    float height,
    int layer) {
    int idx;
    Render2DSpriteBinding* binding;
    if (world == NULL || body == NULL) return 0;
    if (!subsystem_valid_guid(texture_guid)) return 0;
    if (width <= 0.0f || height <= 0.0f) return 0;
    idx = subsystem_find_sprite_index(world, body);
    if (idx < 0) {
        if (world->sprite_count >= SUBSYSTEM_WORKFLOW_MAX_SPRITES) return 0;
        idx = world->sprite_count++;
    }
    binding = &world->sprites[idx];
    memset(binding, 0, sizeof(*binding));
    binding->body = body;
    subsystem_copy_guid(binding->texture_guid, texture_guid);
    binding->width = width;
    binding->height = height;
    binding->layer = layer;
    binding->visible = 1;
    return 1;
}

int subsystem_workflow_bind_audio(
    SubsystemWorkflowWorld* world,
    RigidBody* body,
    const char* clip_guid,
    float gain,
    int loop) {
    int idx;
    AudioSourceBinding* binding;
    if (world == NULL || body == NULL) return 0;
    if (!subsystem_valid_guid(clip_guid)) return 0;
    if (gain < 0.0f) return 0;
    idx = subsystem_find_audio_index(world, body);
    if (idx < 0) {
        if (world->audio_source_count >= SUBSYSTEM_WORKFLOW_MAX_AUDIO_SOURCES) return 0;
        idx = world->audio_source_count++;
    }
    binding = &world->audio_sources[idx];
    memset(binding, 0, sizeof(*binding));
    binding->body = body;
    subsystem_copy_guid(binding->clip_guid, clip_guid);
    binding->gain = gain;
    binding->loop = loop ? 1 : 0;
    binding->pending_play_count = 0;
    return 1;
}

int subsystem_workflow_bind_animation(
    SubsystemWorkflowWorld* world,
    RigidBody* body,
    const AnimationKeyframe* keyframes,
    int keyframe_count,
    int loop) {
    int idx;
    AnimationBinding* binding;
    if (world == NULL || body == NULL || keyframes == NULL) return 0;
    if (keyframe_count <= 0 || keyframe_count > SUBSYSTEM_WORKFLOW_MAX_KEYFRAMES) return 0;
    if (!subsystem_animation_sorted(keyframes, keyframe_count)) return 0;
    idx = subsystem_find_animation_index(world, body);
    if (idx < 0) {
        if (world->animation_count >= SUBSYSTEM_WORKFLOW_MAX_ANIMATIONS) return 0;
        idx = world->animation_count++;
    }
    binding = &world->animations[idx];
    memset(binding, 0, sizeof(*binding));
    binding->body = body;
    memcpy(binding->keyframes, keyframes, (size_t)keyframe_count * sizeof(AnimationKeyframe));
    binding->keyframe_count = keyframe_count;
    binding->current_time_s = keyframes[0].time_s;
    binding->loop = loop ? 1 : 0;
    binding->active = 1;
    subsystem_animation_apply(binding);
    return 1;
}

void subsystem_workflow_tick_animation(SubsystemWorkflowWorld* world, float dt_s) {
    int i;
    if (world == NULL || dt_s < 0.0f) return;
    for (i = 0; i < world->animation_count; i++) {
        AnimationBinding* binding = &world->animations[i];
        float duration;
        if (!binding->active || binding->body == NULL || binding->keyframe_count <= 0) continue;
        duration = binding->keyframes[binding->keyframe_count - 1].time_s;
        binding->current_time_s += dt_s;
        if (binding->loop && duration > binding->keyframes[0].time_s) {
            float start_t = binding->keyframes[0].time_s;
            float loop_span = duration - start_t;
            while (binding->current_time_s > duration) {
                binding->current_time_s -= loop_span;
            }
        } else if (binding->current_time_s >= duration) {
            binding->current_time_s = duration;
            binding->active = 0;
        }
        subsystem_animation_apply(binding);
    }
}

int subsystem_workflow_request_audio_play(SubsystemWorkflowWorld* world, RigidBody* body) {
    int idx;
    if (world == NULL || body == NULL) return 0;
    idx = subsystem_find_audio_index(world, body);
    if (idx < 0) return 0;
    world->audio_sources[idx].pending_play_count++;
    return 1;
}

int subsystem_workflow_build_draw_commands(
    const SubsystemWorkflowWorld* world,
    Render2DDrawCommand* out_commands,
    int max_commands) {
    int i;
    int out_count = 0;
    if (world == NULL || out_commands == NULL || max_commands <= 0) return 0;
    for (i = 0; i < world->sprite_count && out_count < max_commands; i++) {
        const Render2DSpriteBinding* binding = &world->sprites[i];
        if (binding->body == NULL || !binding->visible) continue;
        memset(&out_commands[out_count], 0, sizeof(out_commands[out_count]));
        subsystem_copy_guid(out_commands[out_count].texture_guid, binding->texture_guid);
        out_commands[out_count].position = binding->body->position;
        out_commands[out_count].angle = binding->body->angle;
        out_commands[out_count].width = binding->width;
        out_commands[out_count].height = binding->height;
        out_commands[out_count].layer = binding->layer;
        out_count++;
    }
    return out_count;
}

int subsystem_workflow_collect_audio_commands(
    SubsystemWorkflowWorld* world,
    AudioPlayCommand* out_commands,
    int max_commands) {
    int i;
    int out_count = 0;
    if (world == NULL || out_commands == NULL || max_commands <= 0) return 0;
    for (i = 0; i < world->audio_source_count && out_count < max_commands; i++) {
        AudioSourceBinding* binding = &world->audio_sources[i];
        while (binding->body != NULL && binding->pending_play_count > 0 && out_count < max_commands) {
            memset(&out_commands[out_count], 0, sizeof(out_commands[out_count]));
            subsystem_copy_guid(out_commands[out_count].clip_guid, binding->clip_guid);
            out_commands[out_count].position = binding->body->position;
            out_commands[out_count].gain = binding->gain;
            out_commands[out_count].loop = binding->loop;
            binding->pending_play_count--;
            out_count++;
        }
    }
    return out_count;
}
