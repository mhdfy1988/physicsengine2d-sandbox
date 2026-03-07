#ifndef HIERARCHY_LAYOUT_H
#define HIERARCHY_LAYOUT_H

#include <d2d1.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    D2D1_RECT_F hierarchy_rect;
    D2D1_RECT_F project_rect;
    D2D1_RECT_F hierarchy_viewport_rect;
    float row_height;
} HierarchyPanelLayout;

void hierarchy_panel_layout_build(D2D1_RECT_F left_rect, HierarchyPanelLayout* out_layout);

#ifdef __cplusplus
}
#endif

#endif
