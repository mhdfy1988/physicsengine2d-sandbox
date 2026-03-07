#include "inspector_renderer.hpp"

#include <stdio.h>
#include <string.h>

static D2D1_RECT_F inspector_renderer_rc(float l, float t, float r, float b) {
    D2D1_RECT_F v = {l, t, r, b};
    return v;
}

static D2D1_COLOR_F inspector_renderer_rgba(float r, float g, float b, float a) {
    D2D1_COLOR_F v = {r, g, b, a};
    return v;
}

static int inspector_renderer_point_in_rect(D2D1_POINT_2F p, D2D1_RECT_F rect) {
    return p.x >= rect.left && p.x <= rect.right && p.y >= rect.top && p.y <= rect.bottom;
}

static const wchar_t* inspector_renderer_row_label(int row, const Constraint* c, RigidBody* b) {
    if (c != 0 && c->active) {
        if (row == 0) return L"目标长度";
        if (row == 1) return L"刚度";
        if (row == 2) return L"阻尼";
        if (row == 3) return L"断裂阈值";
        if (row == 4) return L"连体碰撞";
        if (row == 5) return L"弹性预设";
        return L"字段";
    }
    if (b != 0) {
        if (row == 0) return L"质量";
        if (row == 1) return L"位置X";
        if (row == 2) return L"位置Y";
        if (row == 3) return L"速度X";
        if (row == 4) return L"速度Y";
        if (row == 5) return L"角速度";
        if (row == 6) return L"阻尼";
        if (row == 7) return L"弹性";
    }
    return L"--";
}

static void inspector_renderer_row_value(const InspectorRendererContext* context,
                                         int row,
                                         wchar_t* out_line,
                                         int out_capacity) {
    const Constraint* c = context->state->selected_constraint;
    RigidBody* b = context->state->selected_body;
    if (out_line == 0 || out_capacity <= 0) return;
    out_line[0] = L'\0';
    if (c != 0 && c->active) {
        if (row == 0) swprintf(out_line, (size_t)out_capacity, L"%.2f", c->rest_length);
        if (row == 1) swprintf(out_line, (size_t)out_capacity, L"%.2f", c->stiffness);
        if (row == 2) swprintf(out_line, (size_t)out_capacity, L"%.2f", c->damping);
        if (row == 3) swprintf(out_line, (size_t)out_capacity, L"%.0f", c->break_force);
        if (row == 4) swprintf(out_line, (size_t)out_capacity, L"%ls", c->collide_connected ? L"允许" : L"禁止");
        if (row == 5 && c->type == CONSTRAINT_SPRING) {
            int preset = context->spring_preset_for_constraint ? context->spring_preset_for_constraint(c) : -1;
            swprintf(out_line, (size_t)out_capacity, L"%ls",
                     (preset == 0) ? L"软" : ((preset == 1) ? L"中" : ((preset == 2) ? L"高" : L"自定义")));
        }
        return;
    }
    if (b != 0) {
        if (row == 0) swprintf(out_line, (size_t)out_capacity, L"%.2f", b->mass);
        if (row == 1) swprintf(out_line, (size_t)out_capacity, L"%.2f", b->position.x);
        if (row == 2) swprintf(out_line, (size_t)out_capacity, L"%.2f", b->position.y);
        if (row == 3) swprintf(out_line, (size_t)out_capacity, L"%.2f", b->velocity.x);
        if (row == 4) swprintf(out_line, (size_t)out_capacity, L"%.2f", b->velocity.y);
        if (row == 5) swprintf(out_line, (size_t)out_capacity, L"%.2f", b->angular_velocity);
        if (row == 6) swprintf(out_line, (size_t)out_capacity, L"%.2f", b->damping);
        if (row == 7) swprintf(out_line, (size_t)out_capacity, L"%.2f", (b->shape ? b->shape->restitution : 0.0f));
    }
}

