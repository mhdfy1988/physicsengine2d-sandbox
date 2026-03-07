#ifndef UI_THEME_H
#define UI_THEME_H

#include "ui_component_style.hpp"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    UiColorTokens colors;
    UiMetricTokens metrics;
    UiComponentStyle components;
} UiTheme;

void ui_theme_build(int light_theme, UiTheme* out_theme);

#ifdef __cplusplus
}
#endif

#endif
