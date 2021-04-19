#include <obs-module.h>
#include <Windows.h>
#include <stdint.h>

OBS_DECLARE_MODULE()

bool obs_module_load() {
    HMODULE obs_dll = LoadLibrary("obs.dll");
    if (!obs_dll) return false;

    uint8_t* os_inhibit_sleep_set_active = (uint8_t*)GetProcAddress(obs_dll, "os_inhibit_sleep_set_active");
    if (!os_inhibit_sleep_set_active) return false;

    DWORD oldProtection;
    VirtualProtect(os_inhibit_sleep_set_active, 8, PAGE_EXECUTE_READWRITE, &oldProtection);
    *os_inhibit_sleep_set_active = 0xC3;
    VirtualProtect(os_inhibit_sleep_set_active, 8, oldProtection, &oldProtection);

    return true;
}
