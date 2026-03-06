#include "runtime_param_service.hpp"

static float clampf_local(float v, float min_v, float max_v) {
    if (v < min_v) return min_v;
    if (v > max_v) return max_v;
    return v;
}

void runtime_param_sync_engine(PhysicsEngine* engine, SceneConfig* cfg) {
    int i;
    if (engine == NULL || cfg == NULL) return;
    physics_engine_set_gravity(engine, vec2(0.0f, cfg->gravity_y));
    physics_engine_set_time_step(engine, cfg->time_step);
    physics_engine_set_damping(engine, cfg->damping);
    physics_engine_set_iterations(engine, cfg->iterations);
    for (i = 0; i < physics_engine_get_body_count(engine); i++) {
        RigidBody* b = physics_engine_get_body(engine, i);
        if (b == NULL || b->shape == NULL || b->type != BODY_DYNAMIC) continue;
        if (b->shape->type == SHAPE_CIRCLE) {
            b->mass = cfg->ball_mass;
            b->inv_mass = (b->mass > 0.0f) ? (1.0f / b->mass) : 0.0f;
            b->inertia = shape_get_moment_of_inertia(b->shape, b->mass);
            b->inv_inertia = (b->inertia > 0.0f) ? (1.0f / b->inertia) : 0.0f;
            b->shape->restitution = cfg->ball_restitution;
        } else if (b->shape->type == SHAPE_POLYGON) {
            b->mass = cfg->box_mass;
            b->inv_mass = (b->mass > 0.0f) ? (1.0f / b->mass) : 0.0f;
            b->inertia = shape_get_moment_of_inertia(b->shape, b->mass);
            b->inv_inertia = (b->inertia > 0.0f) ? (1.0f / b->inertia) : 0.0f;
            b->shape->restitution = cfg->box_restitution;
        }
    }
}

int runtime_param_debug_adjust(SceneConfig* cfg, int idx, int sign) {
    if (cfg == NULL || sign == 0) return 0;
    if (idx == 0) cfg->gravity_y = clampf_local(cfg->gravity_y + 0.2f * sign, 0.0f, 30.0f);
    if (idx == 1) cfg->time_step = clampf_local(cfg->time_step + 0.001f * sign, 0.001f, 0.05f);
    if (idx == 2) {
        cfg->iterations += sign;
        if (cfg->iterations < 1) cfg->iterations = 1;
        if (cfg->iterations > 64) cfg->iterations = 64;
    }
    return 1;
}

float runtime_param_get_min(int param_index) {
    switch (param_index) {
        case 0: return -20.0f;
        case 1: return 0.002f;
        case 2: return 0.90f;
        case 3: return 1.0f;
        case 4: return 0.0f;
        case 5: return 0.0f;
        case 6: return 0.1f;
        default: return 0.1f;
    }
}

float runtime_param_get_max(int param_index) {
    switch (param_index) {
        case 0: return 40.0f;
        case 1: return 0.05f;
        case 2: return 1.0f;
        case 3: return 30.0f;
        case 4: return 1.0f;
        case 5: return 1.0f;
        case 6: return 50.0f;
        default: return 50.0f;
    }
}

float runtime_param_get_value(const SceneConfig* cfg, int param_index) {
    if (cfg == NULL) return 0.0f;
    switch (param_index) {
        case 0: return cfg->gravity_y;
        case 1: return cfg->time_step;
        case 2: return cfg->damping;
        case 3: return (float)cfg->iterations;
        case 4: return cfg->ball_restitution;
        case 5: return cfg->box_restitution;
        case 6: return cfg->ball_mass;
        default: return cfg->box_mass;
    }
}

int runtime_param_can_adjust(const SceneConfig* cfg, int param_index, int sign) {
    float v;
    float min_v;
    float max_v;
    const float eps = 1e-6f;
    if (cfg == NULL) return 0;
    if (sign != -1 && sign != 1) return 0;
    v = runtime_param_get_value(cfg, param_index);
    min_v = runtime_param_get_min(param_index);
    max_v = runtime_param_get_max(param_index);
    if (sign < 0) return v > min_v + eps;
    return v < max_v - eps;
}

int runtime_param_adjust(SceneConfig* cfg, int focused_param, int sign) {
    if (cfg == NULL) return 0;
    if (!runtime_param_can_adjust(cfg, focused_param, sign)) return 0;
    if (focused_param == 0) {
        cfg->gravity_y += sign * 0.5f;
        if (cfg->gravity_y < -20.0f) cfg->gravity_y = -20.0f;
        if (cfg->gravity_y > 40.0f) cfg->gravity_y = 40.0f;
    } else if (focused_param == 1) {
        cfg->time_step += sign * 0.001f;
        if (cfg->time_step < 0.002f) cfg->time_step = 0.002f;
        if (cfg->time_step > 0.05f) cfg->time_step = 0.05f;
    } else if (focused_param == 2) {
        cfg->damping += sign * 0.001f;
        if (cfg->damping < 0.90f) cfg->damping = 0.90f;
        if (cfg->damping > 1.0f) cfg->damping = 1.0f;
    } else if (focused_param == 3) {
        cfg->iterations += sign;
        if (cfg->iterations < 1) cfg->iterations = 1;
        if (cfg->iterations > 30) cfg->iterations = 30;
    } else if (focused_param == 4) {
        cfg->ball_restitution += sign * 0.01f;
        if (cfg->ball_restitution < 0.0f) cfg->ball_restitution = 0.0f;
        if (cfg->ball_restitution > 1.0f) cfg->ball_restitution = 1.0f;
    } else if (focused_param == 5) {
        cfg->box_restitution += sign * 0.01f;
        if (cfg->box_restitution < 0.0f) cfg->box_restitution = 0.0f;
        if (cfg->box_restitution > 1.0f) cfg->box_restitution = 1.0f;
    } else if (focused_param == 6) {
        cfg->ball_mass += sign * 0.1f;
        if (cfg->ball_mass < 0.1f) cfg->ball_mass = 0.1f;
        if (cfg->ball_mass > 50.0f) cfg->ball_mass = 50.0f;
    } else {
        cfg->box_mass += sign * 0.1f;
        if (cfg->box_mass < 0.1f) cfg->box_mass = 0.1f;
        if (cfg->box_mass > 50.0f) cfg->box_mass = 50.0f;
    }
    return 1;
}
