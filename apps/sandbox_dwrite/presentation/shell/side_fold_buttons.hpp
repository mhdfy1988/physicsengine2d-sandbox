#ifndef SIDE_FOLD_BUTTONS_H
#define SIDE_FOLD_BUTTONS_H

#include <d2d1.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int show_left_panel;
    int show_right_panel;
    float mouse_x;
    float mouse_y;
} SideFoldButtonsModel;

typedef struct {
    D2D1_RECT_F left_fold_rect;
    D2D1_RECT_F right_fold_rect;
} SideFoldButtonsOutput;

typedef struct {
    void (*draw_card_round)(D2D1_RECT_F rect, float radius, D2D1_COLOR_F fill, D2D1_COLOR_F border);
    void (*draw_icon_chevron_lr)(ID2D1HwndRenderTarget* target, ID2D1SolidColorBrush* brush, D2D1_RECT_F rect, int point_right, D2D1_COLOR_F color, float stroke_width);
} SideFoldButtonsCallbacks;

void side_fold_buttons_render(
    ID2D1HwndRenderTarget* target,
    ID2D1SolidColorBrush* brush,
    D2D1_RECT_F top_rect,
    float work_bottom,
    D2D1_RECT_F left_rect,
    D2D1_RECT_F center_rect,
    D2D1_RECT_F right_rect,
    const SideFoldButtonsModel* model,
    const SideFoldButtonsCallbacks* callbacks,
    SideFoldButtonsOutput* output);

#ifdef __cplusplus
}
#endif

#endif
