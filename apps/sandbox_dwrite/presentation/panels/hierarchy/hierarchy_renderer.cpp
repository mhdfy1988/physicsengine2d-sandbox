#include "hierarchy_renderer.hpp"

#include <stdio.h>
#include <string.h>
#include <wchar.h>

static D2D1_RECT_F hierarchy_renderer_rc(float l, float t, float r, float b) {
    D2D1_RECT_F v = {l, t, r, b};
    return v;
}

static D2D1_COLOR_F hierarchy_renderer_rgba(float r, float g, float b, float a) {
    D2D1_COLOR_F v = {r, g, b, a};
    return v;
}

static int hierarchy_renderer_matches_filter(const HierarchyRendererState* state, const wchar_t* line) {
    if (state == 0 || state->hierarchy_filter_len <= 0 || state->hierarchy_filter_buf == 0) return 1;
    return wcsstr(line, state->hierarchy_filter_buf) != 0;
}

static float hierarchy_renderer_render_scene_section(const HierarchyRendererContext* context,
                                                     D2D1_RECT_F hierarchy_rect,
                                                     float y,
                                                     float row_h,
                                                     HierarchyRendererOutput* output) {
    int i;
    int scene_index;
    int row_count = 0;
    wchar_t line[128];
    float content_right = context->viewport_rect.right - 2.0f;
    float header_width;
    float shortcut_width = 0.0f;
    const wchar_t* shortcut_text = 0;
    output->header_rects.scene_header_rect = hierarchy_renderer_rc(hierarchy_rect.left + 10.0f, y, hierarchy_rect.right - 10.0f, y + row_h);
    header_width = output->header_rects.scene_header_rect.right - output->header_rects.scene_header_rect.left;
    if (header_width >= 280.0f) {
        shortcut_text = L"F2重命名 / F3资产GUID";
        shortcut_width = 184.0f;
    } else if (header_width >= 210.0f) {
        shortcut_text = L"F2/F3";
        shortcut_width = 56.0f;
    }
    context->draw_tree_disclosure_icon(hierarchy_renderer_rc(output->header_rects.scene_header_rect.left + 2.0f,
                                                             output->header_rects.scene_header_rect.top + 3.0f,
                                                             output->header_rects.scene_header_rect.left + 14.0f,
                                                             output->header_rects.scene_header_rect.bottom - 3.0f),
                                       context->state->tree_scene_expanded,
                                       hierarchy_renderer_rgba(0.82f, 0.87f, 0.94f, 1.0f), 1.4f);
    context->draw_text(L"场景",
                       hierarchy_renderer_rc(output->header_rects.scene_header_rect.left + 16.0f, output->header_rects.scene_header_rect.top,
                                             output->header_rects.scene_header_rect.right - shortcut_width,
                                             output->header_rects.scene_header_rect.bottom),
                       context->fmt_mono, hierarchy_renderer_rgba(0.82f, 0.87f, 0.94f, 1.0f));
    if (shortcut_text != 0) {
        context->draw_text(shortcut_text,
                           hierarchy_renderer_rc(output->header_rects.scene_header_rect.right - shortcut_width,
                                                 output->header_rects.scene_header_rect.top,
                                                 output->header_rects.scene_header_rect.right - 6.0f,
                                                 output->header_rects.scene_header_rect.bottom),
                           context->fmt_info, hierarchy_renderer_rgba(0.63f, 0.72f, 0.84f, 1.0f));
    }
    y += row_h;
    output->scene_count = 0;
    if (!context->state->tree_scene_expanded) return y;
    for (i = 0; i < SCENE_COUNT && output->scene_count < HIERARCHY_RENDERER_SCENE_MAX_ITEMS; i++) {
        D2D1_RECT_F row;
        scene_index = context->scene_order_index_at(i);
        swprintf(line, 128, L"#%d %ls", scene_index + 1, context->scene_display_name(scene_index));
        if (!hierarchy_renderer_matches_filter(context->state, line)) continue;
        row = hierarchy_renderer_rc(hierarchy_rect.left + 24.0f, y + row_count * (row_h + 4.0f), content_right,
                                    y + row_count * (row_h + 4.0f) + row_h);
        output->scene_rects[output->scene_count] = row;
        output->scene_indexes[output->scene_count] = scene_index;
        context->draw_text(line, row, context->fmt_mono,
                           (scene_index == context->state->current_scene_index)
                               ? hierarchy_renderer_rgba(0.98f, 0.78f, 0.42f, 1.0f)
                               : hierarchy_renderer_rgba(0.88f, 0.92f, 0.97f, 1.0f));
        output->scene_count++;
        row_count++;
    }
    if (row_count == 0) {
        context->draw_text(L"(无匹配场景)",
                           hierarchy_renderer_rc(hierarchy_rect.left + 24.0f, y, content_right, y + row_h),
                           context->fmt_info, hierarchy_renderer_rgba(0.62f, 0.70f, 0.82f, 1.0f));
        return y + row_h + 4.0f;
    }
    return y + row_count * (row_h + 4.0f) + 4.0f;
}

