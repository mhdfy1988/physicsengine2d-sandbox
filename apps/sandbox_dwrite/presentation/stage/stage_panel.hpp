#ifndef STAGE_PANEL_H
#define STAGE_PANEL_H

#include <d2d1.h>
#include <dwrite.h>

#include "physics_core/physics.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const wchar_t* scene_name;
    int pie_active;
    int running;
} StageHeaderModel;

typedef struct {
    PhysicsEngine* engine;
    D2D1_COLOR_F fill;
    D2D1_COLOR_F border;
} StagePanelModel;

typedef struct {
    void (*draw_panel_header_band)(D2D1_RECT_F rect, float header_height, float radius);
    void (*draw_text_vcenter)(const wchar_t* text, D2D1_RECT_F rect, IDWriteTextFormat* format, D2D1_COLOR_F color);
    void (*draw_text_right_vcenter)(const wchar_t* text, D2D1_RECT_F rect, IDWriteTextFormat* format, D2D1_COLOR_F color);
    void (*draw_outer_shadow_rr)(D2D1_ROUNDED_RECT rr);
    void (*set_brush_color)(float r, float g, float b, float a);
    void (*draw_body_2d)(RigidBody* body);
    void (*draw_velocity)(RigidBody* body);
    void (*draw_constraints_debug)(void);
    void (*draw_contacts)(void);
    void (*release_unknown)(IUnknown** unknown);
} StagePanelCallbacks;

void stage_header_render(
    IDWriteTextFormat* fmt_ui,
    IDWriteTextFormat* fmt_info,
    D2D1_RECT_F center_rect,
    const StageHeaderModel* model,
    const StagePanelCallbacks* callbacks);

void stage_panel_render(
    ID2D1Factory* d2d_factory,
    ID2D1HwndRenderTarget* target,
    ID2D1SolidColorBrush* brush,
    D2D1_ROUNDED_RECT stage_rr,
    D2D1_RECT_F stage_rect,
    const StagePanelModel* model,
    const StagePanelCallbacks* callbacks);

#ifdef __cplusplus
}
#endif

#endif