void inspector_renderer_render(const InspectorRendererContext* context, InspectorRendererOutput* out_output) {
    wchar_t line[128];
    float step = 32.0f;
    float lx0;
    float lx1;
    float vx0;
    float vx1;
    float content_h;
    float view_h;
    float offset_y;
    float content_right;
    int focused_row;
    int ri;
    if (out_output != 0) memset(out_output, 0, sizeof(*out_output));
    if (context == 0 || out_output == 0 || context->state == 0) return;

    context->draw_card_round(context->inspector_rect, 10.0f,
                             inspector_renderer_rgba(0.16f, 0.18f, 0.22f, 1.0f),
                             inspector_renderer_rgba(0.29f, 0.33f, 0.40f, 1.0f));
    context->draw_panel_header_band(context->inspector_rect, 38.0f, 12.0f);
    context->draw_text_vcenter(L"属性",
                               inspector_renderer_rc(context->inspector_rect.left + 16.0f, context->inspector_rect.top + 4.0f,
                                                     context->inspector_rect.right - 12.0f, context->inspector_rect.top + 40.0f),
                               context->fmt_ui, inspector_renderer_rgba(0.88f, 0.92f, 0.97f, 1.0f));

    out_output->viewport_rect = context->viewport_rect;
    lx0 = context->inspector_rect.left + 12.0f;
    lx1 = context->inspector_rect.left + 110.0f;
    vx0 = context->inspector_rect.left + 116.0f;
    vx1 = context->inspector_rect.right - 12.0f;
    content_right = context->viewport_rect.right - 2.0f;
    view_h = context->viewport_rect.bottom - context->viewport_rect.top;
    focused_row = context->state->focused_row;
    if (focused_row >= context->state->row_count) focused_row = context->state->row_count - 1;
    if (focused_row < 0) focused_row = 0;
    content_h = (context->state->row_count == 0) ? 28.0f : (context->state->row_count * step + 44.0f);
    out_output->scroll_max = 0;
    if (content_h > view_h) out_output->scroll_max = (int)(content_h - view_h + 0.5f);
    offset_y = context->viewport_rect.top + 2.0f - (float)context->state->scroll_offset;

    ID2D1HwndRenderTarget_PushAxisAlignedClip(context->target, &context->viewport_rect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
    if (context->state->row_count == 0) {
        context->draw_text(L"未选中对象（点击舞台或左侧列表）",
                           inspector_renderer_rc(lx0, offset_y + 2.0f, vx1, offset_y + 26.0f),
                           context->fmt_info, inspector_renderer_rgba(0.76f, 0.82f, 0.92f, 1.0f));
    } else {
        for (ri = 0; ri < context->state->row_count && ri < INSPECTOR_RENDERER_MAX_ROWS; ri++) {
            const wchar_t* label = inspector_renderer_row_label(ri, context->state->selected_constraint, context->state->selected_body);
            D2D1_RECT_F rr = inspector_renderer_rc(context->inspector_rect.left + 10.0f, offset_y + ri * step, content_right, offset_y + ri * step + 26.0f);
            D2D1_RECT_F minus_btn = inspector_renderer_rc(rr.right - 52.0f, rr.top + 3.0f, rr.right - 30.0f, rr.bottom - 3.0f);
            D2D1_RECT_F plus_btn = inspector_renderer_rc(rr.right - 26.0f, rr.top + 3.0f, rr.right - 4.0f, rr.bottom - 3.0f);
            out_output->row_rects[ri] = rr;
            out_output->minus_rects[ri] = minus_btn;
            out_output->plus_rects[ri] = plus_btn;
            context->draw_card_round(rr, 5.0f,
                                     (ri == focused_row) ? inspector_renderer_rgba(0.24f, 0.30f, 0.40f, 1.0f)
                                                         : inspector_renderer_rgba(0.19f, 0.22f, 0.28f, 1.0f),
                                     inspector_renderer_rgba(0.31f, 0.37f, 0.46f, 1.0f));
            inspector_renderer_row_value(context, ri, line, 128);
            context->draw_text(label, inspector_renderer_rc(rr.left + 6.0f, rr.top + 1.0f, lx1, rr.bottom - 1.0f),
                               context->fmt_info, inspector_renderer_rgba(0.70f, 0.77f, 0.86f, 1.0f));
            context->draw_text(line, inspector_renderer_rc(vx0, rr.top + 1.0f, rr.right - 58.0f, rr.bottom - 1.0f),
                               context->fmt_info, inspector_renderer_rgba(0.89f, 0.93f, 0.97f, 1.0f));
            context->draw_card_round(minus_btn, 3.0f,
                                     inspector_renderer_point_in_rect(context->mouse_point, minus_btn)
                                         ? inspector_renderer_rgba(0.31f, 0.40f, 0.54f, 1.0f)
                                         : inspector_renderer_rgba(0.23f, 0.30f, 0.42f, 1.0f),
                                     inspector_renderer_rgba(0.44f, 0.55f, 0.71f, 1.0f));
            context->draw_card_round(plus_btn, 3.0f,
                                     inspector_renderer_point_in_rect(context->mouse_point, plus_btn)
                                         ? inspector_renderer_rgba(0.31f, 0.40f, 0.54f, 1.0f)
                                         : inspector_renderer_rgba(0.23f, 0.30f, 0.42f, 1.0f),
                                     inspector_renderer_rgba(0.44f, 0.55f, 0.71f, 1.0f));
            context->draw_text(L"-", minus_btn, context->fmt_info, inspector_renderer_rgba(0.90f, 0.94f, 0.99f, 1.0f));
            context->draw_text(L"+", plus_btn, context->fmt_info, inspector_renderer_rgba(0.90f, 0.94f, 0.99f, 1.0f));
        }
        context->draw_text(L"提示: Enter输入  双击快速输入  ↑↓切换  ←→或-/+微调",
                           inspector_renderer_rc(lx0, offset_y + context->state->row_count * step + 8.0f,
                                                 vx1, offset_y + context->state->row_count * step + 28.0f),
                           context->fmt_info, inspector_renderer_rgba(0.62f, 0.70f, 0.80f, 1.0f));
        context->draw_text(context->inspector_row_hint_text ? context->inspector_row_hint_text() : L"",
                           inspector_renderer_rc(lx0, offset_y + context->state->row_count * step + 34.0f,
                                                 vx1, offset_y + context->state->row_count * step + 56.0f),
                           context->fmt_info, inspector_renderer_rgba(0.69f, 0.76f, 0.86f, 1.0f));
    }
    ID2D1HwndRenderTarget_PopAxisAlignedClip(context->target);
}
