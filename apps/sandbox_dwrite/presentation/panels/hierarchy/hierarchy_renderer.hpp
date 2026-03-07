#ifndef HIERARCHY_RENDERER_H
#define HIERARCHY_RENDERER_H

#include <d2d1.h>
#include <windows.h>

#include "physics_core/physics.h"
#include "../../../application/scene_catalog.hpp"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    HIERARCHY_RENDERER_EXPLORER_MAX_ITEMS = 512,
    HIERARCHY_RENDERER_SCENE_MAX_ITEMS = SCENE_COUNT
};

typedef struct {
    D2D1_RECT_F scene_header_rect;
    D2D1_RECT_F bodies_header_rect;
    D2D1_RECT_F constraints_header_rect;
    D2D1_RECT_F body_circle_header_rect;
    D2D1_RECT_F body_polygon_header_rect;
    D2D1_RECT_F constraint_distance_header_rect;
    D2D1_RECT_F constraint_spring_header_rect;
} HierarchyHeaderRects;

typedef struct {
    int tree_scene_expanded;
    int tree_bodies_expanded;
    int tree_constraints_expanded;
    int tree_body_circle_expanded;
    int tree_body_polygon_expanded;
    int tree_constraint_distance_expanded;
    int tree_constraint_spring_expanded;
    int hierarchy_filter_len;
    const wchar_t* hierarchy_filter_buf;
    int current_scene_index;
    RigidBody* selected_body;
    int selected_constraint_index;
} HierarchyRendererState;

typedef struct {
    int body_visible_count;
    int constraint_visible_count;
    int body_circle_visible_count;
    int body_polygon_visible_count;
    int constraint_distance_visible_count;
    int constraint_spring_visible_count;
} HierarchyRendererStats;

typedef struct {
    D2D1_RECT_F scene_rects[HIERARCHY_RENDERER_SCENE_MAX_ITEMS];
    int scene_indexes[HIERARCHY_RENDERER_SCENE_MAX_ITEMS];
    int scene_count;
    D2D1_RECT_F body_rects[HIERARCHY_RENDERER_EXPLORER_MAX_ITEMS];
    RigidBody* body_ptrs[HIERARCHY_RENDERER_EXPLORER_MAX_ITEMS];
    int body_count;
    D2D1_RECT_F constraint_rects[HIERARCHY_RENDERER_EXPLORER_MAX_ITEMS];
    int constraint_indexes[HIERARCHY_RENDERER_EXPLORER_MAX_ITEMS];
    int constraint_count;
    HierarchyHeaderRects header_rects;
} HierarchyRendererOutput;

typedef struct {
    ID2D1HwndRenderTarget* target;
    ID2D1SolidColorBrush* brush;
    IDWriteTextFormat* fmt_mono;
    IDWriteTextFormat* fmt_info;
    D2D1_RECT_F viewport_rect;
    const HierarchyRendererState* state;
    const HierarchyRendererStats* stats;
    const PhysicsEngine* engine;
    int (*scene_order_index_at)(int order_index);
    const wchar_t* (*scene_display_name)(int scene_index);
    const wchar_t* (*body_kind_name)(const RigidBody* body);
    const wchar_t* (*body_shape_name)(const RigidBody* body);
    void (*draw_text)(const wchar_t* text, D2D1_RECT_F rect, IDWriteTextFormat* fmt, D2D1_COLOR_F color);
    void (*draw_tree_disclosure_icon)(D2D1_RECT_F rect, int expanded, D2D1_COLOR_F color, float stroke);
} HierarchyRendererContext;

float hierarchy_renderer_render_sections(const HierarchyRendererContext* context,
                                         D2D1_RECT_F hierarchy_rect,
                                         float start_y,
                                         float row_h,
                                         HierarchyRendererOutput* output);

#ifdef __cplusplus
}
#endif

#endif
