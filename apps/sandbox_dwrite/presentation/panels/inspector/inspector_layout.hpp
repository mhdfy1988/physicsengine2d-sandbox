#ifndef INSPECTOR_LAYOUT_H
#define INSPECTOR_LAYOUT_H

#include <d2d1.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    D2D1_RECT_F inspector_rect;
    D2D1_RECT_F debug_rect;
    D2D1_RECT_F inspector_viewport_rect;
    D2D1_RECT_F debug_viewport_rect;
} InspectorPanelLayout;

void inspector_panel_layout_build(D2D1_RECT_F right_rect, InspectorPanelLayout* out_layout);

#ifdef __cplusplus
}
#endif

#endif
