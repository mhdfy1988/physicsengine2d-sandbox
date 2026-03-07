#ifndef TOP_TOOLBAR_H
#define TOP_TOOLBAR_H

#include <d2d1.h>

#include "../render/ui_widgets.hpp"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    D2D1_RECT_F run_rect;
    D2D1_RECT_F step_rect;
    D2D1_RECT_F reset_rect;
    D2D1_RECT_F save_rect;
    D2D1_RECT_F undo_rect;
    D2D1_RECT_F redo_rect;
    D2D1_RECT_F grid_rect;
    D2D1_RECT_F collision_rect;
    D2D1_RECT_F velocity_rect;
    D2D1_RECT_F constraint_rect;
    D2D1_RECT_F spring_rect;
    D2D1_RECT_F chain_rect;
    D2D1_RECT_F rope_rect;
} TopToolbarLayout;

typedef struct {
    int running;
    int runtime_bus_congested;
    int draw_centers;
    int draw_contacts;
    int draw_velocity;
    int constraint_create_mode;
    float mouse_x;
    float mouse_y;
} TopToolbarRenderState;

void top_toolbar_layout_build(D2D1_RECT_F toolbar_rect, float leading_inset, TopToolbarLayout* out_layout);
void top_toolbar_render(ID2D1HwndRenderTarget* target,
                        ID2D1SolidColorBrush* brush,
                        const TopToolbarLayout* layout,
                        const TopToolbarRenderState* state);
void top_toolbar_runtime_badge_rect(const TopToolbarLayout* layout, D2D1_RECT_F* out_rect);

#ifdef __cplusplus
}
#endif

#endif
