#ifndef SANDBOX_DWRITE_COM_CPP_COMPAT_HPP
#define SANDBOX_DWRITE_COM_CPP_COMPAT_HPP

#ifdef __cplusplus

#define ID2D1Factory_CreateHwndRenderTarget(self, render_props, hwnd_props, out_target) ((self)->CreateHwndRenderTarget(*(render_props), *(hwnd_props), (out_target)))
#define ID2D1Factory_CreatePathGeometry(self, out_geometry) ((self)->CreatePathGeometry((out_geometry)))
#define ID2D1Factory_CreateRoundedRectangleGeometry(self, rounded_rect, out_geometry) ((self)->CreateRoundedRectangleGeometry(*(rounded_rect), (out_geometry)))

#define ID2D1GeometrySink_AddLines(self, points, count) ((self)->AddLines((points), (count)))
#define ID2D1GeometrySink_BeginFigure(self, start_point, begin_type) ((self)->BeginFigure((start_point), (begin_type)))
#define ID2D1GeometrySink_Close(self) ((self)->Close())
#define ID2D1GeometrySink_EndFigure(self, end_type) ((self)->EndFigure((end_type)))

#define ID2D1HwndRenderTarget_BeginDraw(self) ((self)->BeginDraw())
#define ID2D1HwndRenderTarget_Clear(self, color) ((self)->Clear((color)))
#define ID2D1HwndRenderTarget_CreateBitmapFromWicBitmap(self, bitmap, props, out_bitmap) ((self)->CreateBitmapFromWicBitmap((bitmap), (props), (out_bitmap)))
#define ID2D1HwndRenderTarget_CreateLayer(self, size, out_layer) ((self)->CreateLayer((size), (out_layer)))
#define ID2D1HwndRenderTarget_CreateSolidColorBrush(self, color, props, out_brush) ((self)->CreateSolidColorBrush(*(color), (props), (out_brush)))
#define ID2D1HwndRenderTarget_DrawBitmap(self, bitmap, dest_rect, opacity, interp_mode, src_rect) ((self)->DrawBitmap((bitmap), (dest_rect), (opacity), (interp_mode), (src_rect)))
#define ID2D1HwndRenderTarget_DrawEllipse(self, ellipse, brush, stroke_width, stroke_style) ((self)->DrawEllipse(*(ellipse), (brush), (stroke_width), (stroke_style)))
#define ID2D1HwndRenderTarget_DrawLine(self, p0, p1, brush, stroke_width, stroke_style) ((self)->DrawLine((p0), (p1), (brush), (stroke_width), (stroke_style)))
#define ID2D1HwndRenderTarget_DrawRectangle(self, rect, brush, stroke_width, stroke_style) ((self)->DrawRectangle((rect), (brush), (stroke_width), (stroke_style)))
#define ID2D1HwndRenderTarget_DrawRoundedRectangle(self, rr, brush, stroke_width, stroke_style) ((self)->DrawRoundedRectangle(*(rr), (brush), (stroke_width), (stroke_style)))
#define ID2D1HwndRenderTarget_DrawText(self, text, text_len, fmt, rect, brush, options, measuring_mode) ((self)->DrawText((text), (text_len), (fmt), (rect), (brush), (options), (measuring_mode)))
#define ID2D1HwndRenderTarget_DrawTextLayout(self, origin, layout, brush, options) ((self)->DrawTextLayout((origin), (layout), (brush), (options)))
#define ID2D1HwndRenderTarget_EndDraw(self, tag1, tag2) ((self)->EndDraw((tag1), (tag2)))
#define ID2D1HwndRenderTarget_FillEllipse(self, ellipse, brush) ((self)->FillEllipse(*(ellipse), (brush)))
#define ID2D1HwndRenderTarget_FillGeometry(self, geometry, brush, opacity_brush) ((self)->FillGeometry((geometry), (brush), (opacity_brush)))
#define ID2D1HwndRenderTarget_FillRectangle(self, rect, brush) ((self)->FillRectangle((rect), (brush)))
#define ID2D1HwndRenderTarget_FillRoundedRectangle(self, rr, brush) ((self)->FillRoundedRectangle(*(rr), (brush)))
#define ID2D1HwndRenderTarget_PopAxisAlignedClip(self) ((self)->PopAxisAlignedClip())
#define ID2D1HwndRenderTarget_PopLayer(self) ((self)->PopLayer())
#define ID2D1HwndRenderTarget_PushAxisAlignedClip(self, rect, aa_mode) ((self)->PushAxisAlignedClip((rect), (aa_mode)))
#define ID2D1HwndRenderTarget_PushLayer(self, params, layer) ((self)->PushLayer(*(params), (layer)))
#define ID2D1HwndRenderTarget_Resize(self, size) ((self)->Resize(*(size)))
#define ID2D1HwndRenderTarget_SetDpi(self, dpi_x, dpi_y) ((self)->SetDpi((dpi_x), (dpi_y)))

#define ID2D1PathGeometry_Open(self, out_sink) ((self)->Open((out_sink)))
#define ID2D1SolidColorBrush_SetColor(self, color) ((self)->SetColor(*(color)))

#define IDWriteFactory_CreateTextFormat(self, family_name, collection, weight, style, stretch, font_size, locale_name, out_format) ((self)->CreateTextFormat((family_name), (collection), (weight), (style), (stretch), (font_size), (locale_name), (out_format)))
#define IDWriteFactory_CreateTextLayout(self, text, text_len, fmt, max_width, max_height, out_layout) ((self)->CreateTextLayout((text), (text_len), (fmt), (max_width), (max_height), (out_layout)))

#define IDWriteTextFormat_GetParagraphAlignment(self) ((self)->GetParagraphAlignment())
#define IDWriteTextFormat_GetTextAlignment(self) ((self)->GetTextAlignment())
#define IDWriteTextFormat_SetParagraphAlignment(self, align) ((self)->SetParagraphAlignment((align)))
#define IDWriteTextFormat_SetTextAlignment(self, align) ((self)->SetTextAlignment((align)))

#define IDWriteTextLayout_GetMetrics(self, metrics) ((self)->GetMetrics((metrics)))

#define IWICBitmapDecoder_GetFrame(self, index, out_frame) ((self)->GetFrame((index), (out_frame)))
#define IWICFormatConverter_Initialize(self, source, dst_format, dither, palette, alpha_threshold, palette_type) ((self)->Initialize((source), *(dst_format), (dither), (palette), (alpha_threshold), (palette_type)))
#define IWICImagingFactory_CreateDecoderFromFilename(self, filename, vendor, access, metadata_flags, out_decoder) ((self)->CreateDecoderFromFilename((filename), (vendor), (access), (metadata_flags), (out_decoder)))
#define IWICImagingFactory_CreateFormatConverter(self, out_converter) ((self)->CreateFormatConverter((out_converter)))

#endif

#endif
