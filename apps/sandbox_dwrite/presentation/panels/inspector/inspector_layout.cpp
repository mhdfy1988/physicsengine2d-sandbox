#include "inspector_layout.hpp"

static D2D1_RECT_F inspector_layout_rc(float l, float t, float r, float b) {
    D2D1_RECT_F v = {l, t, r, b};
    return v;
}

void inspector_panel_layout_build(D2D1_RECT_F right_rect, InspectorPanelLayout* out_layout) {
    float px;
    float pw;
    float section_gap;
    float panel_h;
    float inspector_h;
    float debug_h;
    if (out_layout == 0) return;
    px = right_rect.left + 10.0f;
    pw = (right_rect.right - right_rect.left) - 20.0f;
    section_gap = 10.0f;
    panel_h = (right_rect.bottom - right_rect.top) - 20.0f;
    if (panel_h < 120.0f) panel_h = 120.0f;
    inspector_h = (panel_h - section_gap) * 0.56f;
    debug_h = panel_h - section_gap - inspector_h;
    if (inspector_h < 140.0f) {
        inspector_h = 140.0f;
        debug_h = panel_h - section_gap - inspector_h;
    }
    if (debug_h < 120.0f) {
        debug_h = 120.0f;
        inspector_h = panel_h - section_gap - debug_h;
        if (inspector_h < 120.0f) inspector_h = 120.0f;
    }
    out_layout->inspector_rect = inspector_layout_rc(px, right_rect.top + 10.0f, px + pw, right_rect.top + 10.0f + inspector_h);
    out_layout->debug_rect = inspector_layout_rc(px, out_layout->inspector_rect.bottom + section_gap, px + pw,
                                                 out_layout->inspector_rect.bottom + section_gap + debug_h);
    out_layout->inspector_viewport_rect = inspector_layout_rc(out_layout->inspector_rect.left + 8.0f, out_layout->inspector_rect.top + 44.0f,
                                                              out_layout->inspector_rect.right - 14.0f, out_layout->inspector_rect.bottom - 8.0f);
    out_layout->debug_viewport_rect = inspector_layout_rc(out_layout->debug_rect.left + 8.0f, out_layout->debug_rect.top + 40.0f,
                                                          out_layout->debug_rect.right - 14.0f, out_layout->debug_rect.bottom - 8.0f);
}
