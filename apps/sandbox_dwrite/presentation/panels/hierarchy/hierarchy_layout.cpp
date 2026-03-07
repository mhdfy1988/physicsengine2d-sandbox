#include "hierarchy_layout.hpp"

static D2D1_RECT_F hierarchy_layout_rc(float l, float t, float r, float b) {
    D2D1_RECT_F v = {l, t, r, b};
    return v;
}

void hierarchy_panel_layout_build(D2D1_RECT_F left_rect, HierarchyPanelLayout* out_layout) {
    float split_y;
    if (out_layout == 0) return;
    split_y = left_rect.top + (left_rect.bottom - left_rect.top) * 0.58f;
    out_layout->row_height = 24.0f;
    out_layout->hierarchy_rect = hierarchy_layout_rc(left_rect.left + 8.0f, left_rect.top + 8.0f, left_rect.right - 8.0f, split_y - 4.0f);
    out_layout->project_rect = hierarchy_layout_rc(left_rect.left + 8.0f, split_y + 4.0f, left_rect.right - 8.0f, left_rect.bottom - 8.0f);
    out_layout->hierarchy_viewport_rect =
        hierarchy_layout_rc(out_layout->hierarchy_rect.left + 10.0f, out_layout->hierarchy_rect.top + 40.0f,
                            out_layout->hierarchy_rect.right - 18.0f, out_layout->hierarchy_rect.bottom - 10.0f);
}
