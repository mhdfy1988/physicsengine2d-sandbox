#include "side_fold_buttons.hpp"

static D2D1_RECT_F side_fold_rect(float left, float top, float right, float bottom) {
    D2D1_RECT_F rect;
    rect.left = left;
    rect.top = top;
    rect.right = right;
    rect.bottom = bottom;
    return rect;
}

static int side_fold_point_in_rect(float x, float y, D2D1_RECT_F rect) {
    return x >= rect.left && x <= rect.right && y >= rect.top && y <= rect.bottom;
}

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
    SideFoldButtonsOutput* output) {
    float fold_top;
    D2D1_COLOR_F fold_bg;
    D2D1_COLOR_F fold_bd;
    D2D1_COLOR_F fold_fg;
    if (target == 0 || brush == 0 || model == 0 || callbacks == 0 || output == 0) return;
    fold_top = ((top_rect.bottom + 8.0f) + work_bottom) * 0.5f - 13.0f;
    fold_bg = D2D1::ColorF(0.19f, 0.22f, 0.29f, 1.0f);
    fold_bd = D2D1::ColorF(0.33f, 0.39f, 0.49f, 1.0f);
    fold_fg = D2D1::ColorF(0.86f, 0.91f, 0.97f, 1.0f);

    if (model->show_left_panel) {
        output->left_fold_rect = side_fold_rect(left_rect.right - 10.0f, fold_top, left_rect.right + 10.0f, fold_top + 26.0f);
        callbacks->draw_card_round(output->left_fold_rect, 5.0f,
                                   side_fold_point_in_rect(model->mouse_x, model->mouse_y, output->left_fold_rect) ? D2D1::ColorF(0.24f, 0.30f, 0.40f, 1.0f) : fold_bg,
                                   fold_bd);
        callbacks->draw_icon_chevron_lr(target, brush, output->left_fold_rect, 1, fold_fg, 1.5f);
    } else {
        output->left_fold_rect = side_fold_rect(center_rect.left + 2.0f, fold_top, center_rect.left + 22.0f, fold_top + 26.0f);
        callbacks->draw_card_round(output->left_fold_rect, 5.0f,
                                   side_fold_point_in_rect(model->mouse_x, model->mouse_y, output->left_fold_rect) ? D2D1::ColorF(0.24f, 0.30f, 0.40f, 1.0f) : fold_bg,
                                   fold_bd);
        callbacks->draw_icon_chevron_lr(target, brush, output->left_fold_rect, 0, fold_fg, 1.5f);
    }

    if (model->show_right_panel) {
        output->right_fold_rect = side_fold_rect(right_rect.left - 10.0f, fold_top, right_rect.left + 10.0f, fold_top + 26.0f);
        callbacks->draw_card_round(output->right_fold_rect, 5.0f,
                                   side_fold_point_in_rect(model->mouse_x, model->mouse_y, output->right_fold_rect) ? D2D1::ColorF(0.24f, 0.30f, 0.40f, 1.0f) : fold_bg,
                                   fold_bd);
        callbacks->draw_icon_chevron_lr(target, brush, output->right_fold_rect, 0, fold_fg, 1.5f);
    } else {
        output->right_fold_rect = side_fold_rect(center_rect.right - 22.0f, fold_top, center_rect.right - 2.0f, fold_top + 26.0f);
        callbacks->draw_card_round(output->right_fold_rect, 5.0f,
                                   side_fold_point_in_rect(model->mouse_x, model->mouse_y, output->right_fold_rect) ? D2D1::ColorF(0.24f, 0.30f, 0.40f, 1.0f) : fold_bg,
                                   fold_bd);
        callbacks->draw_icon_chevron_lr(target, brush, output->right_fold_rect, 1, fold_fg, 1.5f);
    }
}
