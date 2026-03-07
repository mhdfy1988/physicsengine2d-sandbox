#ifndef INSPECTOR_PANEL_SHELL_H
#define INSPECTOR_PANEL_SHELL_H

#include <d2d1.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void (*clear_ui_state)(void);
    D2D1_RECT_F (*render_inspector_section)(D2D1_RECT_F right_rect);
    void (*render_debug_section)(D2D1_RECT_F debug_rect);
} InspectorPanelShellCallbacks;

void inspector_panel_shell_render(D2D1_RECT_F right_rect, int show_right_panel, const InspectorPanelShellCallbacks* callbacks);

#ifdef __cplusplus
}
#endif

#endif
