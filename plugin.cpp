#include <obs-module.h>
#include <Windows.h>
#include <stdint.h>

typedef void (*api_func)();
typedef bool (*api_func_bool)();

OBS_DECLARE_MODULE()

void monitor_loop();
HWND create_monitor_window();

api_func start_replay;
api_func stop_replay;

api_func_bool replay_active;

bool obs_module_load() {
    HMODULE obs_dll = LoadLibrary("obs.dll");
    if (!obs_dll) return false;

    HMODULE obs_frontend = LoadLibrary("obs-frontend-api.dll");
    if (!obs_frontend) return false;

    start_replay = (api_func)GetProcAddress(obs_frontend, "obs_frontend_replay_buffer_start");
    if (!start_replay) return false;

    stop_replay = (api_func)GetProcAddress(obs_frontend, "obs_frontend_replay_buffer_stop");
    if (!stop_replay) return false;

    replay_active = (api_func_bool)GetProcAddress(obs_frontend, "obs_frontend_replay_buffer_active");
    if (!replay_active) return false;

    uint8_t* os_inhibit_sleep_set_active = (uint8_t*)GetProcAddress(obs_dll, "os_inhibit_sleep_set_active");
    if (!os_inhibit_sleep_set_active) return false;

    DWORD oldProtection;
    VirtualProtect(os_inhibit_sleep_set_active, 8, PAGE_EXECUTE_READWRITE, &oldProtection);
    *os_inhibit_sleep_set_active = 0xC3;
    VirtualProtect(os_inhibit_sleep_set_active, 8, oldProtection, &oldProtection);

    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&monitor_loop, 0, 0, NULL);
    return true;
}

void monitor_loop() {
    HWND hwnd = create_monitor_window();
    MSG msg = {};
    while (GetMessage(&msg, hwnd, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

bool replay_active_on_suspend = false;
LRESULT CALLBACK window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_POWERBROADCAST) {
        if (wParam == PBT_APMRESUMEAUTOMATIC) {
            //Is this thread safe? Who knows lol
            if (replay_active_on_suspend)
                Sleep(10000);
                start_replay();

        } else if (wParam == PBT_APMSUSPEND) {
            replay_active_on_suspend = replay_active();

            if (replay_active_on_suspend)
                stop_replay();
        }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

HWND create_monitor_window()
{
    const char* name = "OBS sleep power state monitor window";

    WNDCLASS wc = {};
    wc.lpfnWndProc = window_proc;
    wc.style = CS_NOCLOSE;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = name;

    if (!RegisterClass(&wc)) {
        return NULL;
    }

    return CreateWindowEx(0, name, name, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL);
}
