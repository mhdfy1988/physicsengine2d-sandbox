#include "modal_content.hpp"

#include <stdio.h>

static D2D1_RECT_F modal_content_rect(float left, float top, float right, float bottom) {
    D2D1_RECT_F rect;
    rect.left = left;
    rect.top = top;
    rect.right = right;
    rect.bottom = bottom;
    return rect;
}

void help_modal_content_render(
    D2D1_RECT_F modal_rect,
    IDWriteTextFormat* fmt_title,
    IDWriteTextFormat* fmt_mono,
    const HelpModalContentModel* model,
    const ModalContentCallbacks* callbacks) {
    if (fmt_title == 0 || fmt_mono == 0 || model == 0 || callbacks == 0) return;
    if (model->help_modal_page == 1) {
        callbacks->draw_text(L"物理调试指南", modal_content_rect(modal_rect.left + 18.0f, modal_rect.top + 14.0f, modal_rect.right - 60.0f, modal_rect.top + 46.0f), fmt_title, D2D1::ColorF(0.88f, 0.92f, 0.97f, 1.0f));
        callbacks->draw_text(L"1) 打开约束调试（菜单: 组件 -> 约束调试开关）", modal_content_rect(modal_rect.left + 22.0f, modal_rect.top + 72.0f, modal_rect.right - 24.0f, modal_rect.top + 102.0f), fmt_mono, D2D1::ColorF(0.74f, 0.81f, 0.90f, 1.0f));
        callbacks->draw_text(L"2) 用 N 单步并观察右侧碰撞日志（v/p变化）", modal_content_rect(modal_rect.left + 22.0f, modal_rect.top + 102.0f, modal_rect.right - 24.0f, modal_rect.top + 132.0f), fmt_mono, D2D1::ColorF(0.74f, 0.81f, 0.90f, 1.0f));
        callbacks->draw_text(L"3) 点碰撞日志条目可快速选中相关对象", modal_content_rect(modal_rect.left + 22.0f, modal_rect.top + 132.0f, modal_rect.right - 24.0f, modal_rect.top + 162.0f), fmt_mono, D2D1::ColorF(0.74f, 0.81f, 0.90f, 1.0f));
        callbacks->draw_text(L"4) 在检查器里用 ←/→ 或 +/- 微调，Enter/双击输入精确值", modal_content_rect(modal_rect.left + 22.0f, modal_rect.top + 162.0f, modal_rect.right - 24.0f, modal_rect.top + 192.0f), fmt_mono, D2D1::ColorF(0.74f, 0.81f, 0.90f, 1.0f));
        callbacks->draw_text(L"5) 打开底部性能页，观察 FPS 与物理耗时曲线", modal_content_rect(modal_rect.left + 22.0f, modal_rect.top + 192.0f, modal_rect.right - 24.0f, modal_rect.top + 222.0f), fmt_mono, D2D1::ColorF(0.74f, 0.81f, 0.90f, 1.0f));
    } else if (model->help_modal_page == 2) {
        callbacks->draw_text(L"版本信息", modal_content_rect(modal_rect.left + 18.0f, modal_rect.top + 14.0f, modal_rect.right - 60.0f, modal_rect.top + 46.0f), fmt_title, D2D1::ColorF(0.88f, 0.92f, 0.97f, 1.0f));
        callbacks->draw_text(L"Physics Engine Sandbox Editor", modal_content_rect(modal_rect.left + 22.0f, modal_rect.top + 74.0f, modal_rect.right - 24.0f, modal_rect.top + 104.0f), fmt_mono, D2D1::ColorF(0.74f, 0.81f, 0.90f, 1.0f));
        callbacks->draw_text(L"版本: v0.3", modal_content_rect(modal_rect.left + 22.0f, modal_rect.top + 104.0f, modal_rect.right - 24.0f, modal_rect.top + 134.0f), fmt_mono, D2D1::ColorF(0.74f, 0.81f, 0.90f, 1.0f));
        callbacks->draw_text(L"核心: 2D 刚体 / 约束 / 碰撞 / 编辑器化 UI", modal_content_rect(modal_rect.left + 22.0f, modal_rect.top + 134.0f, modal_rect.right - 24.0f, modal_rect.top + 164.0f), fmt_mono, D2D1::ColorF(0.74f, 0.81f, 0.90f, 1.0f));
        callbacks->draw_text(L"构建: DirectWrite + 自研物理内核", modal_content_rect(modal_rect.left + 22.0f, modal_rect.top + 164.0f, modal_rect.right - 24.0f, modal_rect.top + 194.0f), fmt_mono, D2D1::ColorF(0.74f, 0.81f, 0.90f, 1.0f));
    } else {
        callbacks->draw_text(L"使用说明", modal_content_rect(modal_rect.left + 18.0f, modal_rect.top + 14.0f, modal_rect.right - 60.0f, modal_rect.top + 46.0f), fmt_title, D2D1::ColorF(0.88f, 0.92f, 0.97f, 1.0f));
        callbacks->draw_text(L"1) 场景: F1~F9切换；F2/双击重命名；F3编辑资产GUID；Alt+Up/Down排序", modal_content_rect(modal_rect.left + 22.0f, modal_rect.top + 72.0f, modal_rect.right - 24.0f, modal_rect.top + 102.0f), fmt_mono, D2D1::ColorF(0.74f, 0.81f, 0.90f, 1.0f));
        callbacks->draw_text(L"2) PIE: Space进入/运行/暂停  N单步  Esc退出回滚  R重置场景", modal_content_rect(modal_rect.left + 22.0f, modal_rect.top + 102.0f, modal_rect.right - 24.0f, modal_rect.top + 132.0f), fmt_mono, D2D1::ColorF(0.74f, 0.81f, 0.90f, 1.0f));
        callbacks->draw_text(L"3) 文件/编辑: Ctrl+S保存 Ctrl+O加载 Ctrl+C复制 Ctrl+V粘贴", modal_content_rect(modal_rect.left + 22.0f, modal_rect.top + 132.0f, modal_rect.right - 24.0f, modal_rect.top + 162.0f), fmt_mono, D2D1::ColorF(0.74f, 0.81f, 0.90f, 1.0f));
        callbacks->draw_text(L"4) 创建与删除: 1生成圆 2生成方块 Delete删除选中", modal_content_rect(modal_rect.left + 22.0f, modal_rect.top + 162.0f, modal_rect.right - 24.0f, modal_rect.top + 192.0f), fmt_mono, D2D1::ColorF(0.74f, 0.81f, 0.90f, 1.0f));
        callbacks->draw_text(L"5) 显示开关: C接触点 V速度向量 X中心点", modal_content_rect(modal_rect.left + 22.0f, modal_rect.top + 192.0f, modal_rect.right - 24.0f, modal_rect.top + 222.0f), fmt_mono, D2D1::ColorF(0.74f, 0.81f, 0.90f, 1.0f));
        callbacks->draw_text(L"6) 鼠标左键: 选中并拖拽动态物体", modal_content_rect(modal_rect.left + 22.0f, modal_rect.top + 222.0f, modal_rect.right - 24.0f, modal_rect.top + 252.0f), fmt_mono, D2D1::ColorF(0.74f, 0.81f, 0.90f, 1.0f));
        callbacks->draw_text(L"7) 约束: J距离, K弹簧, L链条, P橡皮绳（均为两次选择）", modal_content_rect(modal_rect.left + 22.0f, modal_rect.top + 252.0f, modal_rect.right - 24.0f, modal_rect.top + 282.0f), fmt_mono, D2D1::ColorF(0.74f, 0.81f, 0.90f, 1.0f));
        callbacks->draw_text(L"8) 约束编辑: T/G刚度 Y/H阻尼 U是否碰撞 B断裂阈值", modal_content_rect(modal_rect.left + 22.0f, modal_rect.top + 282.0f, modal_rect.right - 24.0f, modal_rect.top + 312.0f), fmt_mono, D2D1::ColorF(0.74f, 0.81f, 0.90f, 1.0f));
        callbacks->draw_text(L"9) M 开关约束调试绘制, Esc 关闭当前弹窗", modal_content_rect(modal_rect.left + 22.0f, modal_rect.top + 312.0f, modal_rect.right - 24.0f, modal_rect.top + 342.0f), fmt_mono, D2D1::ColorF(0.74f, 0.81f, 0.90f, 1.0f));
        callbacks->draw_text(L"10) 物理调试: 点击碰撞日志条目可快速选中对象", modal_content_rect(modal_rect.left + 22.0f, modal_rect.top + 342.0f, modal_rect.right - 24.0f, modal_rect.top + 372.0f), fmt_mono, D2D1::ColorF(0.74f, 0.81f, 0.90f, 1.0f));
    }
}

