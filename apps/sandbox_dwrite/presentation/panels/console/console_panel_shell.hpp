#ifndef CONSOLE_PANEL_SHELL_H
#define CONSOLE_PANEL_SHELL_H

#include <d2d1.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void (*render_console_tab)(D2D1_RECT_F bottom_rect);
    void (*render_perf_tab)(D2D1_RECT_F bottom_rect);
} ConsolePanelShellCallbacks;

void console_panel_shell_render(
    D2D1_RECT_F bottom_rect,
    int show_bottom_panel,
    int bottom_panel_collapsed,
    int bottom_active_tab,
    const ConsolePanelShellCallbacks* callbacks);

#ifdef __cplusplus
}
#endif

#endif