static float hierarchy_renderer_render_bodies_section(const HierarchyRendererContext* context,
                                                      D2D1_RECT_F hierarchy_rect,
                                                      float y,
                                                      float row_h,
                                                      HierarchyRendererOutput* output) {
    int i;
    int bi = 0;
    int bidx = 0;
    wchar_t line[128];
    float content_right = context->viewport_rect.right - 2.0f;
    output->header_rects.bodies_header_rect = hierarchy_renderer_rc(hierarchy_rect.left + 10.0f, y, hierarchy_rect.right - 10.0f, y + row_h);
    swprintf(line, 128, L"物体 (%d)", context->stats->body_visible_count);
    context->draw_tree_disclosure_icon(hierarchy_renderer_rc(output->header_rects.bodies_header_rect.left + 2.0f,
                                                             output->header_rects.bodies_header_rect.top + 3.0f,
                                                             output->header_rects.bodies_header_rect.left + 14.0f,
                                                             output->header_rects.bodies_header_rect.bottom - 3.0f),
                                       context->state->tree_bodies_expanded,
                                       hierarchy_renderer_rgba(0.82f, 0.87f, 0.94f, 1.0f), 1.4f);
    context->draw_text(line,
                       hierarchy_renderer_rc(output->header_rects.bodies_header_rect.left + 16.0f, output->header_rects.bodies_header_rect.top,
                                             output->header_rects.bodies_header_rect.right, output->header_rects.bodies_header_rect.bottom),
                       context->fmt_mono, hierarchy_renderer_rgba(0.82f, 0.87f, 0.94f, 1.0f));
    y += row_h;
    output->header_rects.body_circle_header_rect = hierarchy_renderer_rc(0, 0, 0, 0);
    output->header_rects.body_polygon_header_rect = hierarchy_renderer_rc(0, 0, 0, 0);
    output->body_count = 0;
    if (!context->state->tree_bodies_expanded || context->engine == 0) return y;

    if (context->stats->body_circle_visible_count > 0) {
        int circle_idx = 0;
        output->header_rects.body_circle_header_rect = hierarchy_renderer_rc(hierarchy_rect.left + 24.0f, y, content_right, y + row_h);
        swprintf(line, 128, L"圆 (%d)", context->stats->body_circle_visible_count);
        context->draw_tree_disclosure_icon(hierarchy_renderer_rc(output->header_rects.body_circle_header_rect.left + 2.0f,
                                                                 output->header_rects.body_circle_header_rect.top + 3.0f,
                                                                 output->header_rects.body_circle_header_rect.left + 14.0f,
                                                                 output->header_rects.body_circle_header_rect.bottom - 3.0f),
                                           context->state->tree_body_circle_expanded,
                                           hierarchy_renderer_rgba(0.74f, 0.82f, 0.92f, 1.0f), 1.3f);
        context->draw_text(line,
                           hierarchy_renderer_rc(output->header_rects.body_circle_header_rect.left + 16.0f, output->header_rects.body_circle_header_rect.top,
                                                 output->header_rects.body_circle_header_rect.right, output->header_rects.body_circle_header_rect.bottom),
                           context->fmt_info, hierarchy_renderer_rgba(0.74f, 0.82f, 0.92f, 1.0f));
        y += row_h;
        if (context->state->tree_body_circle_expanded) {
            for (i = 0; i < physics_engine_get_body_count((PhysicsEngine*)context->engine) && bidx < HIERARCHY_RENDERER_EXPLORER_MAX_ITEMS; i++) {
                RigidBody* b = physics_engine_get_body((PhysicsEngine*)context->engine, i);
                D2D1_RECT_F row;
                if (b == 0 || b->type != BODY_DYNAMIC || b->shape == 0 || b->shape->type != SHAPE_CIRCLE) continue;
                swprintf(line, 128, L"#%d %ls/%ls", bi + 1, context->body_kind_name(b), context->body_shape_name(b));
                if (!hierarchy_renderer_matches_filter(context->state, line)) continue;
                row = hierarchy_renderer_rc(hierarchy_rect.left + 38.0f, y + circle_idx * (row_h + 4.0f), content_right,
                                            y + circle_idx * (row_h + 4.0f) + row_h);
                output->body_rects[bidx] = row;
                output->body_ptrs[bidx] = b;
                context->draw_text(line, row, context->fmt_info,
                                   (b == context->state->selected_body)
                                       ? hierarchy_renderer_rgba(0.98f, 0.78f, 0.42f, 1.0f)
                                       : hierarchy_renderer_rgba(0.87f, 0.91f, 0.96f, 1.0f));
                bi++;
                bidx++;
                circle_idx++;
            }
            y += circle_idx * (row_h + 4.0f);
        }
    }

    if (context->stats->body_polygon_visible_count > 0) {
        int poly_idx = 0;
        output->header_rects.body_polygon_header_rect = hierarchy_renderer_rc(hierarchy_rect.left + 24.0f, y, content_right, y + row_h);
        swprintf(line, 128, L"多边形 (%d)", context->stats->body_polygon_visible_count);
        context->draw_tree_disclosure_icon(hierarchy_renderer_rc(output->header_rects.body_polygon_header_rect.left + 2.0f,
                                                                 output->header_rects.body_polygon_header_rect.top + 3.0f,
                                                                 output->header_rects.body_polygon_header_rect.left + 14.0f,
                                                                 output->header_rects.body_polygon_header_rect.bottom - 3.0f),
                                           context->state->tree_body_polygon_expanded,
                                           hierarchy_renderer_rgba(0.74f, 0.82f, 0.92f, 1.0f), 1.3f);
        context->draw_text(line,
                           hierarchy_renderer_rc(output->header_rects.body_polygon_header_rect.left + 16.0f, output->header_rects.body_polygon_header_rect.top,
                                                 output->header_rects.body_polygon_header_rect.right, output->header_rects.body_polygon_header_rect.bottom),
                           context->fmt_info, hierarchy_renderer_rgba(0.74f, 0.82f, 0.92f, 1.0f));
        y += row_h;
        if (context->state->tree_body_polygon_expanded) {
            for (i = 0; i < physics_engine_get_body_count((PhysicsEngine*)context->engine) && bidx < HIERARCHY_RENDERER_EXPLORER_MAX_ITEMS; i++) {
                RigidBody* b = physics_engine_get_body((PhysicsEngine*)context->engine, i);
                D2D1_RECT_F row;
                if (b == 0 || b->type != BODY_DYNAMIC || b->shape == 0 || b->shape->type != SHAPE_POLYGON) continue;
                swprintf(line, 128, L"#%d %ls/%ls", bi + 1, context->body_kind_name(b), context->body_shape_name(b));
                if (!hierarchy_renderer_matches_filter(context->state, line)) continue;
                row = hierarchy_renderer_rc(hierarchy_rect.left + 38.0f, y + poly_idx * (row_h + 4.0f), content_right,
                                            y + poly_idx * (row_h + 4.0f) + row_h);
                output->body_rects[bidx] = row;
                output->body_ptrs[bidx] = b;
                context->draw_text(line, row, context->fmt_info,
                                   (b == context->state->selected_body)
                                       ? hierarchy_renderer_rgba(0.98f, 0.78f, 0.42f, 1.0f)
                                       : hierarchy_renderer_rgba(0.87f, 0.91f, 0.96f, 1.0f));
                bi++;
                bidx++;
                poly_idx++;
            }
            y += poly_idx * (row_h + 4.0f);
        }
    }
    output->body_count = bidx;
    return y;
}