void config_modal_content_render(
    D2D1_RECT_F modal_rect,
    IDWriteTextFormat* fmt_title,
    IDWriteTextFormat* fmt_mono,
    const ConfigModalContentModel* model,
    const ModalContentCallbacks* callbacks,
    ConfigModalContentOutput* output) {
    static const wchar_t* row_name[8] = {L"重力 Y", L"时间步长", L"阻尼", L"迭代次数", L"球弹性", L"箱弹性", L"球质量", L"箱质量"};
    wchar_t line[128];
    int r;
    if (fmt_title == 0 || fmt_mono == 0 || model == 0 || model->config == 0 || callbacks == 0 || output == 0) return;
    callbacks->draw_text(L"配置面板", modal_content_rect(modal_rect.left + 18.0f, modal_rect.top + 14.0f, modal_rect.right - 60.0f, modal_rect.top + 46.0f), fmt_title, D2D1::ColorF(0.88f, 0.92f, 0.97f, 1.0f));
    callbacks->draw_text(L"上下选择，左右调参，也可点 - / +", modal_content_rect(modal_rect.left + 22.0f, modal_rect.top + 60.0f, modal_rect.right - 24.0f, modal_rect.top + 88.0f), fmt_mono, D2D1::ColorF(0.66f, 0.74f, 0.84f, 1.0f));
    for (r = 0; r < 8; r++) {
        D2D1_RECT_F row = modal_content_rect(modal_rect.left + 22.0f, modal_rect.top + 98.0f + r * 50.0f, modal_rect.right - 22.0f, modal_rect.top + 136.0f + r * 50.0f);
        D2D1_RECT_F minus_btn = modal_content_rect(row.right - 84.0f, row.top + 7.0f, row.right - 48.0f, row.bottom - 7.0f);
        D2D1_RECT_F plus_btn = modal_content_rect(row.right - 42.0f, row.top + 7.0f, row.right - 6.0f, row.bottom - 7.0f);
        D2D1_ROUNDED_RECT row_rr;
        output->row_rect[r] = row;
        output->minus_rect[r] = minus_btn;
        output->plus_rect[r] = plus_btn;
        row_rr.rect = row;
        row_rr.radiusX = 7.0f;
        row_rr.radiusY = 7.0f;
        callbacks->draw_outer_shadow_rr(row_rr);
        callbacks->draw_card_round(row, 7.0f,
                                   (r == model->focused_param) ? D2D1::ColorF(0.24f, 0.30f, 0.40f, 1.0f) : D2D1::ColorF(0.17f, 0.20f, 0.26f, 1.0f),
                                   (r == model->focused_param) ? D2D1::ColorF(0.43f, 0.55f, 0.71f, 1.0f) : D2D1::ColorF(0.29f, 0.34f, 0.42f, 1.0f));
        if (r == 0) swprintf(line, 128, L"%.2f", model->config->gravity_y);
        if (r == 1) swprintf(line, 128, L"%.4f", model->config->time_step);
        if (r == 2) swprintf(line, 128, L"%.3f", model->config->damping);
        if (r == 3) swprintf(line, 128, L"%d", model->config->iterations);
        if (r == 4) swprintf(line, 128, L"%.2f", model->config->ball_restitution);
        if (r == 5) swprintf(line, 128, L"%.2f", model->config->box_restitution);
        if (r == 6) swprintf(line, 128, L"%.1f", model->config->ball_mass);
        if (r == 7) swprintf(line, 128, L"%.1f", model->config->box_mass);
        callbacks->draw_text(row_name[r], modal_content_rect(row.left + 12.0f, row.top + 9.0f, row.left + 180.0f, row.bottom - 8.0f), fmt_mono, D2D1::ColorF(0.73f, 0.80f, 0.89f, 1.0f));
        callbacks->draw_text(line, modal_content_rect(row.left + 190.0f, row.top + 9.0f, row.right - 94.0f, row.bottom - 8.0f), fmt_mono, D2D1::ColorF(0.90f, 0.94f, 0.99f, 1.0f));
        callbacks->draw_text(L"-", minus_btn, fmt_mono, callbacks->can_adjust_param(model->config, r, -1) ? D2D1::ColorF(0.78f, 0.84f, 0.93f, 1.0f) : D2D1::ColorF(0.49f, 0.55f, 0.64f, 1.0f));
        callbacks->draw_text(L"+", plus_btn, fmt_mono, callbacks->can_adjust_param(model->config, r, 1) ? D2D1::ColorF(0.78f, 0.84f, 0.93f, 1.0f) : D2D1::ColorF(0.49f, 0.55f, 0.64f, 1.0f));
    }
}
