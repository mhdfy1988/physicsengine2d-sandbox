#include "top_toolbar.hpp"

#include "../components/button/button.hpp"

static D2D1_RECT_F top_toolbar_rc(float l, float t, float r, float b) {
    D2D1_RECT_F v = {l, t, r, b};
    return v;
}

static D2D1_POINT_2F top_toolbar_pt(float x, float y) {
    D2D1_POINT_2F v = {x, y};
    return v;
}

static D2D1_COLOR_F top_toolbar_rgba(float r, float g, float b, float a) {
    D2D1_COLOR_F v = {r, g, b, a};
    return v;
}

static int top_toolbar_point_in_rect(float x, float y, D2D1_RECT_F rect) {
    return x >= rect.left && x <= rect.right && y >= rect.top && y <= rect.bottom;
}

static void top_toolbar_set_brush_color(ID2D1SolidColorBrush* brush, D2D1_COLOR_F color) {
    ID2D1SolidColorBrush_SetColor(brush, &color);
}

void top_toolbar_layout_build(D2D1_RECT_F toolbar_rect, float leading_inset, TopToolbarLayout* out_layout) {
    float btn_w = 50.0f;
    float btn_y0;
    float btn_y1;
    float x;
    if (out_layout == 0) return;
    btn_y0 = toolbar_rect.top + 6.0f;
    btn_y1 = toolbar_rect.bottom - 6.0f;
    x = toolbar_rect.left + leading_inset;
    out_layout->run_rect = top_toolbar_rc(x, btn_y0, x + btn_w, btn_y1); x += btn_w + 4.0f;
    out_layout->step_rect = top_toolbar_rc(x, btn_y0, x + btn_w, btn_y1); x += btn_w + 4.0f;
    out_layout->reset_rect = top_toolbar_rc(x, btn_y0, x + btn_w, btn_y1); x += btn_w + 8.0f;
    out_layout->save_rect = top_toolbar_rc(x, btn_y0, x + btn_w, btn_y1); x += btn_w + 4.0f;
    out_layout->undo_rect = top_toolbar_rc(x, btn_y0, x + btn_w, btn_y1); x += btn_w + 4.0f;
    out_layout->redo_rect = top_toolbar_rc(x, btn_y0, x + btn_w, btn_y1); x += btn_w + 4.0f;
    out_layout->grid_rect = top_toolbar_rc(x, btn_y0, x + btn_w, btn_y1); x += btn_w + 4.0f;
    out_layout->collision_rect = top_toolbar_rc(x, btn_y0, x + btn_w, btn_y1); x += btn_w + 4.0f;
    out_layout->velocity_rect = top_toolbar_rc(x, btn_y0, x + btn_w, btn_y1); x += btn_w + 4.0f;
    out_layout->constraint_rect = top_toolbar_rc(x, btn_y0, x + btn_w, btn_y1); x += btn_w + 4.0f;
    out_layout->spring_rect = top_toolbar_rc(x, btn_y0, x + btn_w, btn_y1); x += btn_w + 4.0f;
    out_layout->chain_rect = top_toolbar_rc(x, btn_y0, x + btn_w, btn_y1); x += btn_w + 4.0f;
    out_layout->rope_rect = top_toolbar_rc(x, btn_y0, x + btn_w, btn_y1);
}

void top_toolbar_runtime_badge_rect(const TopToolbarLayout* layout, D2D1_RECT_F* out_rect) {
    if (layout == 0 || out_rect == 0) return;
    *out_rect = top_toolbar_rc(layout->run_rect.right - 13.0f, layout->run_rect.top + 3.0f,
                               layout->run_rect.right - 3.0f, layout->run_rect.top + 13.0f);
}

