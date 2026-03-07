#ifndef EDITOR_SCREEN_RENDERER_H
#define EDITOR_SCREEN_RENDERER_H

#include <windows.h>
#include <d2d1.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    HWND hwnd;
    D2D1_COLOR_F clear_color;
    float width;
    float height;
    float menu_height;
    float toolbar_height;
    float work_bottom;
    D2D1_RECT_F top_rect;
    D2D1_RECT_F status_rect;
    D2D1_RECT_F bottom_rect;
    D2D1_RECT_F left_rect;
    D2D1_RECT_F right_rect;
    D2D1_RECT_F center_rect;
    D2D1_RECT_F stage_rect;
    D2D1_ROUNDED_RECT left_rr;
    D2D1_ROUNDED_RECT center_rr;
    D2D1_ROUNDED_RECT right_rr;
    D2D1_ROUNDED_RECT bottom_rr;
    D2D1_ROUNDED_RECT status_rr;
    D2D1_ROUNDED_RECT stage_rr;
} EditorScreenRenderFrame;

typedef struct {
    void (*reset_splitter_rects)(void);
    void (*update_stage_bounds_and_fit)(D2D1_RECT_F stage_rect);
    void (*begin_render_pass)(D2D1_COLOR_F clear_color);
    void (*render_shell_panels)(D2D1_RECT_F left_rect, D2D1_RECT_F center_rect, D2D1_RECT_F right_rect,
                                D2D1_RECT_F bottom_rect, D2D1_RECT_F status_rect,
                                D2D1_ROUNDED_RECT left_rr, D2D1_ROUNDED_RECT center_rr, D2D1_ROUNDED_RECT right_rr,
                                D2D1_ROUNDED_RECT bottom_rr, D2D1_ROUNDED_RECT status_rr);
    void (*render_top_bar_content)(D2D1_RECT_F top_rect, float width, float menu_h, float toolbar_h, HWND hwnd);
    void (*render_workspace_content)(D2D1_RECT_F top_rect, float work_bottom,
                                     D2D1_RECT_F left_rect, D2D1_RECT_F right_rect, D2D1_RECT_F center_rect,
                                     D2D1_ROUNDED_RECT stage_rr, D2D1_RECT_F stage_rect,
                                     D2D1_RECT_F bottom_rect, D2D1_RECT_F status_rect, float width, float height);
    void (*finalize_render_pass)(void);
} EditorScreenRendererCallbacks;

void editor_screen_renderer_draw_frame(const EditorScreenRenderFrame* frame, const EditorScreenRendererCallbacks* callbacks);

#ifdef __cplusplus
}
#endif

#endif
