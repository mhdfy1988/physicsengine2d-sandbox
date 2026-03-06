#ifndef RUNTIME_PARAM_SERVICE_H
#define RUNTIME_PARAM_SERVICE_H

#include "physics.hpp"
#include "scene_catalog.hpp"

void runtime_param_sync_engine(PhysicsEngine* engine, SceneConfig* cfg);
int runtime_param_debug_adjust(SceneConfig* cfg, int idx, int sign);
float runtime_param_get_min(int param_index);
float runtime_param_get_max(int param_index);
float runtime_param_get_value(const SceneConfig* cfg, int param_index);
int runtime_param_can_adjust(const SceneConfig* cfg, int param_index, int sign);
int runtime_param_adjust(SceneConfig* cfg, int focused_param, int sign);

#endif
