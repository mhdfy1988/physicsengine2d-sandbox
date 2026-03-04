#include "scene_catalog.h"

static const wchar_t* SCENE_NAMES[SCENE_COUNT] = {
    L"默认", L"高速", L"质量比", L"高堆叠", L"斜坡摩擦", L"弹性矩阵",
    L"拖拽压测", L"边界", L"性能"
};

static const SceneConfig SCENE_DEFAULTS[SCENE_COUNT] = {
    {9.8f, 1.0f / 60.0f, 0.992f, 8, 1.0f, 2.0f, 2.2f, 5.0f, 0.80f, 0.45f},
    {9.8f, 1.0f / 120.0f, 0.997f, 16, 0.6f, 2.5f, 1.0f, 4.0f, 0.75f, 0.35f},
    {9.8f, 1.0f / 60.0f, 0.995f, 14, 0.2f, 20.0f, 1.8f, 6.0f, 0.55f, 0.18f},
    {9.8f, 1.0f / 60.0f, 0.998f, 22, 1.0f, 4.0f, 2.0f, 5.5f, 0.25f, 0.05f},
    {9.8f, 1.0f / 90.0f, 0.996f, 14, 1.0f, 2.0f, 1.5f, 4.5f, 0.35f, 0.20f},
    {9.8f, 1.0f / 120.0f, 0.999f, 20, 1.0f, 2.0f, 1.6f, 4.5f, 0.90f, 0.10f},
    {9.8f, 1.0f / 60.0f, 0.992f, 14, 1.0f, 2.0f, 2.2f, 5.0f, 0.70f, 0.30f},
    {9.8f, 1.0f / 60.0f, 0.996f, 14, 1.2f, 2.0f, 2.0f, 5.0f, 0.65f, 0.30f},
    {9.8f, 1.0f / 90.0f, 0.990f, 10, 0.8f, 1.6f, 1.2f, 3.6f, 0.30f, 0.08f}
};

const wchar_t* scene_catalog_name(int scene_index) {
    if (scene_index < 0 || scene_index >= SCENE_COUNT) {
        return SCENE_NAMES[0];
    }
    return SCENE_NAMES[scene_index];
}

const SceneConfig* scene_catalog_default(int scene_index) {
    if (scene_index < 0 || scene_index >= SCENE_COUNT) {
        return &SCENE_DEFAULTS[0];
    }
    return &SCENE_DEFAULTS[scene_index];
}

void scene_catalog_copy_defaults(SceneConfig* out_configs, int max_count) {
    int i;
    if (out_configs == 0 || max_count <= 0) {
        return;
    }
    for (i = 0; i < SCENE_COUNT && i < max_count; i++) {
        out_configs[i] = SCENE_DEFAULTS[i];
    }
}
