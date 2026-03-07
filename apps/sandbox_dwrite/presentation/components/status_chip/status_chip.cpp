#include "status_chip.hpp"

#include "../../render/ui_primitives.hpp"
#include "../../render/ui_text.hpp"

void ui_status_chip_draw(ID2D1HwndRenderTarget* target,
                         ID2D1SolidColorBrush* brush,
                         IDWriteTextFormat* text_format,
                         D2D1_RECT_F rect,
                         const wchar_t* text,
                         D2D1_COLOR_F fill,
                         D2D1_COLOR_F border,
                         D2D1_COLOR_F text_color) {
    if (target == 0 || brush == 0 || text_format == 0 || text == 0) return;
    ui_draw_card_round(target, brush, rect, 6.0f, fill, border);
    ui_draw_text(target, brush, text, rect, text_format, text_color);
}
