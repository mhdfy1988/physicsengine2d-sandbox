#include "ui_theme.hpp"

static D2D1_COLOR_F ui_theme_rgba(float r, float g, float b, float a) {
    D2D1_COLOR_F color;
    color.r = r;
    color.g = g;
    color.b = b;
    color.a = a;
    return color;
}

void ui_theme_build(int light_theme, UiTheme* out_theme) {
    UiColorTokens colors;
    UiMetricTokens metrics;

    if (out_theme == 0) return;

    if (light_theme) {
        colors.clear_canvas = ui_theme_rgba(0.92f, 0.94f, 0.98f, 1.0f);
        colors.shell_panel_fill = ui_theme_rgba(0.96f, 0.97f, 0.995f, 1.0f);
        colors.shell_panel_border = ui_theme_rgba(0.80f, 0.84f, 0.92f, 1.0f);
        colors.status_fill = ui_theme_rgba(0.94f, 0.95f, 0.985f, 1.0f);
        colors.status_border = ui_theme_rgba(0.77f, 0.82f, 0.90f, 1.0f);
        colors.toolbar_fill = ui_theme_rgba(0.92f, 0.94f, 0.98f, 1.0f);
        colors.toolbar_border = ui_theme_rgba(0.75f, 0.80f, 0.90f, 1.0f);
        colors.toolbar_separator = ui_theme_rgba(0.74f, 0.80f, 0.90f, 1.0f);
        colors.stage_fill = ui_theme_rgba(0.95f, 0.965f, 0.99f, 1.0f);
        colors.stage_border = ui_theme_rgba(0.73f, 0.79f, 0.89f, 1.0f);
        colors.text_primary = ui_theme_rgba(0.18f, 0.23f, 0.32f, 1.0f);
        colors.text_secondary = ui_theme_rgba(0.32f, 0.41f, 0.54f, 1.0f);
        colors.text_muted = ui_theme_rgba(0.45f, 0.55f, 0.69f, 1.0f);
        colors.text_inverse = ui_theme_rgba(0.95f, 0.97f, 1.0f, 1.0f);
        colors.accent = ui_theme_rgba(0.31f, 0.58f, 0.88f, 1.0f);
        colors.warning = ui_theme_rgba(0.94f, 0.65f, 0.25f, 1.0f);
        colors.success = ui_theme_rgba(0.30f, 0.67f, 0.39f, 1.0f);
        colors.overlay_scrim = ui_theme_rgba(0.04f, 0.06f, 0.10f, 0.18f);
        colors.scroll_track_fill = ui_theme_rgba(0.82f, 0.86f, 0.92f, 1.0f);
        colors.scroll_track_border = ui_theme_rgba(0.70f, 0.76f, 0.84f, 1.0f);
        colors.scroll_thumb_fill = ui_theme_rgba(0.53f, 0.64f, 0.78f, 1.0f);
        colors.scroll_thumb_hover = ui_theme_rgba(0.46f, 0.60f, 0.76f, 1.0f);
        colors.scroll_thumb_active = ui_theme_rgba(0.40f, 0.56f, 0.75f, 1.0f);
        colors.scroll_thumb_border = ui_theme_rgba(0.40f, 0.52f, 0.67f, 1.0f);
    } else {
        colors.clear_canvas = ui_theme_rgba(0.10f, 0.11f, 0.13f, 1.0f);
        colors.shell_panel_fill = ui_theme_rgba(0.18f, 0.19f, 0.22f, 1.0f);
        colors.shell_panel_border = ui_theme_rgba(0.30f, 0.32f, 0.37f, 1.0f);
        colors.status_fill = ui_theme_rgba(0.15f, 0.16f, 0.19f, 1.0f);
        colors.status_border = ui_theme_rgba(0.27f, 0.29f, 0.34f, 1.0f);
        colors.toolbar_fill = ui_theme_rgba(0.16f, 0.17f, 0.20f, 1.0f);
        colors.toolbar_border = ui_theme_rgba(0.30f, 0.32f, 0.37f, 1.0f);
        colors.toolbar_separator = ui_theme_rgba(0.25f, 0.29f, 0.35f, 1.0f);
        colors.stage_fill = ui_theme_rgba(0.11f, 0.12f, 0.14f, 1.0f);
        colors.stage_border = ui_theme_rgba(0.24f, 0.27f, 0.32f, 1.0f);
        colors.text_primary = ui_theme_rgba(0.90f, 0.93f, 0.98f, 1.0f);
        colors.text_secondary = ui_theme_rgba(0.77f, 0.84f, 0.93f, 1.0f);
        colors.text_muted = ui_theme_rgba(0.55f, 0.65f, 0.79f, 1.0f);
        colors.text_inverse = ui_theme_rgba(0.95f, 0.97f, 1.0f, 1.0f);
        colors.accent = ui_theme_rgba(0.45f, 0.62f, 0.84f, 1.0f);
        colors.warning = ui_theme_rgba(0.98f, 0.72f, 0.40f, 1.0f);
        colors.success = ui_theme_rgba(0.44f, 0.90f, 0.44f, 1.0f);
        colors.overlay_scrim = ui_theme_rgba(0.0f, 0.0f, 0.0f, 0.48f);
        colors.scroll_track_fill = ui_theme_rgba(0.13f, 0.15f, 0.19f, 1.0f);
        colors.scroll_track_border = ui_theme_rgba(0.24f, 0.28f, 0.35f, 1.0f);
        colors.scroll_thumb_fill = ui_theme_rgba(0.33f, 0.43f, 0.57f, 1.0f);
        colors.scroll_thumb_hover = ui_theme_rgba(0.40f, 0.52f, 0.69f, 1.0f);
        colors.scroll_thumb_active = ui_theme_rgba(0.45f, 0.60f, 0.80f, 1.0f);
        colors.scroll_thumb_border = ui_theme_rgba(0.52f, 0.65f, 0.82f, 1.0f);
    }

    metrics.radius_panel = 9.0f;
    metrics.radius_status = 8.0f;
    metrics.radius_toolbar = 7.0f;
    metrics.space_shell_gap = 8.0f;
    metrics.control_height_toolbar = 30.0f;
    metrics.status_bar_height = 32.0f;

    out_theme->colors = colors;
    out_theme->metrics = metrics;

    out_theme->components.shell_panel.fill = colors.shell_panel_fill;
    out_theme->components.shell_panel.border = colors.shell_panel_border;
    out_theme->components.shell_panel.radius = metrics.radius_panel;

    out_theme->components.stage_panel.fill = colors.stage_fill;
    out_theme->components.stage_panel.border = colors.stage_border;
    out_theme->components.stage_panel.radius = metrics.radius_panel;

    out_theme->components.toolbar.fill = colors.toolbar_fill;
    out_theme->components.toolbar.border = colors.toolbar_border;
    out_theme->components.toolbar.text = colors.text_primary;
    out_theme->components.toolbar.radius = metrics.radius_toolbar;

    out_theme->components.status_bar.fill = colors.status_fill;
    out_theme->components.status_bar.border = colors.status_border;
    out_theme->components.status_bar.text = colors.text_secondary;
    out_theme->components.status_bar.meta_text = colors.text_muted;
    out_theme->components.status_bar.hint_text = colors.text_primary;
    out_theme->components.status_bar.radius = metrics.radius_status;
}
