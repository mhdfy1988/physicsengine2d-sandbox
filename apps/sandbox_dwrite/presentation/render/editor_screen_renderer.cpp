#include "editor_screen_renderer.hpp"

void editor_screen_renderer_draw_frame(const EditorScreenRenderFrame* frame, const EditorScreenRendererCallbacks* callbacks) {
    if (frame == NULL || callbacks == NULL) return;
    if (callbacks->reset_splitter_rects) callbacks->reset_splitter_rects();
    if (callbacks->update_stage_bounds_and_fit) callbacks->update_stage_bounds_and_fit(frame->stage_rect);
    if (callbacks->begin_render_pass) callbacks->begin_render_pass(frame->clear_color);
    if (callbacks->render_shell_panels) {
        callbacks->render_shell_panels(frame->left_rect, frame->center_rect, frame->right_rect,
                                       frame->bottom_rect, frame->status_rect,
                                       frame->left_rr, frame->center_rr, frame->right_rr,
                                       frame->bottom_rr, frame->status_rr);
    }
    if (callbacks->render_top_bar_content) {
        callbacks->render_top_bar_content(frame->top_rect, frame->width, frame->menu_height, frame->toolbar_height, frame->hwnd);
    }
    if (callbacks->render_workspace_content) {
        callbacks->render_workspace_content(frame->top_rect, frame->work_bottom,
                                            frame->left_rect, frame->right_rect, frame->center_rect,
                                            frame->stage_rr, frame->stage_rect,
                                            frame->bottom_rect, frame->status_rect, frame->width, frame->height);
    }
    if (callbacks->finalize_render_pass) callbacks->finalize_render_pass();
}
