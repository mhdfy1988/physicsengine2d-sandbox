#ifndef WINDOW_HOST_H
#define WINDOW_HOST_H

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

int window_host_initialize_platform(void);
int window_host_register_main_window_class(HINSTANCE inst,
                                           WNDPROC wnd_proc,
                                           const wchar_t* class_name,
                                           int icon_resource_id,
                                           HICON* out_large_icon,
                                           HICON* out_small_icon,
                                           WNDCLASSEXW* out_wc);
HWND window_host_create_main_window(HINSTANCE inst,
                                    const wchar_t* class_name,
                                    const wchar_t* window_title,
                                    int show_command);
int window_host_run_message_loop(void);

#ifdef __cplusplus
}
#endif

#endif
