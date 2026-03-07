#ifndef UI_COMPONENT_STYLE_H
#define UI_COMPONENT_STYLE_H

#include "ui_tokens.hpp"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    D2D1_COLOR_F fill;
    D2D1_COLOR_F border;
    float radius;
} PanelStyle;

typedef struct {
    D2D1_COLOR_F fill;
    D2D1_COLOR_F border;
    D2D1_COLOR_F text;
    float radius;
} ToolbarStyle;

typedef struct {
    D2D1_COLOR_F fill;
    D2D1_COLOR_F border;
    D2D1_COLOR_F text;
    D2D1_COLOR_F meta_text;
    D2D1_COLOR_F hint_text;
    float radius;
} StatusBarStyle;

typedef struct {
    PanelStyle shell_panel;
    PanelStyle stage_panel;
    ToolbarStyle toolbar;
    StatusBarStyle status_bar;
} UiComponentStyle;

#ifdef __cplusplus
}
#endif

#endif