void top_toolbar_render(ID2D1HwndRenderTarget* target,
                        ID2D1SolidColorBrush* brush,
                        const TopToolbarLayout* layout,
                        const TopToolbarRenderState* state) {
    D2D1_RECT_F badge_rect;
    if (target == 0 || brush == 0 || layout == 0 || state == 0) return;

    ui_icon_button_draw(target, brush, layout->run_rect,
                        state->running ? TB_ICON_PAUSE : TB_ICON_RUN,
                        state->running,
                        top_toolbar_point_in_rect(state->mouse_x, state->mouse_y, layout->run_rect));
    if (state->runtime_bus_congested) {
        D2D1_ELLIPSE badge;
        top_toolbar_runtime_badge_rect(layout, &badge_rect);
        badge.point = top_toolbar_pt((badge_rect.left + badge_rect.right) * 0.5f, (badge_rect.top + badge_rect.bottom) * 0.5f);
        badge.radiusX = 4.0f;
        badge.radiusY = 4.0f;
        top_toolbar_set_brush_color(brush, top_toolbar_rgba(0.92f, 0.22f, 0.20f, 1.0f));
        ID2D1HwndRenderTarget_FillEllipse(target, &badge, (ID2D1Brush*)brush);
        if (top_toolbar_point_in_rect(state->mouse_x, state->mouse_y, badge_rect)) {
            top_toolbar_set_brush_color(brush, top_toolbar_rgba(0.99f, 0.80f, 0.78f, 1.0f));
            ID2D1HwndRenderTarget_DrawEllipse(target, &badge, (ID2D1Brush*)brush, 1.0f, NULL);
        }
    }

    ui_icon_button_draw(target, brush, layout->step_rect, TB_ICON_STEP, 0, top_toolbar_point_in_rect(state->mouse_x, state->mouse_y, layout->step_rect));
    ui_icon_button_draw(target, brush, layout->reset_rect, TB_ICON_RESET, 0, top_toolbar_point_in_rect(state->mouse_x, state->mouse_y, layout->reset_rect));
    ui_icon_button_draw(target, brush, layout->save_rect, TB_ICON_SAVE, 0, top_toolbar_point_in_rect(state->mouse_x, state->mouse_y, layout->save_rect));
    ui_icon_button_draw(target, brush, layout->undo_rect, TB_ICON_UNDO, 0, top_toolbar_point_in_rect(state->mouse_x, state->mouse_y, layout->undo_rect));
    ui_icon_button_draw(target, brush, layout->redo_rect, TB_ICON_REDO, 0, top_toolbar_point_in_rect(state->mouse_x, state->mouse_y, layout->redo_rect));
    ui_icon_button_draw(target, brush, layout->grid_rect, TB_ICON_GRID, state->draw_centers, top_toolbar_point_in_rect(state->mouse_x, state->mouse_y, layout->grid_rect));
    ui_icon_button_draw(target, brush, layout->collision_rect, TB_ICON_COLLISION, state->draw_contacts, top_toolbar_point_in_rect(state->mouse_x, state->mouse_y, layout->collision_rect));
    ui_icon_button_draw(target, brush, layout->velocity_rect, TB_ICON_VELOCITY, state->draw_velocity, top_toolbar_point_in_rect(state->mouse_x, state->mouse_y, layout->velocity_rect));
    ui_icon_button_draw(target, brush, layout->constraint_rect, TB_ICON_CONSTRAINT, state->constraint_create_mode == 1,
                        top_toolbar_point_in_rect(state->mouse_x, state->mouse_y, layout->constraint_rect));
    ui_icon_button_draw(target, brush, layout->spring_rect, TB_ICON_SPRING, state->constraint_create_mode == 2,
                        top_toolbar_point_in_rect(state->mouse_x, state->mouse_y, layout->spring_rect));
    ui_icon_button_draw(target, brush, layout->chain_rect, TB_ICON_CHAIN, state->constraint_create_mode == 3,
                        top_toolbar_point_in_rect(state->mouse_x, state->mouse_y, layout->chain_rect));
    ui_icon_button_draw(target, brush, layout->rope_rect, TB_ICON_ROPE, state->constraint_create_mode == 4,
                        top_toolbar_point_in_rect(state->mouse_x, state->mouse_y, layout->rope_rect));
}
