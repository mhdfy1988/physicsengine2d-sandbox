#include "scene_catalog.hpp"

#include <wchar.h>

static const wchar_t* SCENE_NAMES[SCENE_COUNT] = {
    L"\u9ed8\u8ba4",
    L"\u9ad8\u901f\u96a7\u9053",
    L"\u8d28\u91cf\u6bd4",
    L"\u9ad8\u5806\u53e0",
    L"\u659c\u5761\u6469\u64e6",
    L"\u5f39\u6027\u77e9\u9635",
    L"\u62d6\u62fd\u538b\u6d4b",
    L"\u8fb9\u754c\u6314\u538b",
    L"\u6027\u80fd\u6781\u9650"
};

static const SceneConfig SCENE_DEFAULTS[SCENE_COUNT] = {
    {9.8f, 1.0f / 60.0f, 0.992f, 8, 1.0f, 2.0f, 2.2f, 5.0f, 0.80f, 0.45f, "asset://none"},
    {9.8f, 1.0f / 120.0f, 0.997f, 16, 0.6f, 2.5f, 1.0f, 4.0f, 0.75f, 0.35f, "asset://none"},
    {9.8f, 1.0f / 60.0f, 0.995f, 14, 0.2f, 20.0f, 1.8f, 6.0f, 0.55f, 0.18f, "asset://none"},
    {9.8f, 1.0f / 60.0f, 0.998f, 22, 1.0f, 4.0f, 2.0f, 5.5f, 0.25f, 0.05f, "asset://none"},
    {9.8f, 1.0f / 90.0f, 0.996f, 14, 1.0f, 2.0f, 1.5f, 4.5f, 0.35f, 0.20f, "asset://none"},
    {9.8f, 1.0f / 120.0f, 0.999f, 20, 1.0f, 2.0f, 1.6f, 4.5f, 0.90f, 0.10f, "asset://none"},
    {9.8f, 1.0f / 60.0f, 0.992f, 14, 1.0f, 2.0f, 2.2f, 5.0f, 0.70f, 0.30f, "asset://none"},
    {9.8f, 1.0f / 60.0f, 0.996f, 14, 1.2f, 2.0f, 2.0f, 5.0f, 0.65f, 0.30f, "asset://none"},
    {9.8f, 1.0f / 90.0f, 0.990f, 10, 0.8f, 1.6f, 1.2f, 3.6f, 0.30f, 0.08f, "asset://none"}
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

void scene_catalog_copy_default_names(wchar_t out_names[SCENE_COUNT][SCENE_NAME_MAX], int max_count) {
    int i;
    if (out_names == 0 || max_count <= 0) {
        return;
    }
    for (i = 0; i < SCENE_COUNT && i < max_count; i++) {
        wcsncpy(out_names[i], SCENE_NAMES[i], SCENE_NAME_MAX - 1);
        out_names[i][SCENE_NAME_MAX - 1] = L'\0';
    }
}