static float hierarchy_renderer_render_constraints_section(const HierarchyRendererContext* context,
                                                           D2D1_RECT_F hierarchy_rect,
                                                           float y,
                                                           float row_h,
                                                           HierarchyRendererOutput* output) {
    int i;
    int ci = 0;
    int cidx = 0;
    wchar_t line[128];
    float content_right = context->viewport_rect.right - 2.0f;
    output->header_rects.constraints_header_rect = hierarchy_renderer_rc(hierarchy_rect.left + 10.0f, y, hierarchy_rect.right - 10.0f, y + row_h);
    swprintf(line, 128, L"约束 (%d)", context->stats->constraint_visible_count);
    context->draw_tree_disclosure_icon(hierarchy_renderer_rc(output->header_rects.constraints_header_rect.left + 2.0f,
                                                             output->header_rects.constraints_header_rect.top + 3.0f,
                                                             output->header_rects.constraints_header_rect.left + 14.0f,
                                                             output->header_rects.constraints_header_rect.bottom - 3.0f),
                                       context->state->tree_constraints_expanded,
                                       hierarchy_renderer_rgba(0.82f, 0.87f, 0.94f, 1.0f), 1.4f);
    context->draw_text(line,
                       hierarchy_renderer_rc(output->header_rects.constraints_header_rect.left + 16.0f, output->header_rects.constraints_header_rect.top,
                                             output->header_rects.constraints_header_rect.right, output->header_rects.constraints_header_rect.bottom),
                       context->fmt_mono, hierarchy_renderer_rgba(0.82f, 0.87f, 0.94f, 1.0f));
    y += row_h;
    output->header_rects.constraint_distance_header_rect = hierarchy_renderer_rc(0, 0, 0, 0);
    output->header_rects.constraint_spring_header_rect = hierarchy_renderer_rc(0, 0, 0, 0);
    output->constraint_count = 0;
    if (!context->state->tree_constraints_expanded || context->engine == 0) return y;

    if (context->stats->constraint_distance_visible_count > 0) {
        int dist_idx = 0;
        output->header_rects.constraint_distance_header_rect = hierarchy_renderer_rc(hierarchy_rect.left + 24.0f, y, content_right, y + row_h);
        swprintf(line, 128, L"距离约束 (%d)", context->stats->constraint_distance_visible_count);
        context->draw_tree_disclosure_icon(hierarchy_renderer_rc(output->header_rects.constraint_distance_header_rect.left + 2.0f,
                                                                 output->header_rects.constraint_distance_header_rect.top + 3.0f,
                                                                 output->header_rects.constraint_distance_header_rect.left + 14.0f,
                                                                 output->header_rects.constraint_distance_header_rect.bottom - 3.0f),
                                           context->state->tree_constraint_distance_expanded,
                                           hierarchy_renderer_rgba(0.74f, 0.82f, 0.92f, 1.0f), 1.3f);
        context->draw_text(line,
                           hierarchy_renderer_rc(output->header_rects.constraint_distance_header_rect.left + 16.0f,
                                                 output->header_rects.constraint_distance_header_rect.top,
                                                 output->header_rects.constraint_distance_header_rect.right,
                                                 output->header_rects.constraint_distance_header_rect.bottom),
                           context->fmt_info, hierarchy_renderer_rgba(0.74f, 0.82f, 0.92f, 1.0f));
        y += row_h;
        if (context->state->tree_constraint_distance_expanded) {
            for (i = 0; i < physics_engine_get_constraint_count((PhysicsEngine*)context->engine) && cidx < HIERARCHY_RENDERER_EXPLORER_MAX_ITEMS; i++) {
                const Constraint* c = physics_engine_get_constraint((PhysicsEngine*)context->engine, i);
                D2D1_RECT_F row;
                if (c == 0 || !c->active || c->type != CONSTRAINT_DISTANCE) continue;
                swprintf(line, 128, L"#%d 距离", ci + 1);
                if (!hierarchy_renderer_matches_filter(context->state, line)) continue;
                row = hierarchy_renderer_rc(hierarchy_rect.left + 38.0f, y + dist_idx * (row_h + 4.0f), content_right,
                                            y + dist_idx * (row_h + 4.0f) + row_h);
                output->constraint_rects[cidx] = row;
                output->constraint_indexes[cidx] = i;
                context->draw_text(line, row, context->fmt_info,
                                   (i == context->state->selected_constraint_index)
                                       ? hierarchy_renderer_rgba(0.98f, 0.78f, 0.42f, 1.0f)
                                       : hierarchy_renderer_rgba(0.87f, 0.91f, 0.96f, 1.0f));
                ci++;
                cidx++;
                dist_idx++;
            }
            y += dist_idx * (row_h + 4.0f);
        }
    }

    if (context->stats->constraint_spring_visible_count > 0) {
        int spring_idx = 0;
        output->header_rects.constraint_spring_header_rect = hierarchy_renderer_rc(hierarchy_rect.left + 24.0f, y, content_right, y + row_h);
        swprintf(line, 128, L"弹簧约束 (%d)", context->stats->constraint_spring_visible_count);
        context->draw_tree_disclosure_icon(hierarchy_renderer_rc(output->header_rects.constraint_spring_header_rect.left + 2.0f,
                                                                 output->header_rects.constraint_spring_header_rect.top + 3.0f,
                                                                 output->header_rects.constraint_spring_header_rect.left + 14.0f,
                                                                 output->header_rects.constraint_spring_header_rect.bottom - 3.0f),
                                           context->state->tree_constraint_spring_expanded,
                                           hierarchy_renderer_rgba(0.74f, 0.82f, 0.92f, 1.0f), 1.3f);
        context->draw_text(line,
                           hierarchy_renderer_rc(output->header_rects.constraint_spring_header_rect.left + 16.0f,
                                                 output->header_rects.constraint_spring_header_rect.top,
                                                 output->header_rects.constraint_spring_header_rect.right,
                                                 output->header_rects.constraint_spring_header_rect.bottom),
                           context->fmt_info, hierarchy_renderer_rgba(0.74f, 0.82f, 0.92f, 1.0f));
        y += row_h;
        if (context->state->tree_constraint_spring_expanded) {
            for (i = 0; i < physics_engine_get_constraint_count((PhysicsEngine*)context->engine) && cidx < HIERARCHY_RENDERER_EXPLORER_MAX_ITEMS; i++) {
                const Constraint* c = physics_engine_get_constraint((PhysicsEngine*)context->engine, i);
                D2D1_RECT_F row;
                if (c == 0 || !c->active || c->type != CONSTRAINT_SPRING) continue;
                swprintf(line, 128, L"#%d 弹簧", ci + 1);
                if (!hierarchy_renderer_matches_filter(context->state, line)) continue;
                row = hierarchy_renderer_rc(hierarchy_rect.left + 38.0f, y + spring_idx * (row_h + 4.0f), content_right,
                                            y + spring_idx * (row_h + 4.0f) + row_h);
                output->constraint_rects[cidx] = row;
                output->constraint_indexes[cidx] = i;
                context->draw_text(line, row, context->fmt_info,
                                   (i == context->state->selected_constraint_index)
                                       ? hierarchy_renderer_rgba(0.98f, 0.78f, 0.42f, 1.0f)
                                       : hierarchy_renderer_rgba(0.87f, 0.91f, 0.96f, 1.0f));
                ci++;
                cidx++;
                spring_idx++;
            }
            y += spring_idx * (row_h + 4.0f);
        }
    }
    output->constraint_count = cidx;
    return y;
}

float hierarchy_renderer_render_sections(const HierarchyRendererContext* context,
                                         D2D1_RECT_F hierarchy_rect,
                                         float start_y,
                                         float row_h,
                                         HierarchyRendererOutput* output) {
    float y;
    if (context == 0 || output == 0) return start_y;
    memset(output, 0, sizeof(*output));
    y = hierarchy_renderer_render_scene_section(context, hierarchy_rect, start_y, row_h, output);
    y = hierarchy_renderer_render_bodies_section(context, hierarchy_rect, y, row_h, output);
    y = hierarchy_renderer_render_constraints_section(context, hierarchy_rect, y, row_h, output);
    return y;
}
