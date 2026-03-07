#include "stage_panel.hpp"

#include <stdio.h>

static D2D1_RECT_F stage_panel_rect(float left, float top, float right, float bottom) {
    D2D1_RECT_F rect;
    rect.left = left;
    rect.top = top;
    rect.right = right;
    rect.bottom = bottom;
    return rect;
}

void stage_header_render(
    IDWriteTextFormat* fmt_ui,
    IDWriteTextFormat* fmt_info,
    D2D1_RECT_F center_rect,
    const StageHeaderModel* model,
    const StagePanelCallbacks* callbacks) {
    wchar_t line[128];
    if (fmt_ui == 0 || fmt_info == 0 || model == 0 || callbacks == 0) return;
    callbacks->draw_panel_header_band(center_rect, 38.0f, 10.0f);
    callbacks->draw_text_vcenter(L"场景", stage_panel_rect(center_rect.left + 16.0f, center_rect.top + 4.0f, center_rect.right - 12.0f, center_rect.top + 40.0f),
                                 fmt_ui, D2D1::ColorF(0.90f, 0.93f, 0.98f, 1.0f));
    if (model->pie_active) {
        swprintf(line, 128, L"%ls | PIE:%ls", model->scene_name, model->running ? L"运行中" : L"已暂停");
    } else {
        swprintf(line, 128, L"%ls | 编辑态", model->scene_name);
    }
    callbacks->draw_text_right_vcenter(line, stage_panel_rect(center_rect.left + 130.0f, center_rect.top + 4.0f, center_rect.right - 36.0f, center_rect.top + 40.0f),
                                       fmt_info, D2D1::ColorF(0.67f, 0.75f, 0.88f, 1.0f));
}

void stage_panel_render(
    ID2D1Factory* d2d_factory,
    ID2D1HwndRenderTarget* target,
    ID2D1SolidColorBrush* brush,
    D2D1_ROUNDED_RECT stage_rr,
    D2D1_RECT_F stage_rect,
    const StagePanelModel* model,
    const StagePanelCallbacks* callbacks) {
    if (d2d_factory == 0 || target == 0 || brush == 0 || model == 0 || callbacks == 0) return;
    callbacks->draw_outer_shadow_rr(stage_rr);
    callbacks->set_brush_color(model->fill.r, model->fill.g, model->fill.b, model->fill.a);
    ID2D1HwndRenderTarget_FillRoundedRectangle(target, &stage_rr, (ID2D1Brush*)brush);
    callbacks->set_brush_color(model->border.r, model->border.g, model->border.b, model->border.a);
    ID2D1HwndRenderTarget_DrawRoundedRectangle(target, &stage_rr, (ID2D1Brush*)brush, 1.0f, NULL);
    if (model->engine != NULL) {
        int i;
        HRESULT clip_hr;
        ID2D1RoundedRectangleGeometry* stage_clip_geo = NULL;
        ID2D1Layer* stage_layer = NULL;
        int used_layer_clip = 0;
        D2D1_LAYER_PARAMETERS layer_params;
        D2D1_MATRIX_3X2_F identity;

        clip_hr = ID2D1Factory_CreateRoundedRectangleGeometry(d2d_factory, &stage_rr, &stage_clip_geo);
        if (SUCCEEDED(clip_hr)) clip_hr = ID2D1HwndRenderTarget_CreateLayer(target, NULL, &stage_layer);
        if (SUCCEEDED(clip_hr) && stage_layer != NULL && stage_clip_geo != NULL) {
            identity._11 = 1.0f;
            identity._12 = 0.0f;
            identity._21 = 0.0f;
            identity._22 = 1.0f;
            identity._31 = 0.0f;
            identity._32 = 0.0f;
            layer_params.contentBounds = stage_rect;
            layer_params.geometricMask = (ID2D1Geometry*)stage_clip_geo;
            layer_params.maskAntialiasMode = D2D1_ANTIALIAS_MODE_PER_PRIMITIVE;
            layer_params.maskTransform = identity;
            layer_params.opacity = 1.0f;
            layer_params.opacityBrush = NULL;
            layer_params.layerOptions = D2D1_LAYER_OPTIONS_NONE;
            ID2D1HwndRenderTarget_PushLayer(target, &layer_params, stage_layer);
            used_layer_clip = 1;
        } else {
            ID2D1HwndRenderTarget_PushAxisAlignedClip(target, &stage_rect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
        }

        for (i = 0; i < physics_engine_get_body_count(model->engine); i++) {
            RigidBody* body = physics_engine_get_body(model->engine, i);
            callbacks->draw_body_2d(body);
            callbacks->draw_velocity(body);
        }
        callbacks->draw_constraints_debug();
        callbacks->draw_contacts();
        if (used_layer_clip) {
            ID2D1HwndRenderTarget_PopLayer(target);
        } else {
            ID2D1HwndRenderTarget_PopAxisAlignedClip(target);
        }
        callbacks->release_unknown((IUnknown**)&stage_layer);
        callbacks->release_unknown((IUnknown**)&stage_clip_geo);
    }
}
