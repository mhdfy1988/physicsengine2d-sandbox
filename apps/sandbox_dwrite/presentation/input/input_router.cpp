#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include "input_router.hpp"

#include <windowsx.h>

LRESULT input_router_dispatch(const InputRouterCallbacks* callbacks, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    if (callbacks == NULL) return DefWindowProcW(hwnd, msg, wparam, lparam);

    switch (msg) {
        case WM_NCCALCSIZE:
            return 0;
        case WM_NCHITTEST:
            return callbacks->handle_nchittest ? callbacks->handle_nchittest(hwnd, msg, wparam, lparam)
                                               : DefWindowProcW(hwnd, msg, wparam, lparam);
        case WM_PAINT:
            if (callbacks->handle_paint) callbacks->handle_paint(hwnd);
            return 0;
        case WM_SIZE:
            if (callbacks->handle_size) callbacks->handle_size(lparam);
            return 0;
        case WM_TIMER:
            if (callbacks->handle_timer) callbacks->handle_timer(hwnd);
            return 0;
        case WM_KEYDOWN:
            if (callbacks->handle_keydown) callbacks->handle_keydown(hwnd, wparam);
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        case WM_CHAR:
            if (callbacks->handle_char) callbacks->handle_char(hwnd, wparam);
            return 0;
        case WM_SYSKEYDOWN:
            return callbacks->handle_syskeydown_message ? callbacks->handle_syskeydown_message(hwnd, msg, wparam, lparam)
                                                        : DefWindowProcW(hwnd, msg, wparam, lparam);
        case WM_MOUSEMOVE:
            if (callbacks->handle_mousemove) callbacks->handle_mousemove(hwnd, lparam);
            return 0;
        case WM_MOUSEWHEEL:
            if (callbacks->handle_mousewheel) callbacks->handle_mousewheel(hwnd, wparam, lparam);
            return 0;
        case WM_SETCURSOR:
            if (callbacks->handle_setcursor && callbacks->handle_setcursor(hwnd, lparam)) return TRUE;
            break;
        case WM_LBUTTONDOWN:
            if (callbacks->mouse_x != NULL) *callbacks->mouse_x = (float)GET_X_LPARAM(lparam);
            if (callbacks->mouse_y != NULL) *callbacks->mouse_y = (float)GET_Y_LPARAM(lparam);
            if (callbacks->handle_lbuttondown) callbacks->handle_lbuttondown(hwnd);
            return 0;
        case WM_LBUTTONUP:
            if (callbacks->handle_lbuttonup) callbacks->handle_lbuttonup();
            return 0;
        case WM_LBUTTONDBLCLK:
            if (callbacks->handle_lbuttondblclk_message) callbacks->handle_lbuttondblclk_message(hwnd, lparam);
            return 0;
        case WM_DESTROY:
            if (callbacks->handle_destroy) callbacks->handle_destroy();
            return 0;
    }

    return DefWindowProcW(hwnd, msg, wparam, lparam);
}
