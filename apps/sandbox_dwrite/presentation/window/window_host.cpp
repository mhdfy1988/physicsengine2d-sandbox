#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include "window_host.hpp"

#include <objbase.h>

int window_host_initialize_platform(void) {
    SetProcessDPIAware();
    return SUCCEEDED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)) ? 1 : 0;
}

int window_host_register_main_window_class(HINSTANCE inst,
                                           WNDPROC wnd_proc,
                                           const wchar_t* class_name,
                                           int icon_resource_id,
                                           HICON* out_large_icon,
                                           HICON* out_small_icon,
                                           WNDCLASSEXW* out_wc) {
    HICON large_icon;
    HICON small_icon;
    if (wnd_proc == NULL || class_name == NULL || out_wc == NULL) return 0;

    ZeroMemory(out_wc, sizeof(*out_wc));
    out_wc->cbSize = sizeof(*out_wc);
    out_wc->style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    out_wc->lpfnWndProc = wnd_proc;
    out_wc->hInstance = inst;

    large_icon = (HICON)LoadImageW(inst, MAKEINTRESOURCEW(icon_resource_id), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
    small_icon = (HICON)LoadImageW(inst, MAKEINTRESOURCEW(icon_resource_id), IMAGE_ICON,
                                   GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
    if (out_large_icon != NULL) *out_large_icon = large_icon;
    if (out_small_icon != NULL) *out_small_icon = small_icon;

    out_wc->hIcon = large_icon ? large_icon : LoadIconW(NULL, (LPCWSTR)IDI_APPLICATION);
    out_wc->hIconSm = small_icon ? small_icon : out_wc->hIcon;
    out_wc->hCursor = LoadCursorW(NULL, (LPCWSTR)IDC_ARROW);
    out_wc->hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    out_wc->lpszClassName = class_name;
    return RegisterClassExW(out_wc) ? 1 : 0;
}

HWND window_host_create_main_window(HINSTANCE inst,
                                    const wchar_t* class_name,
                                    const wchar_t* window_title,
                                    int show_command) {
    HWND hwnd = CreateWindowExW(0, class_name, window_title,
                                (WS_POPUP | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU),
                                CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720, NULL, NULL, inst, NULL);
    if (hwnd == NULL) return NULL;
    ShowWindow(hwnd, show_command);
    UpdateWindow(hwnd);
    SetTimer(hwnd, 1, 16, NULL);
    return hwnd;
}

int window_host_run_message_loop(void) {
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return (int)msg.wParam;
}
