#include "inspector_scrollbar.hpp"

#include "../../render/ui_primitives.hpp"

static D2D1_RECT_F inspector_scrollbar_rc(float l, float t, float r, float b) {
    D2D1_RECT_F v = {l, t, r, b};
    return v;
}

static int inspector_scrollbar_point_in_rect(float x, float y, D2D1_RECT_F rect) {
    return x >= rect.left && x <= rect.right && y >= rect.top && y <= rect.bottom;
}

void inspector_scrollbar_render(ID2D1HwndRenderTarget* target,
                                ID2D1SolidColorBrush* brush,
                                const UiTheme* theme,
                                const InspectorScrollbarModel* model,
                                InspectorScrollbarResult* out_result) {
    float track_h;
    float thumb_h;
    float travel;
    float ratio;
    float thumb_top;
    D2D1_COLOR_F thumb_fill;
    if (out_result != 0) {
        out_result->track_rect = inspector_scrollbar_rc(0, 0, 0, 0);
        out_result->thumb_rect = inspector_scrollbar_rc(0, 0, 0, 0);
    }
    if (target == 0 || brush == 0 || theme == 0 || model == 0) return;
    if (model->scroll_max <= 0 || model->content_height <= 0.0f) return;
    if (out_result != 0) {
        out_result->track_rect = inspector_scrollbar_rc(model->host_rect.right - 10.0f, model->viewport_rect.top,
                                                        model->host_rect.right - 6.0f, model->viewport_rect.bottom);
    }
    track_h = ((out_result != 0) ? out_result->track_rect.bottom - out_result->track_rect.top
                                 : (model->viewport_rect.bottom - model->viewport_rect.top));
    thumb_h = (model->viewport_height / model->content_height) * track_h;
    if (thumb_h < 20.0f) thumb_h = 20.0f;
    if (thumb_h > track_h) thumb_h = track_h;
    travel = track_h - thumb_h;
    ratio = (model->scroll_max > 0) ? ((float)model->scroll_offset / (float)model->scroll_max) : 0.0f;
    thumb_top = ((out_result != 0) ? out_result->track_rect.top : model->viewport_rect.top) + travel * ratio;
    if (out_result != 0) {
        out_result->thumb_rect = inspector_scrollbar_rc(out_result->track_rect.left, thumb_top,
                                                        out_result->track_rect.right, thumb_top + thumb_h);
    }
    ui_draw_card_round(target, brush,
                       (out_result != 0) ? out_result->track_rect
                                         : inspector_scrollbar_rc(model->host_rect.right - 10.0f, model->viewport_rect.top,
                                                                  model->host_rect.right - 6.0f, model->viewport_rect.bottom),
                       2.0f, theme->colors.scroll_track_fill, theme->colors.scroll_track_border);
    thumb_fill = (out_result != 0 && inspector_scrollbar_point_in_rect(model->mouse_x, model->mouse_y, out_result->thumb_rect))
                     ? theme->colors.scroll_thumb_hover
                     : theme->colors.scroll_thumb_fill;
    ui_draw_card_round(target, brush,
                       (out_result != 0) ? out_result->thumb_rect
                                         : inspector_scrollbar_rc(model->host_rect.right - 10.0f, thumb_top,
                                                                  model->host_rect.right - 6.0f, thumb_top + thumb_h),
                       2.0f, thumb_fill, theme->colors.scroll_thumb_border);
}
