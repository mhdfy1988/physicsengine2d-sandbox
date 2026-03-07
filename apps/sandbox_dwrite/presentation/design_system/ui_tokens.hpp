#ifndef UI_TOKENS_H
#define UI_TOKENS_H

#include <d2d1.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    D2D1_COLOR_F clear_canvas;
    D2D1_COLOR_F shell_panel_fill;
    D2D1_COLOR_F shell_panel_border;
    D2D1_COLOR_F status_fill;
    D2D1_COLOR_F status_border;
    D2D1_COLOR_F toolbar_fill;
    D2D1_COLOR_F toolbar_border;
    D2D1_COLOR_F toolbar_separator;
    D2D1_COLOR_F stage_fill;
    D2D1_COLOR_F stage_border;
    D2D1_COLOR_F text_primary;
    D2D1_COLOR_F text_secondary;
    D2D1_COLOR_F text_muted;
    D2D1_COLOR_F text_inverse;
    D2D1_COLOR_F accent;
    D2D1_COLOR_F warning;
    D2D1_COLOR_F success;
    D2D1_COLOR_F overlay_scrim;
    D2D1_COLOR_F scroll_track_fill;
    D2D1_COLOR_F scroll_track_border;
    D2D1_COLOR_F scroll_thumb_fill;
    D2D1_COLOR_F scroll_thumb_hover;
    D2D1_COLOR_F scroll_thumb_active;
    D2D1_COLOR_F scroll_thumb_border;
} UiColorTokens;

typedef struct {
    float radius_panel;
    float radius_status;
    float radius_toolbar;
    float space_shell_gap;
    float control_height_toolbar;
    float status_bar_height;
} UiMetricTokens;

#ifdef __cplusplus
}
#endif

#endif
