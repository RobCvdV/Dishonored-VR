// tools/modmenu_plugin - DishonoredVR.dll, a Dishonored Mod Loader plugin (Crimsab, Nexus mod 383).
// Installed as Binaries\Win32\Mods\plugins\DishonoredVR.dll. The loader calls these exports; this
// file only forwards to the VR mod (d3d9.dll), which owns the menu, its values and its ini keys
// (core/ui/modloader_menu.cpp). Without the Mod Loader this DLL is never loaded. Without the VR
// mod, Plg_QueryMenu returns nothing and the loader lists the entry without a menu.
#include <windows.h>

namespace {
struct Info { const wchar_t* name; const wchar_t* description; const wchar_t* label; int enabledByDefault; };
Info g_info = { L"Dishonored VR", L"VR settings. The F10 panel has the rest.", L"Dishonored VR", 1 };

typedef const void* (__cdecl* QueryFn)();
QueryFn vr_query()
{
    HMODULE vr = GetModuleHandleW(L"d3d9.dll");   // the VR mod is the game's d3d9.dll
    return vr ? (QueryFn)GetProcAddress(vr, "DvrModMenuQuery") : nullptr;
}
} // namespace

extern "C" void __cdecl Plg_Init(const char*) {}

extern "C" const void* __cdecl Plg_Query()
{
    if (!vr_query()) g_info.description = L"The Dishonored VR mod is not running.";
    return &g_info;
}

extern "C" const void* __cdecl Plg_QueryMenu()
{
    QueryFn q = vr_query();
    return q ? q() : nullptr;
}

extern "C" void __cdecl Plg_SetEnabled(int) {}   // the menu has no master switch (flags bit 0)

BOOL WINAPI DllMain(HINSTANCE h, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH) DisableThreadLibraryCalls(h);
    return TRUE;
}
