#ifndef INPUT_ROUTER_H
#define INPUT_ROUTER_H

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    LRESULT (*handle_nchittest)(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
    int (*handle_paint)(HWND hwnd);
    int (*handle_size)(LPARAM lparam);
    int (*handle_timer)(HWND hwnd);
    int (*handle_keydown)(HWND hwnd, WPARAM wparam);
    int (*handle_char)(HWND hwnd, WPARAM wparam);
    LRESULT (*handle_syskeydown_message)(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
    int (*handle_mousemove)(HWND hwnd, LPARAM lparam);
    int (*handle_mousewheel)(HWND hwnd, WPARAM wparam, LPARAM lparam);
    int (*handle_setcursor)(HWND hwnd, LPARAM lparam);
    int (*handle_lbuttondown)(HWND hwnd);
    int (*handle_lbuttonup)(void);
    int (*handle_lbuttondblclk_message)(HWND hwnd, LPARAM lparam);
    int (*handle_destroy)(void);
    float* mouse_x;
    float* mouse_y;
} InputRouterCallbacks;

LRESULT input_router_dispatch(const InputRouterCallbacks* callbacks, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

#ifdef __cplusplus
}
#endif

#endif
