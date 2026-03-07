#include "inspector_panel_shell.hpp"

void inspector_panel_shell_render(D2D1_RECT_F right_rect, int show_right_panel, const InspectorPanelShellCallbacks* callbacks) {
    D2D1_RECT_F debug_rect;
    if (callbacks == 0) return;
    if (!show_right_panel) {
        if (callbacks->clear_ui_state) callbacks->clear_ui_state();
        return;
    }
    if (callbacks->render_inspector_section == 0 || callbacks->render_debug_section == 0) return;
    debug_rect = callbacks->render_inspector_section(right_rect);
    callbacks->render_debug_section(debug_rect);
}
