#ifndef HIERARCHY_SCROLLBAR_H
#define HIERARCHY_SCROLLBAR_H

#include <d2d1.h>

#include "../../design_system/ui_theme.hpp"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    D2D1_RECT_F host_rect;
    D2D1_RECT_F viewport_rect;
    float viewport_height;
    float content_height;
    int scroll_max;
    int scroll_offset;
    int dragging;
    float mouse_x;
    float mouse_y;
} HierarchyScrollbarModel;

typedef struct {
    D2D1_RECT_F track_rect;
    D2D1_RECT_F thumb_rect;
} HierarchyScrollbarRenderResult;

void hierarchy_scrollbar_render(ID2D1HwndRenderTarget* target,
                                ID2D1SolidColorBrush* brush,
                                const UiTheme* theme,
                                const HierarchyScrollbarModel* model,
                                HierarchyScrollbarRenderResult* out_result);

#ifdef __cplusplus
}
#endif

#endif
