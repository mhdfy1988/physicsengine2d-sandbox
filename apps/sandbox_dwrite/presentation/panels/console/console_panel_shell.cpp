#include "console_panel_shell.hpp"

void console_panel_shell_render(
    D2D1_RECT_F bottom_rect,
    int show_bottom_panel,
    int bottom_panel_collapsed,
    int bottom_active_tab,
    const ConsolePanelShellCallbacks* callbacks) {
    if (!show_bottom_panel || bottom_panel_collapsed || callbacks == 0) return;
    if (bottom_active_tab == 0) {
        if (callbacks->render_console_tab) callbacks->render_console_tab(bottom_rect);
    } else {
        if (callbacks->render_perf_tab) callbacks->render_perf_tab(bottom_rect);
    }
}
